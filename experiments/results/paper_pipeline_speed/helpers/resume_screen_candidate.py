from pathlib import Path
import hashlib,json,shutil,subprocess,time,traceback
import numpy as np
r=Path('/workspace/localization-env/paper-speed');tag='fe607199'
p=json.loads((r/'protocol.json').read_text());meta=json.loads((r/(tag+'_binary.json')).read_text());base=json.loads((r/'baseline_binary.json').read_text())
assert json.loads((r/'baseline_exit.json').read_text())['exit_code']==0
assert all(x['exit_code']==0 for x in json.loads((r/'checks'/tag/'commands.json').read_text()))
assert hashlib.sha256(Path(meta['binary_path']).read_bytes()).hexdigest()==meta['binary_sha256']
assert hashlib.sha256(Path(base['binary_path']).read_bytes()).hexdigest()==base['binary_sha256']
worktree=r/(tag+'_worktree');active=r/'after_active.json';screening={};reused=[]
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
try:
 for method,cfg in p['method_configs'].items():
  for mode,binary_path,tree,source,sha in [('control',base['binary_path'],r/'baseline_worktree',base['source_commit'],base['binary_sha256']),('default',meta['binary_path'],worktree,meta['source_commit'],meta['binary_sha256']),('accelerated',meta['binary_path'],worktree,meta['source_commit'],meta['binary_sha256'])]:
   label=f'{tag}_{mode}_short_{method}';out=r/'runs'/label;out.mkdir(parents=True,exist_ok=True)
   cwd=tree/'dogfooding_results/paper_speed'/label;cwd.mkdir(parents=True,exist_ok=True)
   cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-w',str(cwd),'localization-zoo-dev',binary_path,str(r/'kitti/dataset/sequences/07/velodyne'),str(tree/'experiments/reference_data/kitti_seq_07_full_gt.csv'),'120','--methods',method,'--summary-json',str(out/'summary.json'),*cfg['args']]
   if mode=='accelerated':cmd+=['--paper-pipeline-accelerated']
   if (out/'command.json').exists():
    receipt=json.loads((out/'command.json').read_text());assert receipt['exit_code']==0 and receipt['command']==cmd and receipt['source_commit']==source and receipt['binary_sha256']==sha
    reused.append(label);print('Reused completed run '+label,flush=True)
   else:
    assert not (out/'run.log').exists(),'Investigate unreceipted attempt before overwriting'
    started=time.time()
    with (out/'run.log').open('w') as log:
     child=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT)
     active.write_text(json.dumps({'label':label,'tag':tag,'pid':child.pid,'started_unix':started},indent=2)+'\n');code=child.wait()
    receipt={'label':label,'command':cmd,'source_commit':source,'binary_sha256':sha,'started_unix':started,'finished_unix':time.time(),'exit_code':code,'profiler_enabled':False}
    (out/'command.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt),flush=True)
    if code:raise SystemExit(code)
   summary=json.loads((out/'summary.json').read_text());assert summary['evaluator_git_commit']==source
   assert summary['methods'][0]['frames']==120 and summary['methods'][0]['status']=='ok'
   name={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method]
   artifact=cwd/'dogfooding_results'/(name+'.txt')
   if (out/'poses.txt').exists():assert (out/'poses.txt').read_bytes()==artifact.read_bytes()
   else:shutil.copy2(artifact,out/'poses.txt')
  dirs={mode:r/'runs'/f'{tag}_{mode}_short_{method}' for mode in ['control','default','accelerated']}
  data={mode:json.loads((d/'summary.json').read_text())['methods'][0] for mode,d in dirs.items()}
  poses={mode:np.loadtxt(d/'poses.txt') for mode,d in dirs.items()}
  assert all(v.shape==(120,12) for v in poses.values())
  screening[method]={'control_time_ms':data['control']['time_ms'],'default_time_ms':data['default']['time_ms'],'accelerated_time_ms':data['accelerated']['time_ms'],'short_speedup':data['control']['time_ms']/data['accelerated']['time_ms'],'default_pose_max_abs_difference':float(np.max(np.abs(poses['control']-poses['default']))),'accelerated_pose_max_abs_difference':float(np.max(np.abs(poses['control']-poses['accelerated'])))}
  (r/(tag+'_screening_partial.json')).write_text(json.dumps(screening,indent=2)+'\n');print(json.dumps({method:screening[method]}),flush=True)
 (r/(tag+'_screening.json')).write_text(json.dumps(screening,indent=2)+'\n')
 (r/(tag+'_screening_reused.json')).write_text(json.dumps({'reused_runs':reused,'reason':'Previous driver stdout broke after the exec-server transport disconnected; saved CLI exit0 and source/binary/command hashes verified before reuse.'},indent=2)+'\n')
 (r/(tag+'_screening_exit.json')).write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
 active.write_text(json.dumps({'label':'screening_complete','tag':tag,'finished_unix':time.time()},indent=2)+'\n')
except BaseException as error:
 code=error.code if isinstance(error,SystemExit) and isinstance(error.code,int) else 1
 (r/(tag+'_resume_exit.json')).write_text(json.dumps({'exit_code':code,'error':str(error),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
