"""Frozen actual-prefix geometry collection. Retain failed commands and raw inputs."""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import shlex
import subprocess
import time

E = Path(__file__).resolve().parent
W = E.parents[2]
R = W.parent
PY = '/workspace/localization-env/host-venv/bin/python'
ENV = dict(os.environ, OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           PYTHONDONTWRITEBYTECODE='1', PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'])


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


build = module('geometry_build', E/'build.py')
read = lambda p: json.loads(Path(p).read_text())


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, allow_nan=False)+'\n')


def git(*args):
    return subprocess.check_output(['git', *args], cwd=W, text=True).strip()


def run(label, command, folder, payload=None):
    folder.mkdir(parents=True, exist_ok=True)
    assert not (folder/(label+'_command.json')).exists()
    start = time.time()
    with (folder/(label+'.stdout')).open('xb') as out, (folder/(label+'.stderr')).open('xb') as err:
        result = subprocess.run(command, input=payload, cwd=W, env=ENV, stdout=out, stderr=err)
    write(folder/(label+'_command.json'), dict(command=command, cwd=str(W),
          source_commit=git('rev-parse', 'HEAD'), started_unix=start, finished_unix=time.time(),
          exit_code=result.returncode, environment={k: ENV[k] for k in
          ('OMP_NUM_THREADS', 'OPENBLAS_NUM_THREADS', 'PYTHONDONTWRITEBYTECODE')},
          stdin_sha256=hashlib.sha256(payload).hexdigest() if payload else None))
    print(label, result.returncode, flush=True)
    return result.returncode


def main():
    p = read(E/'protocol.json')
    assert sha(W/'papers/inten_loam/src/inten_loam_paper.cpp') == p['native_source_sha256']
    assert sha(E/'diagnostic_native.cpp') == p['diagnostic_source_sha256']
    stage = R/'staging'
    stage.mkdir()
    flags = build.flags(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir')
    flags += ['-I'+str(W/'evaluation/src'), '-I'+str(E)]
    obj = R/'native.o'
    assert run('compile_native', build.docker()+['c++', *flags, '-c', str(E/'diagnostic_native.cpp'), '-o', str(obj)], stage) == 0
    link = shlex.split((build.OLD/'evaluation/CMakeFiles/pcd_dogfooding.dir/link.txt').read_text())
    libs = []
    pins = {}
    for token in link[link.index('-o')+2:]:
        if token.startswith('-'):
            libs.append(token)
            continue
        lib = Path(token)
        lib = lib if lib.is_absolute() else (build.OLD/'evaluation'/lib).resolve()
        if lib == build.OLD/'papers/inten_loam/libinten_loam.a':
            continue
        if lib.suffix == '.a':
            pins[str(lib)] = sha(lib)
        libs.append(str(lib))
    binary = R/'probe'
    assert run('compile_probe', build.docker()+['c++', *flags, str(E/'probe.cpp'), str(obj), *libs, '-o', str(binary)], stage) == 0
    write(stage/'binary.json', dict(path=str(binary), sha256=sha(binary), object_sha256=sha(obj),
          producer_source_commit=git('rev-parse', 'HEAD'), linked_static_libraries=pins,
          compiler_metadata_sha256=sha(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make')))
    for attempt in range(1, 5):
        if run('fetch', [PY, 'evaluation/scripts/fetch_kitti_odometry_sequences.py', '--output-root',
                       str(R/'kitti'), '--sequence', '01', '--workers', '4'], R/'fetch'/str(attempt)) == 0:
            break
    else:
        raise RuntimeError('Official fetch failed; preserve all inputs')
    raw = R/'kitti/dataset/sequences/01/velodyne'
    inputs = {f.name: dict(sha256=sha(f), bytes=f.stat().st_size) for f in sorted(raw.iterdir()) if f.is_file()}
    assert inputs == read(W/'experiments/results/shared_near_range/sequences/01/inputs.json')['files']
    write(stage/'inputs.json', dict(sequence='01', owned_input_dir=str(raw), files=inputs,
          exact_previous_official_hashes=True))
    payload = ('\n'.join(map(str, [max(int(k[:-4]) for k in inputs if k.endswith(".bin")),*p['detailed_frames']]))+'\n').encode()
    (stage/'input.txt').write_bytes(payload)
    assert run('native', build.docker()+[str(binary), str(raw), str(stage/'correspondences.jsonl'),
               str(stage/'masks.jsonl'),str(stage/'ranges.jsonl')], stage, payload) == 0
    assert sha(binary) == read(stage/'binary.json')['sha256']
    write(R/'collection_complete.json', dict(exit_code=0, stage=str(stage), raw_retained=True))


if __name__ == '__main__':
    main()
