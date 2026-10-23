"""Independent native residual reconstruction and fixed-pose support attribution."""
from pathlib import Path
import gzip,hashlib,importlib.util,json,lzma
import numpy as np
from scipy.spatial import cKDTree
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
P=module('support_prior_checker',E.parent/'inten_prior_control/verify_portable.py')
F=module('support_legacy_field',E.parent/'inten_intensity_continuity/verify.py')
read=P.read
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def require(ok,why):
    if not ok:raise ValueError(why)
def close(a,b,why):
    require(np.allclose(a,b,rtol=1e-10,atol=1e-12),why)
def decode(folder,name):
    e=read(folder/'archive.json')['files'][name];data=(folder/e['path']).read_bytes()
    require(hashlib.sha256(data).hexdigest()==e['stored_sha256'],'Stored evidence changed')
    raw=lzma.decompress(data) if e['storage']=='xz' else gzip.decompress(data) if e['storage']=='gzip' else data
    require(len(raw)==e['bytes'] and hashlib.sha256(raw).hexdigest()==e['sha256'],'Restored evidence changed')
    return raw
def extras(folder):
    return [json.loads(line) for line in decode(folder,'support.extra.jsonl').splitlines()]
def verify_transport(folder,rows):
    ref=read(folder/'native.stdout.reference.json');parent=W/ref['parent_archive']
    require(sha(parent)==ref['parent_archive_sha256'],'Parent archive changed')
    raw=P.decode(parent.parent,ref['parent_name']);lines=iter(raw.splitlines(keepends=True))
    digest=hashlib.sha256();size=0;pos=0
    for row in rows:
        require(row['before_parent_line']>=pos,'Interleave ordering changed')
        while pos<row['before_parent_line']:
            data=next(lines);digest.update(data);size+=len(data);pos+=1
        data=row['raw_line'].encode();digest.update(data);size+=len(data)
    for data in lines:digest.update(data);size+=len(data);pos+=1
    require(pos==ref['parent_lines'] and len(rows)==ref['extra_records'],'Incomplete native stream')
    require(size==ref['bytes'] and digest.hexdigest()==ref['sha256'],'Full native stdout changed')
    require(ref['all_parent_native_stdout_bytes_exact'],'Historical native controls changed')
    return [json.loads(x) for x in raw.splitlines()]
def float_distances(points,query):
    delta=points.astype(np.float32)-query.astype(np.float32)
    d=delta*delta
    return ((d[:,0]+d[:,1])+d[:,2]).astype(float)
def empty():return dict(H=np.zeros((6,6)),g=np.zeros(6),count=0,cost=0.)
def reduce_rows(x,a,r,valid):
    x=x[valid];a=a[valid];r=r[valid]
    J=np.column_stack((a,np.cross(x,a)))
    weights=np.ones(len(r));large=np.abs(r)>.1;weights[large]=.1/np.abs(r[large])
    return dict(H=(weights[:,None]*J).T@J,g=(weights[:,None]*J).T@r,
        count=len(r),cost=float(np.sum(weights*r*r)))
