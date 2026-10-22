"""Reconstruct fixed-prior intervention, every inherited control and the frozen gate."""
from pathlib import Path
import gzip,hashlib,importlib.util,json,lzma
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
P=module('prior_control_parent',E.parent/'inten_reciprocal_support/verify_portable.py')
read=P.read;require=P.checker.require;sha=P.checker.sha;matrix=P.checker.matrix
def decode(folder,name):
    entry=read(folder/'archive.json')['files'][name];data=(folder/entry['path']).read_bytes()
    require(hashlib.sha256(data).hexdigest()==entry['stored_sha256'],'Stored measurement hash changed')
    raw=lzma.decompress(data) if entry['storage']=='xz' else gzip.decompress(data) if entry['storage']=='gzip' else data
    require(len(raw)==entry['bytes'] and hashlib.sha256(raw).hexdigest()==entry['sha256'],'Raw measurement does not restore exactly')
    return raw
def cross(t):return np.array([[0.,-t[2],t[1]],[t[2],0.,-t[0]],[-t[1],t[0],0.]])
def derived_information(H,t):
    B=np.eye(6);B[:3,3:]=cross(t);centered=B.T@H@B
    S=centered[:3,:3]-centered[:3,3:]@np.linalg.solve(centered[3:,3:]+1e-6*np.eye(3),centered[3:,:3])
    values,vectors=np.linalg.eigh((S+S.T)*.5)
    extra=np.maximum(.03*max(0.,float(values.max()))-values,0.)
    return (vectors*extra)@vectors.T
def check_prior(record,solver,reference,step):
    information=np.asarray(record['information'],float).reshape(3,3)
    require(record['reference_mode']==6+record['mode']%2,'Wrong reference arm')
    require(information.tolist()==np.asarray(reference['translation_prior_information'],float).reshape(3,3).tolist(),'Reference prior information changed')
    require(solver['translation_prior_information']==reference['translation_prior_information'],'Solver did not retain fixed reference information')
    prediction=np.asarray(record['prediction'],float);t=np.asarray(record['t'],float)
    require(np.array_equal(prediction,matrix(solver['input'])[:3,3]),'Prior center changed')
    require(np.array_equal(t,matrix(step['linearization_pose'])[:3,3]),'Prior current translation changed')
    H=np.asarray(record['H_before_prior'],float).reshape(6,6);g=np.asarray(record['g_before_prior'],float)
    J=np.column_stack((np.eye(3),-cross(t)))
    expected_H=H+J.T@information@J;expected_g=g+J.T@information@(t-prediction)
    actual_H=np.asarray(step['H'],float).reshape(6,6);actual_g=np.asarray(step['g'],float)
    require(np.allclose(actual_H,expected_H,rtol=1e-10,atol=1e-12),'Fixed prior H contribution differs')
    require(np.allclose(actual_g,expected_g,rtol=1e-10,atol=1e-12),'Fixed prior g contribution differs')
    require(all(np.isfinite(v).all() for v in (H,g,information,prediction,t)),'Nonfinite fixed prior observation')
    return dict(max_H_error=float(np.max(np.abs(actual_H-expected_H))),max_g_error=float(np.max(np.abs(actual_g-expected_g))),
        derived_information=derived_information(H,t).ravel().tolist() if step['index']==0 else None)
