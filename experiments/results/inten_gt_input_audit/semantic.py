from pathlib import Path
import csv,json,math,sys
import numpy as np
R=Path(__file__).resolve().parent;W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree')
sys.path.insert(0,str(W/'experiments/results/inten_dense_continuation'))
from common import write,sha
def poses(path):
    rows=np.loadtxt(path,delimiter=',',skiprows=1);out=[]
    for _,x,y,z,roll,pitch,yaw in rows:
        sr,cr=math.sin(roll),math.cos(roll);sp,cp=math.sin(pitch),math.cos(pitch);sy,cy=math.sin(yaw),math.cos(yaw)
        T=np.eye(4);T[:3,:3]=[[cy*cp,cy*sp*sr-sy*cr,cy*sp*cr+sy*sr],[sy*cp,sy*sp*sr+cy*cr,sy*sp*cr-cy*sr],[-sp,cp*sr,cp*cr]];T[:3,3]=[x,y,z];out.append(T)
    return rows,np.asarray(out)
reports=[]
for i in range(11):
    seq=f'{i:02d}';old, A=poses(W/'experiments/reference_data'/f'kitti_seq_{seq}_full_gt.csv');new,B=poses(R/f'regenerated_{seq}.csv')
    assert A.shape==B.shape
    reports.append(dict(sequence=seq,poses=len(A),max_csv_field_difference=float(np.max(np.abs(old-new))),max_translation_difference_m=float(np.max(np.abs(A[:,:3,3]-B[:,:3,3]))),max_rotation_matrix_difference=float(np.max(np.abs(A[:,:3,:3]-B[:,:3,:3]))),max_position_row=int(np.argmax(np.max(np.abs(A[:,:3,3]-B[:,:3,3]),axis=1)))))
_,G=poses(R/'regenerated_08.csv')
rows=[]
for f in range(38,47):
    T=np.linalg.inv(G[f-1])@G[f]
    rows.append(dict(frame=f,translation_xyz_m=T[:3,3].tolist(),length_m=float(np.linalg.norm(T[:3,3]))))
write(R/'semantic_report.json',dict(exit_code=0,sequences=reports,official_08_relative=rows))
print(json.dumps(reports,indent=2))
