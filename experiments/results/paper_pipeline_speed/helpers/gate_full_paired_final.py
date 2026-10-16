from pathlib import Path
import json,shutil,subprocess,time,traceback
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');
while not (r/'final_source_ready.json').exists():time.sleep(5)
tag=json.loads((r/'final_source_ready.json').read_text())['tag'];e=repo/'experiments/results/paper_pipeline_speed'
try:
 while not (r/(tag+'_screening_exit.json')).exists():
  if (r/(tag+'_candidate_exit.json')).exists():raise RuntimeError('Candidate failed; inspect preserved build/test/benchmark receipts')
  time.sleep(5)
 assert json.loads((r/(tag+'_screening_exit.json')).read_text())['exit_code']==0
 subprocess.run(['/workspace/localization-env/host-venv/bin/python',str(r/'import_candidate.py'),'--tag',tag],cwd=repo,check=True)
 shutil.copy2(r/'final_full_gate_launcher.json',e/'candidates'/tag/'full_gate_launcher.json')
 subprocess.run(['git','add','experiments/results/paper_pipeline_speed'],cwd=repo,check=True)
 if subprocess.run(['git','diff','--cached','--quiet'],cwd=repo).returncode:subprocess.run(['git','commit','-m','Preserve normal-bound candidate screening and exact-trajectory checks'],cwd=repo,check=True)
 s=json.loads((r/(tag+'_screening.json')).read_text());match=all(v['default_pose_max_abs_difference']==0 and v['accelerated_pose_max_abs_difference']==0 for v in s.values())
 gate={'screening_raw_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'saved_poses_match':match,'screening':s,'proceed_full':match,'reason':'Full sequence establishes speed and accuracy, including negative results; windows only guide development. Primary reference is a fresh full frozen-source control immediately before each method.'}
 (r/(tag+'_full_gate.json')).write_text(json.dumps(gate,indent=2)+'\n')
 if match:
  with (r/(tag+'_full_stdout.log')).open('w') as f:result=subprocess.run(['/workspace/localization-env/host-venv/bin/python',str(r/'run_paired_full_candidate.py'),'--tag',tag],stdout=f,stderr=subprocess.STDOUT,cwd=repo)
  if result.returncode:raise SystemExit(result.returncode)
 else:print('Saved poses changed in screening; diagnose before full runs.',flush=True)
 (r/'final_paired_full_gate_exit.json').write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (r/'final_paired_full_gate_exit.json').write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
