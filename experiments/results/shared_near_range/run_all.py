"""Replay the frozen whole-zoo KITTI selection one sequence at a time.

This records historical settings, not a parameter experiment. Resume is allowed
only at a fetch failure with no native evidence in that sequence; completed
checkpoints must be committed and have their owned scans deleted.
"""
from support import *
import argparse
import csv
import shutil
import traceback

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--resume-from', choices=PROTOCOL['order'])
options = parser.parse_args()
state = dict(source_commit=SOURCE, status='starting', sequence=None, completed=[])


def update(**fields):
    state.update(fields)
    write(R / 'active.json', state)


def audit(seq, folder, methods=(), frozen=False, extra=()):
    command = docker(frozen) + [CONTAINER_PY, 'evaluation/scripts/audit_kitti_drift.py',
        '--binary', str(WRAPPER), '--sequence', seq, '--data-root', str(R / 'data'),
        '--work-dir', str(folder / 'replays'), '--output', str(folder / 'report.json'),
        '--jobs', '3']
    for method in methods:
        command += ['--method', method]
    for arg in extra:
        command += ['--extra-arg=' + arg]
    run('audit', command, folder, allowed=(0, 1))
    report = json.loads((folder / 'report.json').read_text())
    assert all(row['audit_status'] in ('ok', 'drift') for row in report['rows'])
    return report


def compare_frozen(current, baseline, output):
    comparisons = []
    for previous in baseline['rows']:
        now = next(row for row in current['rows'] if row['method'] == previous['method'])
        fields = ('rerun_rpe_trans_pct', 'rerun_ate_m', 'frames',
                  'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')
        deltas = {key: abs(now[key] - previous[key]) for key in fields}
        a = native_output(Path(now['summary_path']))
        b = native_output(Path(previous['summary_path']))
        poses = sorted(p.name for p in b.glob('*.txt') if not p.name.endswith('_evaluated_gt.txt'))
        saved = {name: (a / name).read_bytes() == (b / name).read_bytes() for name in poses}
        assert poses and all(saved.values()) and all(x <= 1e-9 for x in deltas.values())
        comparisons.append(dict(method=now['method'], sequence=now['sequence'],
                                metric_abs_delta=deltas, saved_pose_bytes_equal=saved))
    write(output, dict(checks=comparisons, exit_code=0))


def fetch(seq, folder):
    command = [PY, 'evaluation/scripts/fetch_kitti_odometry_sequences.py',
               '--output-root', str(R / 'kitti'), '--sequence', seq, '--workers', '4']
    for attempt in range(1, 4):
        attempt_dir = folder / 'fetch_attempts' / f'{attempt:02d}'
        if run('fetch', command, attempt_dir, allowed=(0, 1)) == 0:
            return
    raise RuntimeError('Fetch failed before native evaluation; bounded receipts retained.')


try:
    verify_binaries()
    frozen_selection = json.loads((E / 'selection.json').read_text())['rows']
    if options.resume_from:
        previous = PROTOCOL['order'][:PROTOCOL['order'].index(options.resume_from)]
        for seq in previous:
            path = E / 'sequences' / seq / 'cleanup.json'
            committed = subprocess.check_output(['git', 'show', 'HEAD:' + path.relative_to(REPO).as_posix()], cwd=REPO)
            cleanup = json.loads(committed)
            assert cleanup['hashes_verified_before_removal'] and not Path(cleanup['owned_input_dir']).exists()
            assert not (R / 'data/dogfooding_results' / f'kitti_seq_{seq}_full').is_symlink()
        target = E / 'sequences' / options.resume_from
        assert not any((target / name).exists() for name in ('inputs.json', 'audit', 'frozen_baseline', 'cleanup.json'))
        state['completed'] = previous
        if (target / 'fetch_attempts').exists():
            archive = target / ('failed_fetch_' + str(int(time.time())))
            (target / 'fetch_attempts').rename(archive)
    elif any((E / 'sequences' / seq).exists() for seq in PROTOCOL['order']):
        raise RuntimeError('Fresh execution requires absent sequence evidence.')

    (R / 'data/dogfooding_results').mkdir(parents=True, exist_ok=True)
    for seq in PROTOCOL['order'][len(state['completed']):]:
        update(status='fetching', sequence=seq)
        folder = E / 'sequences' / seq
        fetch(seq, folder)
        raw = R / 'kitti/dataset/sequences' / seq / 'velodyne'
        alias = R / 'data/dogfooding_results' / f'kitti_seq_{seq}_full'
        assert raw.is_dir() and not raw.is_symlink() and not alias.is_symlink() and not alias.exists()
        alias.symlink_to(raw, target_is_directory=True)
        with (REPO / 'experiments/reference_data' / f'kitti_seq_{seq}_full_gt.csv').open() as stream:
            frames = sum(1 for row in csv.DictReader(stream))
        assert [p.name for p in sorted(raw.glob('*.bin'))] == [f'{i:06d}.bin' for i in range(frames)]
        hashes = {p.name: dict(sha256=digest(p), bytes=p.stat().st_size)
                  for p in sorted(raw.iterdir()) if p.is_file()}
        gt_hash = digest(REPO / 'experiments/reference_data' / f'kitti_seq_{seq}_full_gt.csv')
        previous = json.loads((REPO / 'experiments/results/shared_coarse_start/sequences' / seq / 'inputs.json').read_text())
        assert hashes == previous['files'] and gt_hash == previous['gt_csv_sha256']
        write(folder / 'inputs.json', dict(sequence=seq, scan_frames=frames, gt_frames=frames,
              input_dir=str(raw), gt_csv_sha256=gt_hash, files=hashes, matches_previous_official_inputs=True))
        update(status='auditing', sequence=seq)
        report = audit(seq, folder / 'audit')
        selection = [r for r in frozen_selection if r['sequence'] == seq]
        assert {(r['method'], r['aggregate_path'], r['variant_id']) for r in report['rows']} == {
            (r['method'], r['repo_best_aggregate'], r['repo_best_variant']) for r in selection}
        controls = {r['method'] for r in report['rows'] if r['audit_status'] == 'drift'}
        if seq == '04':
            controls.update(('ELO', 'DiLO'))
        if controls:
            update(status='frozen_comparison', methods=sorted(controls))
            old = audit(seq, folder / 'frozen_baseline', methods=sorted(controls), frozen=True)
            compare_frozen(report, old, folder / 'frozen_comparison.json')
        update(status='verifying_cleanup')
        assert {p.name for p in raw.iterdir()} == set(hashes)
        assert all(digest(raw / name) == meta['sha256'] for name, meta in hashes.items())
        alias.unlink()
        shutil.rmtree(raw)
        write(folder / 'cleanup.json', dict(sequence=seq, owned_input_dir=str(raw),
              removed_files=len(hashes), removed_bytes=sum(m['bytes'] for m in hashes.values()),
              hashes_verified_before_removal=True, input_dir_exists_after=raw.exists()))
        subprocess.run(['git', 'add', '-f', str(folder)], cwd=REPO, check=True)
        subprocess.run(['git', 'commit', '-m', f'Record whole-zoo stored-cell replay on KITTI {seq} after range-filter extraction'], cwd=REPO, check=True)
        state['completed'].append(seq)
        update(status='sequence_committed', last_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip())
    update(status='complete', sequence=None)
    write(R / 'run_exit.json', dict(exit_code=0, completed=state['completed']))
except BaseException as error:
    update(status='failed', error=str(error))
    write(R / 'run_exit.json', dict(exit_code=1, sequence=state['sequence'], error=str(error)))
    traceback.print_exc()
    raise
