"""Record passive postprocessing commands without altering any older evidence."""
from pathlib import Path
import hashlib, importlib.util, json, os, subprocess, time

E = Path(__file__).resolve().parent
W = E.parents[2]
P = E.parent/'inten_support_information'
S = Path('/tmp/localization-zoo-inten-local-rigidity-20261023')
BASE = '8535891bb3362fb03ec336c931fb4c95b0519207'
PY = '/opt/codex/runtimes/codex-primary-runtime/dependencies/python/bin/python3'
PREFIX = ['env', 'PYTHONPATH=/workspace/localization-env/host-venv/lib/python3.12/site-packages', PY]
ENV = dict(os.environ, OPENBLAS_NUM_THREADS='1', OMP_NUM_THREADS='1', PYTHONDONTWRITEBYTECODE='1', TMPDIR=str(S))

def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
    return m

def read(path): return json.loads(Path(path).read_text())
def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, allow_nan=False)+'\n')
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def git(*args): return subprocess.check_output(['git', *args], cwd=W, text=True).strip()
def run(label, command, folder):
    folder.mkdir(parents=True, exist_ok=True); started = time.time(); source=git('rev-parse','HEAD')
    with (folder/(label+'.stdout.log')).open('xb') as out, (folder/(label+'.stderr.log')).open('xb') as err:
        result = subprocess.run(command, cwd=W, env=ENV, stdout=out, stderr=err)
    write(folder/(label+'_command.json'), dict(command=command, cwd=str(W), source_commit=source,
        finished_source_commit=git('rev-parse','HEAD'),
        exit_code=result.returncode, started_unix=started, finished_unix=time.time(),
        environment={k:ENV[k] for k in ('OPENBLAS_NUM_THREADS','OMP_NUM_THREADS','PYTHONDONTWRITEBYTECODE','TMPDIR')}))
    print(label, result.returncode, flush=True)
    return result.returncode
