"""Experimental rectified stereo VO. Inputs are images and calibration, never GT.

Requires numpy and opencv-python-headless. K uses pixel units; baseline is metres.
Both images must already be rectified with the same K and a horizontal baseline.
This module is a baseline, not a reproduction of VINS-Fusion/OKVIS/ORB-SLAM3.
"""
import cv2
import numpy as np


def triangulate(left, right, K, baseline):
    """Return previous-left-camera XYZ and validity mask for rectified matches."""
    left, right = np.asarray(left, float), np.asarray(right, float)
    disparity = left[:, 0] - right[:, 0]
    valid = (np.isfinite(left).all(axis=1) & np.isfinite(right).all(axis=1)
             & (disparity > 0.5) & (np.abs(left[:, 1] - right[:, 1]) < 1.5))
    xyz = np.full((len(left), 3), np.nan)
    z = K[0, 0] * baseline / disparity[valid]
    xyz[valid] = np.column_stack(((left[valid, 0] - K[0, 2]) * z / K[0, 0],
                                 (left[valid, 1] - K[1, 2]) * z / K[1, 1], z))
    return xyz, valid


def estimate_motion(xyz, pixels, K):
    """Estimate current-camera <- previous-camera; no prediction or GT seed."""
    if len(xyz) < 12:
        return None, 0
    ok, rvec, tvec, inliers = cv2.solvePnPRansac(
        np.asarray(xyz, np.float64), np.asarray(pixels, np.float64), K, None,
        iterationsCount=200, reprojectionError=2.0, confidence=0.999,
        flags=cv2.SOLVEPNP_EPNP)
    count = 0 if inliers is None else len(inliers)
    if not ok or count < 12 or count < len(xyz) * 0.5:
        return None, count
    R = cv2.Rodrigues(rvec)[0]
    transformed = np.asarray(xyz)[inliers[:, 0]] @ R.T + tvec.ravel()
    if not np.isfinite(transformed).all() or np.any(transformed[:, 2] <= 0):
        return None, count
    T = np.eye(4)
    T[:3, :3], T[:3, 3] = R, tvec.ravel()
    return T, count


class StereoVO:
    """Stateful VO with explicit tracking loss; reset starts a new trajectory."""
    def __init__(self, K, baseline):
        self.K = np.asarray(K, np.float64).copy()
        if (self.K.shape != (3, 3) or not np.isfinite(self.K).all()
                or self.K[0, 0] <= 0 or self.K[1, 1] <= 0
                or not np.allclose(self.K[2], [0, 0, 1])
                or self.K[0, 1] != 0 or self.K[1, 0] != 0
                or not np.isfinite(baseline) or baseline <= 0):
            raise ValueError('Invalid rectified camera calibration')
        self.baseline = baseline
        self.orb = cv2.ORB_create(nfeatures=2500)
        self.reset()

    def reset(self):
        self.pose = np.eye(4)  # world <- current left camera
        self.previous = None
        self.timestamp = None
        self.lost = False

    @staticmethod
    def matches(a, b):
        if a is None or b is None or len(b) < 2:
            return []
        pairs = cv2.BFMatcher(cv2.NORM_HAMMING).knnMatch(a, b, k=2)
        good = [p[0] for p in pairs if len(p) == 2 and p[0].distance < 0.75 * p[1].distance]
        # Unique target assignments prevent duplicate 3D constraints.
        result, used = [], set()
        for match in sorted(good, key=lambda m: m.distance):
            if match.trainIdx not in used:
                result.append(match)
                used.add(match.trainIdx)
        return result

    def process(self, left, right, timestamp):
        if (left.ndim != 2 or right.shape != left.shape
                or left.dtype != np.uint8 or right.dtype != np.uint8):
            raise ValueError('Expected same-size rectified uint8 grayscale images')
        if not np.isfinite(timestamp) or (self.timestamp is not None and timestamp <= self.timestamp):
            raise ValueError('Timestamps must increase strictly')
        if self.previous is not None and left.shape != self.previous[3]:
            raise ValueError('Image dimensions changed')
        if self.lost:
            return {'status': 'lost', 'pose': None, 'inliers': 0}
        kl, dl = self.orb.detectAndCompute(left, None)
        kr, dr = self.orb.detectAndCompute(right, None)
        stereo = self.matches(dl, dr)
        l = np.array([kl[m.queryIdx].pt for m in stereo]).reshape(-1, 2)
        r = np.array([kr[m.trainIdx].pt for m in stereo]).reshape(-1, 2)
        xyz, valid = triangulate(l, r, self.K, self.baseline)
        points = {m.queryIdx: p for m, p, keep in zip(stereo, xyz, valid) if keep}
        count = 0
        status = 'initialized'
        if self.previous is not None:
            previous_descriptors, previous_points, _, _ = self.previous
            temporal = [m for m in self.matches(previous_descriptors, dl) if m.queryIdx in previous_points]
            T, count = estimate_motion([previous_points[m.queryIdx] for m in temporal],
                                       [kl[m.trainIdx].pt for m in temporal], self.K)
            if T is None:
                self.lost = True
                self.timestamp = timestamp
                return {'status': 'lost', 'pose': None, 'inliers': count}
            self.pose = self.pose @ np.linalg.inv(T)
            status = 'tracking'
        if len(points) < 12:
            self.lost = True
            self.timestamp = timestamp
            return {'status': 'lost', 'pose': None, 'inliers': count}
        self.previous = (dl, points, timestamp, left.shape)
        self.timestamp = timestamp
        return {'status': status, 'pose': self.pose.copy(), 'inliers': count,
                'stereo_points': len(points)}


def main():
    """Replay a timestamp,left,right CSV of rectified image pairs."""
    import argparse
    import csv
    import json
    from pathlib import Path
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pairs', type=Path)
    parser.add_argument('calibration', type=Path, help='JSON with K and baseline_m')
    parser.add_argument('output', type=Path, help='JSONL; failed frames retain status and null pose')
    args = parser.parse_args()
    calibration = json.loads(args.calibration.read_text())
    vo = StereoVO(calibration['K'], calibration['baseline_m'])
    # Refuse overwriting an existing result or input.
    with args.output.open('x') as output, args.pairs.open() as source:
        for row in csv.DictReader(source):
            left = cv2.imread(str(args.pairs.parent / row['left']), cv2.IMREAD_GRAYSCALE)
            right = cv2.imread(str(args.pairs.parent / row['right']), cv2.IMREAD_GRAYSCALE)
            if left is None or right is None:
                raise ValueError('Could not read stereo pair: ' + str(row))
            result = vo.process(left, right, float(row['timestamp']))
            result['timestamp'] = float(row['timestamp'])
            if result['pose'] is not None:
                result['pose'] = result['pose'].tolist()
            output.write(json.dumps(result, allow_nan=False) + '\n')


if __name__ == '__main__':
    main()
