from pathlib import Path
import json,shutil,subprocess,sys,time,traceback
repo=Path('/workspace/localization_zoo');r=Path('/workspace/localization-env/shared-coarse-start');e=repo/'experiments/results/shared_coarse_start'
def write(path,payload):path.write_text(json.dumps(payload,indent=2)+'\n')
def stage(value,**kw):write(r/'finish_state.json',{'stage':value,**kw})
try:
 stage('waiting_native_completion')
 while True:
  if (r/'run_exit.json').exists():
   final=json.loads((r/'run_exit.json').read_text())
   if final['exit_code']==0:break
   stage('waiting_native_recovery',last_failure=final)
  time.sleep(45)
 assert len(final['completed'])==11 and set(final['completed'])=={f'{i:02d}' for i in range(11)}
 for seq in final['completed']:
  assert not (r/'kitti/dataset/sequences'/seq/'velodyne').exists()
  alias=r/'data/dogfooding_results'/f'kitti_seq_{seq}_full'
  assert not alias.exists() and not alias.is_symlink()
 final['controller_launch_receipts']={p.name:json.loads(p.read_text()) for p in sorted((e/'attempts').glob('*launch.json'))}
 final['recorded_at_unix']=time.time();write(e/'attempts/final_completion.json',final)
 stage('generating_full_report')
 report=e/'report';assert not report.exists();report.mkdir()
 command=[sys.executable,str(e/'summarize_results.py'),'--output-dir',str(report)];started=time.time()
 with (report/'execution.log').open('w') as log:result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
 write(report/'command.json',{'command':command,'cwd':str(repo),'started_unix':started,'finished_unix':time.time(),'actual_exit_code':result.returncode});assert result.returncode==0
 subprocess.run(['git','add','-f',str(report),str(e/'attempts/final_completion.json')],cwd=repo,check=True)
 subprocess.run(['git','commit','-m','Record complete fixed coarse-start KITTI 00-10 report and preservation audit'],cwd=repo,check=True)
 stage('writing_final_docs');subprocess.run([sys.executable,str(r/'write_final_docs.py')],cwd=repo,check=True)
 stage('regenerating_and_checking');subprocess.run([sys.executable,str(r/'finalize_checks.py')],cwd=repo,check=True)
 shutil.copyfile(r/'finalize_checks.py',e/'checks/final/driver.py')
 stage('checks_complete_pending_review')
except BaseException as error:
 stage('failed',error=str(error));traceback.print_exc();raise
