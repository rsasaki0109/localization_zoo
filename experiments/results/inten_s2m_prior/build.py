"""Build the changed native library/tests and link unchanged pinned dependencies."""
from pathlib import Path
import hashlib
import json
import os
import shlex
import subprocess
import time

E = Path(__file__).resolve().parent
W = E.parents[2]
R = W.parent
OLD = Path('/workspace/localization-env/inten-natural-traces/build')
ENV = dict(os.environ, OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', PYTHONDONTWRITEBYTECODE='1')


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, allow_nan=False)+'\n')


def docker(cwd=W):
    return ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
            '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
            '--host=unix:///var/run/docker.sock', 'exec', '-i', '--user', '1000:1000',
            '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
            '-w', str(cwd), 'localization-zoo-dev']


def run(label, command, folder):
    folder.mkdir(parents=True, exist_ok=True)
    started = time.time()
    with (folder/(label+'.stdout')).open('xb') as out, (folder/(label+'.stderr')).open('xb') as err:
        result = subprocess.run(command, cwd=W, env=ENV, stdout=out, stderr=err)
    write(folder/(label+'_command.json'), dict(command=command, cwd=str(W),
        source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=W, text=True).strip(),
        started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
        environment={'OMP_NUM_THREADS':'1', 'OPENBLAS_NUM_THREADS':'1'}))
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise RuntimeError(label+' failed')


def flags(folder):
    path = folder/'flags.make'
    raw = path.read_text().replace(str(OLD.parent/'worktree'), str(W))
    result = []
    for line in raw.splitlines():
        if line.startswith(('CXX_DEFINES =', 'CXX_INCLUDES =', 'CXX_FLAGS =')):
            result.extend(shlex.split(line.split('=', 1)[1]))
    return result


def main():
    if (R/'build').exists() or (R/'pcd_dogfooding').exists():
        raise RuntimeError('Fresh build paths required; retain failures before recovery')
    folder = E/'checks/build'
    run('configure', docker()+['cmake', '-S', str(W), '-B', str(R/'build'),
        '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_PYTHON_BINDINGS=OFF'], folder)
    run('build', docker()+['cmake', '--build', str(R/'build'), '--target',
        'test_inten_loam', 'test_inten_loam_paper', 'test_inten_trace_jsonl', '-j2'], folder)
    run('ctest', docker()+['ctest', '--test-dir', str(R/'build'), '--output-on-failure',
        '-R', '^(test_inten_loam|test_inten_loam_paper|test_inten_trace_jsonl)$'], folder)
    metadata = OLD/'evaluation/CMakeFiles/pcd_dogfooding.dir'
    link = shlex.split((metadata/'link.txt').read_text())
    library_pins = {}
    libs = []
    for token in link[link.index('-o')+2:]:
        if token.startswith('-'):
            libs.append(token)
            continue
        path = Path(token)
        if not path.is_absolute():
            path = (OLD/'evaluation'/path).resolve()
        if path == OLD/'papers/inten_loam/libinten_loam.a':
            path = R/'build/papers/inten_loam/libinten_loam.a'
        if path.suffix == '.a':
            library_pins[str(path)] = sha(path)
        libs.append(str(path))
    run('runner', docker()+['c++', *flags(metadata), str(W/'evaluation/src/pcd_dogfooding.cpp'),
        *libs, '-o', str(R/'pcd_dogfooding')], folder)
    # Same public forced-pair diagnostic as the parent, with the fresh default-off library.
    probe_metadata = OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir'
    run('probe', docker()+['c++', *flags(probe_metadata), '-I'+str(W/'evaluation/src'),
        str(E/'compatibility_probe.cpp'), *libs, '-o', str(R/'compatibility_probe')], folder)
    run('intervention_probe', docker()+['c++', *flags(probe_metadata), '-I'+str(W/'evaluation/src'),
        str(E/'intervention_probe.cpp'), *libs, '-o', str(R/'intervention_probe')], folder)
    write(E/'binaries.json', dict(runner=dict(path=str(R/'pcd_dogfooding'), sha256=sha(R/'pcd_dogfooding')),
        probe=dict(path=str(R/'compatibility_probe'), sha256=sha(R/'compatibility_probe')),
        intervention_probe=dict(path=str(R/'intervention_probe'), sha256=sha(R/'intervention_probe')),
        linked_static_libraries=library_pins,
        resolved_parent_metadata={str(metadata/name):sha(metadata/name) for name in ('flags.make','link.txt')},
        explanation='Only the changed InTEn library and runner are freshly compiled. Other native libraries are unchanged pinned parent dependencies; no old build file or binary is modified. Full project CI is required before review.'))


if __name__ == '__main__':
    main()
