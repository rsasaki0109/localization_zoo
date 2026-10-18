"""Regenerate/check docs after importing the closed VLOM00 frozen control."""
import json
from pathlib import Path
import subprocess
import sys

E = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(E))
from support import REPO, PROTOCOL, CONTAINER_PY, digest, docker, run

for control in ('00_closed_controls', '00_vlom_control'):
    report_path = E / 'checks' / control / 'summary.json'
    raw = subprocess.check_output(['git', 'show', 'HEAD:' + report_path.relative_to(REPO).as_posix()], cwd=REPO)
    assert raw == report_path.read_bytes()
    report = json.loads(raw)
    for name, expected in report['evidence_files'].items():
        path = REPO / name
        committed = subprocess.check_output(['git', 'show', 'HEAD:' + name], cwd=REPO)
        assert committed == path.read_bytes() and digest(path) == expected
for module in ('aloam', 'floam', 'lego_loam', 'vlom'):
    block = (REPO / 'papers' / module / 'README.md').read_text().split('<!-- SHARED_NEAR_RANGE_00_PRIMARY -->')[1]
    assert 'frozen comparison complete' in block and '**zero**' in block
    assert 'historical audit exits **1**' in block and 'original cause remains unresolved' in block
vlom = (REPO / 'papers/vlom/README.md').read_text().split('<!-- SHARED_NEAR_RANGE_00_PRIMARY -->')[1]
assert '1f7b554f9dfe5878067dc42ae18ed45c51a58e35' in vlom
for value in ('0.875872%', '0.914174%', '9.391448 m', '9.520570 m'):
    assert value in vlom
assert 'RPE worsens. ATE worsens too' in vlom
mesh = (REPO / 'papers/mesh_loam/README.md').read_text().split('<!-- SHARED_NEAR_RANGE_00_PRIMARY -->')[1]
assert 'comparison is still running' in mesh and '**unresolved**' in mesh

folder = Path(__file__).parent
prefix = docker() + ['env', 'MPLCONFIGDIR=/tmp/localization-range-plots', CONTAINER_PY]
commands = [
    ('refresh_study_docs', ['evaluation/scripts/refresh_study_docs.py']),
    ('generate_full_variant_table', ['evaluation/scripts/generate_full_variant_table.py']),
    ('generate_paper_ratio_table', ['evaluation/scripts/generate_paper_ratio_table.py']),
    ('python', ['-m', 'unittest', 'discover', '-s', 'tests']),
    ('showcase', ['evaluation/scripts/validate_showcase.py']),
    ('leaderboard', ['evaluation/scripts/generate_leaderboard.py', '--check']),
    ('paper_ready', ['evaluation/scripts/generate_paper_ready_evidence.py', '--check']),
]
for name, argv in commands:
    run(name, prefix + argv, folder)
run('preservation', [sys.executable, str(E / 'verify_preservation.py'), '--output',
    str(folder / 'preservation.json')], folder)
run('branch_whitespace', ['git', 'diff', '--check', PROTOCOL['base_commit']], folder)