def reconstruct(record,clouds,excluded,pca_cache,field_cache):
    frame=record['frame'];cls=record['class'];pose=np.array(record['pose']).reshape(4,4)
    source=clouds[frame][cls];target=clouds[frame-1][cls]
    x=source[:,:3]@pose[:3,:3].T+pose[:3,3]
    rows=record['rows'];require(len(rows)==len(source),'Incomplete feature population')
    native_valid=np.array([v[0] for v in rows],bool);native_keep=np.array([v[1] for v in rows],bool)
    valid=np.zeros(len(source),bool);keep=np.zeros(len(source),bool)
    a=np.zeros((len(source),3));r=np.zeros(len(source))
    if cls==3 and len(source):
        require(all(not v[2] for v in rows),'Field query has geometric neighbours')
        nodes=field_cache.setdefault(frame-1,build_nodes(target))
        valid,value,a,_,_=F.evaluate(nodes,x,.3);r=value-source[:,5];keep[:]=True
    elif cls!=3:
        backward_tree=cKDTree(source[:,:3]) if len(source) else None
        forward_tree=cKDTree(target[:,:3].astype(np.float32).astype(float)) if len(target) else None
        expected=forward_tree.query(x.astype(np.float32).astype(float),k=min(10,len(target)))[0] if forward_tree is not None else None
        for i,row in enumerate(rows):
            ids=row[2]
            masked=cls==2 and int(source[i,4]) in excluded
            if masked:require(not ids,'Excluded facade was queried');continue
            require(len(ids)==min(10,len(target)) and all(isinstance(k,int) and 0<=k<len(target) for k in ids),'Invalid native neighbour population')
            if not ids:continue
            require(len(set(ids))==len(ids),'Duplicate neighbours')
            q=target[ids];d=float_distances(q[:,:3],x[i])
            # Forward indexing uses float coordinates and float arithmetic.
            # Double-tree ranking is checked with its explicit float rounding
            # bound; equation/reciprocal tolerances remain the inherited ones.
            wanted=np.atleast_1d(expected[i])
            bound=8*np.finfo(np.float32).eps*np.maximum(1.,wanted*wanted)
            require(np.all(np.abs(np.sort(d)-wanted*wanted)<=bound),'Forward nearest neighbours differ beyond float rounding')
            backward=pose[:3,:3].T@(q[0,:3]-pose[:3,3])
            best=float(backward_tree.query(backward)[0])**2
            original=float(np.sum((source[i,:3]-backward)**2))
            keep[i]=original<=best+1e-12+1e-10*abs(best)
            if len(ids)<(2 if cls==0 else 3) or d[0]>1.:continue
            if cls==0:
                neighbours=[k for k in range(1,len(ids)) if d[k]<=1. and 1<=abs(q[k,3]-q[0,3])<=2]
                if not neighbours:continue
                direction=q[0,:3]-q[neighbours[0],:3];norm=np.linalg.norm(direction)
                if norm<1e-3:continue
                direction/=norm;delta=x[i]-q[0,:3];perp=delta-direction*np.dot(direction,delta)
                length=np.linalg.norm(perp)
                if length<1e-9:continue
                a[i]=perp/length;r[i]=length
            else:
                same=[k for k in range(1,len(ids)) if d[k]<=1. and q[k,3]==q[0,3]]
                adjacent=[k for k in range(1,len(ids)) if d[k]<=1. and 1<=abs(q[k,3]-q[0,3])<=2]
                if not same or not adjacent:continue
                normal=np.cross(q[0,:3]-q[adjacent[0],:3],q[0,:3]-q[same[0],:3]);norm=np.linalg.norm(normal)
                if norm<1e-6:continue
                normal/=norm;gated=tuple(k for k,v in zip(ids,d) if v<=1.)
                key=(frame,cls,gated)
                if key not in pca_cache:
                    points=target[list(gated),:3];centered=points-points.mean(axis=0)
                    values,vectors=np.linalg.eigh(centered.T@centered)
                    pca_cache[key]=vectors[:,0] if len(points)>=3 and values[1]>1e-12*values[2] else None
                candidate=pca_cache[key]
                if candidate is not None:normal=candidate
                a[i]=normal;r[i]=np.dot(normal,x[i]-q[0,:3])
            valid[i]=True
    require(np.array_equal(valid,native_valid),'Native residual validity differs')
    require(np.array_equal(keep,native_keep),'Reciprocal partition differs')
    checked={}
    for name,mask in (('all',valid),('kept',valid&keep),('removed',valid&~keep)):
        rebuilt=reduce_rows(x,a,r,mask);native=record[name]
        require(rebuilt['count']==native['count'],'Residual count differs')
        for key in ('H','g','cost'):
            expected=np.array(native[key]).reshape(rebuilt[key].shape) if key!='cost' else native[key]
            close(rebuilt[key],expected,'Native '+name+' '+key+' reconstruction differs')
        checked[name]=rebuilt
    for key in ('H','g','cost'):close(checked['all'][key],checked['kept'][key]+checked['removed'][key],'Partition additivity differs')
    return checked,dict(queries=len(rows),invalid=int((~valid).sum()),invalid_reciprocal_rejections=int((~valid&~keep).sum()),valid_removed=int((valid&~keep).sum()))
def build_nodes(features):
    nodes={}
    for point in features:
        scaled=point[:3]/.3;key=tuple((np.sign(scaled)*np.floor(np.abs(scaled)+.5)).astype(int))
        total,count=nodes.get(key,(0.,0));nodes[key]=(total+point[5],count+1)
    return {k:total/count for k,(total,count) in nodes.items()}
def cross(t):return np.array([[0.,-t[2],t[1]],[t[2],0.,-t[0]],[-t[1],t[0],0.]])
def centered(H,g,t,axis):
    B=np.eye(6);B[:3,3:]=cross(t);A=B.T@H@B;b=B.T@g
    rr=A[3:,3:]+1e-6*np.eye(3)
    S=A[:3,:3]-A[:3,3:]@np.linalg.solve(rr,A[3:,:3]);S=(S+S.T)*.5
    rhs=b[:3]-A[:3,3:]@np.linalg.solve(rr,b[3:])
    return dict(eigenvalues=np.linalg.eigvalsh(S).tolist(),longitudinal_information=float(axis@S@axis) if axis is not None else None,
        longitudinal_rhs=float(axis@rhs) if axis is not None else None)
