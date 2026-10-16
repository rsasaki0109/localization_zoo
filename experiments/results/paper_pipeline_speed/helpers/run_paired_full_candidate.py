from pathlib import Path
import argparse,hashlib,json,shutil,subprocess,time
ap=argparse.ArgumentParser();ap.add_argument('--tag',required=True);a=ap.parse_args()
r=Path('/workspace/localization-env/paper-speed');tag=a.tag
p=json.loads((r/'protocol.json').read_text());meta=json.loads((r/(tag+'_binary.json')).read_text());base=json.loads((r/'baseline_binary.json').read_text())
assert json.loads((r/(tag+'_screening_exit.json')).read_text())['exit_code']==0
screen=json.loads((r/(tag+'_screening.json')).read_text());assert all(v['default_pose_max_abs_difference']==0 and v['accelerated_pose_max_abs_difference']==0 for v in screen.values())
for m in [meta,base]:assert hashlib.sha256(Path(m['binary_path']).read_bytes()).hexdigest()==m['binary_sha256']
tree=r/(tag+'_worktree');assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=tree,text=True).strip()==meta['source_commit']
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock'];records=[]
def snapshot():
 paths=['/proc/loadavg','/sys/fs/cgroup/cpu.max','/sys/fs/cgroup/cpu.stat','/sys/fs/cgroup/memory.current','/sys/fs/cgroup/memory.max']
 return {s:Path(s).read_text().strip() for s in paths if Path(s).exists()}
try:
 for method,cfg in p['method_configs'].items():
  for mode,m,t in [('control',base,r/'baseline_worktree'),('accelerated',meta,tree)]:
   label=f'{tag}_{mode}_full_{method}';out=r/'runs'/label;out.mkdir(parents=True,exist_ok=False)
   cwd=t/'dogfooding_results/paper_speed'/label;cwd.mkdir(parents=True,exist_ok=False)
   cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-w',str(cwd),'localization-zoo-dev',m['binary_path'],str(r/'kitti/dataset/sequences/07/velodyne'),str(t/'experiments/reference_data/kitti_seq_07_full_gt.csv'),'1101','--methods',method,'--summary-json',str(out/'summary.json'),*cfg['args']]
   if mode=='accelerated':cmd+=['--paper-pipeline-accelerated']
   before=snapshot();started=time.time()
   with (out/'run.log').open('w') as log:
    child=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT);(r/'final_active.json').write_text(json.dumps({'label':label,'pid':child.pid,'started_unix':started},indent=2)+'\n');code=child.wait()
   receipt={'label':label,'command':cmd,'source_commit':m['source_commit'],'binary_sha256':m['binary_sha256'],'profiler_enabled':False,'pairing':'full frozen control immediately followed by accelerated run for each method','started_unix':started,'finished_unix':time.time(),'exit_code':code,'host_before':before,'host_after':snapshot()}
   (out/'command.json').write_text(json.dumps(receipt,indent=2)+'\n');records.append(receipt);(r/(tag+'_full_commands.json')).write_text(json.dumps(records,indent=2)+'\n')
   if code:raise SystemExit(code)
   summary=json.loads((out/'summary.json').read_text());row=summary['methods'][0];assert summary['evaluator_git_commit']==m['source_commit'] and row['frames']==1101 and row['status']=='ok' and row['kitti_rte_trans_pct'] is not None
   name={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method];shutil.copy2(cwd/'dogfooding_results'/(name+'.txt'),out/'poses.txt')
 (r/(tag+'_full_exit.json')).write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n');(r/'final_active.json').write_text(json.dumps({'label':'full_candidate_complete','tag':tag},indent=2)+'\n')
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (r/(tag+'_full_exit.json')).write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');raise
