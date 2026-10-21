"""One completion process, waiting for the single collection; never retry failed science."""
from pathlib import Path
import importlib.util,json,os,shutil,subprocess,time
R=Path(__file__).resolve().parent;W=R/'worktree';E=W/'experiments/results/inten_map_plane_spread';PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(os.environ,PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1',MPLCONFIGDIR=str(R/'matplotlib'),XDG_CACHE_HOME=str(R/'cache'))
def read(p):return json.loads(p.read_text())
def write(p,v):p.write_text(json.dumps(v,indent=2,allow_nan=False)+'\n')
def git(*a):return subprocess.check_output(['git',*a],cwd=W,text=True).strip()
def run(label,argv):
    stage=R/'workflow_record';stage.mkdir(exist_ok=True);started=time.time();source=git('rev-parse','HEAD')
    with (stage/(label+'.stdout')).open('xb') as out,(stage/(label+'.stderr')).open('xb') as err:p=subprocess.run(argv,cwd=W,env=ENV,stdout=out,stderr=err)
    write(stage/(label+'_command.json'),dict(command=argv,cwd=str(W),source_commit=source,started_unix=started,finished_unix=time.time(),exit_code=p.returncode));print(label,p.returncode,flush=True)
    if p.returncode:raise RuntimeError(label+' failed: do not silently restart')
while not (R/'start_driver.json').exists():time.sleep(2)
if read(R/'start_driver.json')['exit_code']!=0:raise RuntimeError('Study execution failed; retained for inspection')
run('finish_collection',[PY,str(R/'finish_collection.py')]);run('finalize_docs',[PY,str(E/'finalize_docs.py')])
s=importlib.util.spec_from_file_location('archive',W/'experiments/results/inten_natural_traces/run_sequences.py');a=importlib.util.module_from_spec(s);s.loader.exec_module(a)
a.archive(R/'workflow_record',E/'workflow_execution');folder=E/'workflow_execution'
for n in ('start_collection.py','finish_collection.py','complete_study.py'):shutil.copyfile(R/n,folder/n)
subprocess.run(['git','add','-f',str(folder)],cwd=W,check=True);subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True);subprocess.run(['git','commit','-m','Archive completed full plane-spread reporting workflow receipts'],cwd=W,check=True)
run('push',['git','push','origin','HEAD:refs/heads/codex/inten-map-plane-spread']);write(R/'completion.json',dict(exit_code=0,head=git('rev-parse','HEAD'),clean=not git('status','--porcelain'),all_full_sequences_complete=True,pr_created=False,ci_pending=True))
