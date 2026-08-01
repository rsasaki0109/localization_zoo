import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = REPO_ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "causal_pose_graph_correction_smoothing",
    SCRIPT_DIR / "smooth_odometry_causal_pose_graph_correction.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load causal pose-graph correction smoother")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def pose(x: float = 0.0, yaw_deg: float = 0.0) -> np.ndarray:
    result = np.eye(4)
    angle = np.radians(yaw_deg)
    result[:2, :2] = [
        [np.cos(angle), -np.sin(angle)],
        [np.sin(angle), np.cos(angle)],
    ]
    result[0, 3] = x
    return result


class CausalPoseGraphCorrectionSmoothingTests(unittest.TestCase):
    def test_translation_correction_is_limited_by_past_motion(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected = [pose(0.0), pose(1.0), pose(12.0)]
        output = MODULE.smooth_causal_pose_graph_correction(raw, corrected)
        self.assertAlmostEqual(output[2][0, 3], 2.005, places=12)

    def test_rotation_correction_is_limited_by_past_rotation(self) -> None:
        raw = [pose(yaw_deg=0.0), pose(yaw_deg=10.0), pose(yaw_deg=20.0)]
        corrected = [pose(yaw_deg=0.0), pose(yaw_deg=10.0), pose(yaw_deg=40.0)]
        output = MODULE.smooth_causal_pose_graph_correction(raw, corrected)
        published_yaw = np.degrees(np.arctan2(output[2][1, 0], output[2][0, 0]))
        self.assertAlmostEqual(published_yaw, 20.05, places=10)

    def test_future_change_does_not_change_output_prefix(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(2.0), pose(3.0)]
        corrected_b = [pose(0.0), pose(2.0), pose(300.0)]
        output_a = MODULE.smooth_causal_pose_graph_correction(raw, corrected_a)
        output_b = MODULE.smooth_causal_pose_graph_correction(raw, corrected_b)
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)

    def test_matching_inputs_are_unchanged(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        output = MODULE.smooth_causal_pose_graph_correction(raw, raw)
        for actual, expected in zip(output, raw):
            np.testing.assert_allclose(actual, expected, atol=1e-12)

    def test_pose_count_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "pose counts must match"):
            MODULE.smooth_causal_pose_graph_correction([pose()], [pose(), pose(1.0)])

    def test_local_increment_policy_avoids_world_origin_lever_arm(self) -> None:
        raw = [pose(100.0, 0.0), pose(101.0, 10.0)]
        corrected = [pose(100.0, 0.0), pose(101.0, 20.0)]
        output = MODULE.smooth_causal_pose_graph_local_increment(raw, corrected)
        self.assertAlmostEqual(output[1][0, 3], 101.0, places=12)
        self.assertAlmostEqual(output[1][1, 3], 0.0, places=12)
        published_yaw = np.degrees(np.arctan2(output[1][1, 0], output[1][0, 0]))
        self.assertAlmostEqual(published_yaw, 10.05, places=10)

    def test_local_increment_policy_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(2.0), pose(3.0)]
        corrected_b = [pose(0.0), pose(2.0), pose(300.0)]
        output_a = MODULE.smooth_causal_pose_graph_local_increment(raw, corrected_a)
        output_b = MODULE.smooth_causal_pose_graph_local_increment(raw, corrected_b)
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)

    def test_first_correction_is_latched_and_future_changes_are_ignored(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0)]
        corrected = [pose(0.0), pose(1.0), pose(7.0), pose(103.0)]
        output = MODULE.latch_first_causal_pose_graph_correction(raw, corrected)
        self.assertAlmostEqual(output[0][0, 3], 0.0)
        self.assertAlmostEqual(output[1][0, 3], 1.0)
        self.assertAlmostEqual(output[2][0, 3], 7.0)
        self.assertAlmostEqual(output[3][0, 3], 8.0)

    def test_first_correction_latch_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0), pose(7.0)]
        corrected_b = [pose(0.0), pose(1.0), pose(700.0)]
        output_a = MODULE.latch_first_causal_pose_graph_correction(raw, corrected_a)
        output_b = MODULE.latch_first_causal_pose_graph_correction(raw, corrected_b)
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)


if __name__ == "__main__":
    unittest.main()
