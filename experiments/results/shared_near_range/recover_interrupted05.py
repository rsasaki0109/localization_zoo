"""Restart interrupted05 as a complete, separately recorded sequence attempt.

Preserve every interrupted output; reuse the sealed controller's audit and
comparison functions unchanged. Continue its next unstarted sequence afterward.
"""
import argparse
import ast
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
import traceback

sys.path.insert(0, str(Path(__file__).resolve().parent))
from support import (E, REPO, R, PY, CONTAINER_PY, WRAPPER, PROTOCOL, SOURCE,
                     docker, run, write, digest, native_output)


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def verify_owned_inputs(raw, alias, expected):
    require(raw.is_dir() and not raw.is_symlink(), 'Owned scans must be a real directory')
    require(alias.is_symlink() and alias.resolve() == raw.resolve(), 'Owned scan alias differs')
    require({p.name for p in raw.iterdir()} == set(expected), 'Owned scan inventory differs')
    for name, info in expected.items():
        path = raw / name
        require(path.is_file() and not path.is_symlink(), 'Owned scan must be a regular file')
        require(path.stat().st_size == info['bytes'] and digest(path) == info['sha256'],
                'Owned scan bytes differ: ' + name)


def committed(path):
    relative = path.relative_to(REPO).as_posix()
    value = subprocess.check_output(['git', 'show', 'HEAD:' + relative], cwd=REPO)
    require(value == path.read_bytes(), 'Modified/uncommitted recovery input: ' + relative)
    return value


