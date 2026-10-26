"""First-pose-anchored EuRoC stereo evaluation; no scale or trajectory fitting."""
import argparse
from bisect import bisect_left
import csv
import hashlib
import json
from pathlib import Path

import cv2
import numpy as np
from euroc_stereo import Rectifier, read_camera, read_index

FRAME = 'first_rectified_cam0_from_current_rectified_cam0'


def rigid(value):
    T = np.asarray(value, dtype=float)
    if (T.shape != (4, 4) or not np.isfinite(T).all()
            or not np.allclose(T[3], [0, 0, 0, 1])
            or not np.allclose(T[:3, :3].T @ T[:3, :3], np.eye(3), atol=1e-6)
            or not np.isclose(np.linalg.det(T[:3, :3]), 1, atol=1e-6)):
        raise ValueError('Expected finite rigid pose')
    return T


def ground_truth(path):
    stamps, poses = [], []
    with Path(path).open() as source:
        for row in csv.reader(line for line in source if line.strip() and not line.lstrip().startswith('#')):
            stamp = int(row[0])
            p = np.asarray(row[1:4], float)
            q = np.asarray(row[4:8], float)  # EuRoC w,x,y,z, world-from-body
            if (len(p) != 3 or len(q) != 4 or not np.isfinite(p).all()
                    or not np.isfinite(q).all() or not np.isclose(np.linalg.norm(q), 1, atol=1e-3)
                    or (stamps and stamp <= stamps[-1])):
                raise ValueError('Invalid ground truth')
            w, x, y, z = q / np.linalg.norm(q)
            T = np.eye(4)
            T[:3, :3] = [[1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w)],
                         [2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w)],
                         [2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y)]]
            T[:3, 3] = p
            stamps.append(stamp)
            poses.append(T)
    if not stamps:
        raise ValueError('Empty ground truth')
    return stamps, poses


def interpolate(stamps, poses, stamp, max_gap_ns):
    i = bisect_left(stamps, stamp)
    if i < len(stamps) and stamps[i] == stamp:
        return poses[i]
    if i == 0 or i == len(stamps) or stamps[i] - stamps[i-1] > max_gap_ns:
        return None  # Never extrapolate or bridge long missing intervals.
    fraction = (stamp - stamps[i-1]) / (stamps[i] - stamps[i-1])
    a, b = poses[i-1], poses[i]
    T = np.eye(4)
    rotation = cv2.Rodrigues(a[:3, :3].T @ b[:3, :3])[0]
    T[:3, :3] = a[:3, :3] @ cv2.Rodrigues(rotation * fraction)[0]
    T[:3, 3] = a[:3, 3] + fraction * (b[:3, 3] - a[:3, 3])
    return T


def evaluate(mav0, trajectory, max_gap_ns=20000000):
    if type(max_gap_ns) is not int or max_gap_ns <= 0:
        raise ValueError('GT gap must be positive integer nanoseconds')
    mav0, trajectory = Path(mav0), Path(trajectory)
    expected = [t for t, _ in read_index(mav0 / 'cam0')]
    if expected != [t for t, _ in read_index(mav0 / 'cam1')]:
        raise ValueError('Stereo timestamp lists differ')
    left, right = [read_camera(mav0 / c / 'sensor.yaml') for c in ('cam0', 'cam1')]
    rect = Rectifier(left, right)
    T_B_rect = left[3] @ np.linalg.inv(rect.T_rect_raw)
    gt_path = mav0 / 'state_groundtruth_estimate0/data.csv'
    stamps, poses = ground_truth(gt_path)
    rows = [json.loads(line) for line in trajectory.read_text().splitlines() if line.strip()]
    if len(rows) > len(expected):
        raise ValueError('Too many trajectory rows')
    valid, lost = [], []
    has_lost = False
    for index, row in enumerate(rows):
        stamp = row['timestamp_ns']
        if type(stamp) is not int or stamp != expected[index]:
            raise ValueError('Trajectory must be an exact prefix of the camera index')
        if (row['pose_frame'] != FRAME or row['timestamp_origin_ns'] != expected[0]
                or not np.allclose(rigid(row['T_rectified_cam0_raw_cam0']), rect.T_rect_raw, atol=1e-6)):
            raise ValueError('Trajectory frame/calibration mismatch')
        if row['status'] == 'lost':
            if row['pose'] is not None:
                raise ValueError('Lost frame must have null pose')
            lost.append(stamp)
            has_lost = True
        else:
            if has_lost or row['status'] != ('initialized' if index == 0 else 'tracking'):
                raise ValueError('Unexpected tracking status or trajectory reset')
            pose = rigid(row['pose'])
            if index == 0 and not np.allclose(pose, np.eye(4), atol=1e-6):
                raise ValueError('First pose must be identity')
            valid.append((stamp, pose))
    anchor = interpolate(stamps, poses, expected[0], max_gap_ns)
    translation_errors, rotation_errors, unmatched = [], [], []
    for stamp, estimate in valid:
        gt = interpolate(stamps, poses, stamp, max_gap_ns)
        if anchor is None or gt is None:
            unmatched.append(stamp)
            continue
        reference = np.linalg.inv(anchor @ T_B_rect) @ gt @ T_B_rect
        translation_errors.append(float(np.linalg.norm(estimate[:3, 3] - reference[:3, 3])))
        rotation_errors.append(float(np.degrees(np.linalg.norm(cv2.Rodrigues(
            reference[:3, :3].T @ estimate[:3, :3])[0]))))
    sources = [trajectory, gt_path] + [mav0 / c / f for c in ('cam0', 'cam1') for f in ('sensor.yaml', 'data.csv')]
    return dict(protocol='euroc-first-pose-anchor-v1', alignment='first GT pose only; scale fixed at 1',
                max_gt_gap_ns=max_gap_ns, expected_frames=len(expected), output_frames=len(rows),
                complete_output=len(rows) == len(expected), tracked_frames=len(valid),
                tracking_fraction=len(valid)/len(expected), evaluated_frames=len(translation_errors),
                evaluated_fraction=len(translation_errors)/len(expected),
                lost_timestamps_ns=lost, missing_timestamps_ns=expected[len(rows):],
                gt_unmatched_timestamps_ns=unmatched,
                translation_rmse_m=float(np.sqrt(np.mean(np.square(translation_errors)))) if translation_errors else None,
                rotation_rmse_deg=float(np.sqrt(np.mean(np.square(rotation_errors)))) if rotation_errors else None,
                sources=[dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in sources])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mav0', type=Path)
    parser.add_argument('trajectory', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--max-gt-gap-ns', type=int, default=20000000)
    args = parser.parse_args()
    result = evaluate(args.mav0, args.trajectory, args.max_gt_gap_ns)
    with args.output.open('x') as target:
        json.dump(result, target, indent=2, allow_nan=False)
        target.write('\n')


if __name__ == '__main__':
    main()
