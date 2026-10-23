"""Check all exact controls, joint prefixes, native update equations and outcomes."""
from pathlib import Path
import gzip,hashlib,importlib.util,json,math
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
V=module('budget_previous',E.parent/'inten_intensity_consistent/verify.py')
P=V.P
read=V.read;require=V.require;sha=V.sha;decode=V.V.decode

def close(a,b):return np.allclose(a,b,rtol=1e-10,atol=1e-12)
def matrix(x):return np.asarray(x,dtype=float).reshape(4,4)
def check_solver(t,budget,translation_only=False):
    if not t['called']:
        require(not t['iterations'],'Uncalled solver has iterations');return dict(steps=0,max_backward_error=0.,all_finite=True)
    steps=t['iterations'];require(len(steps)<=budget,'Iteration cap exceeded')
    require(steps and close(steps[0]['linearization_pose'],t['input']),'Wrong solver input')
    maximum=0.;finite=True
    for index,it in enumerate(steps):
        require(it['index']==index and sum(it['class_pairs'])==it['pairs'],'Invalid step index or class counts')
        pose=matrix(it['linearization_pose']);expected=pose.copy()
        delta=np.asarray(it['delta'],float)
        if it['update_applied']:
            require(it['step_attempted'] and it['step_finite'] and np.isfinite(delta).all(),'Invalid applied step')
            H=np.asarray(it['H'],float).reshape(6,6);g=np.asarray(it['g'],float)
            n=3 if translation_only else 6
            A=H[:n,:n]+1e-6*np.eye(n);d=delta[:n]
            error=float(np.linalg.norm(A@d+g[:n])/(np.linalg.norm(A)*np.linalg.norm(d)+np.linalg.norm(g[:n])+np.finfo(float).tiny))
            require(error<=1e-10,'Native damped step fails backward equation');maximum=max(maximum,error)
            if translation_only:
                require(np.all(delta[3:]==0),'Conditional refinement changed rotation')
                expected[:3,3]+=delta[:3]
            else:
                w=delta[3:];angle=np.linalg.norm(w);U=np.eye(4)
                if angle>1e-12:
                    K=np.array([[0,-w[2],w[1]],[w[2],0,-w[0]],[-w[1],w[0],0]])
                    U[:3,:3]+=math.sin(angle)/angle*K+(1-math.cos(angle))/(angle*angle)*(K@K)
                U[:3,3]=delta[:3];expected=U@pose
        elif it['step_attempted'] and not it['step_finite']:finite=False
        following=matrix(steps[index+1]['linearization_pose'] if index+1<len(steps) else t['output'])
        require(close(following,expected),'Native pose does not follow its recorded update')
    if t['stop']=='converged':require(np.linalg.norm(np.asarray(steps[-1]['delta'],float))<1e-4,'False convergence label')
    if t['stop']=='iteration_limit':require(len(steps)==budget,'False iteration-limit label')
    return dict(steps=len(steps),max_backward_error=maximum,all_finite=finite,stop=t['stop'])

def check_cli():
    folder=E/'checks/cli'
    for name in read(folder/'archive.json')['files']:decode(folder,name)
    rows={}
    for label,budget in (('default',10),('explicit',10),('extended',100)):
        require(read(folder/(label+'_command.json'))['exit_code']==0,'Production CLI failed')
        require(b'unknown --inten-loam-paper-set key' not in decode(folder,label+'.stderr.log'),'CLI ignored new key')
        rows[label]=[json.loads(r) for r in decode(folder,label+'.trace.jsonl').splitlines()]
        require(rows[label][0]['params']['s2s_iterations']==budget,'Actual CLI budget differs')
        command=read(folder/(label+'_command.json'))['command'];require('--no-gt-seed' in command,'Unexpected GT initialization')
    require(rows['default'][1:]==rows['explicit'][1:],'Explicit default changes estimator output')
    a=rows['default'][0]['params'];b=rows['extended'][0]['params'].copy();b['s2s_iterations']=10
    require(a==b,'Other actual CLI parameters changed')
    require(read(folder/'invalid_fraction_command.json')['exit_code']!=0,'Invalid fraction accepted')
    require(b's2s_iterations must be a non-negative integer' in decode(folder,'invalid_fraction.stderr.log'),'Wrong invalid-input failure')
    return dict(exit_code=0,default_and_explicit_frames_exact=True,budgets_parsed=[10,100],invalid_fraction_rejected=True)

