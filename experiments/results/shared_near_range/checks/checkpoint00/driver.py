import sys
import shutil
from pathlib import Path

repo = Path('/workspace/localization_zoo')
sys.path.insert(0, str(repo / 'experiments/results/shared_near_range'))
from support import *

folder = R / 'checkpoint00_docs'
assert not folder.exists()
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
run('branch_whitespace', ['git', 'diff', '--check', PROTOCOL['base_commit']], folder)
run('native_evidence_unchanged', ['git', 'diff', '--exit-code',
    '10cf31616365b41878ec9311f0b705e798716c51', '--', str(E / 'sequences/00')], folder)
verify_binaries()
shutil.copy2(__file__, folder / 'driver.py')
shutil.copy2(R / 'document_checkpoint00.py', folder / 'documentation_driver.py')
write(folder / 'exit.json', dict(exit_code=0, scope='Committed 04/00 checkpoint docs/checks only. Native historical audit exits1 retained; full177 still incomplete.'))
