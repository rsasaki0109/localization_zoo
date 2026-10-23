"""Reconstruct occupied-weight fields and retain all paired motion outcomes."""
from pathlib import Path
import importlib.util, json, math, hashlib
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
V=module('normalized_previous_field',E.parent/'inten_intensity_continuity/verify.py')
P=module('normalized_previous_motion',E.parent/'inten_nonfacade_consensus_support/validate.py')
def read(p):return json.loads(Path(p).read_text())
def require(ok,why):
    if not ok:raise ValueError(why)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def evaluate(nodes,positions,knot,cells=None):
    x=np.asarray(positions,dtype=float).reshape(-1,3);s=x/knot
    cells=np.floor(s).astype(int) if cells is None else np.asarray(cells,dtype=int)
    unique,inverse=np.unique(cells,axis=0,return_inverse=True)
    observed=np.array([[nodes.get(tuple(key+offset),np.nan) for offset in V.OFFSETS] for key in unique])
    occupied=np.isfinite(observed);counts=occupied.sum(axis=1)[inverse]
    controls=np.where(occupied,observed,0.)[inverse];mask=occupied[inverse]
    t=s-cells
    # Independent nonnegative Bernstein representation avoids cancellation of
    # (1-t)^3 at ULP neighbors. Common positive factors cancel analytically.
    bernstein=np.stack(((1-t)**3,3*t*(1-t)**2,3*t*t*(1-t),t**3),axis=2)
    coefficients=np.array([[1,0,0,0],[4,4,2,1],[1,2,4,4],[0,0,0,1]],dtype=float)/6
    basis=bernstein@coefficients.T
    quadratic=np.stack(((1-t)**2,2*t*(1-t),t*t),axis=2)
    dbasis=quadratic@(3*np.diff(coefficients,axis=1)).T
    indexes=V.OFFSETS+1
    factor=np.ones(len(x))
    for axis in range(3):
        lower=np.min(np.where(occupied,indexes[:,axis],4),axis=1)[inverse]
        upper=np.max(np.where(occupied,indexes[:,axis],-1),axis=1)[inverse]
        rows=np.flatnonzero(lower==upper);index=lower[rows]
        factor[rows]*=basis[rows,axis,index]
        basis[rows,axis,index]=1.
        dbasis[rows,axis,index]=0.
    weights=np.prod(basis[:,np.arange(3)[None,:],indexes],axis=2)
    denominator=np.sum(weights*mask,axis=1);numerator=np.sum(weights*controls,axis=1)
    # At an exact face, a removable common zero can have a finite one-sided
    # limit while eval(face) is invalid. Keep that limit and flag zero weight;
    # continuity claims require positive occupied weight at the face itself.
    valid=(counts>=4)&(denominator>0)&((factor>0) if cells is None else True)
    value=np.divide(numerator,denominator,out=np.zeros(len(x)),where=denominator>0)
    gradient=[]
    for axis in range(3):
        b=basis.copy();b[:,axis,:]=dbasis[:,axis,:]
        dw=np.prod(b[:,np.arange(3)[None,:],indexes],axis=2)
        dn=np.sum(dw*controls,axis=1);dd=np.sum(dw*mask,axis=1)
        gradient.append(np.divide(dn-value*dd,denominator*knot,out=np.zeros(len(x)),where=denominator>0))
    return valid,value,np.array(gradient).T,counts,denominator*factor

def check_field(field,paths):
    previous=V.evaluate;V.evaluate=evaluate
    try:return V.check_field(field,paths)
    finally:V.evaluate=previous

