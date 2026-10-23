#!/usr/bin/env python3
"""Recheck legacy corruption guards with real paths in an isolated checkout."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
p=argparse.ArgumentParser(__doc__)
p.add_argument('--source-root',type=Path,required=True)
p.add_argument('--scratch',type=Path,required=True)
a=p.parse_args();root=a.scratch;root.mkdir(parents=True,exist_ok=False)
(root/'tests').mkdir();(root/'experiments/results').mkdir(parents=True)
shutil.copy2(a.source_root/'tests/test_inten_dense_continuation.py',root/'tests/test_inten_dense_continuation.py')
for source in (a.source_root/'experiments/results').iterdir():
    target=root/'experiments/results'/source.name
    if source.name=='inten_dense_continuation':
        shutil.copytree(source,target,ignore=shutil.ignore_patterns('__pycache__'))
    else:target.symlink_to(source,target_is_directory=source.is_dir())
raise SystemExit(subprocess.run([sys.executable,'-m','unittest','discover','-s','tests'],cwd=root).returncode)
