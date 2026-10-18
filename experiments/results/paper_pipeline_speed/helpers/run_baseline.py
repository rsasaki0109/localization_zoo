from pathlib import Path
import hashlib,json,subprocess,time
root=Path('/workspace/localization-env/paper-speed')
source=root/'baseline_worktree'
p=json.loads((root/'protocol.json').read_text())
base=json.loads((root/'baseline_binary.json').read_text())
assert hashlib.sha256(Path(base['binary_path']).read_bytes()).hexdigest()==base['binary_sha256']
assert len(list((root/'kitti/dataset/sequences/07/velodyne').glob('*.bin')))==p['accuracy_window_frames']
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
results=[]
for stage,frames in [('profile',p['profile_window_frames']),('full',p['accuracy_window_frames'])]:
 for method,cfg in p['method_configs'].items():
  label=f'baseline_{stage}_{method}'
  out=root/'runs'/label;out.mkdir(parents=True,exist_ok=False)
  cwd=source/'dogfooding_results/paper_speed'/label;cwd.mkdir(parents=True,exist_ok=False)
  cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-w',str(cwd)]
  if stage=='profile':cmd+=['-e','CPUPROFILE='+str(out/'cpu.prof'),'-e','CPUPROFILE_FREQUENCY=100','-e','LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so.0']
  cmd+=['localization-zoo-dev',base['binary_path'],str(root/'kitti/dataset/sequences/07/velodyne'),str(source/'experiments/reference_data/kitti_seq_07_full_gt.csv'),str(frames),'--methods',method,'--summary-json',str(out/'summary.json'),*cfg['args']]
  started=time.time()
  (root/'active.json').write_text(json.dumps({'label':label,'command':cmd,'pid':None,'started_unix':started},indent=2)+'\n')
  with (out/'run.log').open('w') as log:
   process=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT)
   (root/'active.json').write_text(json.dumps({'label':label,'command':cmd,'pid':process.pid,'started_unix':started},indent=2)+'\n')
   code=process.wait()
  receipt={'label':label,'command':cmd,'binary_sha256':base['binary_sha256'],'source_commit':base['source_commit'],'started_unix':started,'finished_unix':time.time(),'exit_code':code,'profiler_enabled':stage=='profile'}
  (out/'command.json').write_text(json.dumps(receipt,indent=2)+'\n')
  results.append(receipt);(root/'baseline_commands.json').write_text(json.dumps(results,indent=2)+'\n')
  print(json.dumps(receipt),flush=True)
  if code:raise SystemExit(code)
  summary=json.loads((out/'summary.json').read_text());assert summary['evaluator_git_commit']==base['source_commit'],summary.get('evaluator_git_commit')
  if stage=='profile':
   report=docker+['exec','--user','1000:1000','localization-zoo-dev','google-pprof','--text',base['binary_path'],str(out/'cpu.prof')]
   with (out/'cpu_profile.txt').open('w') as log: result=subprocess.run(report,stdout=log,stderr=subprocess.STDOUT)
   (out/'profile_command.json').write_text(json.dumps({'command':report,'exit_code':result.returncode},indent=2)+'\n')
   if result.returncode:raise SystemExit(result.returncode)
(root/'baseline_exit.json').write_text(json.dumps({'exit_code':0,'completed_unix':time.time()},indent=2)+'\n')
(root/'active.json').write_text(json.dumps({'label':'baseline_complete','finished_unix':time.time()},indent=2)+'\n')