def group_result(group):
    folder=E/'groups'/group;parent=E.parent/'inten_intensity_consistent/groups'/group
    for name in read(folder/'archive.json')['files']:decode(folder,name)
    require(read(folder/'native_command.json')['exit_code']==0,'Native replay failed')
    meta=read(folder/'input_sources.json');oldmeta=read(parent/'input_sources.json')
    require(meta==oldmeta,'Estimated input provenance changed')
    require((folder/'input.txt').read_bytes()==(parent/'input.txt').read_bytes(),'Estimated seeds changed')
    rows=[json.loads(r) for r in decode(folder,'native.stdout.jsonl').splitlines()]
    old=[json.loads(r) for r in decode(parent,'native.stdout.jsonl').splitlines()]
    require([r for r in rows if r['type']=='profile']==[r for r in old if r['type']=='profile'],'Original-seed profiles changed')
    terminal={(r['frame'],r['mode']):r['trace'] for r in rows if r['type']=='terminal'}
    controls={(r['frame'],r['mode']):r['trace'] for r in old if r['type']=='terminal'}
    require(len(terminal)==sum(r['type']=='terminal' for r in rows),'Duplicate native result')
    require(set(terminal)=={(f,m) for f in meta['frames'] for m in range(12)},'Missing or extra endpoint/arm')
    truth=read(E.parent/'inten_zero_consensus_control/groups'/group/'offline_truth.json')
    cases=[];proofs=[]
    for f in meta['frames']:
        metrics={};stops={}
        for m in range(12):
            t=terminal[f,m];budget=10 if m<6 else 100
            if m<6:require(t==controls[f,m],f'Exact inherited trace changed {group}/{f}/{m}')
            else:
                oldjoint=controls[f,m-6]['s2s'];newjoint=t['s2s'];prefix=len(oldjoint['iterations'])
                require(newjoint['input']==oldjoint['input'],'Continuation changes initial prediction')
                require(newjoint['iterations'][:prefix]==oldjoint['iterations'],'Continuation changes original joint prefix')
                if oldjoint['stop']=='converged':require(newjoint==oldjoint,'Early convergence is not exact')
                if 's2s_consensus' in t:
                    c=t['s2s_consensus'];oldc=controls[f,m-6]['s2s_consensus']
                    for key in ('usable','best_index','valid_clusters','votes','admissions','excluded_clusters'):
                        require(c[key]==oldc[key],'Frozen consensus decision changed')
            proof=check_solver(t['s2s'],budget)
            if 's2s_consensus' in t:proof['refinement']=check_solver(t['s2s_consensus']['refinement'],budget,True)
            proofs.append(dict(frame=f,mode=m,**proof));stops[str(m)]=dict(stop=t['s2s']['stop'],iterations=len(t['s2s']['iterations']))
            output=matrix(t['accepted_relative']);gt=matrix(truth[str(f)]);value=P.metrics(output,gt)
            error=output[:3,:3]@gt[:3,:3].T;skew=np.array([error[2,1]-error[1,2],error[0,2]-error[2,0],error[1,0]-error[0,1]])
            value['rotation_error_rad']=float(np.arctan2(np.linalg.norm(skew),np.trace(error)-1));metrics[str(m)]=value
        cases.append(dict(frame=f,metrics=metrics,solver_stops=stops))
    return dict(group=group,cases=cases,solver_proofs=proofs,native_outcomes=len(terminal))

def validate():
    protocol=read(E/'protocol.json');build=E/'checks/build'
    for name in read(build/'archive.json')['files']:decode(build,name)
    sources=read(build/'source_manifest.json')
    for name,digest in protocol['source_sha256'].items():
        require(sources[name]['sha256']==digest,'Measured source pin changed')
        require(hashlib.sha256(gzip.decompress(decode(build,sources[name]['path']))).hexdigest()==digest,'Measured source bytes changed')
        if name.startswith(str(E.relative_to(W))+'/'):require(sha(W/name)==digest,'Frozen harness changed')
    for label in ('baseline_assessment','compiler','native_compile','replay_compile','probe_compile','unit_compile','unit_tests','runner_compile','ldd'):
        require(read(build/(label+'_command.json'))['exit_code']==0,'Build or mechanism test failed')
    require(read(build/'binaries.json')['source_sha256']==protocol['source_sha256'],'Binary source pins differ')
    groups={g:group_result(g) for g in ('old','01','02','08')};cases=[dict(group=g,**c) for g,r in groups.items() for c in r['cases']]
    checks=[]
    for c in cases:
        for a,b in ((0,6),(1,7)):
            old,new=(c['metrics'][str(m)] for m in (a,b))
            checks.append(dict(group=c['group'],frame=c['frame'],arms=[a,b],criterion='no newly bad length/vector labels',passed=(not new['bad_length'] or old['bad_length']) and (not new['bad_vector'] or old['bad_vector'])))
        if c['group']=='old':
            a,b,new=(c['metrics'][str(m)] for m in (0,1,7))
            if (a['bad_length'] or a['bad_vector']) and not (b['bad_length'] or b['bad_vector']):
                checks.append(dict(group='old',frame=c['frame'],criterion='retain original consensus rescue',passed=not(new['bad_length'] or new['bad_vector'])))
    totals={str(m):{key:sum(c['metrics'][str(m)][key] for c in cases) for key in ('length_error','vector_error','rotation_error_rad','bad_length','bad_vector')} for m in range(12)}
    for key in ('length_error','vector_error'):checks.append(dict(criterion='primary extended legacy joint reduces total '+key,passed=totals['6'][key]<totals['0'][key]))
    checks.append(dict(criterion='primary does not increase total rotation error',passed=totals['6']['rotation_error_rad']<=totals['0']['rotation_error_rad']))
    proofs=[p for r in groups.values() for p in r['solver_proofs']]
    checks.append(dict(criterion='all native attempted steps finite',passed=all(p['all_finite'] and p.get('refinement',{}).get('all_finite',True) for p in proofs)))
    return dict(exit_code=0,groups=groups,motion_totals=totals,gate_checks=checks,progression_gate_passed=all(c['passed'] for c in checks),
        native_outcomes=sum(r['native_outcomes'] for r in groups.values()),all_186_inherited_traces_exact=True,
        all_extended_joint_prefixes_exact=True,all_native_updates_and_stops_checked=True,
        max_backward_error=max([p['max_backward_error'] for p in proofs]+[p['refinement']['max_backward_error'] for p in proofs if 'refinement' in p]),cli=check_cli())

if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    report=validate();args.output.write_text(json.dumps(report,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:report[k] for k in ('exit_code','native_outcomes','progression_gate_passed','motion_totals','max_backward_error')}))
