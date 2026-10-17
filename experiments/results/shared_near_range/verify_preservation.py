"""Verify immutable historical results, publication and frozen native sources."""
from support import *
import argparse

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
expected = json.loads((E / 'historical_aggregate_sha256.json').read_text())
actual = {p.relative_to(REPO).as_posix(): digest(p)
          for p in (REPO / 'experiments/results').glob('*_matrix.json')}
failures = []
if actual != expected:
    failures.append(dict(changed_aggregates=[key for key in set(actual) | set(expected)
                                            if actual.get(key) != expected.get(key)]))
published = {}
for path in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
             'evaluation/data/paper_reported_numbers.json'):
    before = subprocess.check_output(['git', 'show', PROTOCOL['base_commit'] + ':' + path], cwd=REPO)
    now = (REPO / path).read_bytes()
    published[path] = dict(base_sha256=hashlib.sha256(before).hexdigest(),
                           current_sha256=hashlib.sha256(now).hexdigest(), identical=before == now)
    if before != now:
        failures.append(dict(changed_publication=path))
command = ['git', 'diff', '--exit-code', PROTOCOL['base_commit'], '--',
           'experiments/results/shared_coarse_start', 'experiments/results/shared_degeneracy_remap']
old = subprocess.run(command, cwd=REPO, capture_output=True, text=True)
if old.returncode:
    failures.append(dict(changed_prior_studies=old.stdout, stderr=old.stderr))
try:
    verify_binaries()
except (AssertionError, subprocess.CalledProcessError) as error:
    failures.append(dict(native_identity=str(error)))
write(args.output, dict(implementation_commit=SOURCE, base_commit=PROTOCOL['base_commit'],
      historical_aggregate_count=len(expected), historical_aggregate_hashes_match=actual == expected,
      published_files=published, prior_studies_identity_command=command,
      prior_studies_identity_exit_code=old.returncode, failures=failures, exit_code=int(bool(failures)),
      scope='Source, frozen executables and immutable results, not replay completion.'))
print(json.dumps(dict(failures=failures, exit_code=int(bool(failures)))))
raise SystemExit(int(bool(failures)))
