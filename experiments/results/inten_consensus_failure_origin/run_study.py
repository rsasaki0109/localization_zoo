"""Capture real subprocess exits and commit numbers before interpretation."""
from pathlib import Path
import importlib.util
import json
import os
import shutil
import subprocess
import time

E = Path(__file__).resolve().parent; W = E.parents[2]; R = W.parent
PY = '/workspace/localization-env/host-venv/bin/python'
ENV = dict(os.environ, PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'])


def git(*args): return subprocess.check_output(['git', *args], cwd=W, text=True).strip()


def execute(name, argv, stage):
    stage.mkdir(parents=True, exist_ok=True)
    started = time.time()
    with (stage/(name+'.stdout.log')).open('xb') as out, (stage/(name+'.stderr.log')).open('xb') as err:
        result = subprocess.run(argv, cwd=W, env=ENV, stdout=out, stderr=err)
    record = dict(command=argv, cwd=str(W), source_commit=git('rev-parse', 'HEAD'),
                  started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
                  environment={k: ENV[k] for k in ('PYTHONDONTWRITEBYTECODE', 'OMP_NUM_THREADS', 'OPENBLAS_NUM_THREADS')})
    (stage/(name+'_command.json')).write_text(json.dumps(record, indent=2)+'\n')
    print(name, result.returncode, flush=True)
    return result.returncode


def archive(stage, target):
    spec = importlib.util.spec_from_file_location('archive', E.parent/'inten_natural_traces/run_sequences.py')
    m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
    m.archive(stage, target)


def commit(message, *extra):
    files = [str(p) for p in E.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    subprocess.run(['git', 'add', '-f', '--', *files, *extra], cwd=W, check=True)
    subprocess.run(['git', 'diff', '--cached', '--check'], cwd=W, check=True)
    subprocess.run(['git', 'commit', '-m', message], cwd=W, check=True)


def main():
    if git('status', '--porcelain'): raise RuntimeError('Committed clean source required')
    for name in ('protocol.json', 'run_study.py'):
        if subprocess.check_output(['git', 'show', 'HEAD:'+str((E/name).relative_to(W))], cwd=W) != (E/name).read_bytes():
            raise RuntimeError('Commit protocol/source first')
    stage = R/'analysis_stage'; output = R/'analysis_report'
    code = execute('analysis', [PY, str(E/'analyze.py'), '--output-dir', str(output)], stage)
    archive(stage, E/'analysis_execution')
    if code:
        if output.exists(): archive(output, E/'analysis_failure')
        commit('Preserve failed consensus stage diagnosis and its actual exit')
        raise SystemExit(code)
    (E/'report').mkdir()
    for p in output.iterdir(): shutil.copyfile(p, E/'report'/p.name)
    commit('Commit complete archived consensus stage diagnosis before interpretation')
    subprocess.run(['git', 'push', '-u', 'origin', git('branch', '--show-current')], cwd=W, check=True)
    print('Numerical evidence committed and pushed before documentation', flush=True)


if __name__ == '__main__': main()
