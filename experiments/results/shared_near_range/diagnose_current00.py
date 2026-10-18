"""Locate error growth in committed current00 replays, without rerunning odometry.

The referenced legacy per-run summaries are checked separately. Current-pose
diagnostics cannot locate a difference from an unavailable historical path.
"""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from support import E, REPO, SOURCE, digest, native_output, run, write

sys.path.insert(0, str(REPO / 'evaluation/scripts'))
from evaluate_external_kitti_odometry import load_kitti_poses, normalize_first_pose

METHODS = {'A-LOAM': ('aloam', 'A_LOAM.txt'), 'F-LOAM': ('floam', 'F_LOAM.txt'),
           'LeGO-LOAM': ('lego_loam', 'LeGO_LOAM.txt'),
           'Mesh-LOAM': ('mesh_loam', 'Mesh_LOAM.txt'), 'VLOM': ('vlom', 'VLOM.txt')}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    if args.output_dir.exists():
        parser.error('Use a fresh output directory; preserve previous evidence.')
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    sources = {}

    def committed(path):
        relative = path.relative_to(REPO).as_posix()
        raw = subprocess.check_output(['git', 'show', revision + ':' + relative], cwd=REPO)
        assert raw == path.read_bytes(), relative
        sources[relative] = hashlib.sha256(raw).hexdigest()
        return raw

    report = json.loads(committed(E / 'sequences/00/audit/report.json'))
    audit_command = json.loads(committed(E / 'sequences/00/audit/audit_command.json'))
    candidate = json.loads(committed(E / 'candidate.json'))
    assert report['exit_code'] == audit_command['exit_code'] == 1
    selected = {row['method']: row for row in report['rows'] if row['audit_status'] == 'drift'}
    assert set(selected) == set(METHODS)
    tool = REPO / 'evaluation/scripts/diagnose_kitti_steps.py'
    committed(tool)
    committed(REPO / 'evaluation/scripts/evaluate_external_kitti_odometry.py')
    traces, rows = {}, []
    for method, (stem, pose_name) in METHODS.items():
        row = selected[method]
        assert row['process_exit_code'] == 0
        aggregate = json.loads(committed(REPO / row['aggregate_path']))
        variant = next(v for v in aggregate['variants'] if v['id'] == row['variant_id'])
        legacy_path = variant['summary_path']
        probe_command = ['git', 'cat-file', '-e', revision + ':' + legacy_path]
        probe = subprocess.run(probe_command, cwd=REPO, capture_output=True, text=True)
        assert probe.returncode in (0, 128), probe.stderr
        history_command = ['git', 'log', '--all', '--format=%H', '-1', '--', legacy_path]
        history = subprocess.check_output(history_command, cwd=REPO, text=True).strip()
        summary_path = Path(row['summary_path'])
        summary = json.loads(committed(summary_path))
        receipt = json.loads(committed(summary_path.with_name(summary_path.stem + '_native_command.json')))
        assert receipt['exit_code'] == 0 and receipt['binary_sha256'] == candidate['sha256']
        root = native_output(summary_path)
        estimate_path, gt_path = root / pose_name, root / 'kitti_seq_00_full_evaluated_gt.txt'
        committed(estimate_path)
        committed(gt_path)
        folder = args.output_dir / stem
        run('steps', [sys.executable, str(tool), '--estimate', str(estimate_path),
            '--gt', str(gt_path), '--summary', str(summary_path), '--method', method,
            '--output', str(folder / 'steps.json')], folder)
        diagnostic = json.loads((folder / 'steps.json').read_text())
        metrics = diagnostic['official_metrics_from_saved_poses']
        expected = next(m for m in summary['methods'] if m['name'] == method)
        deltas = {key: abs(metrics[key] - expected[key]) for key in
                  ('ate_m', 'rpe_trans_pct', 'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')}
        assert all(value <= 0.001 for value in deltas.values()), (method, deltas)
        assert diagnostic['frames'] == row['frames']
        est = np.asarray(normalize_first_pose(load_kitti_poses(estimate_path)))
        gt = np.asarray(normalize_first_pose(load_kitti_poses(gt_path)))
        errors = np.linalg.norm(est[:, :3, 3] - gt[:, :3, 3], axis=1)
        traces[method] = errors
        worst = max(diagnostic['blocks'], key=lambda b: b['position_error_end_m'] - b['position_error_start_m'])
        assert abs(errors[worst['last_pose_frame']] - worst['position_error_end_m']) < 1e-8
        rows.append(dict(method=method, stored_rpe_trans_pct=row['stored_rpe_trans_pct'],
            replayed_rpe_trans_pct=row['rerun_rpe_trans_pct'], stored_ate_m=row['stored_ate_m'],
            replayed_ate_m=row['rerun_ate_m'], frames=row['frames'],
            moving_step_count=diagnostic['moving_step_count'],
            steps_length_off_over_threshold=diagnostic['steps_length_off_over_threshold'],
            steps_vector_off_over_threshold=diagnostic['steps_vector_off_over_threshold'],
            largest_position_error_growth_block=worst,
            saved_pose_metric_abs_delta=deltas, saved_pose_metric_check_abs_tolerance=0.001,
            legacy_summary_path=legacy_path, legacy_summary_exists=(REPO / legacy_path).is_file(),
            legacy_git_probe=dict(command=probe_command, exit_code=probe.returncode, stderr=probe.stderr),
            legacy_history_probe=dict(command=history_command, latest_reachable_commit=history or None),
            paired_historical_path_comparison_available=False,
            diagnostic_path=str(folder / 'steps.json')))

    args.output_dir.mkdir(parents=True, exist_ok=True)
    with (args.output_dir / 'trace.csv').open('w', newline='') as stream:
        writer = csv.writer(stream, lineterminator='\n')
        writer.writerow(['frame', *[METHODS[m][0] + '_position_error_m' for m in METHODS]])
        for frame in range(len(next(iter(traces.values())))):
            writer.writerow([frame, *[float(traces[m][frame]) for m in METHODS]])
    fig, ax = plt.subplots(figsize=(10, 5), layout='constrained')
    colors = ['#4c78a8', '#f58518', '#54a24b', '#b279a2', '#204b70']
    for (method, values), color in zip(traces.items(), colors):
        ax.plot(values, label=method, color=color, linewidth=1.4,
                linestyle='--' if method == 'VLOM' else '-')
    ax.set(xlabel='Pose frame (zero based)', ylabel='Position error against evaluated GT [m]')
    ax.grid(alpha=0.2)
    ax.spines[['top', 'right']].set_visible(False)
    ax.legend(frameon=False, ncols=3)
    ax.set_title('Current KITTI 00 replays: saved-pose error growth\n'
                 'First-pose normalization; no fitted alignment or rescaling; historical paths unresolved')
    fig.savefig(args.output_dir / 'trace.png', dpi=180)
    fig.savefig(args.output_dir / 'trace.svg', metadata={'Date': None})
    plt.close(fig)
    write(args.output_dir / 'summary.json', dict(command=[sys.executable, *sys.argv],
        cwd=str(REPO), input_commit=revision, implementation_commit=SOURCE,
        script_sha256=digest(Path(__file__)), input_sha256=sources,
        shallow_history=subprocess.check_output(['git', 'rev-parse', '--is-shallow-repository'], cwd=REPO, text=True).strip(),
        diagnostic_parameters=dict(block_frames=100, min_step_m=0.1, relative_step_threshold=0.2),
        block_selection='Largest position_error_end_m minus position_error_start_m among all tool blocks; final partial block included.',
        position_error_definition='Euclidean position difference after the existing tool first-pose normalization; no fitted alignment or rescaling.',
        rows=rows, exit_code=0,
        scope='Current committed00 replay paths only. Blocks are diagnostics, not official KITTI RTE or localization of the difference from an unavailable historical path. No native rerun, new setting, cleanup or whole-zoo completion claim.'))
    print(json.dumps(dict(methods=list(METHODS), exit_code=0)))


if __name__ == '__main__':
    main()
