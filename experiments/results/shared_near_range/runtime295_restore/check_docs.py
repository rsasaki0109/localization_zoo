from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import time

out = Path('/workspace/localization-env/shared-near-range/restore295')
repo = out / 'worktree'
check = out / 'checks'
check.mkdir(exist_ok=False)
python = '/workspace/localization-env/host-venv/bin/python'
env = dict(os.environ, PATH=str(Path(python).parent) + os.pathsep + os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           MPLCONFIGDIR=str(out / 'check_matplotlib'))
paths = subprocess.check_output(['git', 'ls-files', 'docs', 'README.md', 'experiments/results/index.json'],
                                cwd=repo, text=True).splitlines()
before = {p: (repo / p).read_bytes() for p in paths if (repo / p).is_file()}
receipts = []


def run(label, args):
    started = time.time()
    with (check / (label + '.log')).open('x') as stream:
        result = subprocess.run(args, cwd=repo, env=env, stdout=stream, stderr=subprocess.STDOUT)
    receipts.append(dict(label=label, command=args, cwd=str(repo), started_unix=started,
                         finished_unix=time.time(), exit_code=result.returncode))
    (check / 'commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise SystemExit(result.returncode)


for label, script in [('refresh', 'refresh_study_docs.py'), ('full_variant', 'generate_full_variant_table.py'),
                      ('paper_ratio', 'generate_paper_ratio_table.py')]:
    run(label, [python, 'evaluation/scripts/' + script])

dates = re.compile(rb'\d{4}-\d{2}-\d{2}(?:T| )\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:\d{2})?')
disposition = []
for path, old in before.items():
    new = (repo / path).read_bytes()
    if new == old:
        continue
    only_timestamp = dates.sub(b'<timestamp>', old) == dates.sub(b'<timestamp>', new)
    disposition.append(dict(path=path, before_sha256=hashlib.sha256(old).hexdigest(),
                            regenerated_sha256=hashlib.sha256(new).hexdigest(),
                            timestamp_only=only_timestamp, retained_prior_bytes=only_timestamp))
    if only_timestamp:
        (repo / path).write_bytes(old)
(check / 'generation_disposition.json').write_text(json.dumps(disposition, indent=2) + '\n')

run('bootstrap_guards', [python, 'experiments/results/shared_near_range/runtime295_restore/test_launch_backup.py'])
run('bootstrap_guards_optimized', [python, '-O', 'experiments/results/shared_near_range/runtime295_restore/test_launch_backup.py'])
run('python', [python, '-m', 'unittest', 'discover', '-s', 'tests'])
run('showcase', [python, 'evaluation/scripts/validate_showcase.py'])
run('leaderboard', [python, 'evaluation/scripts/generate_leaderboard.py', '--check'])
run('paper_ready', [python, 'evaluation/scripts/generate_paper_ready_evidence.py', '--check'])
run('whitespace', ['git', 'diff', '--check', '9ed935a045989d1806c0a19baa98794fa482eb54'])

historical = json.loads((repo / 'experiments/results/shared_near_range/historical_aggregate_sha256.json').read_text())
actual = {p.relative_to(repo).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
          for p in (repo / 'experiments/results').glob('*_matrix.json')}
if actual != historical:
    raise RuntimeError('Historical aggregate bytes changed')
published = {}
for path in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
             'docs/assets/paper/full_variant_results.csv', 'evaluation/data/paper_reported_numbers.json'):
    base = subprocess.check_output(['git', 'show', '9ed935a045989d1806c0a19baa98794fa482eb54:' + path], cwd=repo)
    current = (repo / path).read_bytes()
    if base != current:
        raise RuntimeError('Published bytes changed: ' + path)
    published[path] = dict(sha256=hashlib.sha256(current).hexdigest(), identical=True)
subprocess.run(['git', 'diff', '--exit-code', '9ed935a045989d1806c0a19baa98794fa482eb54', '--',
                'common', 'papers', 'evaluation', 'CMakeLists.txt',
                'experiments/results/shared_coarse_start', 'experiments/results/shared_degeneracy_remap'],
               cwd=repo, check=True)
(check / 'preservation.json').write_text(json.dumps(dict(exit_code=0,
    historical_aggregate_count=len(historical), historical_aggregate_hashes_match=True,
    publication=published, native_method_evaluation_and_prior_study_bytes_preserved=True), indent=2) + '\n')
print('preservation', 0, flush=True)
