import json
from pathlib import Path
import tempfile
import unittest

import cv2
import numpy as np
import yaml
import test_euroc_stereo
from evaluate_euroc import evaluate, ground_truth, interpolate, FRAME
from euroc_stereo import Rectifier, read_camera


class EvaluationTests(unittest.TestCase):
    def fixture(self, root):
        test_euroc_stereo.EurocTests().fixture(root)
        # Both cameras have a nonzero body lever arm and rotated axes.
        for c in ('cam0', 'cam1'):
            path = root / c / 'sensor.yaml'
            data = yaml.safe_load(path.read_text())
            T = np.array(data['T_BS']['data']).reshape(4, 4)
            T[:3, 3] += [0.3, 0.1, 0.05]
            T[:3, :3] = cv2.Rodrigues(np.array([0., 0., 0.1]))[0]
            data['T_BS']['data'] = T.ravel().tolist()
            path.write_text(yaml.safe_dump(data))
        gt = root / 'state_groundtruth_estimate0/data.csv'
        gt.parent.mkdir()
        start = 1400000000000000001
        # End-to-end body rotation exercises the camera lever arm.
        gt.write_text(f'{start},0,0,0,1,0,0,0\n'
                      f'{start+50000000},1,0,0,{np.cos(0.1)},0,0,{np.sin(0.1)}\n')
        left, right = [read_camera(root / c / 'sensor.yaml') for c in ('cam0', 'cam1')]
        rect = Rectifier(left, right)
        extrinsic = left[3] @ np.linalg.inv(rect.T_rect_raw)
        stamps, poses = ground_truth(gt)
        rows = []
        for i in range(2):
            pose = np.linalg.inv(poses[0] @ extrinsic) @ poses[i] @ extrinsic
            rows.append(dict(timestamp_ns=stamps[i], timestamp_origin_ns=start,
                             pose_frame=FRAME, T_rectified_cam0_raw_cam0=rect.T_rect_raw.tolist(),
                             status='initialized' if i == 0 else 'tracking', pose=pose.tolist()))
        trajectory = root / 'trajectory.jsonl'
        self.write(trajectory, rows)
        return trajectory, rows

    def write(self, path, rows):
        path.write_text(''.join(json.dumps(r)+'\n' for r in rows))

    def test_frame_conversion_and_scale_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            trajectory, rows = self.fixture(root)
            result = evaluate(root, trajectory)
            self.assertAlmostEqual(result['translation_rmse_m'], 0, places=10)
            self.assertAlmostEqual(result['rotation_rmse_deg'], 0, places=6)
            self.assertEqual(result['evaluated_fraction'], 1)
            rows[1]['pose'][0][3] += 1
            self.write(trajectory, rows)
            self.assertAlmostEqual(evaluate(root, trajectory)['translation_rmse_m'], 1/np.sqrt(2))

    def test_loss_and_truncated_output_denominator(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            trajectory, rows = self.fixture(root)
            self.write(trajectory, rows[:1])
            result = evaluate(root, trajectory)
            self.assertFalse(result['complete_output'])
            self.assertEqual(result['tracking_fraction'], 0.5)
            self.assertEqual(result['missing_timestamps_ns'], [rows[1]['timestamp_ns']])
            rows[1].update(status='lost', pose=None)
            self.write(trajectory, rows)
            result = evaluate(root, trajectory)
            self.assertTrue(result['complete_output'])
            self.assertEqual(result['tracking_fraction'], 0.5)
            self.assertEqual(result['lost_timestamps_ns'], [rows[1]['timestamp_ns']])
            rows[0].update(status='lost', pose=None)
            self.write(trajectory, rows)
            self.assertIsNone(evaluate(root, trajectory)['translation_rmse_m'])

    def test_interpolation_and_gap_rejection(self):
        a, b = np.eye(4), np.eye(4)
        b[:3, :3] = cv2.Rodrigues(np.array([0., 0., np.pi/2]))[0]
        b[0, 3] = 2
        mid = interpolate([100, 200], [a, b], 150, 100)
        self.assertEqual(mid[0, 3], 1)
        np.testing.assert_allclose(mid[:3, :3], cv2.Rodrigues(np.array([0., 0., np.pi/4]))[0], atol=1e-12)
        self.assertIsNone(interpolate([100, 200], [a, b], 150, 99))
        self.assertIsNone(interpolate([100, 200], [a, b], 99, 100))

    def test_missing_anchor_and_invalid_trajectory(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            trajectory, rows = self.fixture(root)
            gt = root / 'state_groundtruth_estimate0/data.csv'
            gt.write_text(gt.read_text().splitlines()[1]+'\n')
            result = evaluate(root, trajectory)
            self.assertEqual(result['evaluated_frames'], 0)
            self.assertEqual(len(result['gt_unmatched_timestamps_ns']), 2)
            rows[1]['timestamp_ns'] += 1
            self.write(trajectory, rows)
            with self.assertRaisesRegex(ValueError, 'exact prefix'):
                evaluate(root, trajectory)
            rows[1]['timestamp_ns'] -= 1
            rows[1]['pose'][0][0] = 5
            self.write(trajectory, rows)
            with self.assertRaisesRegex(ValueError, 'rigid pose'):
                evaluate(root, trajectory)


if __name__ == '__main__':
    unittest.main()
