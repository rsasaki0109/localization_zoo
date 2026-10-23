"""Validate and report every matched-seed facade-loss diagnostic."""
from pathlib import Path
from collections import defaultdict
from itertools import groupby
import hashlib,gzip,json,math
import numpy as np
from run import E,W,R,read,write,sha,git

def require(ok,reason):
    if not ok:raise ValueError(reason)
def mat(v):return np.asarray(v,dtype=float).reshape(4,4)
def skew(v):
    x,y,z=v;return np.array([[0.,-z,y],[z,0.,-x],[-y,x,0.]])
def distribution(values):
    a=np.asarray(values,dtype=float)
    return dict(n=len(a),minimum=float(a.min()) if len(a) else None,p50=float(np.median(a)) if len(a) else None,
                p90=float(np.quantile(a,.9)) if len(a) else None,maximum=float(a.max()) if len(a) else None)
def same_numeric(a,b):return np.allclose(a,b,atol=1e-8,rtol=1e-12)
def restore_queries():
    folder=W/'experiments/results/inten_prediction_dor/baseline';entry=read(folder/'archive.json')['files']['correspondences.jsonl']
    target=R/'accepted_facades.jsonl'
    if target.exists():
        require(sha(target)==entry['sha256'],'Existing restored queries differ from committed archive')
        return target
    h=hashlib.sha256()
    with target.open('xb') as out:
        for part in entry['parts']:
            path=folder/part['path'];require(sha(path)==part['gzip_sha256'],'Query shard changed')
            raw=gzip.decompress(path.read_bytes());require(hashlib.sha256(raw).hexdigest()==part['raw_sha256'],'Query shard decoding changed')
            h.update(raw);out.write(raw)
    require(h.hexdigest()==entry['sha256'],'Query archive changed')
    return target


def inspect_solver(row,reference,classes):
    s=row['solver'];mode=row['mode'];require(s['called'] and s['input']==reference['s2s']['input'] and s['output']==row['output'],'Wrong terminal trace flow')
    # The forced diagnostic API reports first data-class counts. Its return
    # field is deliberately not the normal caller's last-iteration pair count.
    require(s['returned_pairs']==sum(c['n'] for c in classes),'Wrong forced diagnostic count')
    if mode==0:
        a={k:v for k,v in s.items() if k!='returned_pairs'}
        b={k:v for k,v in reference['s2s'].items() if k!='returned_pairs'}
        require(a==b,'Unchanged terminal solver differs from full natural solver')
    info=np.asarray(s.get('translation_prior_information',[0.]*9)).reshape(3,3)
    first=s['iterations'][0];t=mat(s['input'])[:3,3];J=np.c_[np.eye(3),-skew(t)]
    dataH=sum(np.asarray(c['H']).reshape(6,6) for c in classes)
    datag=sum(np.asarray(c['g']) for c in classes)
    require(same_numeric(np.asarray(first['H']).reshape(6,6),dataH+J.T@info@J),'First H omits data/prior')
    require(same_numeric(np.asarray(first['g']),datag),'Initial prior gradient not zero')
    B=np.eye(6);B[:3,3:]=skew(t);hc=B.T@dataH@B
    marginal=hc[:3,:3]-hc[:3,3:]@np.linalg.solve(hc[3:,3:]+1e-6*np.eye(3),hc[3:,:3])
    ev,U=np.linalg.eigh((marginal+marginal.T)/2)
    expected=(U*np.maximum(.03*max(ev[-1],0)-ev,0))@U.T
    require(np.allclose(expected,info,atol=1e-7,rtol=1e-7),'Adaptive prior changed definition')
    residuals=[]
    for it in s['iterations']:
        if not it['step_finite']:continue
        H=np.asarray(it['H']).reshape(6,6)+1e-6*np.eye(6);g=np.asarray(it['g']);delta=np.asarray(it['delta'])
        backward=float(np.linalg.norm(H@delta+g)/max(np.linalg.norm(H)*np.linalg.norm(delta)+np.linalg.norm(g),1e-30))
        require(backward<=1e-10,'Recorded finite update is inconsistent')
        residuals.append(backward)
    return dict(max_linear_backward_error=max(residuals,default=0.),prior_information=info.tolist(),
                first_data_marginal_eigenvalues=ev.tolist(),iterations=len(s['iterations']),stop=s['stop'])


