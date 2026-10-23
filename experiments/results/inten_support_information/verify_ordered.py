"""Preserve native fixed-size arithmetic while retaining the original strict checks."""
from pathlib import Path
import hashlib,importlib.util,inspect,json
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
s=importlib.util.spec_from_file_location('support_bound_full_field',E/'verify_full_field.py')
field=importlib.util.module_from_spec(s);s.loader.exec_module(field)
checker=field.checker;inputs=field.inputs
def sum3(x):return (x[...,0]+x[...,1])+x[...,2]
def norm3(x):return np.sqrt(sum3(x*x))
def dot3(a,b):return sum3(a*b)
def transform_points(p,T):
    products=p[:,None,:]*T[None,:3,:3]
    return sum3(products)+T[:3,3]
def reduce_rows(x,a,r,valid):
    H=np.zeros((6,6));g=np.zeros(6);cost=0.;count=0
    for point,gradient,residual in zip(x[valid],a[valid],r[valid]):
        J=np.r_[gradient,np.cross(point,gradient)]
        weight=1. if abs(residual)<=.1 else .1/abs(residual)
        weighted=weight*J
        H+=np.outer(weighted,J);g+=weighted*residual
        cost+=weight*residual*residual;count+=1
    return dict(H=H,g=g,cost=cost,count=count)
def transformed_reconstruct():
    source=inspect.getsource(field.original_reconstruct)
    source=source.replace('def reconstruct(','def ordered_reconstruct(',1)
    for old,new in (
      ('x=source[:,:3]@pose[:3,:3].T+pose[:3,3]','x=transform_points(source[:,:3],pose)'),
      ('np.linalg.norm(direction)','norm3(direction)'),('np.linalg.norm(perp)','norm3(perp)'),
      ('np.linalg.norm(normal)','norm3(normal)'),('np.dot(direction,delta)','dot3(direction,delta)'),
      ('np.dot(normal,x[i]-q[0,:3])','dot3(normal,x[i]-q[0,:3])')):
        assert source.count(old)==1,('Unexpected frozen arithmetic source',old)
        source=source.replace(old,new)
    return source
ordered_source=transformed_reconstruct()
namespace=dict(field.original_reconstruct.__globals__,sum3=sum3,norm3=norm3,dot3=dot3,
    transform_points=transform_points,reduce_rows=reduce_rows)
exec(compile(ordered_source,'support_ordered_reconstruction','exec'),namespace)
field.original_reconstruct=namespace['ordered_reconstruct']
def validate():
    recovery=json.loads((E/'arithmetic_recovery_protocol.json').read_text())
    for name,digest in recovery['source_sha256'].items():
        assert hashlib.sha256((W/name).read_bytes()).hexdigest()==digest,('Ordered arithmetic source changed',name)
    assert hashlib.sha256(ordered_source.encode()).hexdigest()==recovery['transformed_function_sha256']
    controls=json.loads((E/'checks/arithmetic_control/result.json').read_text())
    assert controls['native_class_system_reconstructed'] and controls['max_H_error']==0.
    assert controls['controls'][0]['eigen']==controls['controls'][0]['left']!=controls['controls'][0]['right']
    assert controls['controls'][1]['norm2']==controls['controls'][1]['left_norm2']!=controls['controls'][1]['right_norm2']
    return field.validate()
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();result=validate()
    a.output.write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k!='groups'}))
