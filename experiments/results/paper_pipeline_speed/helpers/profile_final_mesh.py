from pathlib import Path
import hashlib,json,shutil,subprocess,time,traceback
root=Path('/workspace/localization-env/paper-speed')
while not (root/'final_source_ready.json').exists():time.sleep(5)
tag=json.loads((root/'final_source_ready.json').read_text())['tag']
while not (root/'final_comparison_ready.json').exists():time.sleep(5)
assert json.loads((root/'final_comparison_ready.json').read_text())['exit_code']==0
source=root/(tag+'_worktree');meta=json.loads((root/(tag+'_binary.json')).read_text())
p=json.loads((root/'protocol.json').read_text());docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
assert hashlib.sha256(Path(meta['binary_path']).read_bytes()).hexdigest()==meta['binary_sha256']
try:
 for method in ['mesh_loam']:
  label=f'{tag}_profile_accelerated_{method}';out=root/'runs'/label;out.mkdir(parents=True,exist_ok=False)
  cwd=source/'dogfooding_results/paper_speed'/label;cwd.mkdir(parents=True,exist_ok=False)
  cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-e','CPUPROFILE='+str(out/'cpu.prof'),'-e','CPUPROFILE_FREQUENCY=100','-e','LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so.0','-w',str(cwd),'localization-zoo-dev',meta['binary_path'],str(root/'kitti/dataset/sequences/07/velodyne'),str(source/'experiments/reference_data/kitti_seq_07_full_gt.csv'),'120','--methods',method,'--summary-json',str(out/'summary.json'),*p['method_configs'][method]['args'],'--paper-pipeline-accelerated']
  started=time.time()
  with (out/'run.log').open('w') as log:
   child=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT);(root/'final_profile_active.json').write_text(json.dumps({'label':label,'pid':child.pid,'started_unix':started},indent=2)+'\n');code=child.wait()
  receipt={'label':label,'command':cmd,'source_commit':meta['source_commit'],'binary_sha256':meta['binary_sha256'],'profiler_enabled':True,'started_unix':started,'finished_unix':time.time(),'exit_code':code}
  (out/'command.json').write_text(json.dumps(receipt,indent=2)+'\n');assert code==0
  summary=json.loads((out/'summary.json').read_text());assert summary['evaluator_git_commit']==meta['source_commit'] and summary['methods'][0]['frames']==120
  name={'mesh_loam':'Mesh_LOAM'}[method];shutil.copy2(cwd/'dogfooding_results'/(name+'.txt'),out/'poses.txt')
  report=docker+['exec','--user','1000:1000','localization-zoo-dev','google-pprof','--text',meta['binary_path'],str(out/'cpu.prof')]
  with (out/'cpu_profile.txt').open('w') as log:result=subprocess.run(report,stdout=log,stderr=subprocess.STDOUT)
  (out/'profile_command.json').write_text(json.dumps({'command':report,'exit_code':result.returncode},indent=2)+'\n');assert result.returncode==0
 (root/(tag+'_final_mesh_profile_exit.json')).write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
except BaseException as err:
 (root/(tag+'_final_mesh_profile_exit.json')).write_text(json.dumps({'exit_code':1,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
