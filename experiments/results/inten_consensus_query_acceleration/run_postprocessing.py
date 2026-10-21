"""Derive and commit numerical evidence before any README interpretation."""
from pathlib import Path
import importlib.util
import json
import os
import shutil
import subprocess
import time

E = Path(__file__).resolve().parent
W = E.parents[2]
R = W.parent
PY = '/workspace/localization-env/host-venv/bin/python'
ENV = dict(os.environ, PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           MPLCONFIGDIR=str(R/'matplotlib'), XDG_CACHE_HOME=str(R/'cache'))


def git(*args):
    return subprocess.check_output(['git', *args], cwd=W, text=True).strip()


def execute(command, stage):
    stage.mkdir(parents=True, exist_ok=False)
    started = time.time()
    with (stage/'stdout.log').open('xb') as out, (stage/'stderr.log').open('xb') as err:
        result = subprocess.run(command, cwd=W, env=ENV, stdout=out, stderr=err)
    receipt = dict(command=command, cwd=str(W), source_commit=git('rev-parse', 'HEAD'),
                   exit_code=result.returncode, started_unix=started, finished_unix=time.time(),
                   environment={**{key: ENV[key] for key in ('OMP_NUM_THREADS', 'OPENBLAS_NUM_THREADS', 'PYTHONDONTWRITEBYTECODE')},
                                'PATH_prefix': str(Path(PY).parent)})
    (stage/'command.json').write_text(json.dumps(receipt, indent=2)+'\n')
    return result.returncode


def main():
    own = Path(__file__).resolve()
    if subprocess.check_output(['git', 'show', 'HEAD:'+own.relative_to(W).as_posix()], cwd=W) != own.read_bytes():
        raise RuntimeError('Postprocessing driver must be committed')
    if (R/'collector.lock').exists() or git('status', '--porcelain'):
        raise RuntimeError('Completed, clean raw collection required')
    if json.loads((E/'collection_execution/command.json').read_text())['exit_code'] != 0:
        raise RuntimeError('Collection failed')
    spec = importlib.util.spec_from_file_location('archive', W/'experiments/results/inten_natural_traces/run_sequences.py')
    archival = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(archival)
    stage = R/'report_record'
    output = R/'full_report'
    code = execute([PY, str(E/'report.py'), '--output-dir', str(output)], stage)
    archival.archive(stage, E/'report_execution')
    if code:
        if output.exists():
            archival.archive(output, E/'report_failure')
        raise RuntimeError('Numerical report failed; retain failure before correcting source')
    archival.archive(output, E/'report')
    print('All full-sequence report checks exited0', flush=True)
    stage = R/'import_record'
    code = execute([PY, str(E/'import_results.py')], stage)
    archival.archive(stage, E/'import_driver_execution')
    # The original command remains byte-identical in the roundtripped archive.
    (E/'import_execution').mkdir(parents=True, exist_ok=True)
    shutil.copyfile(stage/'command.json', E/'import_execution/command.json')
    if code:
        raise RuntimeError('Numerical import failed; retain failure before correcting source')
    files=[str(p) for p in E.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    subprocess.run(['git','add','-f','--',*files,str(W/'experiments/results/index.json')],cwd=W,check=True)
    protocol = json.loads((E/'full_protocol.json').read_text())
    for metadata in protocol['manifests'].values():
        stem = Path(metadata['path']).stem
        subprocess.run(['git', 'add', '-f', 'experiments/results/'+stem+'.json',
                        'experiments/results/runs/'+stem], cwd=W, check=True)
    subprocess.run(['git', 'diff', '--cached', '--check'], cwd=W, check=True)
    subprocess.run(['git', 'commit', '-m', 'Commit every full recursive-consensus comparison and imported numerical result'], cwd=W, check=True)
    subprocess.run(['git', 'push'], cwd=W, check=True)
    print('Complete numerical report/import committed and pushed before prose', flush=True)


if __name__ == '__main__':
    main()
