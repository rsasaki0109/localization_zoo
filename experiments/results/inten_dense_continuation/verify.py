"""Independent double cKDTree reconstruction of every fixed-population score."""
from pathlib import Path
import importlib.util,hashlib,gzip,json
import numpy as np
from scipy.spatial import cKDTree
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
V=module('dense_parent',E.parent/'inten_iteration_budget/verify.py')
read=V.read;require=V.require;sha=V.sha;decode=V.decode

def direction(source,target,T):
    if not len(source) or not len(target):return dict(valid=False,mean=None,population=len(source),within_gate=0)
    transformed=source@T[:3,:3].T+T[:3,3]
    distance,_=cKDTree(target).query(transformed,k=1,eps=0,workers=1)
    squared=distance*distance
    return dict(valid=True,mean=float(np.minimum(squared,1.).mean()),population=len(source),within_gate=int(np.sum(squared<1.)))
def choose(a,b):
    return all(a[k]['valid'] and b[k]['valid'] and a[k]['population']==b[k]['population'] and
        a[k]['mean']-b[k]['mean']>1e-12+1e-10*abs(a[k]['mean']) for k in ('forward','reverse'))

def group_result(group):
    folder=E/'groups'/group;parent=E.parent/'inten_iteration_budget/groups'/group
    for name in read(folder/'archive.json')['files']:decode(folder,name)
    require(read(folder/'native_command.json')['exit_code']==0,'Native dense cost failed')
    meta=read(folder/'input_sources.json');oldmeta=read(parent/'input_sources.json')
    require(meta['frames']==oldmeta['frames'] and meta['feature_cache_sha256']==oldmeta['feature_cache_sha256'],'Frame/cache changed')
    require(meta['no_gt_in_native_input'] and meta['no_raw_acquired'],'Unexpected new input')
    require(meta['parent_archive_sha256']==sha(parent/'archive.json'),'Parent archive changed')
    rows=[json.loads(r) for r in decode(folder,'native.stdout.jsonl').splitlines()]
    original=[json.loads(r) for r in decode(parent,'native.stdout.jsonl').splitlines()]
    poses={(r['frame'],r['mode']):r['trace']['accepted_relative'] for r in original if r['type']=='terminal'}
    expected=[]
    for f in meta['frames']:
        expected.append(str(f))
        for m in range(12):expected.append(' '.join(map(str,[m,*poses[f,m]])))
    require((folder/'input.txt').read_text()=='\n'.join(expected)+'\n','Native input is not the frozen estimated poses')
    clouds={};source_points=0;voxels=0
    for row in rows:
        if row['type']!='cloud':continue
        require(row['frame'] not in clouds,'Duplicate cloud')
        cells=np.asarray(row['cells'],float);require(cells.ndim==2 and cells.shape[1]==7,'Malformed voxel observations')
        keys=cells[:,:3];counts=cells[:,3];points=cells[:,4:]
        require(np.isfinite(cells).all() and np.all(keys==np.floor(keys)) and np.all(counts>0) and np.all(counts==np.floor(counts)),'Invalid native cells')
        keylist=[tuple(k) for k in keys];require(keylist==sorted(set(keylist)),'Voxel order/uniqueness changed')
        require(int(counts.sum())==row['source_points'],'Incomplete source population')
        require(row['voxel_m']==.2 and np.all(points>=keys*.2-1e-12) and np.all(points<=(keys+1)*.2+1e-12),'Centroid left its original cell')
        clouds[row['frame']]=points;source_points+=row['source_points'];voxels+=len(points)
    require(set(clouds)=={i for f in meta['frames'] for i in (f-1,f)},'Cloud omitted')
    scores={(r['frame'],r['mode']):r for r in rows if r['type']=='score'}
    require(len(scores)==sum(r['type']=='score' for r in rows)==len(meta['frames'])*12,'Pose score omitted or duplicated')
    decisions={r['frame']:r['selected_arms'] for r in rows if r['type']=='decision'}
    require(len(decisions)==sum(r['type']=='decision' for r in rows)==len(meta['frames']),'Decision omitted or duplicated')
    parent_cases={c['frame']:c for c in read(E.parent/'inten_iteration_budget/report/summary.json')['groups'][group]['cases']}
    cases=[];max_error=0.
    for f in meta['frames']:
        reconstructed={}
        for m in range(12):
            T=np.asarray(poses[f,m]).reshape(4,4)
            expected={'forward':direction(clouds[f],clouds[f-1],T),'reverse':direction(clouds[f-1],clouds[f],np.linalg.inv(T))}
            reconstructed[m]=expected
            actual=scores[f,m]
            for side in ('forward','reverse'):
                for key in ('valid','population','within_gate'):require(actual[side][key]==expected[side][key],'Native population/coverage differs')
                require(V.close(actual[side]['mean'],expected[side]['mean']),'Native dense cost reconstruction differs')
                max_error=max(max_error,abs(actual[side]['mean']-expected[side]['mean']))
        selected=[m+6 if choose(reconstructed[m],reconstructed[m+6]) else m for m in range(6)]
        require(decisions[f]==selected,'Native bidirectional decision differs')
        cases.append(dict(frame=f,selected_arms=selected,scores={str(m):{k:scores[f,m][k] for k in ('forward','reverse')} for m in range(12)},
            selected_metrics={str(m):parent_cases[f]['metrics'][str(selected[m])] for m in range(6)},
            baseline_metrics={str(m):parent_cases[f]['metrics'][str(m)] for m in range(6)}))
    return dict(group=group,cases=cases,pose_scores=len(scores),source_points=source_points,voxel_points=voxels,clouds=len(clouds),max_cost_abs_error=max_error)

