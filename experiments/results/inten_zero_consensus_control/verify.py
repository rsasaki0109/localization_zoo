"""Reconstruct every archived intervention and its prospectively frozen rejection gate."""
from pathlib import Path
import importlib.util,sys,tempfile,math,copy
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def equal(a,b):
    if isinstance(a,dict):return isinstance(b,dict) and a.keys()==b.keys() and all(equal(a[k],b[k]) for k in a)
    if isinstance(a,list):return isinstance(b,list) and len(a)==len(b) and all(equal(x,y) for x,y in zip(a,b))
    if isinstance(a,float):return isinstance(b,(int,float)) and math.isclose(a,b,rel_tol=1e-10,abs_tol=1e-12)
    return a==b

def profile_check(rows):
    costs={(r['frame'],r['cluster']):r['costs_m2'] for r in rows if r['type']=='profile'}
    expected={}
    for (frame,cluster),values in costs.items():
        if not values:continue
        assert len(values)==121 and all(math.isfinite(v) and v>=0 for v in values)
        minimum=min(values);expected.setdefault(frame,{})[cluster]=sum(1<<k for k,c in enumerate(values) if c-minimum<=.1*.1)
    for r in rows:
        if r['type']!='terminal' or r['mode']==0:continue
        c=r['trace']['s2s_consensus']
        actual={b['cluster']:int(b['bits_low_high_hex'][:16],16)|(int(b['bits_low_high_hex'][16:],16)<<64) for b in c['admissions']}
        assert actual==expected.get(r['frame'],{})
    return len(costs)

def validate(history=True):
    oldpath=list(sys.path);sys.path.insert(0,str(E))
    try:
        for name in ('common','build'):sys.modules.pop(name,None)
        driver=module('zero_frozen_driver',E/'run_group.py');c=driver
        p=c.read(E/'protocol.json')
        for path,digest in p['source_sha256'].items():
            source=W/path
            if not history and path=='papers/inten_loam/src/inten_loam_paper.cpp':
                source=E.parent/'inten_intensity_continuity/reference_native.cpp'
            assert c.sha(source)==digest,path
        summaries={};profiles=0;calls=0;failures=[]
        with tempfile.TemporaryDirectory() as temporary:
            tmp=Path(temporary)
            for group in ('old','01','02','08'):
                folder=E/'groups'/group
                # Every output/receipt is restored with compressed and raw SHA checks.
                manifest=c.read(folder/'archive.json');assert manifest['byte_roundtrip_verified']
                for name in manifest['files']:c.restore(folder,name,tmp/group/name)
                rows=list(map(__import__('json').loads,(tmp/group/'native.stdout.jsonl').read_text().splitlines()))
                profiles+=profile_check(rows);calls+=sum(r['type']=='terminal' for r in rows)
                if group=='old':
                    path=tmp/'old_control.jsonl';c.restore(c.PARENT/'checks/cached_parity_balanced','native_4.stdout.jsonl',path)
                    inputs=list(map(__import__('json').loads,path.read_text().splitlines()))
                else:
                    inputs={int(f):row for f,row in c.read(tmp/group/'original_history.json').items()}
                    archived={r['frame_index']:r for r in c.DIAG.trace_records(c.PARENT/'sequences'/group,'candidate') if r['type']=='frame' and r['frame_index'] in inputs}
                    assert inputs==archived
                    raw=c.read(folder/'raw_inputs.json');parent=c.read(c.PARENT/'sequences'/group/'inputs.json')['files']
                    assert all(v==parent[k] for k,v in raw['files'].items())
                    assert c.read(E/'cleanup'/(group+'.json'))['all_archive_bytes_roundtrip_rechecked']
                truths={int(f):np.asarray(v).reshape(4,4) for f,v in c.read(tmp/group/'offline_truth.json').items()}
                source=c.read(tmp/group/'input_sources.json')
                result=c.evaluate(rows,inputs,truths,source['frames'],source['entry_frames'])
                actual=c.read(folder/'result.json')
                for key,value in result.items():assert equal(value,actual[key]),(group,key)
                assert actual['exit_code']==0 and c.read(folder/'native_command.json')['exit_code']==0
                for label in (() if group=='old' else ('fetch','extract')):assert c.read(folder/(label+'_command.json'))['exit_code']==0
                failures.extend(dict(group=group,**r) for r in result['gate_checks'] if not r['passed'])
                summaries[group]=dict(cases=len(result['cases']),local_gate_passed=result['local_gate_passed'])
        if history:
            import subprocess
            changed=subprocess.check_output(['git','diff','--name-only',p['parent_commit'],'--'],cwd=W,text=True).splitlines()
            allowed=('experiments/results/inten_zero_consensus_control/','tests/test_inten_zero_consensus_control.py')
            for path in changed:
                assert path.startswith(allowed) or path in ('evaluation/scripts/fetch_kitti_odometry_sequences.py','tests/test_lidar_sota_tools.py','papers/inten_loam/README.md','docs/roadmap.md'),path
        return dict(exit_code=0,groups=summaries,native_calls=calls,profiles=profiles,gate_passed=not failures,failed_gate_checks=failures,all_commands_exit_zero=True,all_archives_roundtrip_verified=True,all_original_same_history_controls_exact=True,all_profiles_votes_masks_projected_steps_and_rotations_verified=True,all_results_reconstructed=True,production_and_existing_results_preserved=history,float_relative_tolerance=1e-10,float_absolute_tolerance=1e-12)
    finally:sys.path[:]=oldpath
if __name__=='__main__':
    import argparse,json
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    args.output.write_text(json.dumps(validate(),indent=2)+'\n')
