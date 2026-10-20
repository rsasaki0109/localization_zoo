"""Wait for the one collector, then archive/report/import/document/check once."""
from pathlib import Path
import importlib.util
import json
import os
import shutil
import subprocess
import time
import traceback

R = Path(__file__).resolve().parent
W = R/'worktree'
E = W/'experiments/results/inten_translation_prior'
PY = '/workspace/localization-env/host-venv/bin/python'
ENV = dict(os.environ, PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1')


def read(p): return json.loads(p.read_text())
def write(p,d): p.write_text(json.dumps(d,indent=2,allow_nan=False)+'\n')
def git(*a): return subprocess.check_output(['git',*a],cwd=W,text=True).strip()


def execute(label, argv):
    stage=R/'workflow_record'/label;stage.mkdir(parents=True)
    record=dict(command=argv,cwd=str(W),source_commit=git('rev-parse','HEAD'),started_unix=time.time(),
        environment={'PYTHONDONTWRITEBYTECODE':'1','OMP_NUM_THREADS':'1','OPENBLAS_NUM_THREADS':'1',
                     'PATH_prefix':str(Path(PY).parent)})
    with (stage/'stdout.log').open('xb') as out,(stage/'stderr.log').open('xb') as err:
        p=subprocess.run(argv,cwd=W,env=ENV,stdout=out,stderr=err)
    record.update(exit_code=p.returncode,finished_unix=time.time());write(stage/'command.json',record)
    print(label,p.returncode,flush=True)
    if p.returncode:raise RuntimeError(label+' failed; retain workflow evidence')


def main():
    # Do not mutate the worktree/index until the collector has exited, including
    # its outer invocation receipt. No second fetch or native runner is launched.
    while True:
        try:record=read(R/'collector_command.json')
        except (FileNotFoundError,json.JSONDecodeError):record={}
        if 'exit_code' in record:
            if record['exit_code']!=0:raise RuntimeError('Collector failed; no automatic restart')
            break
        time.sleep(15)
    execute('finish_collection',[PY,str(R/'finish_collection.py')])
    execute('finalize_docs',[PY,str(E/'finalize_docs.py')])
    stage=R/'workflow_record'
    for name in ('complete_study.py','finish_collection.py'):
        shutil.copyfile(R/name,stage/name)
    spec=importlib.util.spec_from_file_location('archive',W/'experiments/results/inten_natural_traces/run_sequences.py')
    archival=importlib.util.module_from_spec(spec);spec.loader.exec_module(archival)
    archival.archive(stage,E/'workflow_execution')
    subprocess.run(['git','add','-f',str(E/'workflow_execution')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Archive complete translation-prior reporting workflow receipts'],cwd=W,check=True)
    execute('final_push',['git','push','origin','HEAD:refs/heads/codex/inten-translation-prior'])
    write(R/'completion.json',dict(exit_code=0,head=git('rev-parse','HEAD'),
        local_checks_complete=True,pr_created=False,full_native_ros_ci_pending=True))


if __name__=='__main__':
    try:main()
    except BaseException as error:
        write(R/'completion_failure.json',dict(exit_code=1,error=str(error),traceback=traceback.format_exc()));raise
