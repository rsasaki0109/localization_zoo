"""Diagnose committed KITTI01 paths; pair historical paths only when available."""
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

from support import E, REPO, SOURCE, digest, native_output, write

sys.path.insert(0, str(REPO / 'evaluation/scripts'))
from diagnose_kitti_steps import diagnose
from evaluate_external_kitti_odometry import load_kitti_poses, normalize_first_pose

METHODS = {'CT-ICP': ('ct_icp', 'CT_ICP.txt'), 'Mesh-LOAM': ('mesh_loam', 'Mesh_LOAM.txt')}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    if args.output_dir.exists():
        parser.error('Use a fresh directory; preserve previous evidence.')
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    sources = {}

    def committed(path):
        relative = path.relative_to(REPO).as_posix()
        raw = subprocess.check_output(['git', 'show', revision + ':' + relative], cwd=REPO)
        assert raw == path.read_bytes(), relative
        sources[relative] = hashlib.sha256(raw).hexdigest()
        return raw

    committed(Path(__file__))
    protocol = json.loads(committed(E / 'diagnostics/01_protocol.json'))
    report = json.loads(committed(E / 'sequences/01/audit/report.json'))
    command = json.loads(committed(E / 'sequences/01/audit/audit_command.json'))
    candidate = json.loads(committed(E / 'candidate.json'))
    assert report['exit_code'] == command['exit_code'] == 1
    selected = {r['method']: r for r in report['rows'] if r['audit_status'] == 'drift'}
    assert set(selected) == set(METHODS)
    for name in ('diagnose_kitti_steps.py', 'evaluate_external_kitti_odometry.py'):
        committed(REPO / 'evaluation/scripts' / name)
    pp = protocol['diagnostic_parameters']

    def steps(estimate, gt, summary, method):
        committed(estimate)
        committed(gt)
        expected = next(m for m in summary['methods'] if m['name'] == method)
        diagnostic = diagnose(load_kitti_poses(estimate), load_kitti_poses(gt),
                              block_frames=pp['block_frames'], min_step_m=pp['min_step_m'],
                              relative_threshold=pp['relative_step_threshold'])
        delta = {k: abs(diagnostic['official_metrics_from_saved_poses'][k] - expected[k])
                 for k in ('ate_m', 'rpe_trans_pct', 'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')}
        assert all(v <= 0.001 for v in delta.values()), (method, delta)
        est = np.asarray(normalize_first_pose(load_kitti_poses(estimate)))
        reference = np.asarray(normalize_first_pose(load_kitti_poses(gt)))
        errors = np.linalg.norm(est[:, :3, 3] - reference[:, :3, 3], axis=1)
        return diagnostic, delta, est, errors

    results = []
    for method, (stem, pose_name) in METHODS.items():
        row = selected[method]
        current_path = Path(row['summary_path'])
        current = json.loads(committed(current_path))
        receipt = json.loads(committed(current_path.with_name(current_path.stem + '_native_command.json')))
        assert receipt['exit_code'] == 0 and receipt['binary_sha256'] == candidate['sha256']
        current_root = native_output(current_path)
        current_estimate = current_root / pose_name
        current_gt = current_root / 'kitti_seq_01_full_evaluated_gt.txt'
        now, delta, current_poses, current_errors = steps(current_estimate, current_gt, current, method)
        assert now['frames'] == row['frames']
        write(args.output_dir / stem / 'current_steps.json', now)
        worst = max(now['blocks'], key=lambda b: b['position_error_end_m'] - b['position_error_start_m'])
        aggregate = json.loads(committed(REPO / row['aggregate_path']))
        variant = next(v for v in aggregate['variants'] if v['id'] == row['variant_id'])
        assert variant['args'] == row['args']
        legacy_path = REPO / variant['summary_path']
        probe_command = ['git', 'cat-file', '-e', revision + ':' + variant['summary_path']]
        probe = subprocess.run(probe_command, cwd=REPO, capture_output=True, text=True)
        assert probe.returncode in (0, 128)
        history_command = ['git', 'log', '--all', '-1', '--format=%H', '--', variant['summary_path']]
        history = subprocess.check_output(history_command, cwd=REPO, text=True).strip()
        result = dict(method=method, current_saved_pose_metric_abs_delta=delta,
                      metric_check_abs_tolerance=0.001, largest_current_error_growth_block=worst,
                      legacy_summary_path=variant['summary_path'], legacy_summary_exists=legacy_path.is_file(),
                      legacy_git_probe=dict(command=probe_command, exit_code=probe.returncode, stderr=probe.stderr),
                      legacy_history_probe=dict(command=history_command, latest_reachable_commit=history or None),
                      paired_historical_comparison_available=False)
        columns = ['frame', 'current_position_error_m']
        trace = [[i, float(error)] for i, error in enumerate(current_errors)]
        if probe.returncode == 0:
            legacy = json.loads(committed(legacy_path))
            old_estimate = legacy_path.parent / 'trajectory_kitti.txt'
            old_gt = legacy_path.parent / 'evaluated_gt_kitti.txt'
            assert committed(old_gt) == committed(current_gt), 'Evaluated ground truths differ'
            old, old_delta, old_poses, old_errors = steps(old_estimate, old_gt, legacy, method)
            original = next(m for m in legacy['methods'] if m['name'] == method)
            assert original['rpe_trans_pct'] == row['stored_rpe_trans_pct']
            assert original['ate_m'] == row['stored_ate_m'] and len(old_poses) == len(current_poses)
            write(args.output_dir / stem / 'historical_steps.json', old)
            gap = np.linalg.norm(current_poses[:, :3, 3] - old_poses[:, :3, 3], axis=1)
            blocks = []
            for block in now['blocks']:
                first, last = block['first_pose_frame'], block['last_pose_frame']
                blocks.append(dict(first_pose_frame=first, last_pose_frame=last,
                                   gap_start_m=float(gap[first]), gap_end_m=float(gap[last]),
                                   gap_growth_m=float(gap[last] - gap[first])))
            environment = json.loads(committed(REPO / 'experiments/results/mesh_inten_kitti_coverage/environment.json'))
            committed(REPO / 'experiments/results/mesh_inten_kitti_coverage/seq_01_commands.log')
            core = {}
            for name in ('src/mesh_loam.cpp', 'include/mesh_loam/mesh_loam.h'):
                path = 'papers/mesh_loam/' + name
                before = subprocess.check_output(['git', 'show', environment['evaluator_git_commit'] + ':' + path], cwd=REPO)
                after = committed(REPO / path)
                assert before == after
                core[path] = dict(recorded_source_commit=environment['evaluator_git_commit'],
                                  current_source_commit=SOURCE, sha256=hashlib.sha256(after).hexdigest(), identical=True)
            result.update(paired_historical_comparison_available=True,
                          historical_saved_pose_metric_abs_delta=old_delta,
                          same_evaluated_gt_bytes=True, historical_core_identity=core,
                          historical_binary_sha256=environment['binary_sha256'],
                          historical_environment=environment['environment'], current_environment=receipt['environment'],
                          first_different_saved_pose_row=next((i for i, (a, b) in enumerate(zip(
                              old_estimate.read_bytes().splitlines(), current_estimate.read_bytes().splitlines())) if a != b), None),
                          final_path_gap_m=float(gap[-1]), max_path_gap_m=float(gap.max()),
                          max_path_gap_frame=int(gap.argmax()), paired_blocks=blocks,
                          largest_path_gap_growth_block=max(blocks, key=lambda b: b['gap_growth_m']))
            columns += ['historical_position_error_m', 'path_gap_m']
            trace = [[i, float(current_errors[i]), float(old_errors[i]), float(gap[i])] for i in range(len(gap))]
            fig, axes = plt.subplots(2, 1, figsize=(10, 6), sharex=True, layout='constrained')
            axes[0].plot(old_errors, label='Historical Mesh-LOAM', color='#4c78a8')
            axes[0].plot(current_errors, label='Current Mesh-LOAM', color='#f58518', linestyle='--')
            axes[0].set_ylabel('Position error vs GT [m]')
            axes[0].legend(frameon=False)
            axes[1].plot(gap, color='#7b4fa3')
            axes[1].set(xlabel='Pose frame (zero based)', ylabel='Gap between paths [m]')
            for ax in axes:
                ax.grid(alpha=0.2)
                ax.spines[['top', 'right']].set_visible(False)
            fig.suptitle('KITTI 01: saved Mesh-LOAM paths\nFirst-pose normalization; no fitted alignment or rescaling; runtime differs')
            fig.savefig(args.output_dir / stem / 'paired_trace.png', dpi=180)
            plt.close(fig)
        with (args.output_dir / stem / 'trace.csv').open('w', newline='') as stream:
            writer = csv.writer(stream, lineterminator='\n')
            writer.writerow(columns)
            writer.writerows(trace)
        results.append(result)
    write(args.output_dir / 'summary.json', dict(command=[sys.executable, *sys.argv], cwd=str(REPO),
          input_commit=revision, implementation_commit=SOURCE, protocol=protocol,
          input_sha256=sources, candidate_binary_sha256=candidate['sha256'], rows=results,
          exit_code=0, scope='Offline saved-path diagnosis only. Historical/runtime causality, frozen controls and cleanup are not established by this analysis.'))
    print(json.dumps(dict(methods=list(METHODS), exit_code=0)))


if __name__ == '__main__':
    main()
