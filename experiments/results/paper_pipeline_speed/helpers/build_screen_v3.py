from pathlib import Path
import hashlib,json,shutil,subprocess,time,traceback
import numpy as np
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');tag='fe03ed88'
source=subprocess.check_output(['git','rev-parse',tag],cwd=repo,text=True).strip()
p=json.loads((r/'protocol.json').read_text());base=json.loads((r/'baseline_binary.json').read_text())
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
checks=r/'checks'/tag;checks.mkdir(parents=True,exist_ok=False);records=[];screen={}
def checked(label,cmd):
 started=time.time()
 with (checks/(label+'.log')).open('w') as f:
  child=subprocess.Popen(cmd,cwd=repo,stdout=f,stderr=subprocess.STDOUT);(r/'v3_active.json').write_text(json.dumps({'label':label,'pid':child.pid,'started_unix':started},indent=2)+'\n');code=child.wait()
 receipt={'label':label,'command':cmd,'cwd':str(repo),'started_unix':started,'finished_unix':time.time(),'exit_code':code}
 records.append(receipt);(checks/'commands.json').write_text(json.dumps(records,indent=2)+'\n')
 if code:raise SystemExit(code)
try:
 while not (r/'fe607199_profiles_exit.json').exists():time.sleep(5)
 assert json.loads((r/'fe607199_profiles_exit.json').read_text())['exit_code']==0
 subprocess.run(['git','diff','--exit-code',source,'--','papers','evaluation/src','CMakeLists.txt','evaluation/CMakeLists.txt'],cwd=repo,check=True)
 checked('configure',docker+['exec','-w',str(repo),'localization-zoo-dev','cmake','-S','.','-B','build','-DCMAKE_BUILD_TYPE=Release'])
 checked('build',docker+['exec','-w',str(repo),'localization-zoo-dev','cmake','--build','build','-j2'])
 checked('ctest',docker+['exec','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-w',str(repo),'localization-zoo-dev','ctest','--test-dir','build','--output-on-failure'])
 binary=r/(tag+'_pcd_dogfooding');shutil.copy2(repo/'build/evaluation/pcd_dogfooding',binary)
 meta={'source_commit':source,'binary_path':str(binary),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'source_verified_against_commit':True}
 (r/(tag+'_binary.json')).write_text(json.dumps(meta,indent=2)+'\n');tree=r/(tag+'_worktree')
 checked('worktree',['git','worktree','add','--detach',str(tree),source])
 for method,cfg in p['method_configs'].items():
  for mode,m,t in [('control',base,r/'baseline_worktree'),('default',meta,tree),('accelerated',meta,tree)]:
   label=f'{tag}_{mode}_short_{method}';out=r/'runs'/label;out.mkdir(parents=True,exist_ok=False)
   cwd=t/'dogfooding_results/paper_speed'/label;cwd.mkdir(parents=True,exist_ok=False)
   cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-w',str(cwd),'localization-zoo-dev',m['binary_path'],str(r/'kitti/dataset/sequences/07/velodyne'),str(t/'experiments/reference_data/kitti_seq_07_full_gt.csv'),'120','--methods',method,'--summary-json',str(out/'summary.json'),*cfg['args']]
   if mode=='accelerated':cmd+=['--paper-pipeline-accelerated']
   started=time.time()
   with (out/'run.log').open('w') as f:
    child=subprocess.Popen(cmd,stdout=f,stderr=subprocess.STDOUT);(r/'v3_active.json').write_text(json.dumps({'label':label,'pid':child.pid,'started_unix':started},indent=2)+'\n');code=child.wait()
   receipt={'label':label,'command':cmd,'source_commit':m['source_commit'],'binary_sha256':m['binary_sha256'],'profiler_enabled':False,'started_unix':started,'finished_unix':time.time(),'exit_code':code}
   (out/'command.json').write_text(json.dumps(receipt,indent=2)+'\n')
   if code:raise SystemExit(code)
   summary=json.loads((out/'summary.json').read_text());assert summary['evaluator_git_commit']==m['source_commit'] and summary['methods'][0]['frames']==120 and summary['methods'][0]['status']=='ok'
   name={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method];shutil.copy2(cwd/'dogfooding_results'/(name+'.txt'),out/'poses.txt')
  rows={mode:r/'runs'/f'{tag}_{mode}_short_{method}' for mode in ['control','default','accelerated']}
  data={mode:json.loads((d/'summary.json').read_text())['methods'][0] for mode,d in rows.items()};poses={mode:np.loadtxt(d/'poses.txt') for mode,d in rows.items()}
  assert all(v.shape==(120,12) for v in poses.values())
  screen[method]={'control_time_ms':data['control']['time_ms'],'default_time_ms':data['default']['time_ms'],'accelerated_time_ms':data['accelerated']['time_ms'],'short_speedup':data['control']['time_ms']/data['accelerated']['time_ms'],'default_pose_max_abs_difference':float(np.max(np.abs(poses['control']-poses['default']))),'accelerated_pose_max_abs_difference':float(np.max(np.abs(poses['control']-poses['accelerated'])))}
  (r/(tag+'_screening_partial.json')).write_text(json.dumps(screen,indent=2)+'\n')
 (r/(tag+'_screening.json')).write_text(json.dumps(screen,indent=2)+'\n');(r/(tag+'_screening_exit.json')).write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
 (r/'v3_active.json').write_text(json.dumps({'label':'screening_complete','tag':tag},indent=2)+'\n')
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (r/(tag+'_candidate_exit.json')).write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