def field_group(group):
    folder=E/'fields'/group
    for name in read(folder/'archive.json')['files']:V.decode(folder,name)
    require(read(folder/'native_command.json')['exit_code']==0,'Native field query failed')
    rows=[json.loads(r) for r in V.decode(folder,'native.stdout.jsonl').splitlines()]
    fields={r['field']:r for r in rows if r['type']=='field'}
    require(len(fields)==sum(r['type']=='field' for r in rows),'Duplicate field')
    prior=E.parent/'inten_intensity_continuity/groups'/group
    oldrows=[json.loads(r) for r in V.decode(prior,'native.stdout.jsonl').splitlines()]
    require([r for r in rows if r['type']=='field']==[r for r in oldrows if r['type']=='field'],'Native build node populations changed')
    oldpaths={(r['field'],r['index']):r for r in oldrows if r['type']=='path'}
    paths=[r for r in rows if r['type']=='path']
    require(len(paths)==len(oldpaths),'Inherited path omitted')
    for path in paths:
        old=oldpaths[path['field'],path['index']]
        for k in ('field','index','start','delta'):require(path[k]==old[k],'Inherited path changed')
        require([s['x'] for s in V.all_samples(path)]==[s['x'] for s in V.all_samples(old)],'Native query location changed')
    if group!='synthetic':
        require((folder/'paths.txt').read_bytes()==(prior/'paths.txt').read_bytes(),'Path input changed')
        require(read(folder/'input_sources.json')==read(prior/'input_sources.json'),'Cache provenance changed')
    reports=[];details={}
    for key,field in fields.items():
        current=[p for p in paths if p['field']==key]
        r,bs=check_field(field,current)
        for b in bs:
            b['limit_occupied_weight']=b.pop('mean')
            b['zero_weight_limit']=any(w<=0 for w in b['limit_occupied_weight'])
        supported=[b for b in bs if b['both_valid'] and not b['zero_weight_limit']]
        r['zero_weight_limits']=sum(b['zero_weight_limit'] for b in bs)
        r['zero_weight_neighbor_both_valid']=sum(b['zero_weight_limit'] and b['both_valid'] for b in bs)
        r['positive_weight_both_valid']=len(supported)
        r['jumps_above_1e_8']=sum(abs(b['jump'])>1e-8 for b in supported)
        r['max_abs_jump']=max((abs(b['jump']) for b in supported),default=0.)
        r['max_gradient_jump']=max((b['gradient_jump'] for b in supported),default=0.)
        reports.append(r);details[key]=bs
        require(r['jumps_above_1e_8']==0,'Normalized field has a both-valid intensity jump')
        require(r['max_gradient_jump']<=1e-8,'Normalized field gradient has a both-valid jump')
        if group=='synthetic':
            if key.startswith('invalid_three'):require(r['neither_valid']==1,'Invalid support accepted')
            else:require(r['both_valid']==1 and r['max_abs_jump']<=1e-12,'Continuous synthetic control failed')
    return reports,details

def motion_group(group):
    folder=E/'groups'/group;parent=E.parent/'inten_nonfacade_consensus_support/groups'/group
    for name in read(folder/'archive.json')['files']:V.decode(folder,name)
    require(read(folder/'native_command.json')['exit_code']==0,'Native replay failed')
    meta=read(folder/'input_sources.json');pm=read(parent/'input_sources.json')
    require(meta['frames']==pm['frames'] and meta['feature_cache_sha256']==pm['feature_cache_sha256'],'Inherited case set changed')
    require(meta['parent_archive_sha256']==sha(parent/'archive.json'),'Parent archive changed')
    require((folder/'input.txt').read_bytes()==(parent/'input.txt').read_bytes(),'Estimated seeds changed')
    rows=[json.loads(r) for r in V.decode(folder,'native.stdout.jsonl').splitlines()]
    old=[json.loads(r) for r in V.decode(parent,'native.stdout.jsonl').splitlines()]
    require([r for r in rows if r['type']=='profile']==[r for r in old if r['type']=='profile'],'Cluster profiles changed')
    terminal={(r['frame'],r['mode']):r['trace'] for r in rows if r['type']=='terminal'}
    expected={(r['frame'],r['mode']):r['trace'] for r in old if r['type']=='terminal' and r['mode']<2}
    require(set(terminal)=={(f,m) for f in meta['frames'] for m in range(4)},'Missing/extra native arm')
    truths=read(E.parent/'inten_zero_consensus_control/groups'/group/'offline_truth.json')
    cases=[]
    for f in meta['frames']:
        metrics={}
        for m in range(4):
            t=terminal[f,m]
            if m<2:require(t==expected[f,m],f'Default-off exact control differs {group}/{f}/{m}')
            require(t['features']==terminal[f,0]['features'],'Extraction changed')
            output=P.matrix(t['accepted_relative']);truth=P.matrix(truths[str(f)])
            value=P.metrics(output,truth)
            error=output[:3,:3]@truth[:3,:3].T
            skew=np.array([error[2,1]-error[1,2],error[0,2]-error[2,0],error[1,0]-error[0,1]])
            value['rotation_error_rad']=float(np.arctan2(np.linalg.norm(skew),np.trace(error)-1))
            metrics[str(m)]=value
        cases.append(dict(frame=f,metrics=metrics))
    return dict(group=group,cases=cases,native_calls=len(terminal),all_legacy_controls_exact=True)

