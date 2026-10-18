from pathlib import Path
import hashlib,json,shutil,subprocess,sys,time,traceback
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo')
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
p=json.loads((r/'protocol.json').read_text())
records=[];tag='waiting';active=r/'after_active.json'
def run(label,cmd,cwd=repo):
 out=r/'checks'/tag;out.mkdir(parents=True,exist_ok=True)
 started=time.time();log=out/(label+'.log')
 with log.open('w') as f:
  child=subprocess.Popen(cmd,cwd=cwd,stdout=f,stderr=subprocess.STDOUT)
  active.write_text(json.dumps({'label':label,'tag':tag,'pid':child.pid,'started_unix':started},indent=2)+'\n')
  code=child.wait()
 result={'label':label,'command':cmd,'cwd':str(cwd),'started_unix':started,'finished_unix':time.time(),'exit_code':code,'log':str(log)}
 records.append(result);(out/'commands.json').write_text(json.dumps(records,indent=2)+'\n')
 print(json.dumps(result),flush=True)
 if code:raise SystemExit(code)
 return result
try:
 active.write_text(json.dumps({'label':'waiting_for_all_full_baselines','started_unix':time.time()},indent=2)+'\n')
 print('Waiting for successful baseline_exit.json; no builds or other benchmarks run during baseline timing.',flush=True)
 while not (r/'baseline_exit.json').exists():
  cmds=r/'baseline_commands.json'
  if cmds.exists() and any(c['exit_code'] for c in json.loads(cmds.read_text())):raise RuntimeError('Baseline failed; inspect its saved command/log before retrying')
  time.sleep(10)
 assert json.loads((r/'baseline_exit.json').read_text())['exit_code']==0
 ready=json.loads((r/'source_ready.json').read_text());source_commit=ready['source_commit'];tag=source_commit[:8]
 assert subprocess.check_output(['git','branch','--show-current'],cwd=repo,text=True).strip()=='codex/paper-pipeline-speed'
 assert not subprocess.check_output(['git','status','--porcelain'],cwd=repo,text=True).strip(),'Review pending source changes before building'
 subprocess.run(['git','diff','--exit-code',source_commit,'--','papers','evaluation/src','CMakeLists.txt','evaluation/CMakeLists.txt'],cwd=repo,check=True,stdout=subprocess.DEVNULL)
 # Import full baseline evidence before any new numeric documentation.
 e=repo/'experiments/results/paper_pipeline_speed'
 for method in p['method_configs']:
  label=f'baseline_full_{method}';src=r/'runs'/label
  summary=json.loads((src/'summary.json').read_text());m=summary['methods'][0]
  assert summary['evaluator_git_commit']==p['base_commit'] and m['frames']==1101 and m['status']=='ok'
  assert m['kitti_rte_trans_pct'] is not None
  assert json.loads((src/'command.json').read_text())['exit_code']==0
  shutil.copytree(src,e/'runs'/label)
  name={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method]
  shutil.copy2(r/'baseline_worktree/dogfooding_results/paper_speed'/label/'dogfooding_results'/(name+'.txt'),e/'runs'/label/'poses.txt')
 for name in ['baseline_commands.json','baseline_exit.json']:
  shutil.copy2(r/name,e/name)
 shutil.copy2(Path(__file__),e/'helpers'/Path(__file__).name)
 subprocess.run(['git','add','experiments/results/paper_pipeline_speed'],cwd=repo,check=True)
 subprocess.run(['git','commit','-m','Record unprofiled full KITTI 07 baseline timing and accuracy'],cwd=repo,check=True)
 run('configure',docker+['exec','-w',str(repo),'localization-zoo-dev','cmake','-S','.','-B','build','-DCMAKE_BUILD_TYPE=Release'])
 run('build',docker+['exec','-w',str(repo),'localization-zoo-dev','cmake','--build','build','-j2'])
 run('ctest',docker+['exec','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-w',str(repo),'localization-zoo-dev','ctest','--test-dir','build','--output-on-failure'])
 binary=r/(tag+'_pcd_dogfooding');shutil.copy2(repo/'build/evaluation/pcd_dogfooding',binary)
 metadata={'source_commit':source_commit,'binary_path':str(binary),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'source_verified_against_commit':True}
 (r/(tag+'_binary.json')).write_text(json.dumps(metadata,indent=2)+'\n')
 worktree=r/(tag+'_worktree')
 run('worktree', ['git','worktree','add','--detach',str(worktree),source_commit])
 base=json.loads((r/'baseline_binary.json').read_text())
 for mode,binary_path,tree in [('control',base['binary_path'],r/'baseline_worktree'),('default',str(binary),worktree),('accelerated',str(binary),worktree)]:
  for method,cfg in p['method_configs'].items():
   label=f'{tag}_{mode}_short_{method}';out=r/'runs'/label;out.mkdir(parents=True,exist_ok=False)
   cwd=tree/'dogfooding_results/paper_speed'/label;cwd.mkdir(parents=True,exist_ok=False)
   cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-w',str(cwd),'localization-zoo-dev',binary_path,str(r/'kitti/dataset/sequences/07/velodyne'),str(tree/'experiments/reference_data/kitti_seq_07_full_gt.csv'),'120','--methods',method,'--summary-json',str(out/'summary.json'),*cfg['args']]
   if mode=='accelerated':cmd+=['--paper-pipeline-accelerated']
   started=time.time()
   with (out/'run.log').open('w') as log:
    child=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT)
    active.write_text(json.dumps({'label':label,'tag':tag,'pid':child.pid,'started_unix':started},indent=2)+'\n');code=child.wait()
   receipt={'label':label,'command':cmd,'source_commit':base['source_commit'] if mode=='control' else source_commit,'binary_sha256':base['binary_sha256'] if mode=='control' else metadata['binary_sha256'],'started_unix':started,'finished_unix':time.time(),'exit_code':code,'profiler_enabled':False}
   (out/'command.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt),flush=True)
   if code:raise SystemExit(code)
   summary=json.loads((out/'summary.json').read_text());assert summary['evaluator_git_commit']==receipt['source_commit']
   assert summary['methods'][0]['frames']==120 and summary['methods'][0]['status']=='ok'
   name={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method]
   shutil.copy2(cwd/'dogfooding_results'/(name+'.txt'),out/'poses.txt')
 import numpy as np
 screening={}
 for method in p['method_configs']:
  rows={mode:r/'runs'/f'{tag}_{mode}_short_{method}' for mode in ['control','default','accelerated']}
  data={mode:json.loads((path/'summary.json').read_text())['methods'][0] for mode,path in rows.items()}
  poses={mode:np.loadtxt(path/'poses.txt') for mode,path in rows.items()}
  screening[method]={'control_time_ms':data['control']['time_ms'],'default_time_ms':data['default']['time_ms'],'accelerated_time_ms':data['accelerated']['time_ms'],'short_speedup':data['control']['time_ms']/data['accelerated']['time_ms'],'default_pose_max_abs_difference':float(np.max(np.abs(poses['control']-poses['default']))),'accelerated_pose_max_abs_difference':float(np.max(np.abs(poses['control']-poses['accelerated'])))}
 (r/(tag+'_screening.json')).write_text(json.dumps(screening,indent=2)+'\n')
 active.write_text(json.dumps({'label':'screening_complete','tag':tag,'finished_unix':time.time()},indent=2)+'\n')
 (r/(tag+'_screening_exit.json')).write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
 print(json.dumps(screening,indent=2),flush=True)
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (r/(tag+'_candidate_exit.json')).write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n')
 traceback.print_exc();raise
