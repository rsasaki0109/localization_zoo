import importlib.util
import math
import sys
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "evaluation" / "scripts" / "mulran_pose_io.py"
SPEC = importlib.util.spec_from_file_location("mulran_pose_io_test", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load mulran_pose_io")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class MulRanPoseIoTests(unittest.TestCase):
    def test_identity_base_pose_applies_official_lidar_extrinsic(self) -> None:
        base = MODULE.MulRanPoseRow(123, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
        lidar = MODULE.base_pose_to_lidar_pose(base)

        self.assertEqual(lidar.timestamp_ns, 123)
        self.assertAlmostEqual(lidar.x, 1.7042)
        self.assertAlmostEqual(lidar.y, -0.0210)
        self.assertAlmostEqual(lidar.z, 1.8047)
        self.assertAlmostEqual(abs(lidar.yaw), math.pi - math.atan(0.0058), places=6)

    def test_relative_lidar_motion_is_extrinsic_conjugated(self) -> None:
        first = MODULE.MulRanPoseRow(1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
        second = MODULE.MulRanPoseRow(2, 1.0, 0.0, 0.0, 0.0, 0.0, 0.1)
        first_lidar = MODULE.pose_row_to_matrix(
            MODULE.base_pose_to_lidar_pose(first)
        )
        second_lidar = MODULE.pose_row_to_matrix(
            MODULE.base_pose_to_lidar_pose(second)
        )
        actual = MODULE.np.linalg.inv(first_lidar) @ second_lidar
        base_delta = (
            MODULE.np.linalg.inv(MODULE.pose_row_to_matrix(first))
            @ MODULE.pose_row_to_matrix(second)
        )
        extrinsic = MODULE.MULRAN_T_BASE_LIDAR
        expected = MODULE.np.linalg.inv(extrinsic) @ base_delta @ extrinsic

        self.assertTrue(MODULE.np.allclose(actual, expected, atol=1e-10))


if __name__ == "__main__":
    unittest.main()
