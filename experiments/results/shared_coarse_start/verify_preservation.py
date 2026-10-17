"""Verify publication preservation and the experiment's fixed native settings."""
from pathlib import Path
from datetime import datetime, timezone
import argparse
import hashlib
import json
import subprocess
import sys

E = Path(__file__).resolve().parent
REPO = E.parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    protocol = json.loads((E / 'protocol.json').read_text())
    expected = json.loads((E / 'historical_aggregate_sha256.json').read_text())
    failures = []

    def sha(data):
        return hashlib.sha256(data).hexdigest()

    actual = {p.relative_to(REPO).as_posix(): sha(p.read_bytes())
              for p in (REPO / 'experiments/results').glob('*_matrix.json')}
    if actual != expected:
        failures.append({'historical_aggregates': sorted(
            path for path in set(actual) | set(expected) if actual.get(path) != expected.get(path))})

    published = {}
    for path in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
                 'evaluation/data/paper_reported_numbers.json'):
        before = subprocess.check_output(['git', 'show', f"{protocol['base_commit']}:{path}"], cwd=REPO)
        after = (REPO / path).read_bytes()
        published[path] = {'base_sha256': sha(before), 'current_sha256': sha(after)}
        if before != after:
            failures.append({'published_file_changed': path})

    native_command = ['git', 'diff', '--exit-code', protocol['implementation_commit'], '--',
                      'common', 'evaluation/src', 'papers/*/src', 'papers/*/include',
                      'papers/*/test', 'papers/*/CMakeLists.txt',
                      'CMakeLists.txt', 'ros2/localization_zoo_ros/CMakeLists.txt']
    native = subprocess.run(native_command, cwd=REPO, capture_output=True, text=True)
    if native.returncode:
        failures.append({'native_source_changed': native.stdout, 'stderr': native.stderr})

    prior_study_command = ['git', 'diff', '--exit-code', protocol['base_commit'], '--',
                           'experiments/results/shared_degeneracy_remap']
    prior_study = subprocess.run(prior_study_command, cwd=REPO, capture_output=True, text=True)
    if prior_study.returncode:
        failures.append({'prior_study_changed': prior_study.stdout, 'stderr': prior_study.stderr})

    manifests = {}
    for seq in protocol['sequences']:
        path = REPO / 'experiments' / f'suma_kitti_seq_{seq}_full_shared_coarse_start_matrix.json'
        content = path.read_bytes()
        manifest = json.loads(content)
        variants = manifest['variants']
        valid = (len(variants) == 1 and variants[0]['id'] == 'shared_coarse_start_1x5x10'
                 and variants[0]['args'] == protocol['suma_fixed_arguments']
                 and manifest['stable_interface']['methods'] == 'suma'
                 and manifest['problem']['dataset'] == {
                     'pcd_dir': f'dogfooding_results/kitti_seq_{seq}_full',
                     'gt_csv': f'experiments/reference_data/kitti_seq_{seq}_full_gt.csv'})
        manifests[path.relative_to(REPO).as_posix()] = sha(content)
        if not valid:
            failures.append({'manifest_configuration_mismatch': str(path)})

    payload = {'command': [sys.executable, *sys.argv], 'cwd': str(Path.cwd()),
               'checked_checkout_commit': subprocess.check_output(
                   ['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip(),
               'generated_at': datetime.now(timezone.utc).isoformat(),
               'script_sha256': sha(Path(__file__).read_bytes()),
               'base_commit': protocol['base_commit'],
               'implementation_commit': protocol['implementation_commit'],
               'historical_aggregate_count': len(expected),
               'historical_aggregate_hashes_match': actual == expected,
               'published_files': published, 'manifests': manifests,
               'native_identity_command': native_command,
               'native_identity_exit_code': native.returncode,
               'prior_study_identity_command': prior_study_command,
               'prior_study_identity_exit_code': prior_study.returncode,
               'failures': failures, 'exit_code': int(bool(failures)),
               'scope': 'Publication bytes, aggregate hashes, manifest settings and native source identity; not a benchmark-completion claim.'}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + '\n')
    print(json.dumps({'exit_code': payload['exit_code'], 'failures': failures}))
    return payload['exit_code']


if __name__ == '__main__':
    raise SystemExit(main())
