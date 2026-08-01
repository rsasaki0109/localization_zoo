import importlib.util
import math
import sys
import unittest
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = REPO_ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "rotation_consensus_fusion",
    SCRIPT_DIR / "fuse_odometry_rotation_consensus.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load rotation consensus fusion")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def yaw_pose(yaw: float, x: float = 0.0) -> np.ndarray:
    pose = np.eye(4)
    pose[:3, :3] = np.array(
        [
            [math.cos(yaw), -math.sin(yaw), 0.0],
            [math.sin(yaw), math.cos(yaw), 0.0],
            [0.0, 0.0, 1.0],
        ]
    )
    pose[0, 3] = x
    return pose


class RotationConsensusFusionTests(unittest.TestCase):
    def test_identical_trajectories_are_unchanged(self) -> None:
        poses = [yaw_pose(0.0, 0.0), yaw_pose(0.1, 1.0)]
        fused = MODULE.fuse_rotation_consensus(poses, poses, 0.2)
        np.testing.assert_allclose(fused, poses, atol=1e-12)

    def test_reference_translation_is_not_copied(self) -> None:
        primary = [yaw_pose(0.0), yaw_pose(0.0, 1.0)]
        reference = [yaw_pose(0.0, 100.0), yaw_pose(0.0, 200.0)]
        fused = MODULE.fuse_rotation_consensus(primary, reference, 1.0)
        np.testing.assert_allclose(fused, primary, atol=1e-12)

    def test_rotation_update_is_causal_and_gain_scaled(self) -> None:
        primary = [yaw_pose(0.0), yaw_pose(0.0, 1.0)]
        reference = [yaw_pose(0.0), yaw_pose(0.2, 1.0)]
        fused = MODULE.fuse_rotation_consensus(primary, reference, 0.25)
        np.testing.assert_allclose(fused[0], primary[0], atol=1e-12)
        expected = yaw_pose(0.05)
        np.testing.assert_allclose(
            fused[1][:3, :3], expected[:3, :3], atol=1e-12
        )

    def test_rejects_length_mismatch(self) -> None:
        with self.assertRaisesRegex(ValueError, "length mismatch"):
            MODULE.fuse_rotation_consensus([np.eye(4)], [], 0.1)

    def test_rejects_invalid_gain(self) -> None:
        with self.assertRaisesRegex(ValueError, "gain"):
            MODULE.fuse_rotation_consensus([np.eye(4)], [np.eye(4)], 1.1)

    def test_deadband_holds_small_orientation_disagreement(self) -> None:
        primary = [yaw_pose(0.0), yaw_pose(0.0, 1.0)]
        reference = [yaw_pose(0.0), yaw_pose(math.radians(1.0), 1.0)]
        fused = MODULE.fuse_rotation_consensus(
            primary, reference, gain=1.0, deadband_deg=1.25
        )
        np.testing.assert_allclose(fused, primary, atol=1e-12)

    def test_deadband_allows_large_orientation_disagreement(self) -> None:
        primary = [yaw_pose(0.0), yaw_pose(0.0, 1.0)]
        reference = [yaw_pose(0.0), yaw_pose(math.radians(2.0), 1.0)]
        fused = MODULE.fuse_rotation_consensus(
            primary, reference, gain=1.0, deadband_deg=1.25
        )
        np.testing.assert_allclose(
            fused[1][:3, :3], reference[1][:3, :3], atol=1e-12
        )

    def test_causal_failover_on_primary_hold(self) -> None:
        primary = [yaw_pose(0.0, 0.0), yaw_pose(0.0, 0.0), yaw_pose(0.0, 0.0)]
        reference = [yaw_pose(0.0, 0.0), yaw_pose(0.0, 1.0), yaw_pose(0.0, 2.0)]
        diagnostics = {}
        fused = MODULE.fuse_rotation_consensus(
            primary,
            reference,
            gain=0.0,
            failover_reference_motion_m=0.05,
            diagnostics=diagnostics,
        )
        self.assertEqual(diagnostics["failover_index"], 1)
        np.testing.assert_allclose(fused[1], primary[1], atol=1e-12)
        self.assertAlmostEqual(fused[2][0, 3], 1.0)


if __name__ == "__main__":
    unittest.main()
