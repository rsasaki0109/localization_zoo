from pathlib import Path
import json
import os
import subprocess
import time

R = Path('/workspace/localization-env/shared-range-final-audit')
REPO = R / 'worktree'
E = REPO / 'experiments/results/shared_near_range'
# This driver is only invoked after the original native08 commit was imported.
raw = subprocess.check_output(['git', 'show', 'HEAD:experiments/results/shared_near_range/sequences/08/cleanup.json'], cwd=REPO)
if raw != (E / 'sequences/08/cleanup.json').read_bytes():
    raise SystemExit('Cleanup differs from committed evidence')
cleanup = json.loads(raw)
if not cleanup['hashes_verified_before_removal'] or Path(cleanup['owned_input_dir']).exists():
    raise SystemExit('Owned08 input cleanup is incomplete')
OUT = R / 'analysis08'
OUT.mkdir(exist_ok=False)
PYTHON = '/workspace/localization-env/host-venv/bin/python'
env = dict(os.environ, PATH=str(Path(PYTHON).parent) + os.pathsep + os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           MPLCONFIGDIR=str(R / 'analysis_matplotlib08'))
source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
receipts = []
for label, args in [
    ('checkpoint', [PYTHON, 'experiments/results/shared_near_range/checkpoint_report.py',
                    '--output-dir', str(OUT / 'checkpoint')]),
    ('diagnosis', [PYTHON, 'experiments/results/shared_near_range/diagnose_saved02.py',
                   '--sequence', '08', '--output-dir', str(OUT / 'diagnosis')]),
]:
    started = time.time()
    result = subprocess.run(args, cwd=REPO, env=env, capture_output=True, text=True)
    (OUT / (label + '.log')).write_text(result.stdout + result.stderr)
    receipt = dict(label=label, command=args, cwd=str(REPO), source_commit=source,
                   environment={k: env[k] for k in ('PYTHONDONTWRITEBYTECODE', 'OMP_NUM_THREADS',
                                                    'OPENBLAS_NUM_THREADS', 'MPLCONFIGDIR')},
                   started_unix=started, finished_unix=time.time(), exit_code=result.returncode)
    receipts.append(receipt)
    (OUT / 'commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
    if (OUT / label).exists():
        (OUT / label / 'analysis_command.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise SystemExit(result.returncode)
