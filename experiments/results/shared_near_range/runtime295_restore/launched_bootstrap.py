"""Recorded runtime295 bootstrap; estimator/controller sources stay sealed.

Run --preflight first. Without it, start the original controller exactly once
and push only its committed, cleaned sequence checkpoints to a new backup ref.
No partial outputs are published by this supervisor and no finalizer is started.
"""
from pathlib import Path
import fcntl
import hashlib
import json
import os
import subprocess
import sys
import time

REPO = Path('/workspace/localization_zoo')
E = REPO / 'experiments/results/shared_near_range'
R = Path('/workspace/localization-env/shared-near-range')
OUT = R / 'restore295'
BASE = 'c3698e6700199b352d846f70a6580ad007ea35cc'
NATIVE = '0b079dbbcb7c6f58c1d8367ade881c15f003e525'
BACKUP = 'refs/heads/codex/shared-range-replay295'
PYTHON = '/workspace/localization-env/host-venv/bin/python'
PREFIX = ['04', '00', '01', '02', '03']
REMAINING = {'05', '06', '07', '08', '09', '10'}


def git(*args):
    return subprocess.check_output(['git', *args], cwd=REPO, text=True).strip()


def write(name, value):
    target = OUT / name
    temporary = target.with_suffix(target.suffix + '.tmp')
    temporary.write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')
    temporary.replace(target)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(condition, message='runtime restoration guard rejected input'):
    if not condition:
        raise RuntimeError(message)


def preflight():
    require(git('rev-parse', 'HEAD') == BASE)
    require(git('branch', '--show-current') == 'codex/shared-near-range')
    require(not git('status', '--porcelain'))
    require(not (R / 'run_exit.json').exists())
    native_paths = [p for p in git('ls-tree', '-r', '--name-only', NATIVE).splitlines()
                    if (p.startswith(('common/', 'papers/', 'evaluation/src/', 'evaluation/include/', 'cmake/'))
                        and (p.endswith(('.c', '.cc', '.cpp', '.cxx', '.h', '.hpp', '.cmake'))
                             or p.endswith('CMakeLists.txt')))
                    or p in ('CMakeLists.txt', 'evaluation/CMakeLists.txt')]
    require(native_paths)
    require(not git('diff', NATIVE, '--', *native_paths))
    sealed = json.loads((E / 'recovery05_protocol.json').read_text())['sealed_sha256']
    require(all(sha(E / name) == digest for name, digest in sealed.items()))
    extra_sealed = {name: sha(E / name) for name in ('recover_interrupted05.py', 'recovery05_protocol.json')}
    binaries = {}
    for name in ('baseline', 'candidate'):
        spec = json.loads((E / (name + '.json')).read_text())
        actual = sha(Path(spec['binary']))
        require(actual == spec['sha256'])
        binaries[name] = dict(expected_sha256=spec['sha256'], actual_sha256=actual)
    for seq in PREFIX:
        cleanup = json.loads(git('show', f'HEAD:experiments/results/shared_near_range/sequences/{seq}/cleanup.json'))
        require(cleanup['hashes_verified_before_removal'])
        require(not Path(cleanup['owned_input_dir']).exists())
        require(not (R / 'data/dogfooding_results' / f'kitti_seq_{seq}_full').is_symlink())
    for seq in REMAINING:
        folder = E / 'sequences' / seq
        require(not folder.exists())
        require(not (R / 'kitti/dataset/sequences' / seq / 'velodyne').exists())
    own = os.getpid()
    forbidden = {'pcd_dogfooding', 'run_all.py', 'recover_interrupted05.py', 'finish_when_complete.py'}
    for proc in Path('/proc').iterdir():
        if not proc.name.isdigit() or int(proc.name) == own:
            continue
        try:
            args = proc.joinpath('cmdline').read_bytes().split(b'\0')
        except (FileNotFoundError, PermissionError, ProcessLookupError):
            continue
        require(not any(Path(os.fsdecode(a)).name in forbidden for a in args if a), str(proc))
    remote = subprocess.run(['git', 'ls-remote', '--heads', 'origin', BACKUP], cwd=REPO,
                            text=True, capture_output=True)
    write('backup_ref_probe.json', dict(command=remote.args, cwd=str(REPO),
                                      exit_code=remote.returncode, stdout=remote.stdout, stderr=remote.stderr))
    require(remote.returncode == 0 and not remote.stdout.strip(), 'backup ref must be new')
    result = dict(exit_code=0, source_head=BASE, native_source=NATIVE,
                  recursive_native_paths_checked=len(native_paths), sealed_sha256=sealed,
                  extra_sealed_sha256=extra_sealed, binaries=binaries,
                  committed_cleaned_prefix=PREFIX, fresh_remaining=sorted(REMAINING),
                  controller_command=[PYTHON, str(E / 'run_all.py'), '--resume-from', '05'],
                  backup_ref=BACKUP, old_worker_exits=None, recorded_unix=time.time())
    write('resume_preflight.json', result)
    return result


