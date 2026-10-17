"""Recorded study support; native subprocess output is isolated by the wrapper."""
from pathlib import Path
import hashlib
import json
import subprocess
import time

E = Path(__file__).resolve().parent
REPO = E.parents[2]
R = Path('/workspace/localization-env/shared-near-range')
PY = '/workspace/localization-env/host-venv/bin/python'
CONTAINER_PY = '/opt/localization-venv/bin/python'
PROTOCOL = json.loads((E / 'protocol.json').read_text())
SOURCE = PROTOCOL['implementation_commit']
WRAPPER = E / 'native_wrapper.py'


def write(path, payload):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + '.tmp')
    temporary.write_text(json.dumps(payload, indent=2, allow_nan=False) + '\n')
    temporary.replace(path)


def digest(path):
    value = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(chunk)
    return value.hexdigest()


def docker(frozen=False, root=False):
    native = R / ('baseline_pcd_dogfooding' if frozen else 'candidate_pcd_dogfooding')
    return ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
            '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH',
            'docker', '--host=unix:///var/run/docker.sock', 'exec', '--user',
            '0:0' if root else '1000:1000', '-e', 'OMP_NUM_THREADS=1',
            '-e', 'OPENBLAS_NUM_THREADS=1', '-e', 'PYTHONDONTWRITEBYTECODE=1',
            '-e', 'ZOO_RANGE_NATIVE_BINARY=' + str(native),
            '-w', str(REPO), 'localization-zoo-dev']


def run(label, command, folder, allowed=(0,)):
    folder.mkdir(parents=True, exist_ok=True)
    receipt = folder / (label + '_command.json')
    if receipt.exists():
        raise RuntimeError('Refusing to overwrite command evidence: ' + str(receipt))
    started = time.time()
    with (folder / (label + '.log')).open('w') as log:
        result = subprocess.run(command, cwd=REPO, stdout=log, stderr=subprocess.STDOUT)
    write(receipt, dict(label=label, command=command, cwd=str(REPO),
                       implementation_commit=SOURCE, started_unix=started,
                       finished_unix=time.time(), exit_code=result.returncode))
    print(label, result.returncode, flush=True)
    if result.returncode not in allowed:
        raise RuntimeError(f'{label} exited {result.returncode}')
    return result.returncode


def native_output(summary):
    return summary.parent / (summary.stem + '_native') / 'dogfooding_results'


def verify_binaries():
    for name in ('candidate', 'baseline'):
        expected = json.loads((E / (name + '.json')).read_text())
        assert digest(Path(expected['binary'])) == expected['sha256']
    subprocess.run(['git', 'diff', '--exit-code', SOURCE, '--', 'common',
                    'evaluation/src', 'evaluation/CMakeLists.txt', 'papers/*/src',
                    'papers/*/include', 'papers/*/test', 'papers/*/CMakeLists.txt',
                    'CMakeLists.txt'], cwd=REPO, check=True)
