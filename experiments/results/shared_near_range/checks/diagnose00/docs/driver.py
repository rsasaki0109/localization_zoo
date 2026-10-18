"""Check persistent current-path notes, then run required regeneration/checks."""
import json
from pathlib import Path
import subprocess
import sys

E = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(E))
from support import REPO, PROTOCOL, CONTAINER_PY, docker, run
from finish_docs import strip_old_blocks

path = E / 'diagnostics/00/summary.json'
raw = subprocess.check_output(['git', 'show', 'HEAD:' + path.relative_to(REPO).as_posix()], cwd=REPO)
assert raw == path.read_bytes()
report = json.loads(raw)
modules = {'A-LOAM': 'aloam', 'F-LOAM': 'floam', 'LeGO-LOAM': 'lego_loam',
           'Mesh-LOAM': 'mesh_loam', 'VLOM': 'vlom'}
marker = 'SHARED_NEAR_RANGE_00_DIAGNOSTICS'
for row in report['rows']:
    text = (REPO / 'papers' / modules[row['method']] / 'README.md').read_text()
    block = text.split('<!-- ' + marker + ' -->')[1].split('<!-- /' + marker + ' -->')[0]
    b = row['largest_position_error_growth_block']
    for value in (f"{row['steps_length_off_over_threshold']}/{row['moving_step_count']}",
                  f"{row['steps_vector_off_over_threshold']}/{row['moving_step_count']}",
                  f"{b['position_error_start_m']:.6f}", f"{b['position_error_end_m']:.6f}"):
        assert value in block
    assert 'not official KITTI RTE' in block and 'does not locate the cause' in block
    assert block in strip_old_blocks(text), 'Finalizer would erase committed diagnostic notes.'

folder = Path(__file__).parent
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
run('preservation', [sys.executable, str(E / 'verify_preservation.py'), '--output',
    str(folder / 'preservation.json')], folder)
run('branch_whitespace', ['git', 'diff', '--check', PROTOCOL['base_commit']], folder)
