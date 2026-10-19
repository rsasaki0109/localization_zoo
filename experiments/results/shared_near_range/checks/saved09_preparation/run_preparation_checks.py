from pathlib import Path
import gzip
import json
import os
import shutil
import subprocess
import time

R = Path('/workspace/localization-env/shared-range-final-audit')
REPO = R / 'worktree'
OUT = R / 'preparation09'
OUT.mkdir(exist_ok=False)
PYTHON = '/workspace/localization-env/host-venv/bin/python'
env = dict(os.environ, PATH=str(Path(PYTHON).parent) + os.pathsep + os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           MPLCONFIGDIR=str(R / 'matplotlib09'))
source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
receipts = []
commands = [
    ('gate_tests', [PYTHON, '-m', 'unittest', 'discover', '-s', 'tests', '-p',
                    'test_shared_range_saved02_gate.py', '-v'], 0),
    ('gate_tests_optimized', [PYTHON, '-O', '-m', 'unittest', 'discover', '-s', 'tests',
                              '-p', 'test_shared_range_saved02_gate.py', '-v'], 0),
    ('incomplete09_refused', [PYTHON, 'experiments/results/shared_near_range/diagnose_saved02.py',
                             '--sequence', '09', '--output-dir', str(OUT / 'refused_output')], 2),
]
for label, command, expected in commands:
    started = time.time()
    result = subprocess.run(command, cwd=REPO, env=env, capture_output=True, text=True)
    (OUT / (label + '.log')).write_text(result.stdout + result.stderr)
    receipt = dict(label=label, command=command, cwd=str(REPO), source_commit=source,
                   environment={k: env[k] for k in ('PYTHONDONTWRITEBYTECODE', 'OMP_NUM_THREADS',
                                                    'OPENBLAS_NUM_THREADS', 'MPLCONFIGDIR')},
                   started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
                   expected_exit_code=expected)
    receipts.append(receipt)
    (OUT / 'commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
    print(label, 'actual', result.returncode, 'expected', expected, flush=True)
    if result.returncode != expected:
        raise SystemExit(1)
    if label == 'incomplete09_refused':
        if (OUT / 'refused_output').exists() or 'Required input is not committed' not in result.stderr:
            raise SystemExit('Incomplete09 was not rejected by the committed-input gate before output')
target = REPO / 'experiments/results/shared_near_range/checks/saved09_preparation'
target.mkdir(parents=True, exist_ok=False)
shutil.copyfile(OUT / 'commands.json', target / 'commands.json')
shutil.copyfile(Path(__file__), target / 'run_preparation_checks.py')
for path in OUT.glob('*.log'):
    (target / (path.name + '.gz')).write_bytes(gzip.compress(path.read_bytes(), mtime=0))
(target / 'scope.json').write_text(json.dumps(dict(source_commit=source,
    scope='Synthetic publication-gate checks and real incomplete-input refusal only; no native run, live scan read, saved-path diagnosis or coverage increase.',
    native09_completion_claim=False, diagnostic_output_created=False, checks_exit_code=0), indent=2) + '\n')
