#!/usr/bin/env python3
"""KITTI形式のポーズファイルを読み込んで軌跡を比較プロット"""

import argparse
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np


def load_poses(path):
    """KITTI形式 (各行12値) のポーズファイルを読み込み"""
    poses = []
    with open(path) as f:
        for line in f:
            values = [float(v) for v in line.strip().split()]
            if len(values) == 12:
                T = np.eye(4)
                T[:3, :] = np.array(values).reshape(3, 4)
                poses.append(T)
    return poses


def compute_ate(est_poses, gt_poses):
    """ATE (RMSE) を計算"""
    n = min(len(est_poses), len(gt_poses))
    errors = []
    for i in range(n):
        err = np.linalg.norm(est_poses[i][:3, 3] - gt_poses[i][:3, 3])
        errors.append(err * err)
    return np.sqrt(np.mean(errors))


def main():
    parser = argparse.ArgumentParser(description='Plot KITTI trajectories')
    parser.add_argument('--gt', required=True, help='Ground truth poses file')
    parser.add_argument('--est', nargs='+', required=True,
                        help='Estimated poses files (method:path)')
    parser.add_argument('--output', default='trajectory.png',
                        help='Output image path')
    args = parser.parse_args()

    gt_poses = load_poses(args.gt)
    gt_xy = np.array([T[:3, 3] for T in gt_poses])

    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    # 軌跡プロット
    ax = axes[0]
    ax.plot(gt_xy[:, 0], gt_xy[:, 1], 'k-', label='Ground Truth', linewidth=2)

    results = []
    for est_spec in args.est:
        parts = est_spec.split(':')
        name = parts[0]
        path = parts[1]
        est_poses = load_poses(path)
        est_xy = np.array([T[:3, 3] for T in est_poses])
        ax.plot(est_xy[:, 0], est_xy[:, 1], '--', label=name, linewidth=1.5)

        ate = compute_ate(est_poses, gt_poses)
        results.append((name, ate, len(est_poses)))

    ax.set_xlabel('X [m]')
    ax.set_ylabel('Y [m]')
    ax.set_title('Trajectory Comparison')
    ax.legend()
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.3)

    # ATE棒グラフ
    ax2 = axes[1]
    names = [r[0] for r in results]
    ates = [r[1] for r in results]
    bars = ax2.bar(names, ates, color=['#1f77b4', '#ff7f0e', '#2ca02c',
                                        '#d62728', '#9467bd'][:len(names)])
    ax2.set_ylabel('ATE RMSE [m]')
    ax2.set_title('Absolute Trajectory Error')
    for bar, ate in zip(bars, ates):
        ax2.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.1,
                 f'{ate:.2f}m', ha='center', va='bottom', fontsize=10)

    plt.tight_layout()
    plt.savefig(args.output, dpi=150)
    print(f'Saved to {args.output}')

    # テーブル出力
    print('\n=== Results ===')
    print(f'{"Method":<15} {"ATE [m]":<10} {"Frames":<10}')
    print('-' * 35)
    for name, ate, n in results:
        print(f'{name:<15} {ate:<10.3f} {n:<10}')


if __name__ == '__main__':
    main()