def group_result(group):
    folder=E/'groups'/group;parent=E.parent/'inten_reciprocal_support/groups'/group
    for name in read(folder/'archive.json')['files']:decode(folder,name)
    require(read(folder/'native_command.json')['exit_code']==0,'Native replay failed')
    meta=read(folder/'input_sources.json');pins=read(E/'protocol.json')['input_pins'][group]
    seed_parent=E.parent/'inten_iteration_budget/groups'/group
    require(sha(seed_parent/'archive.json')==meta['parent_archive_sha256']==pins['archive_sha256'],'Original input manifest changed')
    require(sha(seed_parent/'input.txt')==pins['input_sha256'],'Original seed pin changed')
    require((folder/'input.txt').read_bytes()==(seed_parent/'input.txt').read_bytes(),'Original estimated seeds changed')
    require(meta['frames']==pins['frames'] and meta['no_gt_in_native_input'] and meta['no_raw_acquired'],'Original input scope changed')
    require(meta['feature_cache_sha256']==read(seed_parent/'input_sources.json')['feature_cache_sha256'],'Feature cache pin changed')
    rows=[json.loads(line) for line in decode(folder,'native.stdout.jsonl').splitlines()]
    old=[json.loads(line) for line in P.decode(parent,'native.stdout.jsonl').splitlines()]
    inherited=[r for r in rows if r['type']!='prior_observation' and (r['type'] in ('profile','features') or r['mode']<14)]
    require(inherited==old,'An inherited full trace/profile/feature/mask observation changed')
    terminals={(r['frame'],r['mode']):r['trace'] for r in rows if r['type']=='terminal'}
    require(len(terminals)==sum(r['type']=='terminal' for r in rows)==len(meta['frames'])*18,'Missing/duplicate terminal')
    require(set(terminals)=={(f,m) for f in meta['frames'] for m in range(18)},'Unexpected native arm')
    clouds={r['frame']:[np.asarray(x,float).reshape(-1,5) for x in r['classes']] for r in rows if r['type']=='features'}
    masks={(r['frame'],r['mode'],r['translation_only'],r['iteration']):r for r in rows if r['type']=='reciprocal_iteration' and r['mode']>=14}
    priors={(r['frame'],r['mode'],r['translation_only'],r['iteration']):r for r in rows if r['type']=='prior_observation'}
    require(len(masks)==sum(r['type']=='reciprocal_iteration' and r['mode']>=14 for r in rows),'Duplicate new mask observation')
    require(len(priors)==sum(r['type']=='prior_observation' for r in rows),'Duplicate prior observation')
    truth_path=E.parent/'inten_zero_consensus_control/groups'/group/'offline_truth.json'
    require(sha(truth_path)==pins['truth_sha256'],'GT scoring pin changed');truth=read(truth_path)
    cases=[];proofs=[];mask_proofs=[];prior_proofs=[];expected_masks=set();expected_priors=set()
    for f in meta['frames']:
        metrics={};stops={};initial={}
        for m in range(18):
            trace=terminals[f,m];pose=matrix(trace['accepted_relative']);gt=matrix(truth[str(f)])
            value=P.V.P.metrics(pose,gt);error=pose[:3,:3]@gt[:3,:3].T
            skew=np.array([error[2,1]-error[1,2],error[0,2]-error[2,0],error[1,0]-error[0,1]])
            value['rotation_error_rad']=float(np.arctan2(np.linalg.norm(skew),np.trace(error)-1));metrics[str(m)]=value
            stops[str(m)]=dict(stop=trace['s2s']['stop'],iterations=len(trace['s2s']['iterations']))
            if m<14:continue
            reference=terminals[f,6+m%2]
            if m>=16:require(trace==reference,'Common-prior unfiltered negative control changed')
            phases=[(False,trace['s2s'],reference['s2s'])]
            if m%2:
                consensus=trace['s2s_consensus'];old_consensus=reference['s2s_consensus']
                for key in ('usable','best_index','valid_clusters','votes','admissions','excluded_clusters'):require(consensus[key]==old_consensus[key],'Frozen consensus decision changed')
                phases.append((True,consensus['refinement'],old_consensus['refinement']))
            for translation_only,solver,reference_solver in phases:
                proofs.append(dict(frame=f,mode=m,translation_only=translation_only,**P.V.check_solver(solver,100,translation_only)))
                for step in solver['iterations']:
                    key=(f,m,translation_only,step['index'])
                    if m<16:
                        expected_masks.add(key);require(key in masks,'Missing new reciprocal observation')
                        record=masks[key];require(record['pose']==step['linearization_pose'],'Mask linearization pose changed')
                        excluded=set(trace['s2s_consensus']['excluded_clusters']) if translation_only else set()
                        mask_proofs.append(P.masks(record,clouds[f],clouds[f-1],excluded))
                    if step['pairs']>=30:
                        expected_priors.add(key);require(key in priors,'Missing fixed prior observation')
                        proof=check_prior(priors[key],solver,reference_solver,step);prior_proofs.append(proof)
                        if step['index']==0:
                            initial[str(m)+('/refinement' if translation_only else '/joint')]=proof['derived_information']
                            if m>=16 or not translation_only:
                                own=reference_solver if m>=16 else terminals[f,m-2]['s2s']
                                require(np.allclose(proof['derived_information'],own['translation_prior_information'],rtol=1e-10,atol=1e-12),'Independent first-system prior reconstruction differs')
        contrasts={}
        for label,a,b,c,d in (('joint',6,12,16,14),('consensus',7,13,17,15)):
            contrasts[label]={k:dict(total=metrics[str(b)][k]-metrics[str(a)][k],
                fixed_information=metrics[str(d)][k]-metrics[str(c)][k],
                prior_feedback=metrics[str(b)][k]-metrics[str(d)][k]) for k in ('length_ratio','length_error','vector_error','rotation_error_rad')}
        cases.append(dict(frame=f,metrics=metrics,solver_stops=stops,initial_derived_information=initial,controlled_contrasts=contrasts))
    require(set(masks)==expected_masks,'Unexpected new mask observation')
    require(set(priors)==expected_priors,'Unexpected prior intervention observation')
    return dict(group=group,cases=cases,solver_proofs=proofs,native_outcomes=len(terminals),
        backward_queries=sum(p['backward_queries'] for p in mask_proofs),max_backward_distance_error=max([0.]+[p['max_backward_distance_error'] for p in mask_proofs]),
        prior_observations_checked=len(prior_proofs),max_prior_H_error=max([0.]+[p['max_H_error'] for p in prior_proofs]),max_prior_g_error=max([0.]+[p['max_g_error'] for p in prior_proofs]))
