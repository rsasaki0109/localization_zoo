"""Own the fixed full collection until exit; retain its actual driver receipt."""
from pathlib import Path
import importlib.util
import json
import os
import subprocess
import time

E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(os.environ,PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],
         PYTHONDONTWRITEBYTECODE='1',OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',
         MPLCONFIGDIR=str(R/'matplotlib'),XDG_CACHE_HOME=str(R/'cache'))


def main():
    stage=R/'collection_record';stage.mkdir()
    command=[PY,str(E/'collect.py')];start=time.time()
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip()
    with (stage/'stdout.log').open('xb') as out,(stage/'stderr.log').open('xb') as err:
        result=subprocess.run(command,cwd=W,env=ENV,stdout=out,stderr=err)
    receipt=dict(command=command,cwd=str(W),source_commit=head,started_unix=start,
                 finished_unix=time.time(),exit_code=result.returncode,
                 environment={k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')})
    (stage/'command.json').write_text(json.dumps(receipt,indent=2)+'\n')
    spec=importlib.util.spec_from_file_location('collection_archive',W/'experiments/results/inten_natural_traces/run_sequences.py')
    archive=importlib.util.module_from_spec(spec);spec.loader.exec_module(archive)
    archive.archive(stage,E/('collection_execution' if result.returncode==0 else 'collection_failure'))
    if result.returncode:raise SystemExit(result.returncode)
    archive.archive(R/'backups',E/'backup_execution')
    subprocess.run(['git','add','-f',str(E/'collection_execution'),str(E/'backup_execution')],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Archive complete fixed facade-Cauchy collection driver and backups'],cwd=W,check=True)
    print('All fixed full pairs and driver completed0',flush=True)


if __name__=='__main__':main()
