"""Reconstruct reciprocal masks, all solver steps and the complete fixed gate."""
from pathlib import Path
import gzip,hashlib,importlib.util,json
import numpy as np
from scipy.spatial import cKDTree
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
V=module('reciprocal_parent',E.parent/'inten_iteration_budget/verify.py')
read=V.read;require=V.require;sha=V.sha;decode=V.decode;matrix=V.matrix
def masks(record,source,target,excluded):
    T=matrix(record['pose']);R=T[:3,:3];t=T[:3,3];checks=accepted=ambiguous=0;maximum=0.
    require(len(record['classes'])==3,'Missing reciprocal class')
    for cls,rows in enumerate(record['classes']):
        src=source[cls];dst=target[cls]
        expected=[i for i in range(len(src)) if not(cls==2 and int(src[i,4]) in excluded)]
        require([r[0] for r in rows]==expected,'Reciprocal query population omitted or duplicated')
        if not rows:continue
        src_ids=np.asarray([r[0] for r in rows],int);anchors=np.asarray([r[1] for r in rows],int);backs=np.asarray([r[2] for r in rows],int)
        actual=np.asarray([r[3] for r in rows],bool)
        require(all(isinstance(r[3],bool) for r in rows),'Malformed reciprocal decision')
        query32=(src[src_ids,:3]@R.T+t).astype(np.float32)
        if len(dst)<(2 if cls==0 else 3):
            require(np.all(anchors==-1) and np.all(backs==-1) and not actual.any(),'Invented forward support');continue
        dst32=dst[:,:3].astype(np.float32).astype(float)
        distance,_=cKDTree(dst32).query(query32.astype(float),k=1,workers=1)
        nearest_sq=distance*distance
        # Explicit forward-query float32 rounding/accumulation bound. It only
        # certifies the unchanged native float tree; it never changes its gate.
        ulps=np.spacing(np.maximum(np.abs(query32),np.float32(1.))).astype(float)
        coordinate_error=2*np.linalg.norm(ulps,axis=1)
        bound=2*distance*coordinate_error+coordinate_error**2+16*np.finfo(np.float32).eps*np.maximum(nearest_sq,np.finfo(float).tiny)
        valid=anchors>=0;invalid=~valid
        require(np.all(anchors[invalid]==-1) and np.all(backs[invalid]==-1) and not actual[invalid].any(),'Invalid forward fallback')
        require(np.all(nearest_sq[invalid]>=1.-bound[invalid]),'Dropped valid forward support')
        require(np.all(anchors[valid]<len(dst)),'Forward index outside source cache')
        require(np.all(nearest_sq[valid]<=1.+bound[valid]),'Forward query exceeds inherited gate')
        actual_forward=np.sum((query32[valid].astype(float)-dst32[anchors[valid]])**2,axis=1)
        require(np.all(actual_forward<=nearest_sq[valid]+bound[valid]),'Forward anchor is not a float nearest neighbour')
        ambiguous+=int(np.sum(np.abs(nearest_sq-1.)<=bound))
        if not valid.any():continue
        require(np.all((backs[valid]>=0)&(backs[valid]<len(src))),'Backward index outside source cache')
        backward=(dst[anchors[valid],:3]-t)@R
        d,_=cKDTree(src[:,:3]).query(backward,k=1,workers=1);minimum=d*d
        observed=np.sum((src[backs[valid],:3]-backward)**2,axis=1)
        require(np.allclose(observed,minimum,rtol=1e-10,atol=1e-12),'Backward nearest neighbour reconstruction differs')
        original=np.sum((src[src_ids[valid],:3]-backward)**2,axis=1)
        predicted=original<=minimum+1e-12+1e-10*np.abs(minimum)
        require(np.array_equal(actual[valid],predicted),'Reciprocal mask differs from independent distances')
        checks+=int(valid.sum());accepted+=int(actual.sum());maximum=max(maximum,float(np.max(np.abs(observed-minimum))))
    return dict(backward_queries=checks,accepted=accepted,forward_float_gate_boundary_cases=ambiguous,max_backward_distance_error=maximum)
