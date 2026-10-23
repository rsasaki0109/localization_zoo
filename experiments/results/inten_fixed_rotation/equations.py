"""Reconstruct actual native first equations, projected steps and offline errors."""
from collections import defaultdict
import numpy as np

def skew(v):
    x,y,z=v
    return np.array([[0.,-z,y],[z,0.,-x],[-y,x,0.]])

def metrics(output,gt):
    T=np.array(output).reshape(4,4);length=np.linalg.norm(gt[:3,3])
    ratio=np.linalg.norm(T[:3,3])/length
    rotation=np.arccos(np.clip((np.trace(T[:3,:3]@gt[:3,:3].T)-1)/2,-1.,1.))*1000
    return dict(length_ratio=float(ratio),length_error=float(abs(ratio-1)),
                vector_error=float(np.linalg.norm(T[:3,3]-gt[:3,3])/length),rotation_error_mrad=float(rotation),bad_length=bool(abs(ratio-1)>.2))

def evaluate(protocol,previous,profiles,selections,native,residual_rows,truth):
    expected=[(i,m) for i in protocol['frames'] for m in range(6)]
    assert [(v['frame'],v['mode']) for v in native]==expected
    terminal={(v['frame'],v['mode']):v for v in native}
    residuals=defaultdict(list)
    for q in residual_rows:
        assert (q['frame'],q['mode']) in expected and q['class'] in range(4)
        residuals[(q['frame'],q['mode'],q['class'])].append(q)
    maxima=dict(first_H_relative=0.,first_g_relative=0.,first_prior_H_relative=0.,first_prior_g_relative=0.,step_backward=0.)
    cases=[];steps=dict(joint=0,translation_only=0)
    for i,selection in zip(protocol['frames'],selections):
        assert selection['frame']==i
        # Independent vote reconstruction, without calling producer consensus.py.
        records=[p for p in profiles if p['frame']==i];valid=[p for p in records if p['costs_m2']]
        bands={p['cluster']:[c-min(p['costs_m2'])<=.1*.1 for c in p['costs_m2']] for p in valid}
        votes=[sum(band[k] for band in bands.values()) for k in range(121)]
        best=min(range(121),key=lambda k:(-votes[k],abs(k-60),k))
        usable=bool(valid and 0<best<120 and 2*votes[best]>len(valid))
        decision=dict(best_index=best,applied_index=best if usable else 60,shift_m=(best-60)*.05 if usable else 0.,usable=usable,valid_clusters=len(valid),votes=votes,
                      excluded_clusters=sorted(c for c,b in bands.items() if usable and not b[best]))
        assert decision==selection['decision']
        seed=list(previous[(i,0)]['solver']['input']);shifted=list(seed);direction=records[0]['direction']
        assert all(p['direction']==direction for p in records) and direction==selection['direction']
        for a in range(3):shifted[4*a+3]+=decision['shift_m']*direction[a]
        baseline=previous[(i,0)]['output'];fixed=list(seed);shift_fixed=list(shifted)
        for a in range(3):
            for b in range(3):fixed[4*a+b]=shift_fixed[4*a+b]=baseline[4*a+b]
        assert selection['estimated_rotation_source']==baseline
        assert selection['seeds']==[seed,shifted,fixed,shift_fixed,fixed,shift_fixed]
        mask=decision['excluded_clusters'];assert selection['masks']==[[],mask,[],[],mask,mask]
        for mode,old in ((0,0),(1,3)):
            assert all(terminal[(i,mode)][k]==previous[(i,old)][k] for k in ('output','first_systems','solver')),('Exact control differs',i,mode)
        for unmasked,masked in ((2,4),(3,5)):
            for cls in (0,1,3):assert terminal[(i,unmasked)]['first_systems'][cls]==terminal[(i,masked)]['first_systems'][cls]
            left=iter(residuals[(i,unmasked,2)])
            for q in residuals[(i,masked,2)]:
                target={k:q[k] for k in ('x','a','r','weight')}
                assert any({k:p[k] for k in target}==target for p in left)
        arms=[]
        for mode in range(6):
            row=terminal[(i,mode)];solver=row['solver'];classes=row['first_systems'];input_pose=selection['seeds'][mode]
            assert solver['called'] and solver['input']==input_pose and solver['output']==row['output']
            assert solver['returned_pairs']==sum(c['n'] for c in classes)
            Hdata,gdata=np.zeros((6,6)),np.zeros(6)
            for cls in range(4):
                H,g=np.zeros((6,6)),np.zeros(6);queries=residuals[(i,mode,cls)]
                for q in queries:
                    x,a=np.array(q['x']),np.array(q['a']);J=np.r_[a,np.cross(x,a)]
                    weight=1. if abs(q['r'])<=.1 else .1/abs(q['r']);assert q['weight']==weight
                    H+=weight*np.outer(J,J);g+=weight*J*q['r']
                assert len(queries)==classes[cls]['n']
                for key,got,want in (('first_H_relative',H,np.array(classes[cls]['H']).reshape(6,6)),('first_g_relative',g,np.array(classes[cls]['g']))):
                    error=float(np.linalg.norm(got-want)/max(1.,np.linalg.norm(got)))
                    maxima[key]=max(maxima[key],error);assert error<1e-11,(i,mode,cls,key)
                Hdata+=H;gdata+=g
            prior=np.array(solver.get('translation_prior_information',[0.]*9)).reshape(3,3)
            Jp=np.c_[np.eye(3),-skew(np.array(input_pose).reshape(4,4)[:3,3])]
            Hfirst=Hdata+Jp.T@prior@Jp
            first=solver['iterations'][0]
            for key,got,want in (('first_prior_H_relative',Hfirst,np.array(first['H']).reshape(6,6)),('first_prior_g_relative',gdata,np.array(first['g']))):
                error=float(np.linalg.norm(got-want)/max(1.,np.linalg.norm(got)))
                maxima[key]=max(maxima[key],error);assert error<1e-11
            iterations=solver['iterations']
            for k,it in enumerate(iterations):
                pose=np.array(it['linearization_pose']).reshape(4,4)
                if mode>=2:assert np.array_equal(pose[:3,:3],np.array(baseline).reshape(4,4)[:3,:3])
                if not it['step_attempted']:continue
                assert it['step_finite'] and it['update_applied']
                H=np.array(it['H']).reshape(6,6)+1e-6*np.eye(6);g=np.array(it['g']);delta=np.array(it['delta'])
                if mode>=2:
                    assert np.array_equal(delta[3:],np.zeros(3))
                    H,g,delta=H[:3,:3],g[:3],delta[:3];steps['translation_only']+=1
                    after=np.array(iterations[k+1]['linearization_pose'] if k+1<len(iterations) else solver['output']).reshape(4,4)
                    assert np.array_equal(after[:3,3],pose[:3,3]+delta)
                    assert np.array_equal(after[:3,:3],pose[:3,:3])
                else:steps['joint']+=1
                error=float(np.linalg.norm(H@delta+g)/max(1.,np.linalg.norm(H)*np.linalg.norm(delta)+np.linalg.norm(g)))
                maxima['step_backward']=max(maxima['step_backward'],error);assert error<1e-10
            if mode>=2:assert np.array_equal(np.array(row['output']).reshape(4,4)[:3,:3],np.array(baseline).reshape(4,4)[:3,:3])
            arms.append(dict(mode=mode,**metrics(row['output'],truth[i]),first_pairs=[c['n'] for c in classes]))
        selected=5 if usable else 0
        cases.append(dict(frame=i,decision=decision,arms=arms,candidate_mode=selected,candidate=arms[selected]))
    lookup={c['frame']:c for c in cases}
    def value(i,key,candidate=True):return lookup[i]['candidate' if candidate else 'arms'][key] if candidate else lookup[i]['arms'][0][key]
    inherited=(all(value(204,m)<value(204,m,False) for m in ('length_error','vector_error'))
               and value(204,'rotation_error_mrad')<=value(204,'rotation_error_mrad',False)
               and all(value(i,m)<=value(i,m,False) for i in (205,247) for m in ('length_error','vector_error'))
               and not any(c['candidate']['bad_length'] and not c['arms'][0]['bad_length'] for c in cases))
    no_translation_regressions=all(c['candidate'][m]<=c['arms'][0][m] for c in cases for m in ('length_error','vector_error'))
    fixed_rotation=all(c['candidate']['rotation_error_mrad']==c['arms'][0]['rotation_error_mrad'] for c in cases)
    counts={}
    for mode in list(range(1,6))+['candidate']:
        counts[str(mode)]={}
        for metric in ('length_error','vector_error','rotation_error_mrad'):
            values=[(c['candidate'] if mode=='candidate' else c['arms'][mode])[metric]-c['arms'][0][metric] for c in cases]
            counts[str(mode)][metric]={label:sum(label==('equal' if abs(v)<=1e-12 else 'lower' if v<0 else 'higher') for v in values) for label in ('lower','higher','equal')}
    return dict(exit_code=0,frames=protocol['frames'],native_calls=len(native),all_original_and_previous_combined_controls_exact=True,
                all_first_queries_reconstructed=True,all_projected_steps_backward_checked=True,rotation_bitwise_fixed_at_every_projected_iteration=True,
                masked_first_facade_queries_ordered_subsets=True,prior_first_H_g_reconstructed=True,maxima=maxima,steps=steps,
                inherited_gate_passed=bool(inherited),all_endpoints_translation_nonregression=bool(no_translation_regressions),
                candidate_gate_passed=bool(inherited and no_translation_regressions and fixed_rotation),metric_counts_vs_baseline=counts,cases=cases,limits=protocol['limits'])
