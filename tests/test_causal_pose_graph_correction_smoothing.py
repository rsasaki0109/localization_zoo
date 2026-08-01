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

    def test_bias_rate_policy_has_no_correction_jump(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0)]
        corrected = [pose(0.0), pose(1.0), pose(7.0), pose(8.0)]
        output = MODULE.apply_first_causal_correction_as_bias_rate(raw, corrected)
        self.assertAlmostEqual(output[2][0, 3], 2.0, places=12)
        self.assertGreater(output[3][0, 3], 3.0)

    def test_bias_rate_policy_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(6.0), pose(7.0)]
        corrected_b = [pose(0.0), pose(6.0), pose(700.0)]
        output_a = MODULE.apply_first_causal_correction_as_bias_rate(raw, corrected_a)
        output_b = MODULE.apply_first_causal_correction_as_bias_rate(raw, corrected_b)
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_translation_bias_can_be_disabled(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0)]
        corrected = [pose(0.0), pose(1.0), pose(7.0), pose(8.0)]
        output = MODULE.apply_first_causal_correction_as_bias_rate(
            raw, corrected, bias_translation_gain=0.0
        )
        self.assertAlmostEqual(output[3][0, 3], 3.0, places=12)

    def test_rotation_bias_can_be_disabled(self) -> None:
        raw = [pose(yaw_deg=0.0), pose(yaw_deg=10.0), pose(yaw_deg=20.0)]
        corrected = [pose(yaw_deg=0.0), pose(yaw_deg=30.0), pose(yaw_deg=40.0)]
        output = MODULE.apply_first_causal_correction_as_bias_rate(
            raw, corrected, bias_rotation_gain=0.0
        )
        published_yaw = np.degrees(np.arctan2(output[2][1, 0], output[2][0, 0]))
        self.assertAlmostEqual(published_yaw, 20.0, places=10)

    def test_left_correction_is_transferred_without_frontend_offset(self) -> None:
        target = [pose(100.0), pose(101.0)]
        source = [pose(0.0), pose(1.0)]
        source_corrected = [pose(0.0), pose(6.0)]
        transferred = MODULE.transfer_causal_left_corrections(
            target, source, source_corrected
        )
        self.assertAlmostEqual(transferred[0][0, 3], 100.0)
        self.assertAlmostEqual(transferred[1][0, 3], 106.0)

    def test_correction_transfer_is_frame_causal(self) -> None:
        target = [pose(100.0), pose(101.0), pose(102.0)]
        source = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(6.0), pose(7.0)]
        corrected_b = [pose(0.0), pose(6.0), pose(700.0)]
        output_a = MODULE.transfer_causal_left_corrections(
            target, source, corrected_a
        )
        output_b = MODULE.transfer_causal_left_corrections(
            target, source, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)

    def test_updated_bias_rate_never_jumps_to_graph_pose(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0)]
        corrected = [pose(0.0), pose(6.0), pose(7.0), pose(103.0)]
        output = MODULE.apply_causal_correction_as_updated_bias_rate(
            raw, corrected
        )
        self.assertAlmostEqual(output[1][0, 3], 1.0, places=12)
        self.assertGreater(output[2][0, 3], 2.0)
        self.assertLess(output[3][0, 3], 103.0)

    def test_updated_bias_rate_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(6.0), pose(7.0)]
        corrected_b = [pose(0.0), pose(6.0), pose(700.0)]
        output_a = MODULE.apply_causal_correction_as_updated_bias_rate(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_correction_as_updated_bias_rate(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_interval_bias_rate_uses_correction_since_previous_update(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0), pose(4.0)]
        corrected = [pose(0.0), pose(2.0), pose(3.0), pose(7.0), pose(8.0)]
        output = MODULE.apply_causal_interval_correction_as_bias_rate(
            raw, corrected, bias_rotation_gain=0.0
        )
        self.assertAlmostEqual(output[1][0, 3], 1.0, places=12)
        self.assertAlmostEqual(output[2][0, 3], 3.0, places=12)
        self.assertAlmostEqual(output[3][0, 3], 5.0, places=12)
        self.assertAlmostEqual(output[4][0, 3], 7.5, places=12)

    def test_interval_bias_rate_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(2.0), pose(3.0)]
        corrected_b = [pose(0.0), pose(2.0), pose(300.0)]
        output_a = MODULE.apply_causal_interval_correction_as_bias_rate(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_interval_correction_as_bias_rate(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_first_translation_latch_is_applied_once(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0)]
        corrected = [pose(0.0), pose(1.0), pose(3.0), pose(5.0)]
        output = MODULE.apply_causal_interval_correction_as_bias_rate(
            raw,
            corrected,
            bias_translation_gain=0.0,
            bias_rotation_gain=0.0,
            latch_first_translation=True,
        )
        self.assertAlmostEqual(output[1][0, 3], 1.0, places=12)
        self.assertAlmostEqual(output[2][0, 3], 3.0, places=12)
        self.assertAlmostEqual(output[3][0, 3], 4.0, places=12)

    def test_interval_consensus_amplifies_agreeing_rotation_axes(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(6)]
        corrected = [
            pose(0.0, 0.0),
            pose(1.0, 10.0),
            pose(2.0, 10.0),
            pose(3.0, 30.0),
            pose(4.0, 30.0),
            pose(5.0, 30.0),
        ]
        output = MODULE.apply_causal_interval_consensus_rotation_bias(
            raw, corrected
        )
        yaw_step_after_second_update = np.degrees(
            MODULE.rotation_angle(
                output[4][:3, :3] @ output[3][:3, :3].T
            )
        )
        self.assertGreater(yaw_step_after_second_update, 10.0)

    def test_interval_consensus_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0)]
        corrected_b = [pose(0.0), pose(1.0, 10.0), pose(2.0, 170.0)]
        output_a = MODULE.apply_causal_interval_consensus_rotation_bias(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_interval_consensus_rotation_bias(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_yaw_consensus_doubles_same_sign_interval_rate(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(6)]
        corrected = [
            pose(0.0, 0.0),
            pose(1.0, 10.0),
            pose(2.0, 10.0),
            pose(3.0, 30.0),
            pose(4.0, 30.0),
            pose(5.0, 30.0),
        ]
        output = MODULE.apply_causal_interval_yaw_consensus_bias(raw, corrected)
        yaw_step = np.degrees(
            np.arctan2(
                (output[4][:3, :3] @ output[3][:3, :3].T)[1, 0],
                (output[4][:3, :3] @ output[3][:3, :3].T)[0, 0],
            )
        )
        self.assertAlmostEqual(yaw_step, 20.0, places=10)

    def test_yaw_consensus_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0)]
        corrected_b = [pose(0.0), pose(1.0, 10.0), pose(2.0, -170.0)]
        output_a = MODULE.apply_causal_interval_yaw_consensus_bias(raw, corrected_a)
        output_b = MODULE.apply_causal_interval_yaw_consensus_bias(raw, corrected_b)
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_robust_interval_rotation_clips_short_interval_outlier(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(7)]
        corrected = [
            pose(0.0, 0.0),
            pose(1.0, 1.0),
            pose(2.0, 1.0),
            pose(3.0, 3.0),
            pose(4.0, 3.0),
            pose(5.0, 33.0),
            pose(6.0, 33.0),
        ]
        output = MODULE.apply_causal_interval_robust_rotation_bias(raw, corrected)
        relative = output[6][:3, :3] @ output[5][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertLess(yaw_step, 10.0)

    def test_robust_interval_rotation_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0)]
        corrected_b = [pose(0.0), pose(1.0, 10.0), pose(2.0, 170.0)]
        output_a = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_robust_consistent_direction_rejects_reversal(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(5)]
        correction_degrees = [0.0, 10.0, 10.0, -10.0, -10.0]
        corrected = [
            pose(0.0, angle) @ raw_pose
            for raw_pose, angle in zip(raw, correction_degrees)
        ]
        output = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected, reject_direction_reversal=True
        )
        relative = output[4][:3, :3] @ output[3][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertGreater(yaw_step, 0.0)

    def test_pair_distance_direction_downweights_short_reversal(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(103)]
        corrections = [0.0] * 100 + [10.0, 8.0, 8.0]
        corrected = [
            pose(0.0, angle) @ raw_pose
            for raw_pose, angle in zip(raw, corrections)
        ]
        output = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected, pair_distance_weight_direction=True
        )
        relative = output[102][:3, :3] @ output[101][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertGreater(yaw_step, 0.0)

    def test_robust_yaw_direction_ignores_roll_drift(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(4)]
        roll = np.eye(4)
        angle = np.radians(10.0)
        roll[:3, :3] = np.array(
            [
                [1.0, 0.0, 0.0],
                [0.0, np.cos(angle), -np.sin(angle)],
                [0.0, np.sin(angle), np.cos(angle)],
            ]
        )
        corrected = [raw[0], roll @ raw[1], roll @ raw[2], roll @ raw[3]]
        output = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected, reject_direction_reversal=True, yaw_only=True
        )
        np.testing.assert_allclose(output[-1][:3, :3], np.eye(3), atol=1e-12)

    def test_robust_motion_yaw_output_combines_causal_components(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(4)]
        corrected = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0), pose(3.0, 8.0)]
        motion = MODULE.apply_causal_interval_robust_rotation_bias(raw, corrected)
        yaw = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected, reject_direction_reversal=True, yaw_only=True
        )
        output = MODULE.apply_causal_interval_robust_motion_yaw_output_bias(
            raw, corrected
        )
        for combined, motion_pose, yaw_pose in zip(output, motion, yaw):
            np.testing.assert_allclose(combined[:3, 3], motion_pose[:3, 3])
            np.testing.assert_allclose(combined[:3, :3], yaw_pose[:3, :3])

    def test_robust_persistent_yaw_keeps_yaw_sign(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(5)]
        corrections = [0.0, 10.0, 10.0, 8.0, 8.0]
        corrected = [
            pose(0.0, angle) @ raw_pose
            for raw_pose, angle in zip(raw, corrections)
        ]
        output = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected, preserve_yaw_sign=True
        )
        relative = output[4][:3, :3] @ output[3][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertGreater(yaw_step, 0.0)

    def test_first_loop_scale_changes_future_increment_without_jump(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0)]
        corrected = [pose(0.0), pose(1.0), pose(1.8), pose(2.8)]
        output = MODULE.apply_causal_interval_robust_rotation_bias(
            raw, corrected, learn_first_scale=True
        )
        self.assertAlmostEqual(output[2][0, 3], 2.0, places=12)
        self.assertAlmostEqual(output[3][0, 3], 2.95, places=12)

    def test_median_vector_rotation_rejects_axis_outlier(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(7)]
        corrected = [
            pose(0.0, 0.0),
            pose(1.0, 1.0),
            pose(2.0, 1.0),
            pose(3.0, 3.0),
            pose(4.0, 3.0),
            pose(5.0, -27.0),
            pose(6.0, -27.0),
        ]
        output = MODULE.apply_causal_interval_median_vector_rotation_bias(
            raw, corrected
        )
        relative = output[6][:3, :3] @ output[5][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertLess(abs(yaw_step), 10.0)

    def test_delayed_median_rejects_third_axis_outlier(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(7)]
        corrected = [
            pose(0.0, 0.0),
            pose(1.0, 1.0),
            pose(2.0, 1.0),
            pose(3.0, 3.0),
            pose(4.0, 3.0),
            pose(5.0, -27.0),
            pose(6.0, -27.0),
        ]
        output = MODULE.apply_causal_interval_delayed_median_rotation_bias(
            raw, corrected
        )
        relative = output[6][:3, :3] @ output[5][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertLess(abs(yaw_step), 10.0)

    def test_delayed_median_rotation_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0)]
        corrected_b = [pose(0.0), pose(1.0, 10.0), pose(2.0, -170.0)]
        output_a = MODULE.apply_causal_interval_delayed_median_rotation_bias(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_interval_delayed_median_rotation_bias(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_distance_weighting_suppresses_short_interval_outlier(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(103)]
        corrected = [pose(float(i), 0.0) for i in range(103)]
        corrected[100] = pose(100.0, 10.0)
        corrected[101] = pose(101.0, -80.0)
        corrected[102] = pose(102.0, -80.0)
        output = MODULE.apply_causal_interval_distance_weighted_rotation_bias(
            raw, corrected
        )
        relative = output[102][:3, :3] @ output[101][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertLess(abs(yaw_step), 2.0)

    def test_distance_weighted_rotation_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0)]
        corrected_b = [pose(0.0), pose(1.0, 10.0), pose(2.0, -170.0)]
        output_a = MODULE.apply_causal_interval_distance_weighted_rotation_bias(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_interval_distance_weighted_rotation_bias(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_distance_weighted_se3_bias_changes_future_without_jump(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0), pose(3.0)]
        corrected = [pose(0.0), pose(1.0), pose(3.0), pose(4.0)]
        output = MODULE.apply_causal_interval_distance_weighted_rotation_bias(
            raw, corrected, learn_translation_bias=True
        )
        self.assertAlmostEqual(output[2][0, 3], 2.0, places=12)
        self.assertGreater(output[3][0, 3], 3.0)

    def test_median_vector_rotation_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0)]
        corrected_b = [pose(0.0), pose(1.0, 10.0), pose(2.0, -170.0)]
        output_a = MODULE.apply_causal_interval_median_vector_rotation_bias(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_interval_median_vector_rotation_bias(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)

    def test_hampel_vector_rotation_replaces_outlying_component(self) -> None:
        raw = [pose(float(i), 0.0) for i in range(9)]
        corrected = [
            pose(0.0, 0.0),
            pose(1.0, 1.0),
            pose(2.0, 1.0),
            pose(3.0, 3.0),
            pose(4.0, 3.0),
            pose(5.0, 5.0),
            pose(6.0, 5.0),
            pose(7.0, -25.0),
            pose(8.0, -25.0),
        ]
        output = MODULE.apply_causal_interval_hampel_vector_rotation_bias(
            raw, corrected
        )
        relative = output[8][:3, :3] @ output[7][:3, :3].T
        yaw_step = np.degrees(np.arctan2(relative[1, 0], relative[0, 0]))
        self.assertLess(abs(yaw_step), 10.0)

    def test_hampel_vector_rotation_is_causal(self) -> None:
        raw = [pose(0.0), pose(1.0), pose(2.0)]
        corrected_a = [pose(0.0), pose(1.0, 10.0), pose(2.0, 10.0)]
        corrected_b = [pose(0.0), pose(1.0, 10.0), pose(2.0, -170.0)]
        output_a = MODULE.apply_causal_interval_hampel_vector_rotation_bias(
            raw, corrected_a
        )
        output_b = MODULE.apply_causal_interval_hampel_vector_rotation_bias(
            raw, corrected_b
        )
        np.testing.assert_allclose(output_a[0], output_b[0], atol=1e-12)
        np.testing.assert_allclose(output_a[1], output_b[1], atol=1e-12)
        np.testing.assert_allclose(output_a[2], output_b[2], atol=1e-12)


if __name__ == "__main__":
    unittest.main()
