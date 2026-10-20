"""Run this fixed natural-trace study using the existing CLI and offline tools.

No estimator, exporter or generic data-collection API is implemented here.
Never resume an interrupted native run; preserve its evidence for review.
"""
from pathlib import Path
import gzip
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
import traceback

E = Path(__file__).resolve().parent
REPO = E.parents[2]
R = Path('/workspace/localization-env/inten-natural-traces')
PY = '/workspace/localization-env/host-venv/bin/python'
P = json.loads((E / 'protocol.json').read_text())
BUILD = R / 'build'
BINARY = BUILD / 'evaluation/pcd_dogfooding'
BRANCH = 'codex/inten-natural-failure-traces'
ENV = dict(os.environ, PATH=str(Path(PY).parent) + os.pathsep + os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           MPLCONFIGDIR=str(R / 'matplotlib'))


def git(*args):
    return subprocess.check_output(['git', *args], cwd=REPO, text=True).strip()


def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(path.name + '.tmp')
    temp.write_text(json.dumps(data, indent=2, allow_nan=False) + '\n')
    temp.replace(path)


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def docker(cwd):
    return ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
            '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
            '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
            '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
            '-e', 'PYTHONDONTWRITEBYTECODE=1', '-e', 'TEST_TMPDIR=' + str(R / 'test_tmp'),
            '-w', str(cwd), 'localization-zoo-dev']


def run(label, command, folder, allowed=(0,), native_argv=None, native_cwd=None):
    folder.mkdir(parents=True, exist_ok=True)
    receipt = folder / (label + '_command.json')
    if receipt.exists():
        raise RuntimeError('Refusing to overwrite receipt: ' + str(receipt))
    started = time.time()
    with (folder / (label + '.log')).open('xb') as log:
        result = subprocess.run(command, cwd=REPO, env=ENV, stdout=log, stderr=subprocess.STDOUT)
    record = dict(command=command, cwd=str(REPO), source_commit=git('rev-parse', 'HEAD'),
                  native_source_commit=P['native_source_commit'], started_unix=started,
                  finished_unix=time.time(), exit_code=result.returncode,
                  environment=P['native_environment'])
    if native_argv is not None:
        record.update(native_argv=native_argv, native_cwd=str(native_cwd), binary_sha256=sha(BINARY))
    write(receipt, record)
    print(label, result.returncode, flush=True)
    if result.returncode not in allowed:
        raise RuntimeError(label + ' failed: ' + str(result.returncode))
    return result.returncode


