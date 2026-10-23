"""Recorded commands and byte-verified restoration of committed diagnostic inputs."""
from pathlib import Path
import gzip
import hashlib
import importlib.util
import json
import os
import subprocess
import time

E = Path(__file__).resolve().parent
W = E.parents[2]
R = Path('/workspace/localization-env/inten-intensity-normalized-20261022/stable')
BASE = '75fd23744089c044226605c85c411146ea0a8508'
PY = '/workspace/localization-env/host-venv/bin/python'
ENV = dict(os.environ, OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           PYTHONDONTWRITEBYTECODE='1', PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'])


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def read(path):
    return json.loads(Path(path).read_text())


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, allow_nan=False)+'\n')


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def git(*args):
    return subprocess.check_output(['git', *args], cwd=W, text=True).strip()


def run(label, command, folder, payload=None, jsonl=False):
    folder.mkdir(parents=True, exist_ok=True)
    outpath = folder/(label+('.stdout.jsonl' if jsonl else '.stdout.log'))
    started = time.time()
    with outpath.open('xb') as out, (folder/(label+'.stderr.log')).open('xb') as err:
        result = subprocess.run(command, cwd=W, env=ENV, input=payload, stdout=out, stderr=err)
    write(folder/(label+'_command.json'), dict(command=command, cwd=str(W), source_commit=git('rev-parse', 'HEAD'),
          started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
          environment={k: ENV[k] for k in ('OMP_NUM_THREADS', 'OPENBLAS_NUM_THREADS', 'PYTHONDONTWRITEBYTECODE')},
          stdin_sha256=hashlib.sha256(payload).hexdigest() if payload is not None else None))
    print(label, result.returncode, flush=True)
    return result.returncode


def restore(folder, name, target):
    entry = read(folder/'archive.json')['files'][name]
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open('xb') as out:
        if entry['storage'] == 'ordered_gzip_shards':
            for part in entry['parts']:
                path = folder/part['path']
                assert sha(path) == part['gzip_sha256']
                raw = gzip.decompress(path.read_bytes())
                assert len(raw) == part['raw_bytes'] and hashlib.sha256(raw).hexdigest() == part['raw_sha256']
                out.write(raw)
        else:
            path = folder/entry['path']
            assert sha(path) == entry['stored_sha256']
            out.write(gzip.decompress(path.read_bytes()) if entry['storage'] == 'gzip' else path.read_bytes())
    assert target.stat().st_size == entry['bytes'] and sha(target) == entry['sha256']
    return dict(archive_manifest=str((folder/'archive.json').relative_to(W)),
                archive_manifest_sha256=sha(folder/'archive.json'), name=name,
                restored_path=str(target), sha256=entry['sha256'], bytes=entry['bytes'])
