from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import time
import traceback

repo = Path('/workspace/localization_zoo')
r = Path('/workspace/localization-env/paper-bindings')
out = r / 'local_cpp'
out.mkdir()
source = json.loads((r / 'source.json').read_text())['source_commit']
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '-e', 'OMP_NUM_THREADS=1',
          '-e', 'OPENBLAS_NUM_THREADS=1', '-e', 'PYTHONDONTWRITEBYTECODE=1',
          '-w', str(repo), 'localization-zoo-dev']
commands = [
    ('unchanged_algorithms', ['git', 'diff', '--exit-code', '134ead2f', '--',
                             'common', 'papers', 'evaluation/src', 'evaluation/scripts']),
    ('source_guard', ['git', 'diff', '--exit-code', source, '--', 'python', 'ros2', '.github/workflows/ci.yml']),
    ('configure', docker + ['cmake', '-S', '.', '-B', 'build', '-DCMAKE_BUILD_TYPE=Release',
                           '-DBUILD_PYTHON_BINDINGS=ON',
                           '-Dpybind11_DIR=/opt/localization-venv/lib/python3.10/site-packages/pybind11/share/cmake/pybind11',
                           '-DPYTHON_EXECUTABLE=/opt/localization-venv/bin/python',
                           '-DPython3_EXECUTABLE=/opt/localization-venv/bin/python']),
    ('build', docker + ['cmake', '--build', 'build', '-j2']),
    ('ctest', docker + ['ctest', '--test-dir', 'build', '--output-on-failure']),
    ('existing_python_example', docker + ['/opt/localization-venv/bin/python', 'python/example_synthetic.py']),
]
for method in ['suma_paper', 'imls_slam_paper', 'dilo_paper', 'litamin2_paper']:
    commands.append(('example_' + method, docker + ['/opt/localization-venv/bin/python',
                     'python/example_paper_odometry.py', method, '--output', str(out / (method + '_poses.csv'))]))
records = []
try:
    for label, command in commands:
        started = time.time()
        (r / 'local_active.json').write_text(json.dumps({'label': label, 'started_unix': started}) + '\n')
        with (out / (label + '.log')).open('w') as log:
            result = subprocess.run(command, cwd=repo, stdout=log, stderr=subprocess.STDOUT)
        records.append({'label': label, 'command': command, 'cwd': str(repo),
                        'implementation_commit': source, 'started_unix': started,
                        'finished_unix': time.time(), 'exit_code': result.returncode})
        (out / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
        if result.returncode:
            raise SystemExit(result.returncode)
    shutil.copy2(repo / 'build/python/paper_python_fixture.json', out / 'paper_python_fixture.json')
    metadata = {'implementation_commit': source,
                'module_sha256': {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                  for p in (repo / 'build/python').glob('localization_zoo*.so')},
                'input_sha256': {str(p.relative_to(repo)): hashlib.sha256(p.read_bytes()).hexdigest()
                                 for p in (repo / 'evaluation/fixtures/mcd_kth_smoke').glob('*/cloud.pcd')}}
    (out / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
    (out / 'exit.json').write_text(json.dumps({'exit_code': 0, 'implementation_commit': source}) + '\n')
except BaseException as error:
    code = error.code if isinstance(error, SystemExit) and isinstance(error.code, int) else 1
    (out / 'exit.json').write_text(json.dumps({'exit_code': code, 'error': str(error)}) + '\n')
    traceback.print_exc()
    raise
