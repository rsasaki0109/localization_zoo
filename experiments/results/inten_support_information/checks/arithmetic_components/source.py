from pathlib import Path
import importlib.util,json
import numpy as np
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_support_information')
S=Path('/tmp/localization-zoo-inten-support-information-20261023')
s=importlib.util.spec_from_file_location('arithmetic_component_archive',E/'verify.py');v=importlib.util.module_from_spec(s);s.loader.exec_module(v)
raw=v.decode(E/'checks/arithmetic_control','arithmetic.stdout.jsonl')
rows=[json.loads(x) for x in raw.splitlines()];native=[r for r in rows if r['type']=='row'];system=next(r for r in rows if r['type']=='system')
stem='old_247_6_0_16_0';folder=E/'checks/equation_diagnosis';record=json.loads((folder/(stem+'.query.json')).read_text());data=np.load(folder/(stem+'.clouds.npz'))
T=np.array(record['pose']).reshape(4,4);p=data['source'][:,:3];q=p[:,None,:]*T[None,:3,:3]
left=(q[:,:,0]+q[:,:,1])+q[:,:,2]+T[:3,3];right=q[:,:,0]+(q[:,:,1]+q[:,:,2])+T[:3,3]
mixed=left.copy();mixed[:,2]=right[:,2];actual=np.array([r['x'] for r in native])
result=dict(coordinates={name:dict(exact=bool(np.array_equal(x,actual)),mismatches_by_axis=np.sum(x!=actual,axis=0).tolist(),maximum_error=float(np.max(np.abs(x-actual)))) for name,x in [('left',left),('right',right),('mixed',mixed)]})
Hleft=np.zeros((6,6));Hright=Hleft.copy();gleft=np.zeros(6);gright=gleft.copy()
for i,row in enumerate(native):
    if not row['valid'] or not record['rows'][i][1]:continue
    x=np.array(row['x']);a=np.array(row['gradient']);r=row['residual'];J=np.r_[a,np.cross(x,a)];w=1. if abs(r)<=.1 else .1/abs(r)
    Hleft+=np.outer(w*J,J);Hright+=w*np.outer(J,J)
    gleft+=(w*J)*r;gright+=J*(w*r)
expected_H=np.array(system['H']).reshape(6,6);expected_g=np.array(system['g'])
result['H']={name:dict(exact=bool(np.array_equal(x,expected_H)),max_error=float(np.max(np.abs(x-expected_H)))) for name,x in [('scaled_vector_first',Hleft),('weight_after_outer',Hright)]}
result['g']={name:dict(exact=bool(np.array_equal(x,expected_g)),max_error=float(np.max(np.abs(x-expected_g)))) for name,x in [('scaled_vector_first',gleft),('combined_scalar_first',gright)]}
(S/'arithmetic_components_result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
