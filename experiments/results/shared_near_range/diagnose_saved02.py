"""Diagnose closed KITTI02/03 saved paths after frozen comparison and cleanup.

No estimator or scan reader is invoked. Uncommitted inputs are rejected before
creating output; historical path availability and runtime limits are retained.
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

from support import E, REPO, R, SOURCE, native_output, write

sys.path.insert(0, str(REPO / 'evaluation/scripts'))
from diagnose_kitti_steps import diagnose
from evaluate_external_kitti_odometry import load_kitti_poses, normalize_first_pose

METHODS = {'A-LOAM': ('aloam', 'A_LOAM.txt'), 'CT-ICP': ('ct_icp', 'CT_ICP.txt'),
           'F-LOAM': ('floam', 'F_LOAM.txt'), 'LeGO-LOAM': ('lego_loam', 'LeGO_LOAM.txt'),
           'Mesh-LOAM': ('mesh_loam', 'Mesh_LOAM.txt')}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sequence', choices=('02', '03'), default='02')
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    sequence = args.sequence
    if args.output_dir.exists():
        parser.error('Use a fresh directory; preserve previous evidence.')
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    sources = {}

    def committed(path):
        relative = path.relative_to(REPO).as_posix()
        probe = subprocess.run(['git', 'show', revision + ':' + relative], cwd=REPO,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if probe.returncode or not path.is_file():
            parser.error('Required input is not committed at ' + revision + ': ' + relative)
        raw = probe.stdout
        if raw != path.read_bytes():
            parser.error('Required input differs from committed bytes: ' + relative)
        sources[relative] = hashlib.sha256(raw).hexdigest()
        return raw

    def require(condition, message):
        if not condition:
            parser.error(message)

    def resolve_summary_path(row):
        # Receipts retain the execution checkout. Read its committed relative
        # artifact in this checkout so an offline worktree needs no live inputs.
        recorded_root = Path(row.get('cwd', str(REPO)))
        recorded_path = Path(row['summary_path'])
        try:
            relative = recorded_path.relative_to(recorded_root)
        except ValueError:
            parser.error('Saved summary is outside its recorded checkout')
        require(recorded_root.is_absolute() and '..' not in relative.parts,
                'Saved summary has an invalid recorded checkout path')
        return REPO / relative

    committed(Path(__file__))
    committed(E / 'support.py')
    execution_protocol = json.loads(committed(E / 'protocol.json'))
    protocol = json.loads(committed(E / f'diagnostics/{sequence}_protocol.json'))
    require(protocol['sequence'] == sequence, 'Diagnostic protocol sequence differs')
    cohort = set(METHODS) if sequence == '02' else {'Mesh-LOAM'}
    require(set(protocol['methods']) == cohort, 'Diagnostic protocol cohort differs')
    methods = {name: details for name, details in METHODS.items() if name in cohort}
    report = json.loads(committed(E / f'sequences/{sequence}/audit/report.json'))
    command = json.loads(committed(E / f'sequences/{sequence}/audit/audit_command.json'))
    candidate = json.loads(committed(E / 'candidate.json'))
    wrapper_sha256 = hashlib.sha256(committed(E / 'native_wrapper.py')).hexdigest()
    require(SOURCE == execution_protocol['implementation_commit'] == candidate['source_commit'],
            'Candidate source does not match the frozen execution protocol')
    baseline = json.loads(committed(E / 'baseline.json'))
    frozen = json.loads(committed(E / f'sequences/{sequence}/frozen_baseline/report.json'))
    frozen_command = json.loads(committed(E / f'sequences/{sequence}/frozen_baseline/audit_command.json'))
    comparison = json.loads(committed(E / f'sequences/{sequence}/frozen_comparison.json'))
    cleanup = json.loads(committed(E / f'sequences/{sequence}/cleanup.json'))
    require(report['exit_code'] == command['exit_code'] == 1,
            'Candidate audit must retain its recorded historical-drift exit')
    require(report['binary_sha256'] == wrapper_sha256, 'Candidate audit wrapper pin differs')
    require(all(r['process_exit_code'] == 0 for r in report['rows']), 'Candidate native run failed')
    selected = {r['method']: r for r in report['rows'] if r['audit_status'] == 'drift'}
    require(set(selected) == cohort, 'Historical-difference cohort differs')
    require(frozen['exit_code'] == frozen_command['exit_code'] and frozen['exit_code'] in (0, 1),
            'Frozen audit did not finish with an allowed recorded exit')
    require(frozen['binary_sha256'] == wrapper_sha256, 'Frozen audit wrapper pin differs')
    require({r['method'] for r in frozen['rows']} == set(selected), 'Frozen control cohort differs')
    require(all(r['process_exit_code'] == 0 for r in frozen['rows']), 'Frozen native run failed')
    require(comparison['exit_code'] == 0, 'Frozen comparison failed')
    require({r['method'] for r in comparison['checks']} == set(selected), 'Frozen comparison cohort differs')
    for check in comparison['checks']:
        require(check['sequence'] == sequence, 'Frozen comparison sequence differs')
        require(all(0 <= value <= 1e-9 for value in check['metric_abs_delta'].values()),
                'Frozen comparison metric difference exceeds tolerance')
        require(check['saved_pose_bytes_equal'] and all(check['saved_pose_bytes_equal'].values()),
                'Frozen comparison saved poses differ')
    require(cleanup['sequence'] == sequence and cleanup['hashes_verified_before_removal'],
            'Owned input cleanup was not verified')
    require(not cleanup['input_dir_exists_after'] and not Path(cleanup['owned_input_dir']).exists(),
            'Owned input cleanup is incomplete')
    alias = R / f'data/dogfooding_results/kitti_seq_{sequence}_full'
    require(not alias.exists() and not alias.is_symlink(), 'Owned input alias still exists')
    for name in ('diagnose_kitti_steps.py', 'evaluate_external_kitti_odometry.py'):
        committed(REPO / 'evaluation/scripts' / name)
    pp = protocol['diagnostic_parameters']

    # Check every required saved path before writing any diagnostic artifact.
    for method, (_, pose_name) in methods.items():
        current_path = resolve_summary_path(selected[method])
        frozen_path = resolve_summary_path(next(r for r in frozen['rows'] if r['method'] == method))
        for summary_path, expected_hash in ((current_path, candidate['sha256']),
                                            (frozen_path, baseline['sha256'])):
            committed(summary_path)
            receipt = json.loads(committed(summary_path.with_name(summary_path.stem + '_native_command.json')))
            require(receipt['exit_code'] == 0 and receipt['binary_sha256'] == expected_hash
                    and 'finished_unix' in receipt, 'Native receipt is incomplete or differs from binary pin')
        for name in (pose_name, 'gt.txt', f'kitti_seq_{sequence}_full_evaluated_gt.txt'):
            require(committed(native_output(current_path) / name) == committed(native_output(frozen_path) / name),
                    'Committed current/frozen saved paths differ: ' + name)
        aggregate = json.loads(committed(REPO / selected[method]['aggregate_path']))
        variant = next(v for v in aggregate['variants'] if v['id'] == selected[method]['variant_id'])
        legacy_path = REPO / variant['summary_path']
        probe = subprocess.run(['git', 'cat-file', '-e', revision + ':' + variant['summary_path']],
                               cwd=REPO, capture_output=True)
        require(probe.returncode in (0, 128), 'Historical-path availability probe failed')
        if probe.returncode == 0:
            for path in (legacy_path, legacy_path.parent / 'trajectory_kitti.txt',
                         legacy_path.parent / 'evaluated_gt_kitti.txt'):
                committed(path)

    def steps(estimate, gt, summary, method):
        committed(estimate)
        committed(gt)
        expected = next(m for m in summary['methods'] if m['name'] == method)
        diagnostic = diagnose(load_kitti_poses(estimate), load_kitti_poses(gt),
                              block_frames=pp['block_frames'], min_step_m=pp['min_step_m'],
                              relative_threshold=pp['relative_step_threshold'])
        delta = {k: abs(diagnostic['official_metrics_from_saved_poses'][k] - expected[k])
                 for k in ('ate_m', 'rpe_trans_pct', 'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')}
        require(all(v <= 0.001 for v in delta.values()), 'Saved-pose metrics disagree with summary: ' + method)
        est = np.asarray(normalize_first_pose(load_kitti_poses(estimate)))
        reference = np.asarray(normalize_first_pose(load_kitti_poses(gt)))
        errors = np.linalg.norm(est[:, :3, 3] - reference[:, :3, 3], axis=1)
        return diagnostic, delta, est, errors

    results = []
    for method, (stem, pose_name) in methods.items():
        row = selected[method]
        current_path = resolve_summary_path(row)
        current = json.loads(committed(current_path))
        receipt = json.loads(committed(current_path.with_name(current_path.stem + '_native_command.json')))
        require(receipt['exit_code'] == 0 and receipt['binary_sha256'] == candidate['sha256']
                and 'finished_unix' in receipt, 'Candidate native receipt differs: ' + method)
        frozen_row = next(r for r in frozen['rows'] if r['method'] == method)
        frozen_path = resolve_summary_path(frozen_row)
        committed(frozen_path)
        frozen_receipt = json.loads(committed(frozen_path.with_name(frozen_path.stem + '_native_command.json')))
        require(frozen_receipt['exit_code'] == 0 and frozen_receipt['binary_sha256'] == baseline['sha256']
                and 'finished_unix' in frozen_receipt, 'Frozen native receipt differs: ' + method)
        current_root = native_output(current_path)
        current_estimate = current_root / pose_name
        current_gt = current_root / f'kitti_seq_{sequence}_full_evaluated_gt.txt'
        now, delta, current_poses, current_errors = steps(current_estimate, current_gt, current, method)
        require(now['frames'] == row['frames'], 'Saved-pose frame count differs: ' + method)
        write(args.output_dir / stem / 'current_steps.json', now)
        worst = max(now['blocks'], key=lambda b: b['position_error_end_m'] - b['position_error_start_m'])
        aggregate = json.loads(committed(REPO / row['aggregate_path']))
        variant = next(v for v in aggregate['variants'] if v['id'] == row['variant_id'])
        require(variant['args'] == row['args'], 'Registration arguments differ from selected historical cell')
        legacy_path = REPO / variant['summary_path']
        probe_command = ['git', 'cat-file', '-e', revision + ':' + variant['summary_path']]
        probe = subprocess.run(probe_command, cwd=REPO, capture_output=True, text=True)
        require(probe.returncode in (0, 128), 'Historical-path availability probe failed')
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
            require(committed(old_gt) == committed(current_gt), 'Historical/current evaluated ground truths differ')
            old, old_delta, old_poses, old_errors = steps(old_estimate, old_gt, legacy, method)
            original = next(m for m in legacy['methods'] if m['name'] == method)
            require(original['rpe_trans_pct'] == row['stored_rpe_trans_pct']
                    and original['ate_m'] == row['stored_ate_m'] and len(old_poses) == len(current_poses),
                    'Historical summary or path length differs from the stored cell')
            write(args.output_dir / stem / 'historical_steps.json', old)
            gap = np.linalg.norm(current_poses[:, :3, 3] - old_poses[:, :3, 3], axis=1)
            blocks = []
            for block in now['blocks']:
                first, last = block['first_pose_frame'], block['last_pose_frame']
                blocks.append(dict(first_pose_frame=first, last_pose_frame=last,
                                   gap_start_m=float(gap[first]), gap_end_m=float(gap[last]),
                                   gap_growth_m=float(gap[last] - gap[first])))
            result.update(paired_historical_comparison_available=True,
                          historical_saved_pose_metric_abs_delta=old_delta,
                          same_evaluated_gt_bytes=True, current_environment=receipt['environment'],
                          first_different_saved_pose_row=next((i for i, (a, b) in enumerate(zip(
                              old_estimate.read_bytes().splitlines(), current_estimate.read_bytes().splitlines())) if a != b), None),
                          final_path_gap_m=float(gap[-1]), max_path_gap_m=float(gap.max()),
                          max_path_gap_frame=int(gap.argmax()), paired_blocks=blocks,
                          largest_path_gap_growth_block=max(blocks, key=lambda b: b['gap_growth_m']))
            if method == 'Mesh-LOAM':
                environment = json.loads(committed(REPO / 'experiments/results/mesh_inten_kitti_coverage/environment.json'))
                committed(REPO / f'experiments/results/mesh_inten_kitti_coverage/seq_{sequence}_commands.log')
                core = {}
                for name in ('src/mesh_loam.cpp', 'include/mesh_loam/mesh_loam.h'):
                    path = 'papers/mesh_loam/' + name
                    before = subprocess.check_output(['git', 'show', environment['evaluator_git_commit'] + ':' + path], cwd=REPO)
                    after = committed(REPO / path)
                    require(before == after, 'Historical/current Mesh core source differs: ' + path)
                    core[path] = dict(recorded_source_commit=environment['evaluator_git_commit'],
                                      current_source_commit=SOURCE, sha256=hashlib.sha256(after).hexdigest(), identical=True)
                result.update(historical_core_identity=core,
                              historical_binary_sha256=environment['binary_sha256'],
                              historical_environment=environment['environment'])
            columns += ['historical_position_error_m', 'path_gap_m']
            trace = [[i, float(current_errors[i]), float(old_errors[i]), float(gap[i])] for i in range(len(gap))]
            fig, axes = plt.subplots(2, 1, figsize=(10, 6), sharex=True, layout='constrained')
            axes[0].plot(old_errors, label='Historical ' + method, color='#4c78a8')
            axes[0].plot(current_errors, label='Current ' + method, color='#f58518', linestyle='--')
            axes[0].set_ylabel('Position error vs GT [m]')
            axes[0].legend(frameon=False)
            axes[1].plot(gap, color='#7b4fa3')
            axes[1].set(xlabel='Pose frame (zero based)', ylabel='Gap between paths [m]')
            for ax in axes:
                ax.grid(alpha=0.2)
                ax.spines[['top', 'right']].set_visible(False)
            fig.suptitle('KITTI ' + sequence + ': saved ' + method + ' paths\nFirst-pose normalization; no fitted alignment or rescaling; runtime not controlled')
            fig.savefig(args.output_dir / stem / 'paired_trace.png', dpi=180)
            plt.close(fig)
        with (args.output_dir / stem / 'trace.csv').open('w', newline='') as stream:
            writer = csv.writer(stream, lineterminator='\n')
            writer.writerow(columns)
            writer.writerows(trace)
        results.append(result)
    require(all(hashlib.sha256((REPO / name).read_bytes()).hexdigest() == expected
                for name, expected in sources.items()), 'Inputs changed during analysis')
    write(args.output_dir / 'summary.json', dict(command=[sys.executable, *sys.argv], cwd=str(REPO),
          input_commit=revision, implementation_commit=SOURCE, sequence=sequence, protocol=protocol,
          input_sha256=sources, candidate_binary_sha256=candidate['sha256'],
          audit_wrapper_sha256=wrapper_sha256, rows=results,
          completion_gate=dict(committed_candidate=True, committed_frozen_comparison=True,
                               committed_cleanup=True, owned_scans_and_alias_absent=True),
          exit_code=0, scope='Offline saved-path diagnosis after committed matching frozen controls and cleanup. '
                             'Blocks locate error growth, not its cause. No estimator/scan read, setting selection, '
                             'new accuracy claim or current-main whole-zoo validation.'))
    print(json.dumps(dict(sequence=sequence, methods=list(methods), exit_code=0)))


if __name__ == '__main__':
    main()
