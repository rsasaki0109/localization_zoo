"""Independent tensor-polynomial reconstruction of every archived native query."""
from pathlib import Path
import gzip, hashlib, importlib.util, json, math
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
def read(p):return json.loads(Path(p).read_text())
def require(ok,why):
    if not ok:raise ValueError(why)
def digest(raw):return hashlib.sha256(raw).hexdigest()
def decode(folder,name):
    entry=read(folder/'archive.json')['files'][name]
    if entry['storage']=='ordered_gzip_shards':
        pieces=[]
        for part in entry['parts']:
            compressed=(folder/part['path']).read_bytes();raw=gzip.decompress(compressed)
            require(digest(compressed)==part['gzip_sha256'],'Compressed bytes changed')
            require(len(raw)==part['raw_bytes'] and digest(raw)==part['raw_sha256'],'Shard bytes changed')
            pieces.append(raw)
        raw=b''.join(pieces)
    else:
        data=(folder/entry['path']).read_bytes();require(digest(data)==entry['stored_sha256'],'Stored bytes changed')
        raw=gzip.decompress(data) if entry['storage']=='gzip' else data
    require(len(raw)==entry['bytes'] and digest(raw)==entry['sha256'],'Full bytes changed')
    return raw

# Polynomial coefficients in ascending powers; derivative follows by multiplication.
COEFFICIENTS=np.array([[1,-3,3,-1],[4,0,-6,3],[1,3,3,-3],[0,0,0,1]],dtype=float)/6
OFFSETS=np.array([(i,j,k) for i in range(-1,3) for j in range(-1,3) for k in range(-1,3)])
def evaluate(nodes,positions,knot,cells=None):
    x=np.asarray(positions,dtype=float).reshape(-1,3);s=x/knot
    cells=np.floor(s).astype(int) if cells is None else np.asarray(cells,dtype=int)
    unique,inverse=np.unique(cells,axis=0,return_inverse=True)
    observed=np.array([[nodes.get(tuple(key+offset),np.nan) for offset in OFFSETS] for key in unique])
    count=np.isfinite(observed).sum(axis=1)
    means=np.nansum(observed,axis=1)/np.maximum(count,1)
    controls=np.where(np.isfinite(observed),observed,means[:,None])[inverse]
    t=s-cells
    powers=t[:,:,None]**np.arange(4);deriv=t[:,:,None]**np.arange(3)
    basis=powers@COEFFICIENTS.T
    dbasis=deriv@(COEFFICIENTS[:,1:]*np.arange(1,4)).T
    indexes=OFFSETS+1
    weights=np.prod(basis[:,np.arange(3)[None,:],indexes],axis=2)
    value=np.sum(weights*controls,axis=1)
    gradient=[]
    for axis in range(3):
        b=basis.copy();b[:,axis,:]=dbasis[:,axis,:]
        gradient.append(np.sum(np.prod(b[:,np.arange(3)[None,:],indexes],axis=2)*controls,axis=1)/knot)
    return count[inverse]>=4,value,np.array(gradient).T,count[inverse],means[inverse]

def close(a,b):return np.allclose(a,b,rtol=1e-10,atol=1e-12)
def node_map(field):
    nodes={tuple(r[:3]):r[3]/r[4] for r in field['nodes']}
    require(len(nodes)==len(field['nodes']),'Duplicate lattice node')
    require(all(r[4]>0 and isinstance(r[4],int) for r in field['nodes']),'Invalid node population')
    require(sum(r[4] for r in field['nodes'])==field['features'],'Incomplete build-input population')
    return nodes
def all_samples(path):
    return path['endpoints']+path['interior']+[s for b in path['boundaries'] for s in b['samples']]
