#!/usr/bin/env python3
"""
マルチデータセット・マルチアルゴリズム ベンチマークスクリプト

使い方:
  # MulRan データセット
  python3 benchmark.py \
    --dataset mulran \
    --data_dir /path/to/MulRan/DCC01/Ouster \
    --gt_file /path/to/MulRan/DCC01/global_pose.csv \
    --methods litamin2 aloam \
    --output_dir results/

  # 結果比較
  python3 benchmark.py --compare results/
"""

import argparse
import csv
import os
import subprocess
import sys
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np


def load_poses_kitti(path):
    """KITTI形式 poses (12 float/line)"""
    poses = []
    with open(path) as f:
        for line in f:
            vals = [float(v) for v in line.strip().split()]
            if len(vals) == 12:
                T = np.eye(4)
                T[:3, :] = np.array(vals).reshape(3, 4)
                poses.append(T)
    return poses


def load_poses_tum(path):
    """TUM形式 (timestamp tx ty tz qx qy qz qw)"""
    from scipy.spatial.transform import Rotation
    poses = []
    with open(path) as f:
        for line in f:
            if line.startswith('#'):
                continue
            vals = [float(v) for v in line.strip().split()]
            if len(vals) >= 8:
                T = np.eye(4)
                T[:3, :3] = Rotation.from_quat(vals[4:8]).as_matrix()
                T[:3, 3] = vals[1:4]
                poses.append(T)
    return poses


def compute_ate(est, gt):
    """ATE RMSE [m]"""
    n = min(len(est), len(gt))
    if n == 0:
        return float('inf')
    errors = [np.linalg.norm(est[i][:3, 3] - gt[i][:3, 3]) for i in range(n)]
    return np.sqrt(np.mean(np.square(errors)))


def compute_rpe(est, gt, delta=100.0):
    """RPE (translation %, rotation deg/m) over segments of length delta"""
    n = min(len(est), len(gt))
    t_errs, r_errs = [], []

    for i in range(n):
        dist = 0
        j = i + 1
        while j < n:
            dist += np.linalg.norm(gt[j][:3, 3] - gt[j-1][:3, 3])
            if dist >= delta:
                break
            j += 1
        if j >= n:
            break

        dT_est = np.linalg.inv(est[i]) @ est[j]
        dT_gt = np.linalg.inv(gt[i]) @ gt[j]
        T_err = np.linalg.inv(dT_gt) @ dT_est

        t_err = np.linalg.norm(T_err[:3, 3])
        R_err = T_err[:3, :3]
        cos_angle = np.clip((np.trace(R_err) - 1) / 2, -1, 1)
        r_err = np.arccos(cos_angle)

        if dist > 0:
            t_errs.append(t_err / dist * 100)
            r_errs.append(np.degrees(r_err) / dist)

    if not t_errs:
        return 0, 0
    return np.mean(t_errs), np.mean(r_errs)


