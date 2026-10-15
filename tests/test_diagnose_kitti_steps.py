"""Independent trajectories check the quantitative KITTI failure diagnostics."""

import sys
from pathlib import Path
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'evaluation/scripts'))
from diagnose_kitti_steps import diagnose


def trajectory(x, y=None):
    poses = []
    for index, value in enumerate(x):
        pose = np.eye(4)
        pose[0, 3] = value
        pose[1, 3] = y[index] if y is not None else 0
        poses.append(pose)
    return poses


class DiagnoseKittiStepsTest(unittest.TestCase):
    def test_identical_trajectory_has_no_drift(self):
        poses = trajectory(np.arange(301))
        result = diagnose(poses, poses)
        self.assertEqual(result['steps_length_off_over_threshold'], 0)
        self.assertEqual(result['steps_vector_off_over_threshold'], 0)
        self.assertAlmostEqual(result['path_length_ratio'], 1)
        self.assertAlmostEqual(result['official_metrics_from_saved_poses']['kitti_rte_trans_pct'], 0)
        self.assertTrue(all(row['block_relative_translation_error_pct'] == 0
                            for row in result['blocks']))

    def test_known_short_steps_and_error_growth_are_localized(self):
        # Ground truth travels 1 m per frame; the estimate loses 30% after frame 100.
        gt = trajectory(np.arange(301))
        x = np.arange(301, dtype=float)
        x[101:] = 100 + (x[101:] - 100) * 0.7
        result = diagnose(trajectory(x), gt)
        self.assertEqual(result['bad_length_step_frame_ranges'], [[101, 300]])
        self.assertEqual(result['steps_length_off_over_threshold'], 200)
        self.assertEqual(result['blocks'][0]['steps_length_off_over_threshold'], 0)
        self.assertAlmostEqual(result['blocks'][1]['block_relative_translation_error_pct'], 30)
        self.assertAlmostEqual(result['blocks'][2]['position_error_end_m'], 60)

    def test_wrong_direction_is_distinct_from_wrong_step_length(self):
        gt = trajectory(np.arange(101))
        est = trajectory(np.zeros(101), np.arange(101))
        result = diagnose(est, gt)
        self.assertEqual(result['steps_length_off_over_threshold'], 0)
        self.assertEqual(result['steps_vector_off_over_threshold'], 100)
        self.assertEqual(result['bad_vector_step_frame_ranges'], [[1, 100]])

    def test_stops_are_excluded_and_incomplete_trajectories_rejected(self):
        gt = trajectory([0, 0, 1, 1, 2])
        result = diagnose(gt, gt)
        self.assertEqual(result['moving_step_count'], 2)
        self.assertEqual(result['bad_length_step_frame_ranges'], [])
        with self.assertRaises(ValueError):
            diagnose(gt[:-1], gt)


if __name__ == '__main__':
    unittest.main()