def group_result(group):
    folder=E/'groups'/group;parent=E.parent/'inten_iteration_budget/groups'/group
    for name in read(folder/'archive.json')['files']:decode(folder,name)
    require(read(folder/'native_command.json')['exit_code']==0,'Native replay failed')
    meta=read(folder/'input_sources.json');old_meta=read(parent/'input_sources.json')
    require(meta['parent_archive_sha256']==sha(parent/'archive.json'),'Parent evidence changed')
    for key in ('frames','feature_cache_sha256','no_gt_in_native_input','no_raw_acquired'):require(meta[key]==old_meta[key],'Original sensor inputs changed')
    require((folder/'input.txt').read_bytes()==(parent/'input.txt').read_bytes(),'Estimated seeds changed')
    pin=read(E/'protocol.json')['input_pins'][group]
    require(sha(parent/'input.txt')==pin['input_sha256'],'Frozen estimated seed pin changed')
    rows=[json.loads(r) for r in decode(folder,'native.stdout.jsonl').splitlines()]
    old=[json.loads(r) for r in decode(parent,'native.stdout.jsonl').splitlines()]
    require([r for r in rows if r['type']=='profile']==[r for r in old if r['type']=='profile'],'Original profiles changed')
    terminals={(r['frame'],r['mode']):r['trace'] for r in rows if r['type']=='terminal'}
    controls={(r['frame'],r['mode']):r['trace'] for r in old if r['type']=='terminal'}
    require(len(terminals)==sum(r['type']=='terminal' for r in rows)==len(meta['frames'])*14,'Endpoint/arm missing or duplicated')
    require(set(terminals)=={(f,m) for f in meta['frames'] for m in range(14)},'Unexpected native arm')
    for key,value in controls.items():require(terminals[key]==value,'Inherited complete native trace changed')
    clouds={}
    for row in rows:
        if row['type']!='features':continue
        require(row['frame'] not in clouds,'Duplicate source features')
        require(len(row['classes'])==3,'Malformed feature classes')
        arrays=[np.asarray(x,float).reshape(-1,5) for x in row['classes']]
        require(all(np.isfinite(x).all() for x in arrays),'Nonfinite source feature')
        clouds[row['frame']]=arrays
    require(set(clouds)=={i for f in meta['frames'] for i in (f-1,f)},'Sensor feature frame omitted')
    observations={(r['frame'],r['mode'],r['translation_only'],r['iteration']):r for r in rows if r['type']=='reciprocal_iteration'}
    require(len(observations)==sum(r['type']=='reciprocal_iteration' for r in rows),'Duplicate reciprocal iteration')
    expected=set();proofs=[];cases=[];mask_proofs=[]
    truth_path=E.parent/'inten_zero_consensus_control/groups'/group/'offline_truth.json'
    require(sha(truth_path)==pin['truth_sha256'],'Frozen official-GT scoring pin changed')
    truth=read(truth_path)
    for f in meta['frames']:
        metrics={};stops={}
        for m in range(14):
            trace=terminals[f,m];pose=matrix(trace['accepted_relative']);gt=matrix(truth[str(f)])
            value=V.P.metrics(pose,gt);error=pose[:3,:3]@gt[:3,:3].T
            skew=np.array([error[2,1]-error[1,2],error[0,2]-error[2,0],error[1,0]-error[0,1]])
            value['rotation_error_rad']=float(np.arctan2(np.linalg.norm(skew),np.trace(error)-1));metrics[str(m)]=value
            stops[str(m)]=dict(stop=trace['s2s']['stop'],iterations=len(trace['s2s']['iterations']))
            if m<12:continue
            phases=[(False,trace['s2s'])]
            if m==13:
                consensus=trace['s2s_consensus'];old_consensus=controls[f,7]['s2s_consensus']
                for key in ('usable','best_index','valid_clusters','votes','admissions','excluded_clusters'):require(consensus[key]==old_consensus[key],'Consensus decision changed')
                phases.append((True,consensus['refinement']))
            for translation_only,solver in phases:
                proofs.append(dict(frame=f,mode=m,translation_only=translation_only,**V.check_solver(solver,100,translation_only)))
                for step in solver['iterations']:
                    key=(f,m,translation_only,step['index']);expected.add(key);require(key in observations,'Missing reciprocal observation')
                    record=observations[key];require(record['pose']==step['linearization_pose'],'Reciprocal pose differs from solver pose')
                    excluded=set(trace['s2s_consensus']['excluded_clusters']) if translation_only else set()
                    mask_proofs.append(masks(record,clouds[f],clouds[f-1],excluded))
        cases.append(dict(frame=f,metrics=metrics,solver_stops=stops))
    require(set(observations)==expected,'Unexpected reciprocal observation')
    return dict(group=group,cases=cases,solver_proofs=proofs,native_outcomes=len(terminals),mask_iterations=len(mask_proofs),
        backward_queries=sum(x['backward_queries'] for x in mask_proofs),accepted=sum(x['accepted'] for x in mask_proofs),
        forward_float_gate_boundary_cases=sum(x['forward_float_gate_boundary_cases'] for x in mask_proofs),
        max_backward_distance_error=max([0.]+[x['max_backward_distance_error'] for x in mask_proofs]))
