import sys
import shutil
import re
import json
import importlib.util
from pathlib import Path

repo = Path('/workspace/localization_zoo')
sys.path.insert(0, str(repo / 'experiments/results/shared_near_range'))
from support import *

folder = R / 'checkpoint01_docs'
assert not folder.exists()
folder.mkdir()
prefix = docker() + ['env', 'MPLCONFIGDIR=/tmp/localization-range-plots', CONTAINER_PY]
for name, argv in [
    ('refresh_study_docs', ['evaluation/scripts/refresh_study_docs.py']),
    ('generate_full_variant_table', ['evaluation/scripts/generate_full_variant_table.py']),
    ('generate_paper_ratio_table', ['evaluation/scripts/generate_paper_ratio_table.py']),
    ('python', ['-m', 'unittest', 'discover', '-s', 'tests']),
    ('showcase', ['evaluation/scripts/validate_showcase.py']),
    ('leaderboard', ['evaluation/scripts/generate_leaderboard.py', '--check']),
    ('paper_ready', ['evaluation/scripts/generate_paper_ready_evidence.py', '--check']),
]:
    run(name, prefix + argv, folder)
run('preservation', [sys.executable, str(E / 'verify_preservation.py'),
                    '--output', str(folder / 'preservation.json')], folder)
closed_revision = json.loads((E / 'checkpoints/01/summary.json').read_text())['evidence_revision']
run('closed_native_evidence_unchanged', ['git', 'diff', '--exit-code',
    closed_revision, '--', str(E / 'sequences/01')], folder)
run('diagnostic_evidence_unchanged', ['git', 'diff', '--exit-code',
    '1426a851c9f021cbffae239e814b6379a5788b25', '--',
    *[str(p) for p in (E / 'diagnostics/01').rglob('*') if p.is_file() and p.name != 'README.md']], folder)
spec = importlib.util.spec_from_file_location('finish_docs', E / 'finish_docs.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
checks = []
for method in ('ct_icp', 'mesh_loam'):
    text = (REPO / 'papers' / method / 'README.md').read_text()
    pattern = r'<!-- SHARED_NEAR_RANGE_01_DIAGNOSTICS -->.*?<!-- /SHARED_NEAR_RANGE_01_DIAGNOSTICS -->'
    block = re.search(pattern, text, re.S).group()
    stripped = module.strip_old_blocks(text)
    assert block in stripped and '<!-- SHARED_NEAR_RANGE_REPLAY -->' not in stripped
    checks.append(dict(method=method, persistent_01_diagnostics_preserved=True, temporary_01_block_removed=True))
pattern = r'<!-- SHARED_NEAR_RANGE_00_DIAGNOSTICS -->.*?<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->'
for method in ('aloam', 'floam', 'lego_loam', 'mesh_loam', 'vlom'):
    path = 'papers/' + method + '/README.md'
    before = subprocess.check_output(['git', 'show', '0eeb969454e77ff8ca0541e4763b1e8cf552b907:' + path], cwd=REPO, text=True)
    after = (REPO / path).read_text()
    assert re.search(pattern, before, re.S).group() == re.search(pattern, after, re.S).group()
    checks.append(dict(method=method, existing_00_diagnostics_unchanged=True))
write(folder / 'diagnostic_block_preservation.json', dict(checks=checks, exit_code=0,
      scope='Exercise the actual sealed completion renderer without changing its files.'))
run('branch_whitespace', ['git', 'diff', '--check', PROTOCOL['base_commit']], folder)
run('full_native_source_identity', ['git', 'diff', '--exit-code', SOURCE, '--', 'common',
    'evaluation/src', 'evaluation/CMakeLists.txt', ':(glob)papers/*/src/**',
    ':(glob)papers/*/include/**', ':(glob)papers/*/test/**',
    ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt'], folder)
verify_binaries()
shutil.copy2(__file__, folder / 'driver.py')
shutil.copy2(R / 'document_checkpoint01.py', folder / 'documentation_driver.py')
write(folder / 'exit.json', dict(exit_code=0,
      scope='Committed cleaned04/00/01 checkpoint and prior saved-path diagnostics; whole177 remains incomplete.'))
