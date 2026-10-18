from pathlib import Path
import hashlib,json,shutil,subprocess,time,traceback
import numpy as np
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/paper_pipeline_speed';release=json.loads((r/'release_source_ready.json').read_text());source=release['source_commit'];tag=release['tag'];candidate=json.loads((r/'final_source_ready.json').read_text())['tag'];out=r/'release_verification'/tag;out.mkdir(parents=True,exist_ok=False);records=[]
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
def checked(name,cmd):
 started=time.time()
 with (out/(name+'.log')).open('w') as f:result=subprocess.run(cmd,cwd=repo,stdout=f,stderr=subprocess.STDOUT)
 records.append({'label':name,'command':cmd,'cwd':str(repo),'release_source_commit':source,'started_unix':started,'finished_unix':time.time(),'exit_code':result.returncode});(out/'commands.json').write_text(json.dumps(records,indent=2)+'\n')
 if result.returncode:raise SystemExit(result.returncode)
try:
 assert json.loads((r/(candidate+'_full_exit.json')).read_text())['exit_code']==0
 assert json.loads((r/(candidate+'_final_mesh_profile_exit.json')).read_text())['exit_code']==0
 checked('release_source_matches_checkout',['git','diff','--exit-code',source,'--','papers/*/src/*','papers/*/include/*','papers/*/test/*','papers/*/CMakeLists.txt','evaluation/src/*','CMakeLists.txt','evaluation/CMakeLists.txt'])
 checked('mesh_sources_restored',['git','diff','--exit-code','134ead2f21a4611ec328a45aa543b31428d6b92c','--','papers/mesh_loam/src','papers/mesh_loam/include','papers/mesh_loam/test','papers/mesh_loam/CMakeLists.txt'])
 paths=[f'papers/{method}/{part}' for method in ['imls_slam','inten_loam'] for part in ['src','include','test','CMakeLists.txt']]
 checked('retained_sources_match_measurement',['git','diff','--exit-code',candidate,'--',*paths])
 libs={m:repo/'build/papers'/m/('lib'+m+'.a') for m in ['imls_slam','inten_loam']};before={m:hashlib.sha256(p.read_bytes()).hexdigest() for m,p in libs.items()}
 checked('configure',docker+['exec','-w',str(repo),'localization-zoo-dev','cmake','-S','.','-B','build','-DCMAKE_BUILD_TYPE=Release'])
 checked('build',docker+['exec','-w',str(repo),'localization-zoo-dev','cmake','--build','build','-j2'])
 after={m:hashlib.sha256(p.read_bytes()).hexdigest() for m,p in libs.items()};assert before==after,'Retained library archives changed during Mesh-only cleanup'
 checked('ctest',docker+['exec','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-w',str(repo),'localization-zoo-dev','ctest','--test-dir','build','--output-on-failure'])
 binary=r/(tag+'_release_pcd_dogfooding');shutil.copy2(repo/'build/evaluation/pcd_dogfooding',binary);meta={'source_commit':source,'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'retained_library_sha256_before':before,'retained_library_sha256_after':after,'mesh_cpp_matches_baseline':True,'retained_cpp_matches_measured_candidate':True};(out/'binary.json').write_text(json.dumps(meta,indent=2)+'\n')
 tree=r/(tag+'_release_worktree');checked('worktree',['git','worktree','add','--detach',str(tree),source])
 protocol=json.loads((r/'protocol.json').read_text());checks={}
 for method,cfg in protocol['method_configs'].items():
  for mode in (['accelerated'] if method=='mesh_loam' else ['default','accelerated']):
   label=method+'_'+mode;run=out/label;run.mkdir();cwd=tree/'dogfooding_results/release_verification'/label;cwd.mkdir(parents=True,exist_ok=False)
   cmd=docker+['exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','-w',str(cwd),'localization-zoo-dev',str(binary),str(r/'kitti/dataset/sequences/07/velodyne'),str(tree/'experiments/reference_data/kitti_seq_07_full_gt.csv'),'120','--methods',method,'--summary-json',str(run/'summary.json'),*cfg['args']]
   if mode=='accelerated':cmd+=['--paper-pipeline-accelerated']
   started=time.time()
   with (run/'run.log').open('w') as f:child=subprocess.Popen(cmd,stdout=f,stderr=subprocess.STDOUT);(r/'release_verification_active.json').write_text(json.dumps({'label':label,'pid':child.pid,'started_unix':started},indent=2)+'\n');code=child.wait()
   (run/'command.json').write_text(json.dumps({'command':cmd,'source_commit':source,'binary_sha256':meta['binary_sha256'],'verification_only':True,'started_unix':started,'finished_unix':time.time(),'exit_code':code},indent=2)+'\n');assert code==0
   summary=json.loads((run/'summary.json').read_text());row=summary['methods'][0];assert summary['evaluator_git_commit']==source and row['frames']==120 and row['status']=='ok'
   name={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method];shutil.copy2(cwd/'dogfooding_results'/(name+'.txt'),run/'poses.txt')
   ref=e/'runs'/('67867d0b_control_short_mesh_loam' if method=='mesh_loam' else f'{candidate}_{mode}_short_{method}')/'poses.txt';actual=np.loadtxt(run/'poses.txt');expected=np.loadtxt(ref)[:120];assert actual.shape==expected.shape==(120,12) and np.array_equal(actual,expected),label
   if method=='mesh_loam':assert 'repository acceleration enabled' not in row['note']
   checks[label]={'reference':str(ref.relative_to(repo)),'saved_poses_equal':True,'saved_pose_significant_digits':9,'verification_only':True};(out/'trajectory_checks.json').write_text(json.dumps(checks,indent=2)+'\n')
 (out/'exit.json').write_text(json.dumps({'exit_code':0,'release_source_commit':source,'finished_unix':time.time()},indent=2)+'\n')
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (out/'exit.json').write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
