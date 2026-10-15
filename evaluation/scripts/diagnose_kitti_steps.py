#!/usr/bin/env python3
"""Locate odometry drift and steps whose length differs from GT by over 20%.

This reads saved trajectories only; it never changes or reruns an estimator.
Block drift is a diagnostic over a fixed number of frames, not official KITTI RTE.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import shlex
import sys

import numpy as np

from evaluate_external_kitti_odometry import (
    compute_metrics,
    load_kitti_poses,
    normalize_first_pose,
    rotation_angle,
)


def contiguous_ranges(indices: list[int]) -> list[list[int]]:
    ranges: list[list[int]] = []
    for index in indices:
        if ranges and index == ranges[-1][1] + 1:
            ranges[-1][1] = index
        else:
            ranges.append([index, index])
    return ranges


def diagnose(
    estimates: list[np.ndarray],
    references: list[np.ndarray],
    *,
    block_frames: int = 100,
    min_step_m: float = 0.1,
    relative_threshold: float = 0.2,
) -> dict:
    if len(estimates) != len(references) or len(estimates) < 2:
        raise ValueError('Complete, equal-length trajectories with at least two poses are required')
    if block_frames < 1 or min_step_m <= 0 or relative_threshold <= 0:
        raise ValueError('Diagnostic thresholds must be positive')
    est = np.asarray(normalize_first_pose(estimates))
    gt = np.asarray(normalize_first_pose(references))
    if not (np.isfinite(est).all() and np.isfinite(gt).all()):
        raise ValueError('Non-finite trajectory')
    est_delta = np.linalg.inv(est[:-1]) @ est[1:]
    gt_delta = np.linalg.inv(gt[:-1]) @ gt[1:]
    gt_step = np.linalg.norm(gt_delta[:, :3, 3], axis=1)
    est_step = np.linalg.norm(est_delta[:, :3, 3], axis=1)
    moving = gt_step >= min_step_m
    ratios = np.divide(est_step, gt_step, out=np.ones_like(gt_step), where=moving)
    vector_error = np.linalg.norm(est_delta[:, :3, 3] - gt_delta[:, :3, 3], axis=1)
    relative_vector_error = np.divide(vector_error, gt_step, out=np.zeros_like(gt_step), where=moving)
    bad_length = moving & (np.abs(ratios - 1) > relative_threshold)
    bad_vector = moving & (relative_vector_error > relative_threshold)
    position_error = np.linalg.norm(est[:, :3, 3] - gt[:, :3, 3], axis=1)
    step_frames = np.arange(1, len(gt))
    blocks = []
    for start in range(1, len(gt), block_frames):
        end = min(start + block_frames, len(gt))
        mask = (step_frames >= start) & (step_frames < end)
        active = mask & moving
        first, last = start - 1, end - 1
        distance = float(gt_step[mask].sum())
        error = np.linalg.inv(np.linalg.inv(est[first]) @ est[last]) @ (np.linalg.inv(gt[first]) @ gt[last])
        blocks.append({
            'first_pose_frame': first,
            'last_pose_frame': last,
            'gt_distance_m': distance,
            'path_length_ratio': float(est_step[mask].sum() / distance) if distance else None,
            'moving_step_count': int(active.sum()),
            'step_length_ratio_median': float(np.median(ratios[active])) if active.any() else None,
            'steps_length_off_over_threshold': int((mask & bad_length).sum()),
            'steps_vector_off_over_threshold': int((mask & bad_vector).sum()),
            'position_error_start_m': float(position_error[first]),
            'position_error_end_m': float(position_error[last]),
            'block_relative_translation_error_pct': float(np.linalg.norm(error[:3, 3]) / distance * 100) if distance else None,
            'block_relative_rotation_error_deg_per_100m': math.degrees(rotation_angle(error[:3, :3])) / distance * 100 if distance else None,
        })
    return {
        'frames': len(gt),
        'official_metrics_from_saved_poses': compute_metrics(list(est), list(gt)),
        'definitions': {
            'frame_index': 'Zero-based; a step index is its destination frame (i-1 -> i).',
            'step_length_ratio': 'norm(estimated local translation) / norm(GT local translation)',
            'step_vector_error': 'norm(estimated local translation - GT local translation) / norm(GT local translation)',
            'moving_step': f'GT step >= {min_step_m} m; shorter steps excluded from relative step counts.',
            'block_error': 'Relative pose error between block endpoints, divided by GT path length; diagnostic only, not KITTI RTE.',
        },
        'parameters': {'block_frames': block_frames, 'min_step_m': min_step_m,
                       'relative_step_threshold': relative_threshold},
        'path_length_ratio': float(est_step.sum() / gt_step.sum()) if gt_step.sum() else None,
        'moving_step_count': int(moving.sum()),
        'step_length_ratio_median': float(np.median(ratios[moving])) if moving.any() else None,
        'steps_length_off_over_threshold': int(bad_length.sum()),
        'steps_vector_off_over_threshold': int(bad_vector.sum()),
        'bad_length_step_frame_ranges': contiguous_ranges(step_frames[bad_length].tolist()),
        'bad_vector_step_frame_ranges': contiguous_ranges(step_frames[bad_vector].tolist()),
        'blocks': blocks,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--estimate', type=Path, required=True)
    parser.add_argument('--gt', type=Path, required=True, help='Evaluated, sensor-frame KITTI GT saved by the same run')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--summary', type=Path, help='Assert saved-pose KITTI RTE matches this run summary')
    parser.add_argument('--method', help='Method name in --summary')
    args = parser.parse_args()
    result = diagnose(load_kitti_poses(args.estimate), load_kitti_poses(args.gt))
    if args.summary:
        if not args.method:
            parser.error('--method is required with --summary')
        summary = json.loads(args.summary.read_text())
        row = next(row for row in summary['methods'] if row['name'] == args.method)
        actual = result['official_metrics_from_saved_poses']['kitti_rte_trans_pct']
        if actual is None or abs(actual - row['kitti_rte_trans_pct']) > 0.001:
            raise ValueError('Saved-pose KITTI RTE disagrees with the benchmark summary')
        result['benchmark_summary'] = str(args.summary)
        result['benchmark_method'] = args.method
    result.update({'schema_version': 1, 'command': shlex.join(['python3', *sys.argv]),
                   'estimate_path': str(args.estimate), 'gt_path': str(args.gt)})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')


if __name__ == '__main__':
    main()
