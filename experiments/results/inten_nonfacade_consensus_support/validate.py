"""Independent common-primitive geometry/loss/equation and native decision checks."""
from pathlib import Path
import json,gzip,hashlib,math,importlib.util
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2];PARENT=E.parent/'inten_zero_consensus_control'

def read(p):return json.loads(Path(p).read_text())
def matrix(v):return np.asarray(v,dtype=float).reshape(4,4)
def close(a,b):return math.isclose(a,b,rel_tol=1e-10,abs_tol=1e-12)
def require(ok,why):
    if not ok:raise ValueError(why)
def decode(folder,name):
    entry=read(folder/'archive.json')['files'][name]
    if entry['storage']=='ordered_gzip_shards':
        pieces=[]
        for part in entry['parts']:
            compressed=(folder/part['path']).read_bytes();raw=gzip.decompress(compressed)
            require(hashlib.sha256(compressed).hexdigest()==part['gzip_sha256'],'Compressed input changed')
            require(len(raw)==part['raw_bytes'] and hashlib.sha256(raw).hexdigest()==part['raw_sha256'],'Raw input changed')
            pieces.append(raw)
        raw=b''.join(pieces)
    else:
        data=(folder/entry['path']).read_bytes();require(hashlib.sha256(data).hexdigest()==entry['stored_sha256'],'Stored input changed')
        raw=gzip.decompress(data) if entry['storage']=='gzip' else data
    require(len(raw)==entry['bytes'] and hashlib.sha256(raw).hexdigest()==entry['sha256'],'Full input changed')
    return raw

def rho(r,h):return r*r if abs(r)<=h else 2*h*abs(r)-h*h

def check_support(row,primitives):
    report=row['support'];trace=row['trace'];c=trace.get('s2s_consensus')
    if not report['evaluated']:
        require(not primitives and (not c or not c['usable'] or c['refinement']['returned_pairs']<30),'Missing support observation')
        return dict(queried=False,allowed=False,missing_field=0,primitives=0)
    require(c and c['usable'] and c['refinement']['returned_pairs']>=30,'Unexpected guard query')
    joint=matrix(trace['s2s']['output']);proposal=matrix(c['refinement']['output']);delta=np.asarray(report['delta'])
    require(np.array_equal(joint[:3,:3],proposal[:3,:3]),'Refinement rotation changed')
    require(np.array_equal(delta,proposal[:3,3]-joint[:3,3]),'Counterfactual displacement changed')
    counts=[0]*4;original=[0.]*4;candidate=[0.]*4;H=np.zeros((6,6));g=np.zeros(6);weighted=0.;complete=True;missing=0
    for p in primitives:
        cls=p['class'];counts[cls]+=1
        x=np.asarray(p['x']);a=np.asarray(p['a']);anchor=np.asarray(p['anchor']);direction=np.asarray(p['direction']);r=p['r'];h=p['huber'];changed=p['changed']
        if p['kind']=='line':
            d=x-anchor;v=d-direction*float(direction@d);expected=float(np.linalg.norm(v))
            d=x+delta-anchor;new=float(np.linalg.norm(d-direction*float(direction@d)))
            require(np.allclose(a,v/expected,rtol=1e-10,atol=1e-12),'Original line gradient changed')
        elif p['kind']=='plane':expected=float(a@(x-anchor));new=float(a@(x+delta-anchor))
        else:
            require(p['kind']=='field' and cls==3,'Unknown primitive')
            expected=p['scale']*(p['mu']-p['intensity'])
            new=p['scale']*(p['other_mu']-p['intensity']) if p['complete'] else None
        require(close(r,expected),'Original primitive residual changed')
        if p['complete']:require(new is not None and close(changed,new),'Candidate primitive residual changed')
        else:
            require(cls==3 and changed is None and p['other_mu'] is None,'Unsupported non-field sample')
            missing+=1;complete=False
        original[cls]+=rho(r,h)
        if p['complete']:candidate[cls]+=rho(changed,h)
        jac=np.r_[a,np.cross(x,a)];weight=1. if abs(r)<=h else h/abs(r)
        H+=weight*np.outer(jac,jac);g+=weight*jac*r;weighted+=weight*r*r
    require(counts==report['counts'],'Support counts changed')
    require(all(close(a,b) for a,b in zip(original,report['original_costs'])),'Original robust cost changed')
    require(all(close(a,b) for a,b in zip(candidate,report['candidate_costs'])),'Candidate robust cost changed')
    require(report['complete']==complete,'Missing support hidden')
    # Independently reconstructed costs agree above. Branch on exact serialized
    # native sums so tiny portable reconstruction roundoff cannot flip a tie.
    allowed=complete and sum(counts[c] for c in (0,1,3))>0 and sum(report['candidate_costs'][c] for c in (0,1,3))<sum(report['original_costs'][c] for c in (0,1,3))
    require(report['allowed']==allowed,'Native guard decision changed')
    query=report['query'];require(query['called'] and query['input']==query['output']==trace['s2s']['output'],'Passive query moved pose')
    require(query['stop']=='no_iterations' and len(query['iterations'])==1,'Observer took solver step')
    it=query['iterations'][0]
    require(not it['step_attempted'] and not it['update_applied'],'Observer updated pose')
    require(it['class_pairs']==counts and it['pairs']==sum(counts)==query['returned_pairs'],'Observer class count differs')
    require(np.allclose(H,np.asarray(it['H']).reshape(6,6),rtol=1e-10,atol=1e-10),'Observer H reconstruction failed')
    require(np.allclose(g,np.asarray(it['g']),rtol=1e-10,atol=1e-10),'Observer gradient reconstruction failed')
    require(close(weighted,it['weighted_squared_residual']),'Observer IRLS objective differs')
    require(c['applied']==allowed,'Native acceptance ignored guard')
    require(trace['accepted_relative']==(c['refinement']['output'] if allowed else trace['s2s']['output']),'Wrong guarded output')
    return dict(queried=True,allowed=allowed,missing_field=missing,primitives=len(primitives),original_cost=sum(original[c] for c in (0,1,3)),candidate_cost=sum(candidate[c] for c in (0,1,3)))

