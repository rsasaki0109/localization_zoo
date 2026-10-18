"""Import only closed native validation evidence, before writing numeric prose."""
from pathlib import Path
import gzip
import hashlib
import json
import re
import shutil
import subprocess
import time

r = Path(__file__).resolve().parent
repo = r / 'worktree'
e = repo / 'experiments/results/inten_passive_trace'
out = e / 'evidence'
checks = r / 'native_checks_retry'
assert not out.exists()
while not (checks / 'exit.json').exists():
    time.sleep(30)
closed = json.loads((checks / 'exit.json').read_text())
assert closed['exit_code'] == 0, closed
source = json.loads((e / 'protocol.json').read_text())['native_source_commit']
assert source == closed['source_commit']
assert subprocess.check_output(['git', 'branch', '--show-current'], cwd=repo, text=True).strip() == 'codex/inten-passive-trace'
assert not subprocess.check_output(['git', 'status', '--porcelain'], cwd=repo, text=True).strip()
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-e', 'PYTHONDONTWRITEBYTECODE=1', '-w', str(repo), 'localization-zoo-dev']

def run(name, command):
    start = time.time()
    with (r / (name + '.log')).open('w') as stream:
        result = subprocess.run(command, cwd=repo, stdout=stream, stderr=subprocess.STDOUT)
    (r / (name + '_command.json')).write_text(json.dumps(dict(command=command,
        cwd=str(repo), source_commit=source, exit_code=result.returncode,
        started_unix=start, finished_unix=time.time()), indent=2) + '\n')
    assert result.returncode == 0, (name, result.returncode)

run('fixture', ['/workspace/localization-env/host-venv/bin/python', str(e / 'validate_fixture.py'),
                '--output-dir', str(r / 'native_fixture')])
run('preservation', ['/workspace/localization-env/host-venv/bin/python', str(e / 'verify_preservation.py'),
                     '--output', str(r / 'preservation.json')])
last_test = checks / 'LastTest.log'
testlog = last_test.read_text()
new_tests = sorted(set(re.findall(r'\[       OK \] (InTenLoamPaper\.Passive\w+)', testlog)))
assert len(new_tests) == 5, new_tests
ctest = (checks / 'ctest.log').read_text()
matched = re.search(r'100% tests passed, 0 tests failed out of (\d+)', ctest)
assert matched, ctest[-1500:]
out.mkdir()
imports = []

def copy(source_path, target):
    target.parent.mkdir(parents=True, exist_ok=True)
    raw = source_path.read_bytes()
    compress = source_path.suffix == '.log'
    if compress:
        target = target.with_name(target.name + '.gz')
        target.write_bytes(gzip.compress(raw, mtime=0))
        assert gzip.decompress(target.read_bytes()) == raw
    else:
        target.write_bytes(raw)
    imports.append(dict(source=str(source_path), destination=str(target.relative_to(repo)),
        bytes=len(raw), original_sha256=hashlib.sha256(raw).hexdigest(),
        stored_sha256=hashlib.sha256(target.read_bytes()).hexdigest(), lossless_gzip=compress))

for folder, destination in [(checks, out / 'native_checks'),
                            (r / 'native_fixture', out / 'fixture'),
                            (r / 'native_checks', out / 'initial_attempt')]:
    for path in sorted(folder.rglob('*')):
        if path.is_file():
            assert path.suffix not in ('.bin', '.pcd', '.a', '.so', '.o')
            copy(path, destination / path.relative_to(folder))
copy(last_test, out / 'native_checks/LastTest.log')
for name in ['fixture_command.json', 'fixture.log', 'preservation_command.json',
             'preservation.log', 'preservation.json']:
    copy(r / name, out / name)
for name in ['run_checks.py', 'import_native.py', 'retry_ctest.py', 'import_native_retry.py']:
    copy(r / name, out / name)
(out / 'import.json').write_text(json.dumps(dict(files=imports, exit_code=0), indent=2) + '\n')
(out / 'summary.json').write_text(json.dumps(dict(exit_code=0, native_source_commit=source,
    ctest_entries_passed=int(matched.group(1)), new_passive_test_cases_passed=new_tests,
    initial_ctest_exit_code=8, initial_failure='Root-owned /tmp/posture_profile.yaml was unwritable for user1000; unchecked fopen caused the existing posture test to segfault.',
    correction='Fresh user-owned TEST_TMPDIR, same source/binaries/tests. Targeted test and complete suite rerun.',
    original_build_commands='initial_attempt/commands.json',
    build_commands='native_checks/commands.json', fixture_comparison='fixture/summary.json',
    preservation='preservation.json', scope='Synthetic tracing-on parity and short-fixture tracing-disabled compatibility; no full-sequence trace or cause finding.'), indent=2) + '\n')
subprocess.run(['git', 'add', '-f', str(out)], cwd=repo, check=True)
subprocess.run(['git', 'diff', '--cached', '--check'], cwd=repo, check=True)
subprocess.run(['git', 'commit', '-m', 'Record passive trace tests and frozen short-fixture compatibility evidence'], cwd=repo, check=True)
print('Closed native evidence imported and committed before numeric documentation.', flush=True)