def validate():
    protocol=read(E/'protocol.json');build=E/'checks/build';manifest=read(build/'source_manifest.json')
    for name in read(build/'archive.json')['files']:decode(build,name)
    for name,digest in protocol['source_sha256'].items():
        require(manifest[name]['sha256']==digest,'Measured source binding changed')
        require(hashlib.sha256(gzip.decompress(decode(build,manifest[name]['path']))).hexdigest()==digest,'Measured source bytes changed')
        require(sha(W/name)==digest,'Frozen study or dependency changed')
    for label in ('compiler','replay_compile','probe_compile','unit_compile','unit','ldd'):require(read(build/(label+'_command.json'))['exit_code']==0,'Build/mechanism check failed')
    require(read(build/'binaries.json')['source_sha256']==protocol['source_sha256'],'Binary/source binding differs')
    builder=module('prior_control_source_transform',E/'transform.py')
    original=gzip.decompress(decode(build,manifest['papers/inten_loam/src/inten_loam_paper.cpp']['path'])).decode()
    require(builder.transformed(original).encode()==decode(build,'replay_native.cpp'),'Bounded source transform differs')
    groups={g:group_result(g) for g in ('old','01','02','08')};cases=[dict(group=g,**c) for g,r in groups.items() for c in r['cases']];checks=[]
    for c in cases:
        for a,b in ((0,14),(1,15)):
            x,y=(c['metrics'][str(m)] for m in (a,b))
            checks.append(dict(group=c['group'],frame=c['frame'],arms=[a,b],criterion='no newly bad length/vector labels',passed=(not y['bad_length'] or x['bad_length']) and (not y['bad_vector'] or x['bad_vector'])))
        if c['group']=='old':
            a,b,new=(c['metrics'][str(m)] for m in (0,1,15))
            if (a['bad_length'] or a['bad_vector']) and not(b['bad_length'] or b['bad_vector']):checks.append(dict(group='old',frame=c['frame'],criterion='retain original consensus rescue',passed=not(new['bad_length'] or new['bad_vector'])))
            if c['frame']==247:
                y=c['metrics']['14'];checks.append(dict(group='old',frame=247,criterion='retain demonstrated247 recovery',passed=not(y['bad_length'] or y['bad_vector'])))
    totals={str(m):{k:sum(c['metrics'][str(m)][k] for c in cases) for k in ('length_error','vector_error','rotation_error_rad','bad_length','bad_vector')} for m in range(18)}
    for k in ('length_error','vector_error'):checks.append(dict(criterion='primary reduces total '+k,passed=totals['14'][k]<totals['0'][k]))
    checks.append(dict(criterion='primary does not increase rotation total',passed=totals['14']['rotation_error_rad']<=totals['0']['rotation_error_rad']))
    proofs=[x for r in groups.values() for x in r['solver_proofs']]
    checks.append(dict(criterion='all native attempted steps finite',passed=all(x['all_finite'] for x in proofs)))
    return dict(exit_code=0,groups=groups,motion_totals=totals,gate_checks=checks,progression_gate_passed=all(c['passed'] for c in checks),
        native_outcomes=sum(r['native_outcomes'] for r in groups.values()),all_434_inherited_complete_records_exact=True,
        unfiltered_common_prior_negative_controls_exact=True,all_fixed_prior_contributions_and_centers_checked=True,
        backward_queries=sum(r['backward_queries'] for r in groups.values()),max_backward_distance_error=max(r['max_backward_distance_error'] for r in groups.values()),
        prior_observations_checked=sum(r['prior_observations_checked'] for r in groups.values()),
        max_prior_H_error=max(r['max_prior_H_error'] for r in groups.values()),max_prior_g_error=max(r['max_prior_g_error'] for r in groups.values()),
        max_solver_backward_error=max(p['max_backward_error'] for p in proofs))
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();r=validate()
    a.output.write_text(json.dumps(r,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:v for k,v in r.items() if k not in ('groups','gate_checks')}))
