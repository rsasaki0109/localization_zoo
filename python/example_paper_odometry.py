#!/usr/bin/env python3
"""Run one paper odometry pipeline on stored scans, without ground-truth seeding."""
import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
METHODS = {
    'suma_paper': 'SumaPaper',
    'imls_slam_paper': 'ImlsPaperOdometry',
    'dilo_paper': 'DiloPaperOdometry',
    'litamin2_paper': 'Litamin2Odometry',
}


def load_scans(directory):
    sys.path.insert(0, str(ROOT / 'evaluation/scripts'))
    from check_scan_frame import load_scan
    paths = sorted(p for p in Path(directory).rglob('*') if p.suffix.lower() in {'.pcd', '.bin'})
    if not paths:
        raise ValueError(f'no PCD or KITTI XYZI scans in {directory}')
    # The committed PCD fixture stores float32 coordinates, as PointCloud2 does.
    return [(p, np.ascontiguousarray(load_scan(p).points, dtype=np.float64)) for p in paths]


def process(odometry, method, scan):
    result = odometry.process(scan)
    if method == 'dilo_paper':
        return result, len(odometry.window_poses), len(odometry.finalized_poses)
    if method == 'suma_paper':
        return result.pose, result.num_surfels, result.inliers
    if method == 'imls_slam_paper':
        return result.pose, result.model_points, result.samples
    return result.pose, result.map_distributions, result.correspondences


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('method', choices=METHODS)
    parser.add_argument('scans', nargs='?', type=Path, default=ROOT / 'evaluation/fixtures/mcd_kth_smoke')
    parser.add_argument('--module-dir', type=Path, default=ROOT / 'build/python')
    parser.add_argument('--output', type=Path, help='save one row-major world<-scan pose per row')
    args = parser.parse_args()
    sys.path.insert(0, str(args.module_dir.resolve()))
    import localization_zoo as lz
    odometry = getattr(lz, METHODS[args.method])()
    poses = []
    for path, points in load_scans(args.scans):
        pose, state, support = process(odometry, args.method, points)
        if not np.isfinite(pose).all():
            raise RuntimeError(f'{path}: nonfinite pose')
        poses.append(pose.reshape(-1))
        print(f'{path.name}: position={pose[:3, 3]} state={state} support={support}')
    if args.output:
        np.savetxt(args.output, poses, delimiter=',', fmt='%.17g')


if __name__ == '__main__':
    main()
