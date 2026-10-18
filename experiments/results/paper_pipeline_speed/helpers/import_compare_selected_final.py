from pathlib import Path
import json,shutil,subprocess,time,traceback
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');
while not (r/'final_source_ready.json').exists():time.sleep(5)
tag=json.loads((r/'final_source_ready.json').read_text())['tag'];e=repo/'experiments/results/paper_pipeline_speed';python='/workspace/localization-env/host-venv/bin/python'
try:
 while not (r/(tag+'_full_exit.json')).exists():
  if (r/(tag+'_candidate_exit.json')).exists():raise RuntimeError('Final candidate build/test/screening failed; inspect its receipt')
  end=r/'final_paired_full_gate_exit.json'
  if end.exists():
   data=json.loads(end.read_text());assert data['exit_code']==0,data
   gate=r/(tag+'_full_gate.json')
   if gate.exists() and not json.loads(gate.read_text())['proceed_full']:raise RuntimeError('Screening needs a numerical diagnosis before full runs')
  time.sleep(10)
 assert json.loads((r/(tag+'_full_exit.json')).read_text())['exit_code']==0
 subprocess.run([python,str(r/'import_candidate.py'),'--tag',tag],cwd=repo,check=True)
 shutil.copy2(Path(__file__),e/'helpers'/Path(__file__).name)
 shutil.copy2(r/(tag+'_full_gate.json'),e/'candidates'/tag/'full_gate.json')
 subprocess.run(['git','add','experiments/results/paper_pipeline_speed'],cwd=repo,check=True)
 if subprocess.run(['git','diff','--cached','--quiet'],cwd=repo).returncode:subprocess.run(['git','commit','-m','Record paired full KITTI 07 controls and accelerated runs'],cwd=repo,check=True)
 raw_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
 subprocess.run([python,str(r/'compare_full.py'),'--tag',tag],cwd=repo,check=True)
 subprocess.run(['git','add','experiments/results/paper_pipeline_speed/comparisons'],cwd=repo,check=True);subprocess.run(['git','commit','-m','Derive full-sequence speed and accuracy from committed paired executions'],cwd=repo,check=True)
 (r/'final_comparison_ready.json').write_text(json.dumps({'exit_code':0,'raw_commit':raw_commit,'comparison_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'comparison_path':str(e/'comparisons'/(tag+'.json')),'next':'Inspect numerical differences and targets before rendering README tables; leave roadmap open if <3x. Input cleanup, final docs/checks/push/exactHEADCI still pending.'},indent=2)+'\n')
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (r/'final_comparison_ready.json').write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