def group_result(group):
    folder=E/'groups'/group
    for name in read(folder/'archive.json')['files']:decode(folder,name)
    require(read(folder/'native_command.json')['exit_code']==0,'Native passive execution failed')
    rows=extras(folder);parent=verify_transport(folder,rows)
    terminals={(r['frame'],r['mode']):r['trace'] for r in parent if r['type']=='terminal'}
    pins=read(E/'protocol.json')['input_pins'][group]
    require(set(terminals)=={(f,m) for f in pins['frames'] for m in range(18)},'Historical arms/endpoints changed')
    observations=[json.loads(r['raw_line']) for r in rows]
    clouds={r['frame']:[np.array(c,float).reshape(-1,6) for c in r['classes']] for r in observations if r['type']=='support_features'}
    for r in parent:
        if r['type']=='features':require([c[:,:5].tolist() for c in clouds[r['frame']][:3]]==r['classes'],'Historical feature values changed')
    query_rows=[r for r in observations if r['type']=='support_query'];by_step={};counts={};pca_cache={};field_cache={}
    for record in query_rows:
        f,m,translation,it,c=(record[k] for k in ('frame','mode','translation_only','iteration','class'))
        require(m in (6,7,14,15),'Unexpected passive arm')
        trace=terminals[f,m]
        solver=trace['s2s_consensus']['refinement'] if translation else trace['s2s']
        step=solver['iterations'][it];require(step['linearization_pose']==record['pose'],'Passive pose differs')
        excluded=set(trace['s2s_consensus']['excluded_clusters']) if translation else set()
        checked,statistics=reconstruct(record,clouds,excluded,pca_cache,field_cache)
        key=(f,m,translation,it);parts=by_step.setdefault(key,{})
        require(c not in parts,'Duplicate class observation');parts[c]=checked
        for name,n in statistics.items():counts[name]=counts.get(name,0)+n
    steps=[];expected=set()
    for (f,m),trace in terminals.items():
        if m not in (6,7,14,15):continue
        phases=[(False,trace['s2s'])]
        if m%2:phases.append((True,trace['s2s_consensus']['refinement']))
        for translation,solver in phases:
            seed=np.array(solver['input']).reshape(4,4)[:3,3];norm=np.linalg.norm(seed);axis=seed/norm if norm>0 else None
            information=np.array(solver.get('translation_prior_information',[0.]*9)).reshape(3,3)
            for step in solver['iterations']:
                key=(f,m,translation,step['index']);expected.add(key);require(key in by_step and set(by_step[key])==set(range(4)),'Incomplete class/iteration population')
                parts=by_step[key];pose=np.array(step['linearization_pose']).reshape(4,4);t=pose[:3,3]
                sums={name:{k:sum(parts[c][name][k] for c in range(4)) for k in ('H','g','count','cost')} for name in ('all','kept','removed')}
                actual=sums['kept' if m>=14 else 'all']
                J=np.column_stack((np.eye(3),-cross(t)))
                prior_H=J.T@information@J if step['pairs']>=30 else np.zeros((6,6))
                prior_g=J.T@information@(t-seed) if step['pairs']>=30 else np.zeros(6)
                close(actual['H']+prior_H,np.array(step['H']).reshape(6,6),'Passive/native total H differs')
                close(actual['g']+prior_g,step['g'],'Passive/native total g differs')
                require(actual['count']==step['pairs'],'Actual pair count differs')
                require([parts[c]['kept' if m>=14 else 'all']['count'] for c in range(4)]==step['class_pairs'],'Actual class counts differ')
                values={name:centered(sums[name]['H'],sums[name]['g'],t,axis) for name in ('all','kept')}
                steps.append(dict(frame=f,mode=m,translation_only=translation,iteration=step['index'],
                    classes=[{name:dict(count=parts[c][name]['count'],cost=parts[c][name]['cost']) for name in ('all','kept','removed')} for c in range(4)],
                    valid_all=sums['all']['count'],valid_removed=sums['removed']['count'],information=values))
    require(set(by_step)==expected,'Unexpected passive steps')
    return dict(group=group,native_outcomes=len(terminals),all_parent_stdout_bytes_exact=True,
        passive_class_queries=len(query_rows),steps=steps,counts=counts,feature_frames=sorted(clouds))
def validate():
    protocol=read(E/'protocol.json')
    for name,digest in protocol['source_sha256'].items():require(sha(W/name)==digest,'Frozen source changed: '+name)
    for label in ('compiler','replay_compile','probe_compile','unit_compile','unit','ldd'):
        require(read(E/'checks/build'/(label+'_command.json'))['exit_code']==0,'Native build/unit step failed')
    groups={g:group_result(g) for g in ('old','01','02','08')}
    return dict(exit_code=0,groups=groups,all_558_historical_native_outcomes_exact=True,
        passive_class_queries=sum(g['passive_class_queries'] for g in groups.values()),
        counts={k:sum(g['counts'][k] for g in groups.values()) for k in next(iter(groups.values()))['counts']},
        scope='Fixed archived poses; no solver update, candidate, gate revision or trajectory claim.')
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();result=validate()
    a.output.write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k!='groups'}))