def metrics(T,truth):
    length=float(np.linalg.norm(truth[:3,3]));require(length>=.1,'Unexpected nonmoving inherited case')
    ratio=float(np.linalg.norm(T[:3,3])/length);vector=float(np.linalg.norm(T[:3,3]-truth[:3,3])/length)
    return dict(length_ratio=ratio,length_error=abs(ratio-1),vector_error=vector,bad_length=abs(ratio-1)>.2,bad_vector=vector>.2)

def evaluate(group,native_path,primitive_path):
    rows=list(map(json.loads,Path(native_path).read_text().splitlines()));primitive_rows=list(map(json.loads,Path(primitive_path).read_text().splitlines()))
    previous=PARENT/'groups'/group;old=list(map(json.loads,decode(previous,'native.stdout.jsonl').splitlines()))
    expected={(r['frame'],r['mode']):r['trace'] for r in old if r['type']=='terminal' and r['mode']<2}
    prior_costs={(r['frame'],r['cluster']):r['costs_m2'] for r in old if r['type']=='profile'}
    costs={(r['frame'],r['cluster']):r['costs_m2'] for r in rows if r['type']=='profile'}
    require(costs==prior_costs,'Native cluster profiles changed')
    frames=read(previous/'input_sources.json')['frames'];truths={int(k):matrix(v) for k,v in read(previous/'offline_truth.json').items()}
    terminals={(r['frame'],r['mode']):r for r in rows if r['type']=='terminal'};require(len(terminals)==3*len(frames),'Missing native arm')
    primitive_by={f:[] for f in frames}
    for row in primitive_rows:primitive_by[row['frame']].append(row)
    cases=[];checks=[]
    no_worse=lambda a,b:a<=b or close(a,b)
    for f in frames:
        current={m:terminals[f,m] for m in range(3)}
        for mode in (0,1):require(current[mode]['trace']==expected[f,mode],f'Exact original/old control differs: {group}/{f}/{mode}')
        guard=current[2]['trace'];joint=current[0]['trace'];oldc=current[1]['trace'];require(guard['s2s']==joint['s2s'],'Original solver changed by guard')
        require(guard['features']==joint['features'],'Feature counts changed')
        a=guard.get('s2s_consensus');b=oldc.get('s2s_consensus')
        if a or b:
            aa=dict(a);bb=dict(b);aa.pop('applied');bb.pop('applied');require(aa==bb,'Proposal solver/votes/masks changed by guard')
        support=check_support(current[2],primitive_by[f])
        measured={str(m):metrics(matrix(current[m]['trace']['accepted_relative']),truths[f]) for m in range(3)}
        base,oldmetric,new=measured['0'],measured['1'],measured['2']
        checks.append(dict(frame=f,criterion='no higher length/vector error than original joint',passed=no_worse(new['length_error'],base['length_error']) and no_worse(new['vector_error'],base['vector_error'])))
        checks.append(dict(frame=f,criterion='no newly bad length/vector labels relative joint',passed=(not new['bad_length'] or base['bad_length']) and (not new['bad_vector'] or base['bad_vector'])))
        if group=='old':
            checks.append(dict(frame=f,criterion='old rescue/fallback no worse than old consensus',passed=no_worse(new['length_error'],oldmetric['length_error']) and no_worse(new['vector_error'],oldmetric['vector_error'])))
            if not oldc['s2s_consensus']['usable']:require(guard['accepted_relative']==joint['accepted_relative'],'Fallback drift')
        cases.append(dict(frame=f,metrics=measured,support=support,old_applied=bool(b and b['applied']),guard_applied=bool(a and a['applied'])))
    return dict(group=group,cases=cases,gate_checks=checks,gate_passed=all(c['passed'] for c in checks),native_calls=len(terminals),profiles=len(costs),all_original_old_and_proposal_controls_exact=True,all_native_primitive_geometry_loss_equations_and_decisions_reconstructed=True)
