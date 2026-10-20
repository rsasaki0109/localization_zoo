"""Native command helpers. This study reuses the byte-pinned original native runner; no production rebuild is required. Separate diagnostic compilation is in spread.py."""
from pathlib import Path
import hashlib
import json
import os
import shlex
import subprocess
import time

E = Path(__file__).resolve().parent
W = E.parents[2]
R = W.parent
OLD = Path('/workspace/localization-env/inten-natural-traces/build')
ENV = dict(os.environ, OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', PYTHONDONTWRITEBYTECODE='1')


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, allow_nan=False)+'\n')


def docker(cwd=W):
    return ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
            '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
            '--host=unix:///var/run/docker.sock', 'exec', '-i', '--user', '1000:1000',
            '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
            '-w', str(cwd), 'localization-zoo-dev']


def run(label, command, folder):
    folder.mkdir(parents=True, exist_ok=True)
    started = time.time()
    with (folder/(label+'.stdout')).open('xb') as out, (folder/(label+'.stderr')).open('xb') as err:
        result = subprocess.run(command, cwd=W, env=ENV, stdout=out, stderr=err)
    write(folder/(label+'_command.json'), dict(command=command, cwd=str(W),
        source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=W, text=True).strip(),
        started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
        environment={'OMP_NUM_THREADS':'1', 'OPENBLAS_NUM_THREADS':'1'}))
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise RuntimeError(label+' failed')


def flags(folder):
    path = folder/'flags.make'
    raw = path.read_text().replace(str(OLD.parent/'worktree'), str(W))
    result = []
    for line in raw.splitlines():
        if line.startswith(('CXX_DEFINES =', 'CXX_INCLUDES =', 'CXX_FLAGS =')):
            result.extend(shlex.split(line.split('=', 1)[1]))
    return result

