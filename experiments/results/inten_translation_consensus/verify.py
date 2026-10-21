"""Portable data-derived consensus, factorial, equation and preservation checks."""
from pathlib import Path
from collections import defaultdict
import argparse
import hashlib
import importlib.util
import json
import subprocess
import numpy as np

E=Path(__file__).resolve().parent; W=E.parents[2]
BASE='ca767ad6050b2e2ef95e07d61ad98fd1ef8f0636'
spec=importlib.util.spec_from_file_location('consensus_archive_reader',E.parent/'inten_cluster_motion/verify.py')
reader=importlib.util.module_from_spec(spec);spec.loader.exec_module(reader)
restored=reader.restored
read=lambda p:json.loads(Path(p).read_text())
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()


def validate(history=True):
    protocol=read(E/'protocol.json'); post=read(E/'postprocessing_protocol.json')
    for name,digest in protocol['source_sha256'].items():
        if history or name.startswith('experiments/results/'):
            assert sha(W/name)==digest,name
    for name,digest in post['source_sha256'].items(): assert sha(E/name)==digest,name
    for name,digest in post['additional_source_sha256'].items(): assert sha(W/name)==digest,name
    for folder in ('controls','analysis','collection_execution','analysis_execution'):
        manifest=read(E/folder/'archive.json');assert manifest['exit_code']==0 and manifest['byte_roundtrip_verified']
        for name in manifest['files']:restored(E/folder,name)
    for name in ('mechanism_tests','compiler','compile_native','compile_probe','linked_libraries','native'):
        assert read(E/'controls'/(name+'_command.json'))['exit_code']==0
    assert read(E/'collection_execution/collection_command.json')['exit_code']==0
    assert read(E/'analysis_execution/analysis_command.json')['exit_code']==0
    spec=importlib.util.spec_from_file_location('consensus_instrument_check',E/'instrument.py')
    instrument=importlib.util.module_from_spec(spec);spec.loader.exec_module(instrument)
    assert sha(E/'reference_native.cpp')==protocol['source_sha256']['papers/inten_loam/src/inten_loam_paper.cpp']
    assert instrument.transformed((E/'reference_native.cpp').read_text())==(E/'diagnostic_native.cpp').read_text()
    sources=read(E/'controls/input_sources.json')
    assert sources['no_raw_scans_acquired'] and sources['no_gt_in_native_input']
    data={}
    for item in sources['files']:
        manifest=W/item['archive_manifest'];assert sha(manifest)==item['archive_manifest_sha256']
        value=restored(manifest.parent,item['name']);assert hashlib.sha256(value).hexdigest()==item['sha256']
        data[item['name']]=value
    parent=list(map(json.loads,data['native.stdout.jsonl'].splitlines()))
    old={(v['frame'],v['mode']):v for v in parent if v['type']=='terminal'}
    profiles=defaultdict(list)
    for row in parent:
        if row['type']=='profile':profiles[row['frame']].append(row)
    native=list(map(json.loads,restored(E/'controls','native.stdout.jsonl').splitlines()))
    expected=[(i,m) for i in protocol['frames'] for m in range(5)]
    assert [(r['frame'],r['mode']) for r in native]==expected
    terminal={(r['frame'],r['mode']):r for r in native}
    residuals=defaultdict(list)
    for row in map(json.loads,restored(E/'controls','residuals.jsonl').splitlines()):
        assert (row['frame'],row['mode']) in expected and row['class'] in range(4)
        residuals[(row['frame'],row['mode'],row['class'])].append(row)
    selections=read(E/'controls/consensus.json')['frames']; report=read(E/'analysis/report.json')
    assert [s['frame'] for s in selections]==protocol['frames']==report['frames']
    gt={v['frame']:np.array(v['gt_relative']).reshape(4,4) for v in json.loads(data['report.json'])['cases']}
    for selection,case in zip(selections,report['cases']):
        i=selection['frame'];assert case['frame']==i
        valid=[r for r in profiles[i] if r['costs_m2']]
        bands={r['cluster']:[cost-min(r['costs_m2'])<=.1*.1 for cost in r['costs_m2']] for r in valid}
        votes=[sum(allowed[k] for allowed in bands.values()) for k in range(121)]
        best=min(range(121),key=lambda k:(-votes[k],abs(k-60),k))
        usable=bool(valid and 0<best<120 and 2*votes[best]>len(valid))
        decision=dict(best_index=best,applied_index=best if usable else 60,shift_m=(best-60)*.05 if usable else 0.,
                      usable=usable,valid_clusters=len(valid),votes=votes,
                      excluded_clusters=sorted(c for c,allowed in bands.items() if usable and not allowed[best]))
        assert decision==selection['decision']==case['decision']
        seed=old[(i,0)]['solver']['input'];shifted=list(seed)
        for axis in range(3):shifted[4*axis+3]+=decision['shift_m']*selection['direction'][axis]
        assert seed==selection['original_seed'] and shifted==selection['shifted_seed']
        assert selection['masks']==[[],[],decision['excluded_clusters'],decision['excluded_clusters'],sorted(r['cluster'] for r in profiles[i] if r['excluded'])]
        for mode,previous in ((0,0),(4,1)):
            assert all(terminal[(i,mode)][k]==old[(i,previous)][k] for k in ('output','first_systems','solver'))
        for unmasked,masked in ((0,2),(1,3)):
            for c in (0,1,3):assert terminal[(i,unmasked)]['first_systems'][c]==terminal[(i,masked)]['first_systems'][c]
        assert [a['mode'] for a in case['arms']]==list(range(5))
        for mode in range(5):
            row=terminal[(i,mode)];solver=row['solver'];classes=row['first_systems'];arm=case['arms'][mode]
            assert solver['input']==(shifted if mode in (1,3) else seed)
            assert solver['output']==row['output'] and solver['returned_pairs']==sum(c['n'] for c in classes)
            assert arm['first_pairs']==[c['n'] for c in classes]
            for c in range(4):
                H,g=np.zeros((6,6)),np.zeros(6);queries=residuals[(i,mode,c)]
                for q in queries:
                    x,a=np.array(q['x']),np.array(q['a']);J=np.r_[a,np.cross(x,a)]
                    weight=1. if abs(q['r'])<=.1 else .1/abs(q['r']);assert q['weight']==weight
                    H+=weight*np.outer(J,J);g+=weight*J*q['r']
                assert len(queries)==classes[c]['n']
                assert np.linalg.norm(H-np.array(classes[c]['H']).reshape(6,6))/max(1.,np.linalg.norm(H))<1e-11
                assert np.linalg.norm(g-np.array(classes[c]['g']))/max(1.,np.linalg.norm(g))<1e-11
            T=np.array(row['output']).reshape(4,4);truth=gt[i];length=np.linalg.norm(truth[:3,3])
            ratio=np.linalg.norm(T[:3,3])/length;vector=np.linalg.norm(T[:3,3]-truth[:3,3])/length
            rotation=np.arccos(np.clip((np.trace(T[:3,:3]@truth[:3,:3].T)-1)/2,-1.,1.))*1000
            assert np.allclose([ratio,abs(ratio-1),vector],[arm['length_ratio'],arm['length_error'],arm['vector_error']],rtol=1e-10,atol=1e-12)
            assert np.isclose(rotation,arm['rotation_error_mrad'],rtol=1e-8,atol=1e-6)
    assert report['native_calls']==len(native)==40 and report['exit_code']==0
    for key in ('original_and_previous_rejected_controls_exact','non_facade_first_systems_exact_within_seed',
                'all_first_class_systems_independently_reconstructed','adaptive_prior_first_H_reconstructed',
                'all_finite_steps_backward_checked','masked_facade_first_queries_ordered_subsets'):assert report[key]
    cases={c['frame']:c['arms'] for c in report['cases']}
    progression=(all(cases[204][3][m]<cases[204][0][m] for m in ('length_error','vector_error'))
                 and cases[204][3]['rotation_error_mrad']<=cases[204][0]['rotation_error_mrad']
                 and all(cases[i][3][m]<=cases[i][0][m] for i in (205,247) for m in ('length_error','vector_error'))
                 and not any(c[3]['bad_length'] and not c[0]['bad_length'] for c in cases.values()))
    assert bool(progression)==report['candidate_progression_condition_passed']
    if history:
        changed=subprocess.check_output(['git','diff','--name-only',BASE,'--'],cwd=W,text=True).splitlines()
        allowed=('experiments/results/inten_translation_consensus/','tests/test_inten_translation_consensus_study.py')
        for name in changed:assert name.startswith(allowed) or name in ('docs/roadmap.md','papers/inten_loam/README.md'),name
        previous=subprocess.check_output(['git','show',BASE+':papers/inten_loam/README.md'],cwd=W)
        assert (W/'papers/inten_loam/README.md').read_bytes().startswith(previous)
    return dict(exit_code=0,native_calls=len(native),profile_decisions_reconstructed=True,
                original_and_old_negative_controls_exact=True,all_first_equations_reconstructed=True,
                no_raw_scans_acquired=True,history_and_live_production_pins_checked=history)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();args.output.write_text(json.dumps(validate(),indent=2)+'\n')
    print('Consensus factorial, numerical evidence and historical preservation passed')