def push_checkpoint(head, previous):
    if head != BASE:
        subprocess.run(['git', 'merge-base', '--is-ancestor', BASE, head], cwd=REPO, check=True)
        paths = git('diff', '--name-only', BASE, head).splitlines()
        require(paths)
        for path in paths:
            parts = Path(path).parts
            require(parts[:4] == ('experiments', 'results', 'shared_near_range', 'sequences'))
            require(len(parts) > 5 and parts[4] in REMAINING)
        for seq in {Path(p).parts[4] for p in paths}:
            cleanup = json.loads(git('show', f'{head}:experiments/results/shared_near_range/sequences/{seq}/cleanup.json'))
            require(cleanup['hashes_verified_before_removal'] and not cleanup['input_dir_exists_after'])
    command = ['git', 'push', 'origin', f'{head}:{BACKUP}']
    started = time.time()
    result = subprocess.run(command, cwd=REPO, text=True, capture_output=True)
    receipt = dict(command=command, cwd=str(REPO), head=head, previous_backed_up=previous,
                   started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
                   stdout=result.stdout, stderr=result.stderr)
    target = OUT / 'backup_commands.jsonl'
    with target.open('a') as stream:
        stream.write(json.dumps(receipt) + '\n')
    return result.returncode == 0


if __name__ == '__main__':
    require(sys.argv[1:] in ([], ['--preflight']))
    with (OUT / 'supervisor.lock').open('a') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        plan = preflight()
        if sys.argv[1:]:
            print(json.dumps(plan), flush=True)
            raise SystemExit(0)
        require(push_checkpoint(BASE, None), 'Initial backup must succeed before starting native work')
        env = dict(os.environ, PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1',
                   OPENBLAS_NUM_THREADS='1', MPLCONFIGDIR=str(OUT / 'matplotlib'))
        with (OUT / 'run_all.log').open('x') as log:
            started = time.time()
            child = subprocess.Popen(plan['controller_command'], cwd=REPO, env=env,
                                     stdout=log, stderr=subprocess.STDOUT)
            write('resume_launch.json', dict(command=plan['controller_command'], cwd=str(REPO),
                                           pid=child.pid, supervisor_pid=os.getpid(),
                                           source_head=BASE, started_unix=started, exit_code=None,
                                           backup_ref=BACKUP))
            backed_up = BASE
            next_push = 0.0
            while True:
                status = child.poll()
                head = git('rev-parse', 'HEAD')
                if head != backed_up and time.monotonic() >= next_push:
                    if push_checkpoint(head, backed_up):
                        backed_up = head
                    next_push = time.monotonic() + 60
                write('supervisor_status.json', dict(child_pid=child.pid, controller_exit_code=status,
                      head=head, backed_up_head=backed_up, backup_pending=head != backed_up,
                      updated_unix=time.time()))
                if status is not None:
                    write('resume_exit.json', dict(exit_code=status, head=head, backed_up_head=backed_up,
                                                  backup_pending=head != backed_up, finished_unix=time.time()))
                    raise SystemExit(status if status else (0 if backed_up == head else 4))
                time.sleep(15)
