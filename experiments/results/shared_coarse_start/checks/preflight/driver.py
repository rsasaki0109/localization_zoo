from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,time,traceback
repo=Path('/workspace/localization_zoo');r=Path('/workspace/localization-env/shared-coarse-start');e=repo/'experiments/results/shared_coarse_start';out=e/'checks/preflight';out.mkdir(parents=True,exist_ok=False)
records=[]
source=json.loads((e/'candidate.json').read_text())['source_commit']
def docker(user='1000:1000',cwd=repo,binary=None):
 command=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user',user,'-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-e','PYTHONDONTWRITEBYTECODE=1','-e','MPLCONFIGDIR=/tmp/localization-coarse-plots']
 if binary:command+=['-e','ZOO_COARSE_NATIVE_BINARY='+str(binary)]
 return command+['-w',str(cwd),'localization-zoo-dev']
def run(label,command,folder=out,expected=0):
 folder.mkdir(parents=True,exist_ok=True);start=time.time()
 with (folder/(label+'.log')).open('w') as log:result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
 record={'label':label,'command':command,'cwd':str(repo),'implementation_commit':source,'checkout_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'started_unix':start,'finished_unix':time.time(),'expected_exit_code':expected,'exit_code':result.returncode}
 (folder/(label+'_command.json')).write_text(json.dumps(record,indent=2)+'\n');records.append(record);(out/'commands.json').write_text(json.dumps(records,indent=2)+'\n');print(json.dumps({'label':label,'exit_code':result.returncode,'expected_exit_code':expected}),flush=True)
 if result.returncode!=expected:raise RuntimeError(f'{label}: actual{result.returncode}, expected{expected}')
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n')
try:
 run('coverage_guard',[sys.executable,str(e/'summarize_results.py'),'--output-dir',str(out/'refused_incomplete_report')],expected=2)
 before=(r/'active.json').read_bytes() if (r/'active.json').exists() else None
 run('fetch_resume_guard',[sys.executable,str(e/'run_all.py'),'--resume-from','04','--check-resume-only'])
 assert ((r/'active.json').read_bytes() if (r/'active.json').exists() else None)==before
 cases=[('suma_raw','suma',['--no-gt-seed','--suma-paper']),('imls_raw','imls_slam',['--no-gt-seed','--imls-slam-paper-odometry']),('imls_coarse','imls_slam',['--no-gt-seed','--imls-slam-paper-odometry','--imls-slam-paper-coarse-frames','1'])]
 checks=[]
 for label,method,args in cases:
  runs={}
  for mode in ('baseline_wrapped','candidate_wrapped','candidate_direct'):
   folder=e/'wrapper_validation'/label/mode;folder.mkdir(parents=True)
   summary=folder/'summary.json';binary=r/('baseline_pcd_dogfooding' if mode.startswith('baseline') else 'candidate_pcd_dogfooding')
   if mode.endswith('direct'):
    native=folder/'direct_native';(native/'dogfooding_results').mkdir(parents=True);command=docker(cwd=native)+[str(binary)]
   else:
    native=folder/'summary_native';command=docker(binary=binary)+[str(e/'native_wrapper.py')]
   command +=[str(repo/'evaluation/fixtures/mcd_kth_smoke'),str(repo/'evaluation/fixtures/mcd_kth_smoke_gt.csv'),'--methods',method,'--summary-json',str(summary),*args]
   run('run',command,folder)
   data=json.loads(summary.read_text())['methods'];assert len(data)==1 and data[0]['status']=='ok' and data[0]['frames']==3
   path=native/'dogfooding_results'/(data[0]['name'].replace('-','_')+'.txt');runs[mode]=(data[0],path.read_bytes())
  keys=('status','frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
  reference,poses=runs['candidate_wrapped']
  for mode in ('baseline_wrapped','candidate_direct'):
   data,other=runs[mode];equal={k:data[k]==reference[k] for k in keys};check={'case':label,'comparison':mode+'_vs_candidate_wrapped','equal_metrics':equal,'saved_pose_bytes_equal':other==poses};checks.append(check);assert all(equal.values()) and other==poses
 write(e/'wrapper_validation/comparison.json',checks)
 folder=e/'wrapper_validation/suma_coarse_cli';folder.mkdir(parents=True);summary=folder/'summary.json'
 command=docker()+[str(e/'native_wrapper.py'),str(repo/'evaluation/fixtures/mcd_kth_smoke'),str(repo/'evaluation/fixtures/mcd_kth_smoke_gt.csv'),'--methods','suma','--summary-json',str(summary),*json.loads((e/'protocol.json').read_text())['suma_fixed_arguments']]
 run('run',command,folder);item=json.loads(summary.read_text())['methods'][0];assert item['status']=='ok' and item['frames']==3 and 'opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)' in item['note']
 run('full_build',docker(user='0:0')+['cmake','--build','build','-j1'])
 run('all_ctest',docker(user='0:0')+['ctest','--test-dir','build','--output-on-failure'])
 run('python_unittest',docker()+['/opt/localization-venv/bin/python','-m','unittest','discover','-s','tests','-v'])
 for label,name,args in [('showcase','validate_showcase.py',[]),('leaderboard','generate_leaderboard.py',['--check']),('paper_ready','generate_paper_ready_evidence.py',['--check'])]:run(label,docker()+['/opt/localization-venv/bin/python','evaluation/scripts/'+name,*args])
 run('preservation',[sys.executable,str(e/'verify_preservation.py'),'--output',str(out/'preservation.json')])
 run('whitespace',['git','diff','--check'])
 shutil.copyfile(__file__,out/'driver.py');write(r/'preflight_exit.json',{'exit_code':0,'implementation_commit':source})
except BaseException as error:
 write(r/'preflight_exit.json',{'exit_code':1,'error':str(error)});traceback.print_exc();raise
