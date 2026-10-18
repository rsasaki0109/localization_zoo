from pathlib import Path
import gzip
import hashlib
import json
import re
import subprocess
import time

r = Path(__file__).resolve().parent
repo = r / 'worktree'
e = repo / 'experiments/results/inten_passive_trace'
raw = r / 'docs_checks'
raw.mkdir()
protocol = json.loads((e / 'protocol.json').read_text())
base, source = protocol['base_commit'], protocol['native_source_commit']
head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip()
assert (e / 'evidence/summary.json').read_bytes() == subprocess.check_output(
    ['git', 'show', 'HEAD:experiments/results/inten_passive_trace/evidence/summary.json'], cwd=repo)
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-e', 'PYTHONDONTWRITEBYTECODE=1', '-e', 'MPLCONFIGDIR=/tmp/inten-passive-trace',
          '-w', str(repo), 'localization-zoo-dev', '/opt/localization-venv/bin/python']
receipts = []

def run(name, command):
    start = time.time()
    with (raw / (name + '.log')).open('w') as log:
        result = subprocess.run(command, cwd=repo, stdout=log, stderr=subprocess.STDOUT)
    receipts.append(dict(name=name, command=command, cwd=str(repo), checked_head=head,
        native_source_commit=source, exit_code=result.returncode,
        started_unix=start, finished_unix=time.time()))
    (raw / 'commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
    print(name, result.returncode, flush=True)
    assert result.returncode == 0, (name, result.returncode)

run('document_committed_results', ['/workspace/localization-env/host-venv/bin/python', str(r / 'document_results.py')])
for name, argv in [
    ('refresh_study_docs', ['evaluation/scripts/refresh_study_docs.py']),
    ('generate_full_variant_table', ['evaluation/scripts/generate_full_variant_table.py']),
    ('generate_paper_ratio_table', ['evaluation/scripts/generate_paper_ratio_table.py']),
    ('python_unittest', ['-m', 'unittest', 'discover', '-s', 'tests']),
    ('showcase', ['evaluation/scripts/validate_showcase.py']),
    ('leaderboard', ['evaluation/scripts/generate_leaderboard.py', '--check']),
    ('paper_ready', ['evaluation/scripts/generate_paper_ready_evidence.py', '--check']),
]:
    run(name, docker + argv)
run('preservation', ['/workspace/localization-env/host-venv/bin/python', str(e / 'verify_preservation.py'),
                     '--output', str(raw / 'preservation.json')])
run('native_source_identity', ['git', 'diff', '--exit-code', source, '--',
    'common', 'evaluation/src', 'evaluation/CMakeLists.txt', ':(glob)papers/*/src/**',
    ':(glob)papers/*/include/**', ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt'])
run('native_evidence_identity', ['git', 'diff', '--exit-code', head, '--', str(e / 'evidence')])
run('branch_whitespace', ['git', 'diff', '--check', base])
generated = ['docs/' + name + '.md' for name in ('decisions', 'experiments', 'interfaces',
    'paper_assets', 'paper_captions', 'paper_comparison', 'paper_roadmap', 'paper_tracks',
    'reproduction_status', 'variant_analysis')] + ['experiments/results/index.json']
timestamp_checks = []
for name in generated:
    before = subprocess.check_output(['git', 'show', base + ':' + name], cwd=repo, text=True)
    after = (repo / name).read_text()
    if name.endswith('.json'):
        a, b = json.loads(before), json.loads(after)
        a.pop('generated_at')
        b.pop('generated_at')
    else:
        pattern = r'(?m)^(_Generated at |> Generated: )\S+'
        a, ca = re.subn(pattern, r'\1<TIMESTAMP>', before)
        b, cb = re.subn(pattern, r'\1<TIMESTAMP>', after)
        assert ca == cb == 1, name
    assert a == b, name
    timestamp_checks.append(dict(path=name, identical_except_generation_timestamp=True))
(raw / 'generated_identity.json').write_text(json.dumps(dict(checks=timestamp_checks, exit_code=0), indent=2) + '\n')
for name in ['check_docs.py', 'document_results.py']:
    (raw / name).write_bytes((r / name).read_bytes())
(raw / 'exit.json').write_text(json.dumps(dict(exit_code=0, checked_head=head,
    scope='Required generators, Python/docs, immutable publication/prior studies, native source/evidence and generation-timestamp-only checks.')) + '\n')
destination = e / 'checks/docs'
destination.mkdir(parents=True)

def import_logs():
    files = []
    for path in sorted(raw.iterdir()):
        if not path.is_file():
            continue
        data = path.read_bytes()
        target = destination / (path.name + '.gz' if path.suffix == '.log' else path.name)
        target.write_bytes(gzip.compress(data, mtime=0) if path.suffix == '.log' else data)
        if path.suffix == '.log':
            assert gzip.decompress(target.read_bytes()) == data
        files.append(dict(source=str(path), destination=str(target.relative_to(repo)),
            original_sha256=hashlib.sha256(data).hexdigest(),
            stored_sha256=hashlib.sha256(target.read_bytes()).hexdigest()))
    (destination / 'import.json').write_text(json.dumps(dict(files=files, exit_code=0), indent=2) + '\n')

import_logs()
paths = ['docs/roadmap.md', 'docs/inten_loam_passive_trace.md', 'papers/inten_loam/README.md', *generated]
subprocess.run(['git', 'add', '--', *paths], cwd=repo, check=True)
subprocess.run(['git', 'add', '-f', str(e)], cwd=repo, check=True)
run('staged_branch_whitespace', ['git', 'diff', '--cached', '--check', base])
import_logs()
subprocess.run(['git', 'add', '-f', str(e)], cwd=repo, check=True)
subprocess.run(['git', 'diff', '--cached', '--check', base], cwd=repo, check=True)
subprocess.run(['git', 'commit', '-m', 'Document validated passive InTEn traces and required checks'], cwd=repo, check=True)
print('Documentation and required checks committed; ready for push and PR review.', flush=True)
