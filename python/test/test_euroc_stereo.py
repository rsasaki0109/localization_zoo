import json
from pathlib import Path
import sys
import tempfile
import unittest

import cv2
import numpy as np
import yaml

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from euroc_stereo import Rectifier, read_camera, read_index, replay


class EurocTests(unittest.TestCase):
    def fixture(self, root):
        for index in range(2):
            cam = root / f'cam{index}'
            (cam / 'data').mkdir(parents=True)
            T = np.eye(4)
            T[0, 3] = index * 0.2
            calibration = dict(camera_model='pinhole', distortion_model='radial-tangential',
                               intrinsics=[300, 300, 160, 120], resolution=[320, 240],
                               distortion_coefficients=[0, 0, 0, 0],
                               T_BS=dict(rows=4, cols=4, data=T.ravel().tolist()))
            (cam / 'sensor.yaml').write_text(yaml.safe_dump(calibration))
            (cam / 'data.csv').write_text('#timestamp [ns],filename\n1400000000000000001,a.png\n1400000000050000001,b.png\n')
            for name in ('a.png', 'b.png'):
                cv2.imwrite(str(cam / 'data' / name), np.zeros((240, 320), np.uint8))

    def test_rectified_geometry_with_rotated_cameras(self):
        K = np.array([[300., 0, 160], [0, 300, 120], [0, 0, 1]])
        T0, T1 = np.eye(4), np.eye(4)
        T0[:3, :3] = cv2.Rodrigues(np.array([0.01, -0.02, 0.03]))[0]
        T1[:3, :3] = cv2.Rodrigues(np.array([-0.01, 0.03, 0.01]))[0]
        T1[:3, 3] = [0.2, 0.005, 0.001]
        d = np.array([-0.1, 0.01, 0.001, -0.002])
        rect = Rectifier((K, d, (320, 240), T0), (K, d, (320, 240), T1))
        T10 = np.linalg.inv(T1) @ T0
        R0, R1, P0, P1, *_ = cv2.stereoRectify(K, d, K, d, (320, 240),
                                               T10[:3, :3], T10[:3, 3],
                                               flags=cv2.CALIB_ZERO_DISPARITY, alpha=0)
        points = np.array([[-0.4, 0.1, 3.], [0.3, -0.2, 5.], [0.1, 0.2, 4.]])
        uv0 = cv2.projectPoints(points, np.zeros(3), np.zeros(3), K, d)[0]
        uv1 = cv2.projectPoints(points, cv2.Rodrigues(T10[:3, :3])[0], T10[:3, 3], K, d)[0]
        u0 = cv2.undistortPoints(uv0, K, d, R=R0, P=P0).reshape(-1, 2)
        u1 = cv2.undistortPoints(uv1, K, d, R=R1, P=P1).reshape(-1, 2)
        np.testing.assert_allclose(u0[:, 1], u1[:, 1], atol=1e-5)
        from stereo_vo import triangulate
        xyz, valid = triangulate(u0, u1, rect.K, rect.baseline)
        self.assertTrue(valid.all())
        np.testing.assert_allclose(xyz, points @ R0.T, atol=1e-5)
        self.assertAlmostEqual(rect.baseline, np.linalg.norm(T1[:3, 3]))

    def test_replay_preserves_nanoseconds_and_loss(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.fixture(root)
            output = root / 'trajectory.jsonl'
            replay(root, output)
            rows = [json.loads(line) for line in output.read_text().splitlines()]
            self.assertEqual([r['timestamp_ns'] for r in rows], [1400000000000000001, 1400000000050000001])
            self.assertTrue(all(r['status'] == 'lost' and r['pose'] is None for r in rows))
            with self.assertRaises(FileExistsError):
                replay(root, output)

    def test_mismatched_and_duplicate_timestamps_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.fixture(root)
            (root / 'cam1/data.csv').write_text('1400000000000000002,a.png\n')
            with self.assertRaisesRegex(ValueError, 'match exactly'):
                replay(root, root / 'out')
            self.assertFalse((root / 'out').exists())
            (root / 'cam1/data.csv').write_text('1,a.png\n1,b.png\n')
            with self.assertRaisesRegex(ValueError, 'increase strictly'):
                read_index(root / 'cam1')

    def test_invalid_calibration_and_image_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.fixture(root)
            left = read_camera(root / 'cam0/sensor.yaml')
            right = read_camera(root / 'cam1/sensor.yaml')
            rect = Rectifier(left, right)
            with self.assertRaises(ValueError):
                rect.rectify(np.zeros((2, 2), np.uint8), None)
            with self.assertRaises(ValueError):
                Rectifier(left, left)
            path = root / 'cam0/sensor.yaml'
            data = yaml.safe_load(path.read_text())
            data['T_BS']['data'][0] = 2
            path.write_text(yaml.safe_dump(data))
            with self.assertRaises(ValueError):
                read_camera(path)


if __name__ == '__main__':
    unittest.main()
