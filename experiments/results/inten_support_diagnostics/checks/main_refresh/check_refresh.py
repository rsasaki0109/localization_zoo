"""Revalidate the existing historical log analysis against integrated main."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time

REPO = Path('/workspace/localization-env/inten-support/worktree')
STUDY = REPO / 'experiments/results/inten_support_diagnostics'
BASE = 'a597e4c95614e733fc20977321fe4d6aea0fba46'
OLD = 'a834d44eb047729bda90426c4ac3b79edfbf13fd'
PYTHON = '/workspace/localization-env/host-venv/bin/python'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--phase', choices=('analysis', 'docs'), required=True)
parser.add_argument('--output-dir', type=Path, required=True)
args = parser.parse_args()
output = args.output_dir.resolve()
output.mkdir(parents=True, exist_ok=False)
head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
env_delta = dict(OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', PYTHONDONTWRITEBYTECODE='1',
                 MPLCONFIGDIR=str(output / 'mplconfig'),
                 PATH=str(Path(PYTHON).parent) + os.pathsep + os.environ['PATH'])
records = []


def write(name, value):
    (output / name).write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def run(label, command):
    start = time.time()
    with (output / (label + '.log')).open('w') as stream:
        result = subprocess.run(command, cwd=REPO, env=dict(os.environ, **env_delta),
                                stdout=stream, stderr=subprocess.STDOUT)
    records.append(dict(label=label, command=command, cwd=str(REPO), environment_overrides=env_delta,
                        checked_checkout_commit=head, started_unix=start, finished_unix=time.time(),
                        exit_code=result.returncode))
    write('commands.json', records)
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise RuntimeError(label + ' failed')


try:
    run('native_main_identity', ['git', 'diff', '--exit-code', BASE, '--', 'common', 'evaluation/src',
        'evaluation/CMakeLists.txt', 'evaluation/scripts', ':(glob)papers/*/src/**',
        ':(glob)papers/*/include/**', ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt',
        'CMakeLists.txt', 'python/src', 'ros2'])
    run('original_study_unchanged', ['git', 'diff', '--exit-code', OLD, '--',
        str(STUDY / 'analysis'), str(STUDY / 'protocol.json'),
        'experiments/results/runs', 'experiments/results/mesh_inten_kitti_coverage'])
    if args.phase == 'analysis':
        destination = output / 'analysis'
        run('reproduce_analysis', [PYTHON, str(STUDY / 'analyze.py'), '--output-dir', str(destination)])
        previous = json.loads((STUDY / 'analysis/summary.json').read_text())
        current = json.loads((destination / 'summary.json').read_text())
        keys = ('protocol', 'recorded_native_commit', 'recorded_native_binary_sha256',
                'fixed_variant_args', 'sequences', 'bad_length_intervals', 'exit_code')
        unchanged = {key: previous[key] == current[key] for key in keys}
        csv_files = {}
        for name in ('logged_steps.csv', 'bad_length_intervals.csv'):
            old_bytes = (STUDY / 'analysis' / name).read_bytes()
            new_bytes = (destination / name).read_bytes()
            csv_files[name] = dict(byte_identical=old_bytes == new_bytes,
                                   original_sha256=hashlib.sha256(old_bytes).hexdigest(),
                                   reproduced_sha256=hashlib.sha256(new_bytes).hexdigest())
        write('numerical_identity.json', dict(original_commit=OLD, reproduced_commit=head,
              semantic_fields_unchanged=unchanged, csv_files=csv_files,
              added_provenance=current['native_source_provenance'],
              scope='Replay of the same stored log/pose analysis; no scans, estimator run or new accuracy data.'))
        assert all(unchanged.values()) and all(v['byte_identical'] for v in csv_files.values())
    else:
        for name in ('refresh_study_docs', 'generate_full_variant_table', 'generate_paper_ratio_table'):
            run(name, [PYTHON, 'evaluation/scripts/' + name + '.py'])
        run('python_regressions', [PYTHON, '-m', 'unittest', 'discover', '-s', 'tests'])
        for name, flags in (('validate_showcase', []), ('generate_leaderboard', ['--check']),
                            ('generate_paper_ready_evidence', ['--check'])):
            run(name, [PYTHON, 'evaluation/scripts/' + name + '.py', *flags])
        hashes = json.loads((REPO / 'experiments/results/shared_near_range/historical_aggregate_sha256.json').read_text())
        changed = [name for name, expected in hashes.items()
                   if hashlib.sha256((REPO / name).read_bytes()).hexdigest() != expected]
        published = {}
        for name in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
                     'evaluation/data/paper_reported_numbers.json'):
            original = subprocess.check_output(['git', 'show', BASE + ':' + name], cwd=REPO)
            published[name] = original == (REPO / name).read_bytes()
        write('preservation.json', dict(base=BASE, historical_aggregate_count=len(hashes),
              changed_aggregates=changed, published_files_identical=published))
        assert not changed and all(published.values())
    run('branch_whitespace', ['git', 'diff', '--check', BASE])
    write('exit.json', dict(exit_code=0, checked_checkout_commit=head, phase=args.phase))
except BaseException as failure:
    write('exit.json', dict(exit_code=1, checked_checkout_commit=head, phase=args.phase, error=str(failure)))
    raise
