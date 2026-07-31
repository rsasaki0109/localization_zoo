import importlib.util
import math
import sys
import unittest
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPTS))
SPEC = importlib.util.spec_from_file_location(
    "direction_consistent_rotation",
    SCRIPTS / "fuse_odometry_direction_consistent_rotation.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load direction-consistent fusion")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def pose(x: float = 0.0, y: float = 0.0, yaw_deg: float = 0.0) -> np.ndarray:
    yaw = math.radians(yaw_deg)
    result = np.eye(4)
    result[:3, :3] = np.array(
        [[math.cos(yaw), -math.sin(yaw), 0.0],
         [math.sin(yaw), math.cos(yaw), 0.0],
         [0.0, 0.0, 1.0]]
    )
    result[:2, 3] = [x, y]
    return result


class DirectionConsistentRotationFusionTests(unittest.TestCase):
    def test_signed_direction_error(self) -> None:
        self.assertGreater(
            MODULE.signed_xy_direction_error(
                np.array([1.0, 0.0, 0.0]), np.array([1.0, 1.0, 0.0])
            ),
            0.0,
        )

    def test_opposite_direction_signal_rejects_rotation_correction(self) -> None:
        primary = [pose(x=float(i), yaw_deg=0.0) for i in range(4)]
        reference = [pose(x=float(i), y=-0.2 * i, yaw_deg=0.2 * i) for i in range(4)]
        output, counters = MODULE.fuse_direction_consistent_rotations(
            primary,
            reference,
            window_frames=1,
            min_window_displacement_m=0.0,
            min_direction_disagreement_rad=0.0,
            low_speed_rotation_blend=1.0,
            high_speed_rotation_blend=1.0,
            max_strong_blend_fraction=1.0,
            max_cumulative_correction_rad=math.radians(5.0),
        )
        self.assertGreater(counters["direction_rejected_increments"], 0)
        self.assertGreater(counters["direction_reject_orientation_resets"], 0)
        np.testing.assert_allclose(output[-1][:3, :3], primary[-1][:3, :3], atol=1e-12)

    def test_matching_direction_signal_accepts_rotation_correction(self) -> None:
        primary = [pose(x=float(i), yaw_deg=0.0) for i in range(4)]
        reference = [pose(x=float(i), y=0.2 * i, yaw_deg=0.2 * i) for i in range(4)]
        output, counters = MODULE.fuse_direction_consistent_rotations(
            primary,
            reference,
            window_frames=1,
            min_window_displacement_m=0.0,
            min_direction_disagreement_rad=0.0,
            low_speed_rotation_blend=1.0,
            high_speed_rotation_blend=1.0,
            max_strong_blend_fraction=1.0,
            max_cumulative_correction_rad=math.radians(5.0),
        )
        self.assertGreater(counters["direction_accepted_increments"], 0)
        self.assertGreater(math.atan2(output[-1][1, 0], output[-1][0, 0]), 0.0)

    def test_future_reference_change_does_not_change_output_prefix(self) -> None:
        primary = [pose(x=float(i)) for i in range(8)]
        reference = [pose(x=float(i), y=0.1 * i, yaw_deg=0.1 * i) for i in range(8)]
        changed = [item.copy() for item in reference]
        changed[6] = pose(x=100.0, y=-100.0, yaw_deg=-90.0)
        changed[7] = pose(x=200.0, y=-200.0, yaw_deg=-120.0)
        kwargs = dict(
            window_frames=2,
            min_window_displacement_m=0.0,
            min_direction_disagreement_rad=0.0,
            max_strong_blend_fraction=1.0,
            max_cumulative_correction_rad=math.radians(5.0),
        )
        baseline, _ = MODULE.fuse_direction_consistent_rotations(primary, reference, **kwargs)
        modified, _ = MODULE.fuse_direction_consistent_rotations(primary, changed, **kwargs)
        np.testing.assert_allclose(baseline[:6], modified[:6], atol=1e-12)

    def test_reference_translation_is_not_copied_to_output(self) -> None:
        primary = [pose(x=float(i)) for i in range(4)]
        reference = [pose(x=100.0 * i, y=20.0 * i, yaw_deg=0.1 * i) for i in range(4)]
        output, _ = MODULE.fuse_direction_consistent_rotations(
            primary,
            reference,
            window_frames=1,
            min_window_displacement_m=0.0,
            min_direction_disagreement_rad=0.0,
            max_strong_blend_fraction=1.0,
            max_cumulative_correction_rad=math.radians(5.0),
        )
        self.assertLess(np.linalg.norm(output[-1][:3, 3]), 10.0)

    def test_pose_count_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(RuntimeError, "pose counts differ"):
            MODULE.fuse_direction_consistent_rotations([pose()], [pose(), pose()])


if __name__ == "__main__":
    unittest.main()