def check_field(field,paths,expected=None):
    nodes=node_map(field);knot=field['knot'];samples=[s for p in paths for s in all_samples(p)]
    valid,value,grad,_,_=evaluate(nodes,[s['x'] for s in samples],knot)
    require(valid.tolist()==[s['valid'] for s in samples],'Native support validity differs')
    require(all((s['value'] is not None and s['gradient'] is not None) if s['valid'] else (s['value'] is None and s['gradient'] is None) for s in samples),'Invalid query encoded as a value')
    require(close(value[valid],[s['value'] for s in samples if s['valid']]),'Native intensity reconstruction differs')
    native_grad=np.asarray([s['gradient'] for s in samples if s['valid']],dtype=float).reshape(-1,3)
    require(close(grad[valid],native_grad),'Native gradient reconstruction differs')
    boundaries=[];fd_max=0.;fd_count=0;endpoint_count=0
    for p in paths:
        x=np.asarray(p['start']);delta=np.asarray(p['delta'])
        require(close(p['endpoints'][0]['x'],x) and close(p['endpoints'][1]['x'],x+delta),'Path endpoints changed')
        if expected is not None:
            e=expected[int(field['field']),p['index']]
            require(e['x']==p['start'] and e['delta']==p['delta'],'Inherited path input changed')
            a,b=p['endpoints']
            require(a['valid'] and close(a['value'],e['original']),'Original native field control differs')
            require(b['valid']==(e['candidate'] is not None),'Candidate field coverage differs')
            if b['valid']:require(close(b['value'],e['candidate']),'Candidate native field control differs')
            require(close(np.array(a['gradient'])*e['scale'],e['gradient']),'Original native reflector gradient differs')
            endpoint_count+=2
        interior=p['interior'];center=np.asarray(interior[0]['x'])
        expected_center=(np.floor(x/knot)+.375)*knot
        require(close(center,expected_center) and len(interior)==7,'Interior control changed')
        cells=[np.floor(np.asarray(s['x'])/knot).astype(int).tolist() for s in interior]
        require(all(c==cells[0] for c in cells),'Finite differences crossed a cell face')
        require(all(s['valid']==interior[0]['valid'] for s in interior),'Inside-cell validity changed')
        for axis in range(3):
            low,high=interior[1+2*axis:3+2*axis]
            for sign,sample in ((-1,low),(1,high)):
                target=center.copy();target[axis]+=sign*knot*1e-5
                require(close(target,sample['x']),'Finite difference location changed')
            if interior[0]['valid']:
                finite=(high['value']-low['value'])/(2*knot*1e-5)
                error=abs(finite-interior[0]['gradient'][axis]);fd_max=max(fd_max,error);fd_count+=1
                require(error<=1e-8,'Within-cell derivative control failed')
        wanted=[]
        for axis in range(3):
            if delta[axis]==0:continue
            lo=math.floor(min(x[axis],x[axis]+delta[axis])/knot)
            hi=math.floor(max(x[axis],x[axis]+delta[axis])/knot)
            for face in range(lo+1,hi+1):
                t=(face*knot-x[axis])/delta[axis]
                if 0<t<1:wanted.append((axis,face,t))
        require(len(wanted)==len(p['boundaries']),'Crossed cell face missing')
        for b,(axis,face,t) in zip(p['boundaries'],wanted):
            require(b['axis']==axis and b['face']==face and close(b['t'],t),'Crossing geometry changed')
            point=x+t*delta;point[axis]=face*knot
            samples=b['samples'];require(len(samples)==8,'Missing convergence/ULP sample')
            for j,s in enumerate(samples):
                side=-1 if j%2==0 else 1;eps=(1e-3,1e-6,1e-9,0)[j//2]
                pos=np.asarray(s['x']);target=point.copy();target[axis]=(face+side*eps)*knot
                cell=np.floor(pos/knot).astype(int)
                require(cell[axis]==(face-1 if side<0 else face),'Wrong native side of face')
                require(close(pos[np.arange(3)!=axis],point[np.arange(3)!=axis]),'Probe left crossing line')
                if eps:require(close(pos,target),'Epsilon probe changed')
                else:
                    ulp=max(math.ulp(point[axis]),np.finfo(float).tiny)
                    require(abs(pos[axis]-point[axis])<=17*ulp,'Probe is not a nearest-ULP neighbor')
            # Both limiting polynomials evaluated at identical spatial point, with
            # the actual native cell on each side. Controls are fixed per cell.
            c=[np.floor(np.asarray(s['x'])/knot).astype(int) for s in samples[-2:]]
            lv,lvalue,lgrad,count,mean=evaluate(nodes,[point,point],knot,c)
            actual=samples[-2:]
            require(lv.tolist()==[s['valid'] for s in actual],'Limit support differs')
            for j in range(2):
                if lv[j]:require(close(lvalue[j],actual[j]['value']) and close(lgrad[j],actual[j]['gradient']),'Native ULP samples do not match polynomial limits')
            both=bool(lv.all());jump=float(lvalue[1]-lvalue[0]) if both else None
            boundaries.append(dict(index=p['index'],axis=axis,face=face,t=t,both_valid=both,left_valid=bool(lv[0]),right_valid=bool(lv[1]),support=count.tolist(),mean=mean.tolist(),jump=jump,gradient_jump=float(np.linalg.norm(lgrad[1]-lgrad[0])) if both else None,ulp_jump=actual[1]['value']-actual[0]['value'] if both else None))
    supported=[b for b in boundaries if b['both_valid']];jumps=[b for b in supported if abs(b['jump'])>1e-8]
    result=dict(field=field['field'],features=field['features'],nodes=len(nodes),paths=len(paths),queries=sum(len(all_samples(p)) for p in paths),endpoint_controls=endpoint_count,interior_derivative_controls=fd_count,max_interior_derivative_abs_error=fd_max,boundaries=len(boundaries),both_valid=len(supported),mixed=sum(b['left_valid']!=b['right_valid'] for b in boundaries),neither_valid=sum(not b['left_valid'] and not b['right_valid'] for b in boundaries),jumps_above_1e_8=len(jumps),max_abs_jump=max((abs(b['jump']) for b in supported),default=0.),max_gradient_jump=max((b['gradient_jump'] for b in supported),default=0.),largest=max(supported,key=lambda b:abs(b['jump']),default=None))
    return result,boundaries

def validate_group(group,folder):
    for name in read(folder/'archive.json')['files']:decode(folder,name)
    command=read(folder/'native_command.json');require(command['exit_code']==0,'Native process failed')
    rows=[json.loads(line) for line in decode(folder,'native.stdout.jsonl').splitlines()]
    fields={r['field']:r for r in rows if r['type']=='field'}
    require(len(fields)==sum(r['type']=='field' for r in rows),'Duplicate native field')
    paths={key:[r for r in rows if r['type']=='path' and r['field']==key] for key in fields}
    require(sum(map(len,paths.values()))==sum(r['type']=='path' for r in rows),'Path without field')
    expected=None
    if group!='synthetic':
        es=read(folder/'expected.json');expected={(e['frame'],e['index']):e for e in es}
        require(len(expected)==len(es)==sum(map(len,paths.values())),'Missing/duplicate inherited path')
        require({int(k) for k in fields}==set(read(folder/'input_sources.json')['frames']),'Inherited endpoint omitted')
        check_parent_inputs(group,folder,expected)
    reports=[];details={}
    for key,f in fields.items():
        require(len({p['index'] for p in paths[key]})==len(paths[key]),'Duplicate path index')
        r,bs=check_field(f,paths[key],expected);reports.append(r);details[key]=bs
        if group=='synthetic':
            if key.startswith(('dense_ramp','sparse_constant')):require(r['max_abs_jump']<=1e-12 and r['both_valid']==1,'Continuous control failed')
            elif key.startswith('sparse_ramp'):require(r['jumps_above_1e_8']==1,'Sparse nonconstant discontinuity absent')
            else:require(r['both_valid']==0 and r['neither_valid']==1,'Invalid support control failed')
    return reports,details

def check_parent_inputs(group,folder,expected):
    parent=E.parent/'inten_nonfacade_consensus_support';pg=parent/'groups'/group
    meta=read(folder/'input_sources.json');pm=read(pg/'input_sources.json')
    require(meta['frames']==pm['frames'] and meta['feature_cache_sha256']==pm['feature_cache_sha256'],'Parent feature cache provenance changed')
    require(meta['parent_archive_sha256']==digest((pg/'archive.json').read_bytes()),'Parent archive manifest changed')
    require(meta['attribution_archive_sha256']==digest((parent/'attribution/archive.json').read_bytes()),'Parent query manifest changed')
    require(meta['no_raw_acquired'] and meta['no_gt_in_native_input'],'Unexpected input dependency')
    original=[json.loads(s) for s in decode(parent/'attribution',group+'/joint_primitives.jsonl').splitlines()]
    guard=[json.loads(s) for s in decode(pg,'primitives.jsonl').splitlines()]
    poses=[]
    for role in ('joint','refined'):
        poses.append({int(line.split()[0]):list(map(float,line.split()[1:])) for line in (parent/'attribution'/group/(role+'_input.txt')).read_text().splitlines()})
    count=0;commands=[]
    for frame in meta['frames']:
        ps=[p for p in original if p['frame']==frame and p['class']==3]
        previous=[p for p in guard if p['frame']==frame and p['class']==3]
        delta=[poses[1][frame][i]-poses[0][frame][i] for i in (3,7,11)]
        require(not previous or len(ps)==len(previous),'Parent support changed')
        if not previous:require(not any(delta),'Fallback query changed pose')
        for index,p in enumerate(ps):
            e=expected[frame,index];prior=previous[index] if previous else p
            require(p['x']==prior['x'] and p['mu']==prior['mu'],'Original parent queries differ')
            target=dict(frame=frame,index=index,x=p['x'],delta=delta,original=p['mu'],candidate=prior['other_mu'] if previous else p['mu'],intensity=p['intensity'],gradient=p['a'],scale=p['scale'])
            require(e==target,'Parent endpoint expectation changed')
            commands.append(' '.join(map(str,[frame,index,*p['x'],*delta])));count+=1
    require(count==len(expected),'Unexpected inherited paths')
    require((folder/'paths.txt').read_text()=='\n'.join(commands)+'\n','Actual native path input changed')

def validate():
    for path,d in read(E/'protocol.json')['source_sha256'].items():require(digest((W/path).read_bytes())==d,'Pinned source changed: '+path)
    build=E/'checks/build'
    for name in read(build/'archive.json')['files']:decode(build,name)
    for label in ('compiler','native_compile','probe_compile','ldd'):require(read(build/(label+'_command.json'))['exit_code']==0,'Native build failure')
    compile_command=read(build/'native_compile_command.json')['command']
    require(any(s.endswith('/papers/inten_loam/src/inten_loam_paper.cpp') for s in compile_command),'Probe did not compile production source')
    require(read(build/'binaries.json')['source_sha256']==read(E/'protocol.json')['source_sha256'],'Build source hashes changed')
    reports={};details={}
    for group in ('synthetic','old','01','02','08'):
        reports[group],details[group]=validate_group(group,E/'groups'/group)
    actual=[r for g,rs in reports.items() if g!='synthetic' for r in rs]
    totals={key:sum(r[key] for r in actual) for key in ('paths','queries','endpoint_controls','interior_derivative_controls','boundaries','both_valid','mixed','neither_valid','jumps_above_1e_8')}
    totals.update(fields=len(actual),max_abs_jump=max(r['max_abs_jump'] for r in actual),max_gradient_jump=max(r['max_gradient_jump'] for r in actual),max_interior_derivative_abs_error=max(r['max_interior_derivative_abs_error'] for r in actual))
    summary=dict(exit_code=0,all_native_values_gradients_and_one_sided_limits_reconstructed=True,all_parent_field_endpoint_and_gradient_controls_passed=True,synthetic=reports['synthetic'],groups={g:rs for g,rs in reports.items() if g!='synthetic'},totals=totals)
    return summary,details

def equivalent(actual,recorded):
    if isinstance(actual,dict):
        require(isinstance(recorded,dict) and actual.keys()==recorded.keys(),'Recorded report keys differ')
        for key in actual:equivalent(actual[key],recorded[key])
    elif isinstance(actual,list):
        require(isinstance(recorded,list) and len(actual)==len(recorded),'Recorded report length differs')
        for a,b in zip(actual,recorded):equivalent(a,b)
    elif isinstance(actual,float):require(close(actual,recorded),'Recorded report number differs')
    else:require(actual==recorded,'Recorded report value differs')

def check_recorded(summary,details):
    for name in read(E/'report/archive.json')['files']:decode(E/'report',name)
    equivalent(summary,read(E/'report/summary.json'))
    equivalent(details,json.loads(gzip.decompress((E/'report/boundaries.json.gz').read_bytes())))
if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);parser.add_argument('--details',type=Path)
    args=parser.parse_args();summary,details=validate();check_recorded(summary,details);args.output.write_text(json.dumps(summary,indent=2,allow_nan=False)+'\n')
    if args.details:args.details.write_bytes(gzip.compress((json.dumps(details,allow_nan=False)+'\n').encode(),mtime=0))
    print(json.dumps(summary['totals']))
