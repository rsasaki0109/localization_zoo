from pathlib import Path
import json,shutil,subprocess,time,traceback
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');tag='fe03ed88';e=repo/'experiments/results/paper_pipeline_speed'
try:
 while not (r/(tag+'_screening_exit.json')).exists():
  if (r/(tag+'_candidate_exit.json')).exists():raise RuntimeError('Candidate failed; inspect preserved build/test/benchmark receipts')
  time.sleep(5)
 assert json.loads((r/(tag+'_screening_exit.json')).read_text())['exit_code']==0
 subprocess.run(['/workspace/localization-env/host-venv/bin/python',str(r/'import_candidate.py'),'--tag',tag],cwd=repo,check=True)
 shutil.copy2(Path(__file__),e/'helpers'/Path(__file__).name)
 shutil.copy2(r/'v3_full_gate_launcher.json',e/'candidates'/tag/'full_gate_launcher.json')
 subprocess.run(['git','add','experiments/results/paper_pipeline_speed'],cwd=repo,check=True)
 if subprocess.run(['git','diff','--cached','--quiet'],cwd=repo).returncode:subprocess.run(['git','commit','-m','Preserve revised paired-window speed and trajectory comparisons'],cwd=repo,check=True)
 s=json.loads((r/(tag+'_screening.json')).read_text())
 gate={'screening_raw_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'saved_poses_match':all(v['default_pose_max_abs_difference']==0 and v['accelerated_pose_max_abs_difference']==0 for v in s.values()),'all_short_runs_faster_than_both_controls':all(v['accelerated_time_ms']<min(v['control_time_ms'],v['default_time_ms']) for v in s.values()),'screening':s}
 gate['proceed_full']=gate['saved_poses_match'] and gate['all_short_runs_faster_than_both_controls']
 (r/(tag+'_full_gate.json')).write_text(json.dumps(gate,indent=2)+'\n')
 if gate['proceed_full']:
  with (r/(tag+'_full_stdout.log')).open('w') as f:result=subprocess.run(['/workspace/localization-env/host-venv/bin/python',str(r/'run_full_candidate.py'),'--tag',tag],stdout=f,stderr=subprocess.STDOUT,cwd=repo)
  if result.returncode:raise SystemExit(result.returncode)
 else:print('Review the committed screening; full runs not started because a candidate slowed down or changed saved poses.',flush=True)
 (r/'v3_full_gate_exit.json').write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (r/'v3_full_gate_exit.json').write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
