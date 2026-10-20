"""Read-only guards before publishing the finite final-integration protocol."""
from pathlib import Path
import hashlib
import json
import subprocess

REPO = Path('/workspace/localization-env/shared-range-final-audit/worktree')
PRIMARY = Path('/workspace/localization_zoo')
E = REPO / 'experiments/results/shared_near_range'

def git(repo, *args):
    return subprocess.check_output(['git', *args], cwd=repo)

def digest(path):
    value = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(block)
    return value.hexdigest()

def require(value, message):
    if not value:
        raise RuntimeError(message)

p = json.loads((E / 'final_integration_protocol.json').read_text())
require(git(REPO, 'show', 'HEAD:experiments/results/shared_near_range/final_integration_protocol.json')
        == (E / 'final_integration_protocol.json').read_bytes(), 'Protocol differs from commit')
selection = json.loads((E / 'selection.json').read_text())
require(p['planned_cell_count'] == len(selection['rows']), 'Selected-cell count differs')
require(p['selected_cell_sha256'] == digest(E / 'selection.json'), 'Selection pin differs')
require(p['completion_driver']['attempt'] == '03' and not p['completion_driver']['push'],
        'Original finalizer invocation differs')
sealed = {}
for name, expected in p['sealed_file_sha256'].items():
    path = PRIMARY / 'experiments/results/shared_near_range' / name
    old = git(PRIMARY, 'show', 'c3698e6700199b352d846f70a6580ad007ea35cc:'
              + path.relative_to(PRIMARY).as_posix())
    require(path.read_bytes() == old and digest(path) == expected, 'Sealed file differs: ' + name)
    sealed[name] = expected
changed = git(PRIMARY, 'diff', '--name-only', p['native_implementation_commit'], '--',
              'common', 'papers', 'evaluation', 'CMakeLists.txt').decode().splitlines()
require(all(path.endswith('/README.md') for path in changed), 'Frozen native/evaluation source differs')
binaries = {}
for name in ('baseline', 'candidate'):
    pin = p['native_' + name]
    require(digest(Path(pin['binary'])) == pin['sha256'], 'Frozen executable differs: ' + name)
    binaries[name] = pin['sha256']
historical = json.loads((E / 'historical_aggregate_sha256.json').read_text())
actual = {path.relative_to(REPO).as_posix(): digest(path)
          for path in (REPO / 'experiments/results').glob('*_matrix.json')}
require(actual == historical, 'Historical aggregate bytes differ')
published = {}
for path in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
             'docs/assets/paper/full_variant_results.csv', 'evaluation/data/paper_reported_numbers.json'):
    require((REPO / path).read_bytes() == git(REPO, 'show', p['integration_base'] + ':' + path),
            'Published bytes differ: ' + path)
    published[path] = digest(REPO / path)
result = dict(exit_code=0, source_commit=git(REPO, 'rev-parse', 'HEAD').decode().strip(),
              primary_head=git(PRIMARY, 'rev-parse', 'HEAD').decode().strip(),
              selection_preserved=True, historical_aggregate_count=len(historical),
              historical_aggregate_hashes_match=True, sealed_file_sha256=sealed,
              frozen_binary_sha256=binaries, published_sha256=published,
              scope='Read-only startup preservation checks, not native replay or audit completion.')
print(json.dumps(result, indent=2))
