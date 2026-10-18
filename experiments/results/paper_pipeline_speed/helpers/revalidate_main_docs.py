"""Regenerate required documents and verify preserved published evidence."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time

REPO = Path(__file__).resolve().parents[4]
E = REPO / 'experiments/results/paper_pipeline_speed'
BASE = '7505a2914328d53ac9b319d5f8042189b378d457'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output-dir', type=Path, required=True)
args = parser.parse_args()
out = args.output_dir.resolve()
out.mkdir(parents=True, exist_ok=False)
head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-e', 'PYTHONDONTWRITEBYTECODE=1', '-e', 'MPLCONFIGDIR=/tmp/speed-main-refresh',
          '-w', str(REPO), 'localization-zoo-dev', '/opt/localization-venv/bin/python']
commands = []
for name in ('refresh_study_docs', 'generate_full_variant_table', 'generate_paper_ratio_table'):
    commands.append((name, docker + ['evaluation/scripts/' + name + '.py']))
commands.append(('python_unittest', docker + ['-m', 'unittest', 'discover', '-s', 'tests']))
for name, flags in [('validate_showcase', []), ('generate_leaderboard', ['--check']),
                    ('generate_paper_ready_evidence', ['--check'])]:
    commands.append((name, docker + ['evaluation/scripts/' + name + '.py', *flags]))
commands.append(('whitespace', ['git', 'diff', '--check', BASE]))
records = []
try:
    for label, command in commands:
        started = time.time()
        with (out / (label + '.log')).open('w') as log:
            result = subprocess.run(command, cwd=REPO, stdout=log, stderr=subprocess.STDOUT)
        records.append(dict(label=label, command=command, cwd=str(REPO),
                            checked_checkout_commit=head, started_unix=started,
                            finished_unix=time.time(), exit_code=result.returncode))
        (out / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
        print(label, result.returncode, flush=True)
        if result.returncode:
            raise SystemExit(result.returncode)
    historical = json.loads((E / 'historical_aggregate_sha256.json').read_text())
    changed = [name for name, digest in historical.items()
               if hashlib.sha256((REPO / name).read_bytes()).hexdigest() != digest]
    files = ['README.md', 'docs/index.html', 'evaluation/data/paper_reported_numbers.json',
             'docs/assets/paper/paper_ratio_table.csv']
    preserved = {name: subprocess.check_output(['git', 'show', BASE + ':' + name], cwd=REPO)
                 == (REPO / name).read_bytes() for name in files}
    (out / 'preservation.json').write_text(json.dumps(dict(base=BASE,
        historical_aggregate_count=len(historical), changed_aggregates=changed,
        unchanged_published_evidence=preserved), indent=2) + '\n')
    assert not changed and all(preserved.values())
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=0, checked_checkout_commit=head)) + '\n')
except BaseException as error:
    code = error.code if isinstance(error, SystemExit) and isinstance(error.code, int) else 1
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=code, error=str(error))) + '\n')
    raise