def validate():
    build=E/'checks/build';p=read(E/'protocol.json')
    for name in read(build/'archive.json')['files']:decode(build,name)
    manifest=read(build/'source_manifest.json')
    for name,digest in p['source_sha256'].items():
        require(manifest[name]['sha256']==digest,'Measured source pin changed')
        require(hashlib.sha256(gzip.decompress(decode(build,manifest[name]['path']))).hexdigest()==digest,'Measured source bytes changed')
        if name.startswith(str(E.relative_to(W))+'/'):require(sha(W/name)==digest,'Frozen helper source changed')
    for label in ('compiler','replay_compile','probe_compile','unit_compile','unit','ldd'):require(read(build/(label+'_command.json'))['exit_code']==0,'Build/mechanism test failed')
    require(read(build/'binaries.json')['source_sha256']==p['source_sha256'],'Binary source binding changed')
    builder=module('reciprocal_source_transform',E/'transform.py')
    original=gzip.decompress(decode(build,manifest['papers/inten_loam/src/inten_loam_paper.cpp']['path'])).decode()
    require(builder.transformed(original).encode()==decode(build,'replay_native.cpp'),'Native source transform differs')
    groups={g:group_result(g) for g in ('old','01','02','08')};cases=[dict(group=g,**c) for g,r in groups.items() for c in r['cases']];checks=[]
    for c in cases:
        for a,b in ((0,12),(1,13)):
            x,y=(c['metrics'][str(m)] for m in (a,b))
            checks.append(dict(group=c['group'],frame=c['frame'],arms=[a,b],criterion='no newly bad length/vector labels',passed=(not y['bad_length'] or x['bad_length']) and (not y['bad_vector'] or x['bad_vector'])))
        if c['group']=='old':
            a,b,new=(c['metrics'][str(m)] for m in (0,1,13))
            if (a['bad_length'] or a['bad_vector']) and not(b['bad_length'] or b['bad_vector']):checks.append(dict(group='old',frame=c['frame'],criterion='retain original consensus rescue',passed=not(new['bad_length'] or new['bad_vector'])))
            if c['frame']==247:
                new=c['metrics']['12'];checks.append(dict(group='old',frame=247,criterion='retain demonstrated247 recovery',passed=not(new['bad_length'] or new['bad_vector'])))
    totals={str(m):{key:sum(c['metrics'][str(m)][key] for c in cases) for key in ('length_error','vector_error','rotation_error_rad','bad_length','bad_vector')} for m in range(14)}
    for key in ('length_error','vector_error'):checks.append(dict(criterion='primary reduces total '+key,passed=totals['12'][key]<totals['0'][key]))
    checks.append(dict(criterion='primary does not increase rotation total',passed=totals['12']['rotation_error_rad']<=totals['0']['rotation_error_rad']))
    proofs=[x for g in groups.values() for x in g['solver_proofs']]
    checks.append(dict(criterion='all native attempted steps finite',passed=all(x['all_finite'] for x in proofs)))
    return dict(exit_code=0,groups=groups,motion_totals=totals,gate_checks=checks,progression_gate_passed=all(x['passed'] for x in checks),
        native_outcomes=sum(g['native_outcomes'] for g in groups.values()),all_372_inherited_traces_exact=True,
        all_reciprocal_queries_and_decisions_reconstructed=True,all_new_solver_steps_checked=True,
        backward_queries=sum(g['backward_queries'] for g in groups.values()),
        max_backward_distance_error=max(g['max_backward_distance_error'] for g in groups.values()),
        max_solver_backward_error=max(x['max_backward_error'] for x in proofs))
if __name__=='__main__':
    import argparse
    a=argparse.ArgumentParser();a.add_argument('--output',type=Path,required=True);args=a.parse_args();r=validate()
    args.output.write_text(json.dumps(r,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:r[k] for k in ('exit_code','native_outcomes','progression_gate_passed','motion_totals','backward_queries','max_backward_distance_error','max_solver_backward_error')}))
