from pathlib import Path
import hashlib
import json
import subprocess
import time
import traceback

root = Path(__file__).resolve().parent
repo = root / 'worktree'
out = root / 'native_checks'
out.mkdir()
source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip()
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-e', 'PYTHONDONTWRITEBYTECODE=1', '-w', str(repo), 'localization-zoo-dev']
commands = [
    ('source_guard', ['git', 'diff', '--exit-code', source, '--', 'papers', 'common', 'evaluation']),
    ('configure', docker + ['cmake', '-S', '.', '-B', 'build', '-DCMAKE_BUILD_TYPE=Release']),
    ('build', docker + ['cmake', '--build', 'build', '-j2']),
    ('ctest', docker + ['ctest', '--test-dir', 'build', '--output-on-failure']),
]
receipts = []
try:
    for name, command in commands:
        (root / 'active.json').write_text(json.dumps(dict(status=name, source=source)) + '\n')
        start = time.time()
        with (out / (name + '.log')).open('w') as stream:
            result = subprocess.run(command, cwd=repo, stdout=stream, stderr=subprocess.STDOUT)
        receipts.append(dict(name=name, command=command, cwd=str(repo), source_commit=source,
                             exit_code=result.returncode, started_unix=start, finished_unix=time.time()))
        (out / 'commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
        if result.returncode:
            raise RuntimeError(f'{name} exited {result.returncode}')
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=0, source_commit=source)) + '\n')
    (root / 'active.json').write_text(json.dumps(dict(status='native_checks_pass', source=source)) + '\n')
except BaseException as error:
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=1, error=str(error), source_commit=source)) + '\n')
    traceback.print_exc()
    raise
