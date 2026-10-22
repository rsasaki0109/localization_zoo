"""One owned sequence: fetch, verify pair replay, retain every factorial outcome."""
from common import *
import argparse
import numpy as np
from build import docker

PARENT=E.parent/'inten_consensus_query_acceleration'
DIAG=module('zero_parent_diagnosis',E.parent/'inten_consensus_failure_origin/analyze.py')
CHECK=module('zero_check',E/'check.py')

def evaluate(rows, inputs, truths, frames, entries):
    terminals={(r['frame'],r['mode']):r['trace'] for r in rows if r['type']=='terminal'}
    assert len(terminals)==len(frames)*8
    original={(r['frame'],r['mode']):r for r in inputs if r['type']=='terminal'}
    old_history=bool(original)
    findings=[];checks=[]
    for frame in frames:
        traces={m:terminals[frame,m] for m in range(8)}
        if old_history:
            for mode in (0,1): assert traces[mode]==original[frame,mode]['trace'],(frame,mode,'cached original trace')
        else:
            expected=inputs[frame]
            assert traces[0]['s2s']==expected['s2s'],(frame,'joint same history')
            assert traces[1]['s2s']==expected['s2s']
            assert traces[1]['s2s_consensus']==expected['s2s_consensus'],(frame,'full old consensus same history')
            assert traces[1]['accepted_relative']==expected['accepted_relative']
        for mode,trace in traces.items():
            assert trace['s2s']==traces[0]['s2s']
            if mode: CHECK.check_frame(trace,mode)
        metrics={str(m):DIAG.stage_metrics(DIAG.matrix(t['accepted_relative']),truths[frame]) for m,t in traces.items()}
        row=dict(frame=frame,entry=frame in entries,metrics=metrics,
                 original_joint_stop=traces[0]['s2s']['stop'],
                 modes={str(m):dict(stop=t['s2s_consensus']['refinement']['stop'],applied=t['s2s_consensus']['applied'],usable=t['s2s_consensus']['usable'],best_index=t['s2s_consensus']['best_index'],zero_votes=t['s2s_consensus']['votes'][60],valid_clusters=t['s2s_consensus']['valid_clusters'],excluded_clusters=t['s2s_consensus']['excluded_clusters']) for m,t in traces.items() if m})
        a,b,j=metrics['1'],metrics['2'],metrics['0']
        no_worse=lambda x,y:x<=y or DIAG.ratio_agrees(x,y)
        if old_history:
            if not traces[1]['s2s_consensus']['usable']:
                checks.append(dict(frame=frame,criterion='old fallback bitwise joint',passed=traces[2]['accepted_relative']==traces[0]['accepted_relative']))
            else:
                checks.append(dict(frame=frame,criterion='old endpoint no higher length/vector error than old consensus',passed=no_worse(abs(b['length_ratio']-1),abs(a['length_ratio']-1)) and no_worse(b['vector_error'],a['vector_error'])))
                if abs(a['length_ratio']-1)<abs(j['length_ratio']-1) and a['vector_error']<j['vector_error']:
                    checks.append(dict(frame=frame,criterion='old rescue retained versus joint',passed=abs(b['length_ratio']-1)<abs(j['length_ratio']-1) and b['vector_error']<j['vector_error']))
        elif frame in entries:
            checks.append(dict(frame=frame,criterion='immediate damage lower length AND vector, no longer bad length',passed=abs(b['length_ratio']-1)<abs(a['length_ratio']-1) and b['vector_error']<a['vector_error'] and not b['bad_length']))
        else:
            checks.append(dict(frame=frame,criterion='preceding control no newly bad length',passed=not b['bad_length'] or a['bad_length']))
        findings.append(row)
    return dict(cases=findings,gate_checks=checks,local_gate_passed=all(c['passed'] for c in checks),exact_original_and_old_consensus_solver_trace_verified=True,all_eight_modes_retained=True,projected_updates_rotation_and_votes_verified=True)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--group',required=True,choices=['old','01','02','08']);args=parser.parse_args();group=args.group
    assert not git('status','--porcelain')
    p=read(E/'protocol.json')
    for path,digest in p['source_sha256'].items(): assert sha(W/path)==digest,path
    stage=R/('stage_'+group);stage.mkdir();is_old=group=='old'
    entries=p['old_original_history'] if is_old else p['new_actual_candidate_history'][group]
    frames=sorted(set(entries) | (set() if is_old else {f-1 for f in entries}))
    inputs=[];sources=[]
    if is_old:
        feature_meta=read(PARENT/'checks/cached_parity_balanced/features.json')
        features=Path(feature_meta['path']);assert sha(features)==feature_meta['sha256']
        sources.append(dict(path=str(features),sha256=sha(features),original_metadata_sha256=sha(PARENT/'checks/cached_parity_balanced/features.json')))
        restored=R/'old_native.jsonl';sources.append(restore(PARENT/'checks/cached_parity_balanced','native_4.stdout.jsonl',restored))
        inputs=list(map(json.loads,restored.read_text().splitlines()))
        truths={r['frame']:DIAG.matrix(r['gt_relative']) for r in read(E.parent/'inten_current_correspondences/baseline/report.json')['cases']}
        seeds={r['frame']:r['trace']['s2s']['input'] for r in inputs if r['type']=='terminal' and r['mode']==0}
    else:
        assert not any((R/('raw_'+s)).exists() for s in ('01','02','08') if s!=group),'Only one owned raw sequence at a time'
        raw=R/('raw_'+group);assert not raw.exists()
        selected=sorted(set(frames)|{f-1 for f in frames})
        fetch=[PY,str(W/'evaluation/scripts/fetch_kitti_odometry_sequences.py'),'--output-root',str(raw),'--sequence',group,'--workers','4']
        for frame in selected:fetch+=['--frame',str(frame)]
        assert run('fetch',fetch,stage)==0
        scan_dir=raw/'dataset/sequences'/group/'velodyne'
        expected=read(PARENT/'sequences'/group/'inputs.json')['files'];actual={}
        assert sorted(p.name for p in scan_dir.glob('*.bin'))==[f'{f:06d}.bin' for f in selected]
        for scan in sorted(scan_dir.glob('*.bin')):
            actual[scan.name]=dict(sha256=sha(scan),bytes=scan.stat().st_size)
            assert actual[scan.name]==expected[scan.name],scan.name
        write(stage/'raw_inputs.json',dict(sequence=group,files=actual,all_match_parent_inputs=True,parent_sha256=sha(PARENT/'sequences'/group/'inputs.json')))
        features=R/('features_'+group+'.txt')
        payload=('\n'.join(map(str,selected))+'\n').encode()
        (stage/'extract_input.txt').write_bytes(payload)
        assert run('extract',docker()+[str(R/'extract'),str(scan_dir),str(features)],stage,payload)==0
        sources.append(dict(feature_cache_private=str(features),sha256=sha(features),bytes=features.stat().st_size,no_feature_cache_committed=True))
        inputs={r['frame_index']:r for r in DIAG.trace_records(PARENT/'sequences'/group,'candidate') if r['type']=='frame' and r['frame_index'] in frames}
        assert sorted(inputs)==frames
        write(stage/'original_history.json',inputs)
        gt_path=PARENT/'sequences'/group/'candidate/dogfooding_results/velodyne_evaluated_gt.txt'
        gt=np.asarray(DIAG.normalize_first_pose(DIAG.load_kitti_poses(gt_path)))
        truths={f:np.linalg.inv(gt[f-1])@gt[f] for f in frames};sources.append(dict(offline_gt=str(gt_path.relative_to(W)),sha256=sha(gt_path)))
        seeds={f:inputs[f]['s2s']['input'] for f in frames}
    write(stage/'input_sources.json',dict(files=sources,frames=frames,entry_frames=entries,truth_is_offline_only=True,feature_cache_sha256=sha(features)))
    write(stage/'offline_truth.json',{str(f):truths[f].ravel().tolist() for f in frames})
    payload=('\n'.join(str(f)+' '+' '.join(format(v,'.17g') for v in seeds[f]) for f in frames)+'\n').encode();(stage/'input.txt').write_bytes(payload)
    binary=read(E/'checks/build/binaries.json')['binaries']['probe'];assert sha(R/'probe')==binary['sha256']
    assert run('native',docker()+[str(R/'probe'),str(features)],stage,payload,jsonl=True)==0
    assert sha(R/'probe')==binary['sha256']
    rows=list(map(json.loads,(stage/'native.stdout.jsonl').read_text().splitlines()))
    if is_old:
        expected={(r['frame'],r['cluster']):r['costs_m2'] for r in inputs if r['type']=='profile'}
        actual={(r['frame'],r['cluster']):r['costs_m2'] for r in rows if r['type']=='profile'}
        assert actual==expected
    result=evaluate(rows,inputs,truths,frames,entries)
    write(stage/'result.json',dict(exit_code=0,group=group,source_commit=git('rev-parse','HEAD'),**result))
    module('zero_archive',E/'archive.py').archive(stage,E/'groups'/group)
    print(json.dumps(dict(group=group,cases=len(frames),gate=result['local_gate_passed'],failed=[c for c in result['gate_checks'] if not c['passed']])),flush=True)
if __name__=='__main__':main()