def validate():
    protocol=read(E/'protocol.json')
    build=E/'checks/build'
    for name in read(build/'archive.json')['files']:V.decode(build,name)
    measured=read(build/'source_manifest.json')
    import gzip
    for name,digest in protocol['source_sha256'].items():
        require(measured[name]['sha256']==digest,'Measured source pin changed')
        raw=gzip.decompress(V.decode(build,measured[name]['path']))
        require(hashlib.sha256(raw).hexdigest()==digest,'Measured source bytes changed')
        if name.startswith(str(E.relative_to(W))+'/'):
            require(sha(W/name)==digest,'Frozen harness changed: '+name)
    for label in ('compiler','native_compile','replay_compile','probe_compile','field_probe_compile','unit_compile','unit_tests','ldd'):
        require(read(build/(label+'_command.json'))['exit_code']==0,'Build/test failure')
    require(read(build/'binaries.json')['source_sha256']==protocol['source_sha256'],'Build hashes changed')
    fields={};details={}
    for group in ('synthetic','old','01','02','08'):
        fields[group],details[group]=field_group(group)
    groups={g:motion_group(g) for g in ('old','01','02','08')}
    cases=[dict(group=g,**c) for g,v in groups.items() for c in v['cases']]
    checks=[]
    for c in cases:
        for a,b in ((0,2),(1,3)):
            old,new=(c['metrics'][str(m)] for m in (a,b))
            checks.append(dict(group=c['group'],frame=c['frame'],arms=[a,b],criterion='no newly bad length/vector labels',
                passed=(not new['bad_length'] or old['bad_length']) and (not new['bad_vector'] or old['bad_vector'])))
        if c['group']=='old':
            a,b,new=(c['metrics'][str(m)] for m in (0,1,3))
            if (a['bad_length'] or a['bad_vector']) and not (b['bad_length'] or b['bad_vector']):
                checks.append(dict(group='old',frame=c['frame'],criterion='retain old consensus rescue',passed=not(new['bad_length'] or new['bad_vector'])))
    totals={str(m):{key:sum(c['metrics'][str(m)][key] for c in cases) for key in ('length_error','vector_error','rotation_error_rad','bad_length','bad_vector')} for m in range(4)}
    for key in ('length_error','vector_error'):
        checks.append(dict(criterion='normalized joint reduces total '+key,passed=totals['2'][key]<totals['0'][key]))
    checks.append(dict(criterion='normalized joint does not increase total rotation error',passed=totals['2']['rotation_error_rad']<=totals['0']['rotation_error_rad']))
    actual=[r for g,rs in fields.items() if g!='synthetic' for r in rs]
    ft={key:sum(r[key] for r in actual) for key in ('paths','queries','boundaries','both_valid','mixed','neither_valid','jumps_above_1e_8','interior_derivative_controls','positive_weight_both_valid','zero_weight_limits','zero_weight_neighbor_both_valid')}
    ft.update(fields=len(actual),max_abs_jump=max(r['max_abs_jump'] for r in actual),max_gradient_jump=max(r['max_gradient_jump'] for r in actual))
    return dict(exit_code=0,groups=groups,motion_totals=totals,gate_checks=checks,
        progression_gate_passed=all(c['passed'] for c in checks),fields=fields,field_totals=ft,
        native_motion_calls=sum(g['native_calls'] for g in groups.values()),
        all_legacy_controls_exact=True,all_field_values_gradients_and_limits_reconstructed=True),details

if __name__=='__main__':
    import argparse,gzip
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);parser.add_argument('--details',type=Path,required=True)
    args=parser.parse_args();summary,details=validate()
    args.output.write_text(json.dumps(summary,indent=2,allow_nan=False)+'\n')
    args.details.write_bytes(gzip.compress((json.dumps(details,allow_nan=False)+'\n').encode(),mtime=0))
    print(json.dumps(dict(exit_code=0,gate_passed=summary['progression_gate_passed'],native_calls=summary['native_motion_calls'],motion=summary['motion_totals'],field=summary['field_totals'])))
