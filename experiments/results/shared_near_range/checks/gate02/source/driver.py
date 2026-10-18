"""Record actual checks for the KITTI02 diagnosis source, without native execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time

ROOT = Path('/workspace/localization_zoo')
RUNTIME = Path('/workspace/localization-env/shared-near-range')
STUDY = ROOT / 'experiments/results/shared_near_range'
PYTHON = '/workspace/localization-env/host-venv/bin/python'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--phase', choices=('source', 'post_import'), required=True)
parser.add_argument('--output-dir', type=Path, required=True)
args = parser.parse_args()
output = args.output_dir.resolve()
output.mkdir(parents=True, exist_ok=False)
head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
environment = dict(OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
                   PYTHONDONTWRITEBYTECODE='1', MPLCONFIGDIR=str(output / 'mplconfig'),
                   PATH=str(Path(PYTHON).parent) + os.pathsep + os.environ['PATH'])
records = []


def write(name, value):
    (output / name).write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def run(label, command, expected=0):
    start = time.time()
    with (output / (label + '.log')).open('w') as stream:
        result = subprocess.run(command, cwd=ROOT, env=dict(os.environ, **environment),
                                stdout=stream, stderr=subprocess.STDOUT)
    records.append(dict(label=label, command=command, cwd=str(ROOT), environment_overrides=environment,
                        checked_checkout_commit=head, started_unix=start, finished_unix=time.time(),
                        exit_code=result.returncode, expected_exit_code=expected))
    write('commands.json', records)
    print(label, result.returncode, flush=True)
    if result.returncode != expected:
        raise RuntimeError(label + ' returned an unexpected exit')


try:
    if args.phase == 'source':
        run('diagnostic_regressions', [PYTHON, '-m', 'unittest', 'discover', '-s', 'tests',
                                      '-p', 'test_shared_range_saved02_gate.py', '-v'])
        run('optimized_diagnostic_regressions', [PYTHON, '-O', '-m', 'unittest', 'discover',
                                                '-s', 'tests', '-p', 'test_shared_range_saved02_gate.py', '-v'])
        destination = output / 'forbidden_live_output'
        run('uncommitted_live_candidate_rejected', [PYTHON, str(STUDY / 'diagnose_saved02.py'),
                                                   '--output-dir', str(destination)], expected=2)
        message = (output / 'uncommitted_live_candidate_rejected.log').read_text()
        assert not destination.exists()
        assert 'Required input is not committed' in message and 'sequences/02/audit/report.json' in message
        write('live_gate.json', dict(exit_code=2, expected_rejection=True, output_exists=False,
                                     scope='Command checks committed evidence only; no live scan is read.'))
    else:
        for name in ('refresh_study_docs', 'generate_full_variant_table', 'generate_paper_ratio_table'):
            run(name, [PYTHON, 'evaluation/scripts/' + name + '.py'])
        run('python_unittest', [PYTHON, '-m', 'unittest', 'discover', '-s', 'tests'])
        for name, options in (('validate_showcase', []), ('generate_leaderboard', ['--check']),
                              ('generate_paper_ready_evidence', ['--check'])):
            run(name, [PYTHON, 'evaluation/scripts/' + name + '.py', *options])
        run('published_preservation', [PYTHON, str(STUDY / 'verify_preservation.py'),
                                        '--output', str(output / 'preservation.json')])
        run('prior_diagnostic_evidence_unchanged', ['git', 'diff', '--exit-code',
            '6ca512f11c5dc1385c8c394d596f500d0c844f2d', '--', str(STUDY / 'diagnostics/00'),
            str(STUDY / 'diagnostics/01'), ':(glob)papers/*/README.md'])
    native_command = ['git', 'diff', '--exit-code', '0b079dbbcb7c6f58c1d8367ade881c15f003e525', '--',
                      'common', 'evaluation/src', 'evaluation/CMakeLists.txt', ':(glob)papers/*/src/**',
                      ':(glob)papers/*/include/**', ':(glob)papers/*/test/**',
                      ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt']
    run('recursive_native_identity', native_command)
    run('branch_whitespace', ['git', 'diff', '--check', 'e13aab37258ce73c216302c1cc5b61a5d5385ed3'])
    pins = json.loads((RUNTIME / 'completion_state.json').read_text())
    checks = {name: hashlib.sha256((STUDY / name).read_bytes()).hexdigest() == expected
              for name, expected in pins['sealed_file_sha256'].items()}
    for name in ('candidate', 'baseline'):
        reference = json.loads((STUDY / (name + '.json')).read_text())
        checks[name + '_binary'] = hashlib.sha256(Path(reference['binary']).read_bytes()).hexdigest() == reference['sha256']
    assert all(checks.values())
    write('frozen_integrity.json', dict(checks=checks, exit_code=0, checked_unix=time.time()))
    write('exit.json', dict(exit_code=0, checked_checkout_commit=head, phase=args.phase,
                           scope='Diagnosis source/gates only; no completed02 dataset coverage claim.'))
except BaseException as failure:
    write('exit.json', dict(exit_code=1, checked_checkout_commit=head, phase=args.phase, error=str(failure)))
    raise
