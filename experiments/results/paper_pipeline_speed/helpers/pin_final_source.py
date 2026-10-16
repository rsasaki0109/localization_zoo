from pathlib import Path
import subprocess,json,time,traceback
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo')
try:
 while not (r/'67867d0b_binary.json').exists():
  if (r/'67867d0b_candidate_exit.json').exists():raise RuntimeError('V4 build/test failed; inspect before selecting final source')
  time.sleep(2)
 assert not (r/'final_source_ready.json').exists()
 subprocess.run(['git','diff','--exit-code','67867d0b','--','papers/*/src/*','papers/*/include/*','papers/*/CMakeLists.txt','evaluation/src/*'],cwd=repo,check=True)
 subprocess.run(['/workspace/localization-env/host-venv/bin/python',str(r/'reject_worker_pause.py')],check=True)
 subprocess.run(['git','diff','--check'],cwd=repo,check=True);subprocess.run(['git','add','papers/inten_loam'],cwd=repo,check=True)
 subprocess.run(['git','commit','-m','Reject slower OpenMP soft-pause scheduling and retain the direct-query accelerator'],cwd=repo,check=True)
 source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
 (r/'final_source_ready.json').write_text(json.dumps({'source_commit':source,'tag':source[:8]},indent=2)+'\n')
except BaseException as err:
 (r/'pin_final_source_exit.json').write_text(json.dumps({'exit_code':1,'error':str(err)},indent=2)+'\n');traceback.print_exc();raise
