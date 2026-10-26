"""Replay EuRoC MAV cam0/cam1 images through the experimental stereo VO."""
import argparse
import csv
import json
from pathlib import Path

import cv2
import numpy as np
import yaml

from stereo_vo import StereoVO


def read_camera(path):
    data = yaml.safe_load(Path(path).read_text())
    if data['camera_model'] != 'pinhole' or data['distortion_model'] != 'radial-tangential':
        raise ValueError('Only EuRoC pinhole/radial-tangential calibration is supported')
    intrinsics = np.asarray(data['intrinsics'], dtype=float)
    distortion = np.asarray(data['distortion_coefficients'], dtype=float)
    size = tuple(data['resolution'])
    matrix = data['T_BS']
    if matrix['rows'] != 4 or matrix['cols'] != 4:
        raise ValueError('T_BS must be 4x4')
    T = np.asarray(matrix['data'], dtype=float).reshape(4, 4)
    if (intrinsics.shape != (4,) or distortion.shape != (4,)
            or not np.isfinite(intrinsics).all() or not np.isfinite(distortion).all()
            or np.any(intrinsics[:2] <= 0) or len(size) != 2
            or any(type(v) is not int or v <= 0 for v in size)
            or not np.isfinite(T).all() or not np.allclose(T[3], [0, 0, 0, 1])
            or not np.allclose(T[:3, :3].T @ T[:3, :3], np.eye(3), atol=1e-6)
            or not np.isclose(np.linalg.det(T[:3, :3]), 1, atol=1e-6)):
        raise ValueError('Invalid EuRoC calibration')
    fx, fy, cx, cy = intrinsics
    return np.array([[fx, 0, cx], [0, fy, cy], [0, 0, 1]]), distortion, size, T


class Rectifier:
    def __init__(self, left, right):
        K0, d0, self.size, T_B0 = left
        K1, d1, size1, T_B1 = right
        if self.size != size1:
            raise ValueError('Camera resolutions must match')
        # EuRoC T_BS maps sensor coordinates into body coordinates.
        T_10 = np.linalg.inv(T_B1) @ T_B0
        if np.linalg.norm(T_10[:3, 3]) < 1e-9:
            raise ValueError('Stereo baseline must be nonzero')
        R0, R1, P0, P1, _, _, _ = cv2.stereoRectify(
            K0, d0, K1, d1, self.size, T_10[:3, :3], T_10[:3, 3],
            flags=cv2.CALIB_ZERO_DISPARITY, alpha=0)
        self.K = P0[:, :3]
        self.baseline = -P1[0, 3] / P1[0, 0]
        if (self.baseline <= 0 or not np.isfinite(self.baseline)
                or not np.allclose(P1[:, :3], self.K)
                or not np.isclose(P1[1, 3], 0)):
            raise ValueError('Expected horizontal stereo with cam1 to the right of cam0')
        self.T_rect_raw = np.eye(4)
        self.T_rect_raw[:3, :3] = R0
        self.maps = [cv2.initUndistortRectifyMap(K, d, R, self.K, self.size, cv2.CV_32FC1)
                     for K, d, R in [(K0, d0, R0), (K1, d1, R1)]]

    def rectify(self, left, right):
        for img in (left, right):
            if img is None or img.shape != self.size[::-1] or img.dtype != np.uint8:
                raise ValueError('Missing image or image incompatible with calibration')
        return tuple(cv2.remap(img, *maps, interpolation=cv2.INTER_LINEAR)
                     for img, maps in zip((left, right), self.maps))


def read_index(camera):
    camera = Path(camera)
    result = []
    with (camera / 'data.csv').open() as source:
        rows = csv.reader(line for line in source if line.strip() and not line.lstrip().startswith('#'))
        for row in rows:
            if len(row) != 2:
                raise ValueError('Expected timestamp,filename in data.csv')
            stamp = int(row[0])  # Do not round epoch nanoseconds through float.
            name = row[1].strip()
            if stamp < 0 or (result and stamp <= result[-1][0]):
                raise ValueError('Camera timestamps must increase strictly')
            if not name or Path(name).name != name or name in ('.', '..'):
                raise ValueError('Expected image basename')
            result.append((stamp, camera / 'data' / name))
    if not result:
        raise ValueError('Empty camera index')
    return result


def replay(mav0, output):
    mav0, output = Path(mav0), Path(output)
    cameras = [mav0 / name for name in ('cam0', 'cam1')]
    indices = [read_index(camera) for camera in cameras]
    if [t for t, _ in indices[0]] != [t for t, _ in indices[1]]:
        raise ValueError('cam0/cam1 timestamps must match exactly; no frames are silently dropped')
    rectifier = Rectifier(*(read_camera(camera / 'sensor.yaml') for camera in cameras))
    vo = StereoVO(rectifier.K, rectifier.baseline)
    origin = indices[0][0][0]
    # Existing output is preserved. On failure the prefix is diagnostic only.
    with output.open('x') as target:
        for (stamp, left), (_, right) in zip(*indices):
            images = rectifier.rectify(cv2.imread(str(left), cv2.IMREAD_GRAYSCALE),
                                       cv2.imread(str(right), cv2.IMREAD_GRAYSCALE))
            result = vo.process(*images, (stamp - origin) * 1e-9)
            result['timestamp_ns'] = stamp
            result['timestamp_origin_ns'] = origin
            result['pose_frame'] = 'first_rectified_cam0_from_current_rectified_cam0'
            result['T_rectified_cam0_raw_cam0'] = rectifier.T_rect_raw.tolist()
            if result['pose'] is not None:
                result['pose'] = result['pose'].tolist()
            target.write(json.dumps(result, allow_nan=False) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mav0', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    replay(args.mav0, args.output)


if __name__ == '__main__':
    main()
