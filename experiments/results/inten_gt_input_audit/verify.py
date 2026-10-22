"""Offline official-input regeneration; no estimator or truth adjustment."""
from pathlib import Path
import hashlib,io,json,subprocess,sys,tempfile,zipfile
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
read=lambda p:json.loads(p.read_text())
def compare(actual,expected):
    a=np.loadtxt(io.StringIO(actual.decode()),delimiter=',',skiprows=1,ndmin=2)
    b=np.loadtxt(io.StringIO(expected.decode()),delimiter=',',skiprows=1,ndmin=2)
    if a.shape!=b.shape or a.shape[1]!=7 or not np.isfinite(a).all() or not np.isfinite(b).all():
        raise ValueError('GT rows/fields are incomplete or nonfinite')
    if not np.array_equal(a[:,0],b[:,0]) or not np.array_equal(a[:,0],np.arange(len(a))):
        raise ValueError('GT frame IDs changed')
    # One printed decimal quantum plus eight floating-point ULPs. This is
    # serialization precision, not a motion-error gate or sensor correction.
    bound=1e-10+8*np.spacing(np.maximum(np.maximum(np.abs(a[:,1:]),np.abs(b[:,1:])),1.))
    difference=np.abs(a[:,1:]-b[:,1:])
    if np.any(difference>bound):raise ValueError('Numeric GT mismatch')
    return dict(poses=len(a),numeric_exact=bool(np.array_equal(a,b)),max_numeric_difference=float(difference.max()),byte_identical=actual==expected)
def validate():
    manifests=read(E/'download_manifest.json');archives={}
    for record in manifests['archives']:
        path=E/'inputs'/Path(record['path']).name;raw=path.read_bytes()
        if hashlib.sha256(raw).hexdigest()!=record['sha256']:raise ValueError('Official metadata hash changed')
        z=zipfile.ZipFile(io.BytesIO(raw))
        if z.testzip() is not None:raise ValueError('Official metadata CRC failed')
        archives[path.name]=z
    records=[]
    with tempfile.TemporaryDirectory() as name:
        folder=Path(name)
        for i in range(11):
            seq=f'{i:02d}'
            poses=folder/(seq+'.txt');calib=folder/(seq+'_calib.txt');output=folder/(seq+'.csv')
            poses.write_bytes(archives['data_odometry_poses.zip'].read('dataset/poses/'+seq+'.txt'))
            calib.write_bytes(archives['data_odometry_calib.zip'].read('dataset/sequences/'+seq+'/calib.txt'))
            cmd=[sys.executable,str(W/'evaluation/scripts/kitti_poses_to_gt_csv.py'),'--poses-file',str(poses),'--calib-file',str(calib),'--output',str(output)]
            subprocess.run(cmd,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            old=(W/'experiments/reference_data'/('kitti_seq_'+seq+'_full_gt.csv')).read_bytes()
            records.append(dict(sequence=seq,converter_exit_code=0,**compare(old,output.read_bytes())))
    return dict(exit_code=0,sequences=records,source='Official pose/calibration archives, unchanged converter with Tr applied on the right',
                no_gt_correction=True,no_estimator_execution=True,no_sensor_scans=True)
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    result=validate();a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
