from pathlib import Path
import json,os,subprocess,time
r=Path('/workspace/localization-env/shared-range-checkpoint07')
w=r/'worktree'
python='/workspace/localization-env/host-venv/bin/python'
env=dict(os.environ,PATH=str(Path(python).parent)+os.pathsep+os.environ['PATH'],PYTHONDONTWRITEBYTECODE='1',OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',MPLCONFIGDIR=str(r/'matplotlib'))
source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=w,text=True).strip()
checks=r/'source_checks';checks.mkdir(exist_ok=True)
receipts=json.loads((r/'analysis_commands.json').read_text())
for label,args,folder in [
 ('checkpoint',[python,'experiments/results/shared_near_range/checkpoint_report.py','--output-dir',str(r/'checkpoint')],r),
 ('diagnosis',[python,'experiments/results/shared_near_range/diagnose_saved02.py','--sequence','07','--output-dir',str(r/'diagnosis')],r)]:
 started=time.time()
 result=subprocess.run(args,cwd=w,env=env,capture_output=True,text=True)
 (folder/(label+'.log')).write_text(result.stdout+result.stderr)
 receipt=dict(command=args,cwd=str(w),source_commit=source,environment={k:env[k] for k in ('PYTHONDONTWRITEBYTECODE','OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','MPLCONFIGDIR')},started_unix=started,finished_unix=time.time(),exit_code=result.returncode)
 receipts.append(dict(label=label,**receipt))
 (r/'analysis_commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
 if label in ('checkpoint','diagnosis') and (r/label).exists():
  (r/label/'analysis_command.json').write_text(json.dumps(receipt,indent=2)+'\n')
 print(label,result.returncode,flush=True)
 if result.returncode: raise SystemExit(result.returncode)
