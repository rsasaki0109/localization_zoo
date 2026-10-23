#!/usr/bin/env python3
"""Execute an argv without a shell; retain exact exit, logs, cwd and source."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for chunk in iter(lambda: f.read(1024*1024), b''):
            h.update(chunk)
    return h.hexdigest()


def save(path, value):
    Path(path).write_text(json.dumps(value, indent=2, allow_nan=False)+'\n')


def main():
    p = argparse.ArgumentParser(__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('command', nargs=argparse.REMAINDER)
    a = p.parse_args()
    command = a.command[1:] if a.command[:1] == ['--'] else a.command
    if not command:
        p.error('missing command')
    a.output.parent.mkdir(parents=True, exist_ok=True)
    if a.output.exists():
        raise FileExistsError(a.output)
    row = {'argv': command, 'cwd': os.getcwd(), 'source_commit': subprocess.check_output(
        ['git', 'rev-parse', 'HEAD'], text=True).strip(),
        'tracked_dirty': subprocess.check_output(['git', 'diff', 'HEAD', '--name-only'], text=True).splitlines(),
        'platform': platform.platform(), 'python': sys.version,
        'environment': {k: os.environ.get(k) for k in ['OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE']},
        'start_unix': time.time(), 'exit_code': None}
    save(a.output, row)
    start = time.monotonic()
    with a.output.with_suffix('.stdout').open('wb') as out, a.output.with_suffix('.stderr').open('wb') as err:
        try:
            result = subprocess.run(command, stdout=out, stderr=err)
            row['exit_code'] = result.returncode
        except OSError as error:
            row['launch_error'] = repr(error)
            row['exit_code'] = 127
    row['wall_seconds'] = time.monotonic()-start
    row['logs'] = {s: sha(a.output.with_suffix(s)) for s in ['.stdout', '.stderr']}
    save(a.output, row)
    print(json.dumps({'receipt':str(a.output), 'exit_code':row['exit_code']}), flush=True)
    return row['exit_code']


if __name__ == '__main__':
    raise SystemExit(main())
