"""Check prior numeric results/publication and the scope of native changes."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess

E = Path(__file__).resolve().parent
REPO = E.parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    base = json.loads((E / 'protocol.json').read_text())['base_commit']
    expected = json.loads((REPO / 'experiments/results/shared_near_range/historical_aggregate_sha256.json').read_text())
    actual = {p.relative_to(REPO).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in (REPO / 'experiments/results').glob('*_matrix.json')}
    failures, publication = [], {}
    if actual != expected:
        failures.append('Historical aggregate hash inventory differs.')
    for path in ['README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
                 'evaluation/data/paper_reported_numbers.json']:
        before = subprocess.check_output(['git', 'show', base + ':' + path], cwd=REPO)
        now = (REPO / path).read_bytes()
        publication[path] = dict(identical=before == now, base_sha256=hashlib.sha256(before).hexdigest(),
                                 current_sha256=hashlib.sha256(now).hexdigest())
        if before != now:
            failures.append('Changed published result: ' + path)
    prior = subprocess.run(['git', 'diff', '--exit-code', base, '--', 'experiments/results',
                            ':(exclude)experiments/results/inten_passive_trace',
                            ':(exclude)experiments/results/index.json'], cwd=REPO, capture_output=True)
    if prior.returncode:
        failures.append('Prior study evidence changed.')
    changed = subprocess.check_output(['git', 'diff', '--name-only', base, '--',
              'common', 'evaluation/src', 'evaluation/CMakeLists.txt',
              ':(glob)papers/*/src/**', ':(glob)papers/*/include/**',
              ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt'],
              cwd=REPO, text=True).splitlines()
    allowed = ['papers/inten_loam/include/inten_loam/inten_loam_paper.h',
               'papers/inten_loam/src/inten_loam_paper.cpp',
               'papers/inten_loam/test/test_inten_loam_paper.cpp']
    if sorted(changed) != sorted(allowed):
        failures.append('Native change outside the passive trace implementation/tests.')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(dict(base_commit=base,
        historical_aggregate_count=len(expected), historical_aggregate_hashes_match=actual == expected,
        publication=publication, prior_results_identity_exit_code=prior.returncode,
        changed_native_paths=changed, allowed_native_paths=allowed,
        failures=failures, exit_code=int(bool(failures))), indent=2) + '\n')
    print(json.dumps(dict(failures=failures, exit_code=int(bool(failures)))))
    return int(bool(failures))


if __name__ == '__main__':
    raise SystemExit(main())