def validate():
    build=E/'checks/build';p=read(E/'protocol.json')
    for name in read(build/'archive.json')['files']:decode(build,name)
    sources=read(build/'source_manifest.json')
    for name,digest in p['source_sha256'].items():
        require(sources[name]['sha256']==digest and hashlib.sha256(gzip.decompress(decode(build,sources[name]['path']))).hexdigest()==digest,'Measured source pin/bytes changed')
        if name.startswith(str(E.relative_to(W))+'/'):require(sha(W/name)==digest,'Frozen helper changed')
    for label in ('compiler','probe_compile','unit_compile','unit','ldd'):require(read(build/(label+'_command.json'))['exit_code']==0,'Dense helper build/test failed')
    require(read(build/'binaries.json')['source_sha256']==p['source_sha256'],'Binary source pin changed')
    groups={g:group_result(g) for g in ('old','01','02','08')};cases=[dict(group=g,**c) for g,r in groups.items() for c in r['cases']]
    checks=[]
    for c in cases:
        for m in (0,1):
            a,b=c['baseline_metrics'][str(m)],c['selected_metrics'][str(m)]
            checks.append(dict(group=c['group'],frame=c['frame'],arm=m,criterion='no newly bad length/vector labels',passed=(not b['bad_length'] or a['bad_length']) and (not b['bad_vector'] or a['bad_vector'])))
        if c['group']=='old':
            a,b,new=(c[k][str(m)] for k,m in (('baseline_metrics',0),('baseline_metrics',1),('selected_metrics',1)))
            if (a['bad_length'] or a['bad_vector']) and not (b['bad_length'] or b['bad_vector']):
                checks.append(dict(group='old',frame=c['frame'],criterion='retain original consensus rescue',passed=not(new['bad_length'] or new['bad_vector'])))
            if c['frame']==247:
                value=c['selected_metrics']['0'];checks.append(dict(group='old',frame=247,criterion='retain demonstrated247 recovery',passed=not(value['bad_length'] or value['bad_vector'])))
    totals={kind:{str(m):{key:sum(c[kind][str(m)][key] for c in cases) for key in ('length_error','vector_error','rotation_error_rad','bad_length','bad_vector')} for m in range(6)} for kind in ('baseline_metrics','selected_metrics')}
    a,b=totals['baseline_metrics']['0'],totals['selected_metrics']['0']
    for key in ('length_error','vector_error'):checks.append(dict(criterion='primary selector reduces total '+key,passed=b[key]<a[key]))
    checks.append(dict(criterion='primary selector does not increase rotation total',passed=b['rotation_error_rad']<=a['rotation_error_rad']))
    return dict(exit_code=0,groups=groups,totals=totals,gate_checks=checks,progression_gate_passed=all(c['passed'] for c in checks),
        all_pose_inputs_bound_to_committed_estimator_outputs=True,all_native_costs_independently_reconstructed=True,
        pose_scores=sum(r['pose_scores'] for r in groups.values()),max_cost_abs_error=max(r['max_cost_abs_error'] for r in groups.values()))
if __name__=='__main__':
    import argparse
    a=argparse.ArgumentParser();a.add_argument('--output',type=Path,required=True);args=a.parse_args();s=validate()
    args.output.write_text(json.dumps(s,indent=2,allow_nan=False)+'\n');print(json.dumps({k:s[k] for k in ('exit_code','progression_gate_passed','totals','max_cost_abs_error')}))
