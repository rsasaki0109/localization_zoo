from pathlib import Path
import argparse,hashlib,json,shutil,subprocess,time
ap=argparse.ArgumentParser();ap.add_argument('--tag',required=True);a=ap.parse_args()
r=Path('/workspace/localization-env/paper-speed');tag=a.tag
p=json.loads((r/'protocol.json').read_text());meta=json.loads((r/(tag+'_binary.json')).read_text())
assert json.loads((r/(tag+'_screening_exit.json')).read_text())['exit_code']==0
screen=json.loads((r/(tag+'_screening.json')).read_text())
assert all(x['default_pose_max_abs_difference']<=1e-5 and x['accelerated_pose_max_abs_difference']<=1e-5 for x in screen.values()),'Inspect screening drift before full run'
assert hashlib.sha256(Path(meta['binary_path']).read_bytes()).hexdigest()==meta['binary_sha256']
worktree=r/(tag+'_worktree')
assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=worktree,text=True).strip()==meta['source_commit']
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
records=[]
try:
 for method,cfg in p['method_configs'].items():
  label=f'{tag}_accelerated_full_{method}';out=r/'runs'/label;out.mkdir(parents=True,exist_ok=False)
  cwd=worktree/'dogfooding_results/paper_speed'/label;cwd.mkdir(parents=True,exist_ok=False)
  cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-w',str(cwd),'localization-zoo-dev',meta['binary_path'],str(r/'kitti/dataset/sequences/07/velodyne'),str(worktree/'experiments/reference_data/kitti_seq_07_full_gt.csv'),'1101','--methods',method,'--summary-json',str(out/'summary.json'),*cfg['args'],'--paper-pipeline-accelerated']
  started=time.time()
  with (out/'run.log').open('w') as log:
   process=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT)
   (r/'final_active.json').write_text(json.dumps({'label':label,'pid':process.pid,'started_unix':started},indent=2)+'\n');code=process.wait()
  receipt={'label':label,'command':cmd,'source_commit':meta['source_commit'],'binary_sha256':meta['binary_sha256'],'profiler_enabled':False,'started_unix':started,'finished_unix':time.time(),'exit_code':code}
  (out/'command.json').write_text(json.dumps(receipt,indent=2)+'\n');records.append(receipt);(r/(tag+'_full_commands.json')).write_text(json.dumps(records,indent=2)+'\n')
  print(json.dumps(receipt),flush=True)
  if code:raise SystemExit(code)
  summary=json.loads((out/'summary.json').read_text());row=summary['methods'][0]
  assert summary['evaluator_git_commit']==meta['source_commit'] and row['frames']==1101 and row['status']=='ok' and row['kitti_rte_trans_pct'] is not None
  name={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method]
  shutil.copy2(cwd/'dogfooding_results'/(name+'.txt'),out/'poses.txt')
 (r/(tag+'_full_exit.json')).write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
 (r/'final_active.json').write_text(json.dumps({'label':'full_candidate_complete','tag':tag},indent=2)+'\n')
except BaseException as error:
 code=error.code if isinstance(error,SystemExit) and isinstance(error.code,int) else 1
 (r/(tag+'_full_exit.json')).write_text(json.dumps({'exit_code':code,'error':str(error),'finished_unix':time.time()},indent=2)+'\n');raise