def verify_source():
    paths = ['common', 'evaluation/src', 'evaluation/test', 'evaluation/CMakeLists.txt',
             ':(glob)papers/*/src/**', ':(glob)papers/*/include/**',
             ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt']
    if git('diff', '--name-only', P['native_source_commit'], '--', *paths):
        raise RuntimeError('Native source changed')
    for path, expected in P['tool_sha256'].items():
        if sha(REPO / path) != expected:
            raise RuntimeError('Existing tool changed: ' + path)
    for name in ('run_sequences.py', 'protocol.json'):
        path = E / name
        if subprocess.check_output(['git', 'show', 'HEAD:' + path.relative_to(REPO).as_posix()], cwd=REPO) != path.read_bytes():
            raise RuntimeError('Uncommitted study source: ' + name)


def backup(label):
    for attempt in range(1, 4):
        folder = R / 'backups' / label / str(attempt)
        if run('push', ['git', 'push', 'origin', 'HEAD:refs/heads/' + BRANCH], folder,
               allowed=(0, 1, 128)) == 0:
            return
        time.sleep(5)
    raise RuntimeError('Normal backup push failed; no next sequence will be fetched')


def archive(source, target):
    """Losslessly retain every result file; scanners live outside source."""
    target.mkdir(parents=True, exist_ok=False)
    manifest = {}
    for path in sorted(source.rglob('*')):
        if not path.is_file():
            continue
        relative = path.relative_to(source)
        entry = dict(original_path=str(path), bytes=path.stat().st_size, sha256=sha(path))
        if path.suffix == '.jsonl':
            pieces = target / (relative.as_posix() + '.parts')
            pieces.mkdir(parents=True)
            parts = []
            recovered = hashlib.sha256()
            with path.open('rb') as stream:
                while True:
                    raw = stream.read(P['archival']['uncompressed_shard_bytes'])
                    if not raw:
                        break
                    part = pieces / ('part-%05d.gz' % len(parts))
                    part.write_bytes(gzip.compress(raw, compresslevel=6, mtime=0))
                    decoded = gzip.decompress(part.read_bytes())
                    if decoded != raw:
                        raise RuntimeError('Archive shard does not round-trip')
                    recovered.update(decoded)
                    parts.append(dict(path=part.relative_to(target).as_posix(),
                                      gzip_sha256=sha(part), gzip_bytes=part.stat().st_size,
                                      raw_sha256=hashlib.sha256(raw).hexdigest(), raw_bytes=len(raw)))
            if recovered.hexdigest() != entry['sha256'] or sha(path) != entry['sha256']:
                raise RuntimeError('Archive stream differs from original')
            entry.update(storage='ordered_gzip_shards', parts=parts)
        else:
            destination = target / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            if path.suffix == '.log':
                destination = destination.with_suffix('.log.gz')
                destination.write_bytes(gzip.compress(path.read_bytes(), mtime=0))
                restored = gzip.decompress(destination.read_bytes())
            else:
                shutil.copyfile(path, destination)
                restored = destination.read_bytes()
            if hashlib.sha256(restored).hexdigest() != entry['sha256']:
                raise RuntimeError('Archive file differs from original')
            entry.update(storage='gzip' if path.suffix == '.log' else 'verbatim',
                         path=destination.relative_to(target).as_posix(), stored_sha256=sha(destination))
        manifest[relative.as_posix()] = entry
    write(target / 'archive.json', dict(files=manifest, byte_roundtrip_verified=True, exit_code=0))


def main():
    if git('branch', '--show-current') != BRANCH or git('status', '--porcelain'):
        raise RuntimeError('Require the clean study branch')
    if BUILD.exists() or (R / 'staging').exists() or (E / 'sequences').exists():
        raise RuntimeError('Fresh execution only; inspect previous evidence before recovery')
    verify_source()
    state = dict(status='building', completed=[], execution_commit=git('rev-parse', 'HEAD'))
    write(R / 'active.json', state)
    (R / 'test_tmp').mkdir()
    checks = R / 'build_checks'
    run('configure', docker(REPO) + ['cmake', '-S', str(REPO), '-B', str(BUILD),
        '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_PYTHON_BINDINGS=OFF'], checks)
    run('build', docker(REPO) + ['cmake', '--build', str(BUILD), '--target',
        'pcd_dogfooding', 'test_inten_loam_paper', 'test_inten_trace_jsonl', '-j2'], checks)
    run('ctest', docker(REPO) + ['ctest', '--test-dir', str(BUILD), '--output-on-failure',
        '-R', '^(test_inten_loam_paper|test_inten_trace_jsonl)$'], checks)
    pin = dict(binary=str(BINARY), sha256=sha(BINARY), native_source_commit=P['native_source_commit'],
               execution_commit=state['execution_commit'], ctest_exit_code=0)
    write(E / 'binary.json', pin)
    archive(checks, E / 'checks/build')
    subprocess.run(['git', 'add', '-f', str(E / 'binary.json'), str(E / 'checks/build')], cwd=REPO, check=True)
    subprocess.run(['git', 'commit', '-m', 'Record fresh InTEn natural-trace runner build and existing mechanism checks'], cwd=REPO, check=True)
    backup('build')

    effective_configuration = None
    for seq in P['sequence_order']:
        verify_source()
        if sha(BINARY) != pin['sha256']:
            raise RuntimeError('Pinned binary changed')
        stage = R / 'staging' / seq
        stage.mkdir(parents=True)
        state.update(status='fetching', sequence=seq)
        write(R / 'active.json', state)
        for attempt in range(1, 4):
            status = run('fetch', [PY, 'evaluation/scripts/fetch_kitti_odometry_sequences.py',
                '--output-root', str(R / 'kitti'), '--sequence', seq, '--workers', '4'],
                stage / 'fetch' / str(attempt), allowed=(0, 1))
            if status == 0:
                break
        else:
            raise RuntimeError('Official acquisition failed')
        raw = R / 'kitti/dataset/sequences' / seq / 'velodyne'
        files = {p.name: dict(sha256=sha(p), bytes=p.stat().st_size)
                 for p in sorted(raw.iterdir()) if p.is_file()}
        previous = json.loads((REPO / ('experiments/results/shared_near_range/sequences/' + seq + '/inputs.json')).read_text())
        gt = REPO / ('experiments/reference_data/kitti_seq_' + seq + '_full_gt.csv')
        if files != previous['files'] or sha(gt) != previous['gt_csv_sha256']:
            raise RuntimeError('Official scan/GT hashes differ from committed previous inputs')
        write(stage / 'inputs.json', dict(sequence=seq, files=files, gt_csv_sha256=sha(gt),
              owned_input_dir=str(raw), matches_previous_official_inputs=True))
        settings = P['manifests'][seq]
        if sha(REPO / settings['path']) != settings['sha256']:
            raise RuntimeError('Manifest differs')
        for mode in ('off', 'on'):
            state.update(status='native_' + mode)
            write(R / 'active.json', state)
            folder = stage / mode
            folder.mkdir()
            argv = [str(BINARY), str(raw), str(gt), '--methods', 'inten_loam',
                    '--summary-json', str(folder / 'summary.json'), *settings['args']]
            if mode == 'on':
                argv += ['--inten-loam-paper-trace-jsonl', str(folder / 'trace.jsonl')]
            run('native', docker(folder) + argv, folder, native_argv=argv, native_cwd=folder)
        summaries = {m: json.loads((stage / m / 'summary.json').read_text()) for m in ('off', 'on')}
        method = {m: summaries[m]['methods'][0] for m in summaries}
        fields = ('status', 'frames', 'ate_m', 'rpe_trans_pct', 'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')
        if any(method['off'].get(k) != method['on'].get(k) for k in fields):
            raise RuntimeError('Trace changes accuracy/frame fields')
        names = sorted(p.name for p in (stage / 'off/dogfooding_results').glob('*.txt'))
        if not names or any((stage / 'off/dogfooding_results' / n).read_bytes() !=
                            (stage / 'on/dogfooding_results' / n).read_bytes() for n in names):
            raise RuntimeError('Trace changes saved pose/GT bytes')
        write(stage / 'parity.json', dict(exit_code=0, identical_fields=list(fields),
              saved_pose_bytes_equal={n: True for n in names}, runtime_comparison_claim=False))
        state.update(status='validating')
        write(R / 'active.json', state)
        tuple_args = ['--trace', str(stage / 'on/trace.jsonl'), '--summary', str(stage / 'on/summary.json'),
                      '--receipt', str(stage / 'on/native_command.json')]
        run('validate', [PY, 'evaluation/scripts/check_inten_trace.py', *tuple_args,
            '--output', str(stage / 'trace_validation.json')], stage / 'analysis_commands')
        validation = json.loads((stage / 'trace_validation.json').read_text())
        if validation['skipped_frames'] or validation['processed_frames'] != previous['scan_frames']:
            raise RuntimeError('KITTI traversal is skipped or incomplete')
        with (stage / 'on/trace.jsonl').open() as stream:
            header = json.loads(stream.readline())
        configuration = dict(params=header['params'], loader=header['loader'])
        if header['params']['accelerated'] is not False:
            raise RuntimeError('Acceleration must remain disabled')
        if effective_configuration is None:
            effective_configuration = configuration
        elif configuration != effective_configuration:
            raise RuntimeError('Effective settings differ across sequences')
        write(stage / 'effective_configuration.json', configuration)
        run('conditioning', [PY, 'evaluation/scripts/analyze_inten_conditioning.py', *tuple_args,
            '--rotation-length-m', str(P['conditioning']['rotation_length_m']),
            '--rank-rtol', str(P['conditioning']['rank_rtol']), '--output-dir', str(stage / 'conditioning')],
            stage / 'analysis_commands')
        on = stage / 'on'
        evaluated = Path(summaries['on']['artifacts']['evaluated_gt_path'])
        evaluated = evaluated if evaluated.is_absolute() else on / evaluated
        run('steps', [PY, 'evaluation/scripts/diagnose_kitti_steps.py',
            '--estimate', str(on / 'dogfooding_results/InTEn_LOAM.txt'), '--gt', str(evaluated),
            '--summary', str(on / 'summary.json'), '--method', 'InTEn-LOAM',
            '--output', str(stage / 'motion_steps.json')], stage / 'analysis_commands')
        verify_source()
        if sha(BINARY) != pin['sha256'] or set(p.name for p in raw.iterdir()) != set(files):
            raise RuntimeError('Binary/input set changed')
        if any(sha(raw / name) != v['sha256'] for name, v in files.items()):
            raise RuntimeError('Input bytes changed before cleanup')
        target = E / 'sequences' / seq
        state.update(status='archiving')
        write(R / 'active.json', state)
        archive(stage, target)
        shutil.rmtree(raw)
        write(target / 'cleanup.json', dict(sequence=seq, owned_input_dir=str(raw),
              hashes_verified_before_removal=True, removed_files=len(files),
              removed_bytes=sum(v['bytes'] for v in files.values()), input_dir_exists_after=raw.exists()))
        subprocess.run(['git', 'add', '-f', str(target)], cwd=REPO, check=True)
        subprocess.run(['git', 'commit', '-m', 'Record full natural InTEn trace/control on KITTI ' + seq], cwd=REPO, check=True)
        state['completed'].append(seq)
        state.update(status='sequence_committed', last_commit=git('rev-parse', 'HEAD'))
        write(R / 'active.json', state)
        backup(seq)
        shutil.rmtree(stage)  # Only after lossless archive, commit and remote backup.
    state.update(status='complete', sequence=None)
    write(R / 'active.json', state)
    write(R / 'run_exit.json', dict(exit_code=0, completed=state['completed']))


if __name__ == '__main__':
    lock = R / 'collector.lock'
    descriptor = os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    with os.fdopen(descriptor, 'w') as stream:
        stream.write(str(os.getpid()) + '\n')
    try:
        main()
    except BaseException as error:
        write(R / 'failure.json', dict(exit_code=1, error=str(error),
              scope='Keep interrupted inputs and raw evidence. No retry of native execution or next-sequence acquisition.'))
        traceback.print_exc()
        raise
    finally:
        lock.unlink()