def plot_comparison(results, output_path):
    """結果を比較プロット"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    methods = [r['method'] for r in results]
    ates = [r['ate'] for r in results]
    rpe_ts = [r['rpe_trans'] for r in results]
    rpe_rs = [r['rpe_rot'] for r in results]

    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']

    ax = axes[0]
    bars = ax.bar(methods, ates, color=colors[:len(methods)])
    ax.set_ylabel('ATE RMSE [m]')
    ax.set_title('Absolute Trajectory Error')
    for b, v in zip(bars, ates):
        ax.text(b.get_x() + b.get_width()/2, b.get_height(), f'{v:.2f}',
                ha='center', va='bottom', fontsize=9)

    ax = axes[1]
    bars = ax.bar(methods, rpe_ts, color=colors[:len(methods)])
    ax.set_ylabel('RPE Translation [%]')
    ax.set_title('Relative Pose Error (Translation)')
    for b, v in zip(bars, rpe_ts):
        ax.text(b.get_x() + b.get_width()/2, b.get_height(), f'{v:.2f}',
                ha='center', va='bottom', fontsize=9)

    ax = axes[2]
    bars = ax.bar(methods, rpe_rs, color=colors[:len(methods)])
    ax.set_ylabel('RPE Rotation [deg/m]')
    ax.set_title('Relative Pose Error (Rotation)')
    for b, v in zip(bars, rpe_rs):
        ax.text(b.get_x() + b.get_width()/2, b.get_height(), f'{v:.4f}',
                ha='center', va='bottom', fontsize=9)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f'Saved comparison plot to {output_path}')


def compare_results(results_dir):
    """CSVファイルから結果を読み込んで比較"""
    csv_path = os.path.join(results_dir, 'results.csv')
    if not os.path.exists(csv_path):
        print(f'No results.csv found in {results_dir}')
        return

    results = []
    with open(csv_path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            results.append({
                'method': row['method'],
                'ate': float(row['ATE_m']),
                'rpe_trans': float(row['RPE_trans_pct']),
                'rpe_rot': float(row['RPE_rot_deg_per_m']),
            })

    if results:
        plot_comparison(results, os.path.join(results_dir, 'comparison.png'))

        print('\n=== Benchmark Results ===')
        print(f'{"Method":<15} {"ATE [m]":<12} {"RPE-t [%]":<12} {"RPE-r [°/m]":<12}')
        print('-' * 51)
        for r in results:
            print(f'{r["method"]:<15} {r["ate"]:<12.3f} {r["rpe_trans"]:<12.3f} {r["rpe_rot"]:<12.5f}')


def main():
    parser = argparse.ArgumentParser(description='Localization Zoo Benchmark')
    parser.add_argument('--compare', type=str, help='Compare results in directory')
    parser.add_argument('--dataset', choices=['kitti', 'mulran', 'nuscenes', 'tum'],
                        help='Dataset format')
    parser.add_argument('--data_dir', type=str, help='Point cloud directory')
    parser.add_argument('--gt_file', type=str, help='Ground truth file')
    parser.add_argument('--est_files', nargs='+',
                        help='Estimated pose files (method:path)')
    parser.add_argument('--output_dir', type=str, default='results')
    args = parser.parse_args()

    if args.compare:
        compare_results(args.compare)
        return

    if not args.est_files or not args.gt_file:
        parser.error('--gt_file and --est_files required for evaluation')

    # GT読み込み
    if args.dataset in ('kitti', 'mulran'):
        gt = load_poses_kitti(args.gt_file)
    elif args.dataset == 'tum':
        gt = load_poses_tum(args.gt_file)
    else:
        gt = load_poses_kitti(args.gt_file)

    os.makedirs(args.output_dir, exist_ok=True)

    results = []
    for spec in args.est_files:
        method, path = spec.split(':')
        if args.dataset == 'tum':
            est = load_poses_tum(path)
        else:
            est = load_poses_kitti(path)

        ate = compute_ate(est, gt)
        rpe_t, rpe_r = compute_rpe(est, gt)

        results.append({
            'method': method,
            'ate': ate,
            'rpe_trans': rpe_t,
            'rpe_rot': rpe_r,
        })

        print(f'{method}: ATE={ate:.3f}m, RPE-t={rpe_t:.3f}%, RPE-r={rpe_r:.5f}°/m')

    # CSV保存
    csv_path = os.path.join(args.output_dir, 'results.csv')
    with open(csv_path, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=[
            'method', 'ATE_m', 'RPE_trans_pct', 'RPE_rot_deg_per_m'])
        writer.writeheader()
        for r in results:
            writer.writerow({
                'method': r['method'],
                'ATE_m': f'{r["ate"]:.4f}',
                'RPE_trans_pct': f'{r["rpe_trans"]:.4f}',
                'RPE_rot_deg_per_m': f'{r["rpe_rot"]:.6f}',
            })

    plot_comparison(results, os.path.join(args.output_dir, 'comparison.png'))


if __name__ == '__main__':
    main()
