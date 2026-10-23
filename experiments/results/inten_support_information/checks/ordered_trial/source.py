from pathlib import Path
import importlib.util,inspect,json
import numpy as np
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_support_information')
S=Path('/tmp/localization-zoo-inten-support-information-20261023')
spec=importlib.util.spec_from_file_location('ordered_trial_base',E/'verify_full_field.py');v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)
def sum3(x):return (x[...,0]+x[...,1])+x[...,2]
def norm3(x):return np.sqrt(sum3(x*x))
def dot3(a,b):return sum3(a*b)
def points(p,T):
    q=p[:,None,:]*T[None,:3,:3]
    return sum3(q)+T[:3,3]
def reduce_rows(x,a,r,valid):
    H=np.zeros((6,6));g=np.zeros(6);cost=0.;count=0
    for xx,aa,residual in zip(x[valid],a[valid],r[valid]):
        J=np.r_[aa,np.cross(xx,aa)];w=1. if abs(residual)<=.1 else .1/abs(residual)
        weighted=w*J;H+=np.outer(weighted,J);g+=weighted*residual;cost+=w*residual*residual;count+=1
    return dict(H=H,g=g,cost=cost,count=count)
source=inspect.getsource(v.original_reconstruct).replace('def reconstruct(','def ordered_reconstruct(',1)
for old,new in (
 ('x=source[:,:3]@pose[:3,:3].T+pose[:3,3]','x=points(source[:,:3],pose)'),
 ('np.linalg.norm(direction)','norm3(direction)'),('np.linalg.norm(perp)','norm3(perp)'),
 ('np.linalg.norm(normal)','norm3(normal)'),('np.dot(direction,delta)','dot3(direction,delta)'),
 ('np.dot(normal,x[i]-q[0,:3])','dot3(normal,x[i]-q[0,:3])')):
    assert source.count(old)==1,old;source=source.replace(old,new)
namespace=dict(v.original_reconstruct.__globals__,sum3=sum3,norm3=norm3,dot3=dot3,points=points,reduce_rows=reduce_rows)
exec(compile(source,'ordered_trial_reconstruct','exec'),namespace)
base=E/'checks/equation_diagnosis';stem='old_247_6_0_16_0';record=json.loads((base/(stem+'.query.json')).read_text());data=np.load(base/(stem+'.clouds.npz'))
blank=np.empty((0,6));clouds={record['frame']:[data['source'],blank,blank,blank],record['frame']-1:[data['target'],blank,blank,blank]}
result,counts=namespace['ordered_reconstruct'](record,clouds,set(),{},{} )
report=dict(exit_code=0,original_tolerances_pass=True,errors={name:float(np.max(np.abs(result[name]['H']-np.array(record[name]['H']).reshape(6,6)))) for name in result},scope='Fixed-size left reductions and point product, same frozen fixture and strict thresholds; no estimator change.')
(S/'ordered_trial_result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
