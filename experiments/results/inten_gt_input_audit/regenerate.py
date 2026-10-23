from pathlib import Path
import sys,zipfile,concurrent.futures
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree')
sys.path.insert(0,str(W/'experiments/results/inten_dense_continuation'))
from common import run,sha,write,PY
R=Path(__file__).resolve().parent
def one(number):
    seq=f'{number:02d}';inputs=R/'extracted'/seq;inputs.mkdir(parents=True)
    with zipfile.ZipFile(R/'official/data_odometry_poses.zip') as z:
        inputs.joinpath('poses.txt').write_bytes(z.read('dataset/poses/'+seq+'.txt'))
    with zipfile.ZipFile(R/'official/data_odometry_calib.zip') as z:
        inputs.joinpath('calib.txt').write_bytes(z.read('dataset/sequences/'+seq+'/calib.txt'))
        inputs.joinpath('times.txt').write_bytes(z.read('dataset/sequences/'+seq+'/times.txt'))
    output=R/('regenerated_'+seq+'.csv')
    command=[PY,str(W/'evaluation/scripts/kitti_poses_to_gt_csv.py'),'--poses-file',str(inputs/'poses.txt'),'--calib-file',str(inputs/'calib.txt'),'--output',str(output)]
    code=run('convert_'+seq,command,R/'convert_commands')
    assert code==0
    old=W/'experiments/reference_data'/('kitti_seq_'+seq+'_full_gt.csv')
    return dict(sequence=seq,exit_code=code,official_poses_sha256=sha(inputs/'poses.txt'),official_calib_sha256=sha(inputs/'calib.txt'),committed_csv_sha256=sha(old),regenerated_csv_sha256=sha(output),byte_identical=old.read_bytes()==output.read_bytes())
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    records=list(pool.map(one,range(11)))
write(R/'regeneration_report.json',dict(exit_code=0,all_sequences=records,converter_sha256=sha(W/'evaluation/scripts/kitti_poses_to_gt_csv.py')))
print([(r['sequence'],r['byte_identical']) for r in records])