def main():
    require(read(R/'native_complete.json')['exit_code']==0,'Native execution incomplete')
    require(not git('status','--porcelain'),'Analysis source must be committed')
    stage=Path(read(R/'native_complete.json')['stage']);out=R/'analysis';out.mkdir()
    require(read(stage/'native_command.json')['exit_code']==0,'Native command failed')
    reference={}
    with (R/'reference/baseline/trace.jsonl').open() as stream:
        for line in stream:
            row=json.loads(line)
            if row['type']=='frame':reference[row['frame_index']]=row
    legacy={}
    for line in (R/'preceding_terminal/native.stdout').read_text().splitlines():
        row=json.loads(line)
        if row['mode']==0:legacy[row['frame']]=row
    summary=read(R/'reference/baseline/summary.json')
    a=np.loadtxt(R/'reference/baseline'/summary['artifacts']['evaluated_gt_path']).reshape(-1,3,4)
    truth=np.tile(np.eye(4),(len(a),1,1));truth[:,:3,:]=a
    require(len(truth)==len(reference)==1101 and len(legacy)==1100,'Incomplete reference')
    path=restore_queries();metrics=[];weight_data={m:[] for m in (0,1)};residual_data=[];first_errors=[];max_backward=0.
    with path.open() as queries,(stage/'native.stdout.log').open() as native,(out/'metrics.jsonl').open('x') as mout,(out/'cluster_influence.jsonl').open('x') as cout:
        grouped=groupby(map(json.loads,queries),key=lambda r:r['frame']);seen=[]
        for i,values in grouped:
            seen.append(i);q=list(values);rows=[json.loads(next(native)),json.loads(next(native))]
            require([(r['frame'],r['mode']) for r in rows]==[(i,0),(i,1)],'Pair/mode order changed')
            base=rows[0];ref=reference[i]
            require(base['output']==ref['s2s']['output']==legacy[i]['output'],'Baseline output changed')
            require(base['first_systems']==legacy[i]['first_systems'],'Baseline first class equations changed')
            require(len(q)==base['first_systems'][2]['n'],'Query count mismatch')
            require([c['n'] for c in rows[1]['first_systems']]==[c['n'] for c in base['first_systems']],'Candidate first associations/counts changed')
            for c in (0,1,3):require(rows[1]['first_systems'][c]==base['first_systems'][c],'Non-facade first data equations changed')
            gt=np.linalg.inv(truth[i-1])@truth[i];length=float(np.linalg.norm(gt[:3,3]));moving=length>=.1
            direction=gt[:3,3]/length if moving else np.zeros(3)
            t=mat(ref['s2s']['input'])[:3,3];Jposition=np.c_[np.eye(3),-skew(t)]
            systems=[];functionals=[]
            for row in rows:
                control=inspect_solver(row,ref,row['first_systems']);max_backward=max(max_backward,control['max_linear_backward_error'])
                H=np.asarray(row['solver']['iterations'][0]['H']).reshape(6,6)+1e-6*np.eye(6)
                functionals.append(np.linalg.solve(H.T,Jposition.T@direction)/max(length,1e-30)*100.)
                systems.append(control)
            gradients={m:defaultdict(lambda:np.zeros(6)) for m in (0,1)};counts=defaultdict(int);r_by_cluster=defaultdict(list)
            Hfacade={m:np.zeros((6,6)) for m in (0,1)};gfacade={m:np.zeros(6) for m in (0,1)}
            for query in q:
                x=np.asarray(query['x']);normal=np.asarray(query['normal']);r=query['r'];cluster=query['source'][4]
                J=np.r_[normal,np.cross(x,normal)];counts[cluster]+=1;r_by_cluster[cluster].append(abs(r));residual_data.append(abs(r))
                for mode in (0,1):
                    weight=min(1.,.1/max(abs(r),1e-300)) if mode==0 else 1./(1.+(r/.1)**2)
                    weight_data[mode].append(weight);h=weight*np.outer(J,J);g=weight*J*r
                    Hfacade[mode]+=h;gfacade[mode]+=g;gradients[mode][cluster]+=g
            for mode in (0,1):
                cls=rows[mode]['first_systems'][2];h=np.asarray(cls['H']).reshape(6,6);g=np.asarray(cls['g'])
                err=max(float(np.max(np.abs(Hfacade[mode]-h)))/max(float(np.max(np.abs(h))),1.),float(np.max(np.abs(gfacade[mode]-g)))/max(float(np.max(np.abs(g))),1.))
                first_errors.append(err);require(err<1e-12,'Reweighted first facade reconstruction failed')
            for cluster in sorted(counts):
                entry=dict(frame=i,cluster=cluster,pairs=counts[cluster],abs_residual=distribution(r_by_cluster[cluster]),arms=[])
                for mode in (0,1):entry['arms'].append(dict(mode=mode,gradient=gradients[mode][cluster].tolist(),
                    own_system_forward_percent=float(-functionals[mode]@gradients[mode][cluster]),
                    fixed_reference_system_forward_percent=float(-functionals[0]@gradients[mode][cluster])))
                cout.write(json.dumps(entry,separators=(',',':'),allow_nan=False)+'\n')
            record=dict(frame=i,moving=moving,gt_step_m=length,arms=[])
            for mode,row in enumerate(rows):
                T=mat(row['output']);ratio=float(np.linalg.norm(T[:3,3])/max(length,1e-30))
                rotation=T[:3,:3]@gt[:3,:3].T;angle=math.acos(float(np.clip((np.trace(rotation)-1)/2,-1,1)))*1000
                record['arms'].append(dict(mode=mode,length_ratio=ratio,length_error=abs(ratio-1),
                    vector_error=float(np.linalg.norm(T[:3,3]-gt[:3,3])/max(length,1e-30)),rotation_error_mrad=angle,
                    first_class_pairs=[c['n'] for c in row['first_systems']],**systems[mode]))
            record['baseline_bad_length']=moving and record['arms'][0]['length_error']>.2
            record['baseline_bad_vector']=moving and record['arms'][0]['vector_error']>.2
            for key in ('length_error','vector_error','rotation_error_mrad'):
                difference=record['arms'][1][key]-record['arms'][0][key]
                record['arms'][1][key+'_vs_base']='equal' if abs(difference)<=1e-12 else 'lower' if difference<0 else 'higher'
            metrics.append(record);mout.write(json.dumps(record,separators=(',',':'),allow_nan=False)+'\n')
        require(seen==list(range(1,1101)),'Not all natural pairs present')
        require(next(native,None) is None,'Extra native output')
    groups={}
    for name,rows in [('all_moving',[r for r in metrics if r['moving']]),('baseline_bad_length',[r for r in metrics if r['baseline_bad_length']]),('other_moving',[r for r in metrics if r['moving'] and not r['baseline_bad_length']])]:
        groups[name]=dict(frames=len(rows))
        for key in ('length_error','vector_error','rotation_error_mrad'):
            groups[name][key]={label:sum(r['arms'][1][key+'_vs_base']==label for r in rows) for label in ('lower','higher','equal')}
        for label,key in [('length','length_error'),('vector','vector_error')]:
            groups[name]['new_bad_'+label+'_frames']=[r['frame'] for r in rows if r['arms'][0][key]<=.2 and r['arms'][1][key]>.2]
            groups[name]['remaining_bad_'+label+'_frames']=[r['frame'] for r in rows if r['arms'][1][key]>.2]
        groups[name]['stops']={str(mode):{stop:sum(r['arms'][mode]['stop']==stop for r in rows) for stop in sorted({r['arms'][mode]['stop'] for r in rows})} for mode in (0,1)}
    write(out/'report.json',dict(exit_code=0,source_commit=git('rev-parse','HEAD'),native_calls=2200,pairs=1100,
        baseline_full_outputs_and_first_class_systems_exact=True,baseline_solver_iterations_and_state_exact=True,
        forced_count_convention='solver.returned_pairs is the forced diagnostic first-data count, not the normal caller last count; excluded only from full solver parity and checked independently against first class counts.',
        candidate_first_counts_and_non_facade_equations_exact=True,first_facade_reweighting_reconstruction_max_relative_error=max(first_errors),
        max_native_linear_solve_backward_error=max_backward,residuals=distribution(residual_data),
        native_huber_inlier_pairs=sum(v<=.1 for v in residual_data),first_weights={str(m):distribution(v) for m,v in weight_data.items()},
        summary=groups,endpoints=[r for r in metrics if r['frame'] in read(E/'protocol.json')['selected_endpoints']],
        limits=read(E/'protocol.json')['analysis']))
    print('All terminal controls and independent first-facade reconstructions passed',flush=True)


if __name__=='__main__':main()