def other_workers():
    workers = []
    controller_paths = {str(E / 'run_all.py'), str(Path(__file__).resolve())}
    for entry in Path('/proc').iterdir():
        if not entry.name.isdigit() or int(entry.name) == os.getpid():
            continue
        try:
            executable = (entry / 'exe').resolve().name
            argv = [v.decode(errors='replace') for v in (entry / 'cmdline').read_bytes().split(b'\0') if v]
        except OSError:
            continue
        native = executable in ('baseline_pcd_dogfooding', 'candidate_pcd_dogfooding')
        controller = executable.startswith('python') and bool(controller_paths.intersection(argv))
        if native or controller:
            workers.append(dict(pid=int(entry.name), argv=argv))
    return workers


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execute', action='store_true', required=True)
    args = parser.parse_args()
    require(not sys.flags.optimize, 'Recovery requires ordinary Python, without -O')
    committed(Path(__file__).resolve())
    recovery = json.loads(committed(E / 'recovery05_protocol.json'))
    require(not other_workers(), 'An experiment controller/native worker is already running')
    require(not (R / 'run_exit.json').exists(), 'A terminal controller receipt already exists')
    active = json.loads((R / 'active.json').read_text())
    previous = PROTOCOL['order'][:PROTOCOL['order'].index('05')]
    require(active['source_commit'] == SOURCE and active['sequence'] == '05'
            and active['status'] == 'auditing' and active['completed'] == previous,
            'Interrupted controller state differs from the recorded recovery boundary')
    for name, expected in recovery['sealed_sha256'].items():
        require(hashlib.sha256(committed(E / name)).hexdigest() == expected, 'Sealed source/config differs')
    for name in ('candidate', 'baseline'):
        spec = json.loads(committed(E / (name + '.json')))
        require(digest(Path(spec['binary'])) == spec['sha256'], 'Frozen executable differs')
    native_guard = ['git', 'diff', '--exit-code', SOURCE, '--', 'common', 'evaluation/src',
                    'evaluation/CMakeLists.txt', ':(glob)papers/*/src/**', ':(glob)papers/*/include/**',
                    ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt']
    subprocess.run(native_guard, cwd=REPO, check=True)
    for seq in previous:
        cleanup = json.loads(committed(E / 'sequences' / seq / 'cleanup.json'))
        require(cleanup['hashes_verified_before_removal'] and not Path(cleanup['owned_input_dir']).exists(),
                'A previous sequence is not cleaned')
        old_alias = R / 'data/dogfooding_results' / ('kitti_seq_' + seq + '_full')
        require(not old_alias.exists() and not old_alias.is_symlink(), 'A previous scan alias remains')
    for seq in PROTOCOL['order'][PROTOCOL['order'].index('06'):]:
        require(not (E / 'sequences' / seq).exists(), 'A later sequence already has evidence')
    folder = E / 'sequences/05'
    archive = folder / 'interrupted_20261018'
    require(not archive.exists() and (folder / 'audit').is_dir(), 'Interrupted attempt/ archive differs')
    require(not any((folder / name).exists() for name in ('cleanup.json', 'frozen_baseline', 'frozen_comparison.json')),
            'Interrupted05 unexpectedly contains terminal/control evidence')
    inputs = json.loads((folder / 'inputs.json').read_text())
    historical = json.loads(committed(REPO / 'experiments/results/shared_coarse_start/sequences/05/inputs.json'))
    require(inputs['files'] == historical['files'] and inputs['gt_csv_sha256'] == historical['gt_csv_sha256'],
            'Current05 input pins differ from the original official data')
    raw = R / 'kitti/dataset/sequences/05/velodyne'
    alias = R / 'data/dogfooding_results/kitti_seq_05_full'
    require(Path(inputs['input_dir']) == raw and inputs['matches_previous_official_inputs'], 'Owned input path differs')
    require(digest(REPO / 'experiments/reference_data/kitti_seq_05_full_gt.csv') == inputs['gt_csv_sha256'],
            'Ground truth differs')
    verify_owned_inputs(raw, alias, inputs['files'])
    lock_path = R / 'recovery05.lock'
    lock = os.open(lock_path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    with os.fdopen(lock, 'w') as stream:
        stream.write(str(os.getpid()) + '\n')
    state = dict(active)

    def update(**fields):
        state.update(fields)
        write(R / 'active.json', state)

    try:
        require(not other_workers(), 'A second worker appeared before recovery')
        # Compile only the pinned, unchanged helper definitions, never its startup/resume gates.
        original = (E / 'run_all.py').read_text()
        definitions = [node for node in ast.parse(original).body
                       if isinstance(node, ast.FunctionDef) and node.name in ('audit', 'compare_frozen')]
        require({node.name for node in definitions} == {'audit', 'compare_frozen'}, 'Sealed helper definitions differ')
        namespace = dict(globals(), update=update)
        exec(compile(ast.Module(body=definitions, type_ignores=[]), str(E / 'run_all.py'), 'exec'), namespace)
        audit, compare_frozen = namespace['audit'], namespace['compare_frozen']
        inventory = {p.relative_to(folder / 'audit').as_posix(): dict(sha256=digest(p), bytes=p.stat().st_size)
                     for p in (folder / 'audit').rglob('*') if p.is_file()}
        archive.mkdir()
        (folder / 'audit').rename(archive / 'audit')
        shutil.copy2(folder / 'inputs.json', archive / 'inputs.json')
        write(archive / 'interruption.json', dict(previous_active_state=active, output_inventory=inventory,
              controller_exit_code=None, inflight_native_exit_code=None,
              reason='Managed environment restarted; no original controller/native processes survived. '
                     'Unknown process exits are not synthesized. This entire attempt is excluded from the checkpoint.'))
        write(folder / 'recovery.json', dict(command=[sys.executable, *sys.argv], cwd=str(REPO),
              source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip(),
              native_source_commit=SOURCE, protocol=recovery, native_guard_command=native_guard,
              input_hashes_verified=True, archive=str(archive), started_unix=time.time(),
              policy='Rerun every selected05 cell from fresh native state; no cache/result reuse or parameter changes.'))
        update(status='auditing', methods=[], recovery='full05_restart_after_environment_recreation')
        report = audit('05', folder / 'audit')
        selected = [row for row in json.loads(committed(E / 'selection.json'))['rows'] if row['sequence'] == '05']
        require({(r['method'], r['aggregate_path'], r['variant_id']) for r in report['rows']} ==
                {(r['method'], r['repo_best_aggregate'], r['repo_best_variant']) for r in selected}, '05 cohort differs')
        require(all(r['process_exit_code'] == 0 for r in report['rows']), 'A recovered native cell failed')
        controls = {r['method'] for r in report['rows'] if r['audit_status'] == 'drift'}
        if controls:
            update(status='frozen_comparison', methods=sorted(controls))
            old = audit('05', folder / 'frozen_baseline', methods=sorted(controls), frozen=True)
            require(all(r['process_exit_code'] == 0 for r in old['rows']), 'A recovered control failed')
            compare_frozen(report, old, folder / 'frozen_comparison.json')
        update(status='verifying_cleanup')
        verify_owned_inputs(raw, alias, inputs['files'])
        alias.unlink()
        shutil.rmtree(raw)
        write(folder / 'cleanup.json', dict(sequence='05', owned_input_dir=str(raw),
              removed_files=len(inputs['files']), removed_bytes=sum(m['bytes'] for m in inputs['files'].values()),
              hashes_verified_before_removal=True, input_dir_exists_after=raw.exists()))
        require(not subprocess.check_output(['git', 'diff', '--cached', '--name-only'], cwd=REPO).strip(),
                'Unrelated staged work exists before sequence commit')
        subprocess.run(['git', 'add', '-f', str(folder)], cwd=REPO, check=True)
        subprocess.run(['git', 'commit', '-m', 'Record recovered complete KITTI05 replay and preserve interrupted attempt'],
                       cwd=REPO, check=True)
        update(status='sequence_committed', completed=previous + ['05'],
               last_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip())
        # At06 all preceding checkpoints are committed/cleaned and the target has no native evidence.
        result = subprocess.run([PY, str(E / 'run_all.py'), '--resume-from', '06'], cwd=REPO)
        return result.returncode
    except BaseException as error:
        update(status='failed', error=str(error))
        write(R / 'run_exit.json', dict(exit_code=1, sequence='05', recovery=True, error=str(error)))
        traceback.print_exc()
        return 1
    finally:
        lock_path.unlink()


if __name__ == '__main__':
    raise SystemExit(main())
