"""Use the measured fixed3 Eigen packet/tail product and identical ordered sums."""
from pathlib import Path
import hashlib,importlib.util,json
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
s=importlib.util.spec_from_file_location('support_ordered_mixed_base',E/'verify_ordered.py')
base=importlib.util.module_from_spec(s);s.loader.exec_module(base)
checker=base.checker;inputs=base.inputs;sum3=base.sum3
def transform_points(p,T):
    products=p[:,None,:]*T[None,:3,:3]
    result=(products[:,:,0]+products[:,:,1])+products[:,:,2]
    # Eigen's column-major3x3 product uses a two-double packet for rows0/1
    # and a scalar, recursively unrolled dot for the remaining row2.
    result[:,2]=products[:,2,0]+(products[:,2,1]+products[:,2,2])
    return result+T[:3,3]
def accumulate(terms,shape):
    initial=np.zeros((1,)+shape,dtype=np.float64)
    return np.add.accumulate(np.concatenate((initial,terms),axis=0),axis=0)[-1]
def reduce_rows(x,a,r,valid):
    x=x[valid];a=a[valid];r=r[valid]
    J=np.column_stack((a,np.cross(x,a)))
    weights=np.ones(len(r));large=np.abs(r)>.1;weights[large]=.1/np.abs(r[large])
    weighted=weights[:,None]*J
    H=accumulate(weighted[:,:,None]*J[:,None,:],(6,6))
    g=accumulate(weighted*r[:,None],(6,))
    cost=accumulate((weights*r)*r,())
    return dict(H=H,g=g,cost=float(cost),count=len(r))
def check_reduction_identity():
    rng=np.random.default_rng(20261023)
    for n in (0,1,17,129):
        x=rng.standard_normal((n,3))*1e4;a=rng.standard_normal((n,3))*1e-4;r=rng.standard_normal(n)
        r[::3]=0.
        for valid in (np.ones(n,bool),np.arange(n)%2==0,np.zeros(n,bool)):
            expected=base.reduce_rows(x,a,r,valid);actual=reduce_rows(x,a,r,valid)
            assert actual['count']==expected['count']
            for key in ('H','g','cost'):
                assert np.asarray(actual[key],dtype=np.float64).tobytes()==np.asarray(expected[key],dtype=np.float64).tobytes(),('Ordered reduction changed',n,key)
base.namespace['transform_points']=transform_points
base.namespace['reduce_rows']=reduce_rows
check_reduction_identity()
def validate():
    recovery=json.loads((E/'packet_tail_recovery_protocol.json').read_text())
    for name,digest in recovery['source_sha256'].items():
        assert hashlib.sha256((W/name).read_bytes()).hexdigest()==digest,('Packet/tail arithmetic source changed',name)
    controls=json.loads((E/'checks/arithmetic_components/result.json').read_text())
    assert controls['coordinates']['mixed']['exact'] and controls['coordinates']['mixed']['maximum_error']==0.
    assert controls['H']['scaled_vector_first']['exact'] and controls['g']['scaled_vector_first']['exact']
    check_reduction_identity()
    return base.validate()
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();result=validate()
    a.output.write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k!='groups'}))
