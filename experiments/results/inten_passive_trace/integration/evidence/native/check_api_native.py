from pathlib import Path
import hashlib
import json
import subprocess
import time

R = Path(__file__).resolve().parent
W = Path('/workspace/localization-env/inten-passive-trace/worktree')
SOURCE = 'eb5f0c47cc517f3e8d12d1bd36761f92e546faa0'
O = R / 'api_native'
O.mkdir()
(O / 'tmp').mkdir()
receipts = []
prefix = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000']

def docker(argv, threads=1):
    return prefix + ['-e', f'OMP_NUM_THREADS={threads}', '-e', 'OPENBLAS_NUM_THREADS=1',
        '-e', 'PYTHONDONTWRITEBYTECODE=1', '-e', f'TEST_TMPDIR={O / "tmp"}',
        '-w', str(W), 'localization-zoo-dev', *argv]

def run(name, command):
    start = time.time()
    with (O / (name + '.log')).open('w') as log:
        p = subprocess.run(command, cwd=W, stdout=log, stderr=subprocess.STDOUT)
    receipts.append(dict(name=name, command=command, cwd=str(W), source_commit=SOURCE,
        exit_code=p.returncode, started_unix=start, finished_unix=time.time()))
    (O / 'commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
    print(name, p.returncode, flush=True)
    if p.returncode:
        (O / 'exit.json').write_text(json.dumps(dict(exit_code=p.returncode, failed=name)) + '\n')
        raise SystemExit(p.returncode)

run('source_identity', ['git', 'diff', '--exit-code', SOURCE, '--', 'common', 'evaluation/src',
    'evaluation/CMakeLists.txt', ':(glob)papers/*/src/**', ':(glob)papers/*/include/**',
    ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt'])
run('configure', docker(['cmake', '-S', '.', '-B', 'build', '-DCMAKE_BUILD_TYPE=Release']))
run('build', docker(['cmake', '--build', 'build', '-j2']))
run('ctest', docker(['ctest', '--test-dir', 'build', '--output-on-failure']))
run('inten_four_workers', docker(['ctest', '--test-dir', 'build', '--output-on-failure',
    '-R', '^test_inten_loam_paper$'], threads=4))
b = W / 'build/evaluation/pcd_dogfooding'
(O / 'binary.json').write_text(json.dumps(dict(path=str(b),
    sha256=hashlib.sha256(b.read_bytes()).hexdigest(), source_commit=SOURCE), indent=2) + '\n')
(O / 'exit.json').write_text(json.dumps(dict(exit_code=0, source_commit=SOURCE,
    scope='Incremental full Release rebuild after the real API/speed merge, all CTests, and four-worker passive trace lifecycle coverage. No timing claim.')) + '\n')
