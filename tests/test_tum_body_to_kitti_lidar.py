import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/tum_body_to_kitti_lidar.py"
SPEC = importlib.util.spec_from_file_location("body_to_lidar", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load Body-to-LiDAR converter")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class BodyToLidarPoseTests(unittest.TestCase):
    def test_adds_rotated_body_to_lidar_translation(self) -> None:
        pose = np.eye(4)
        pose[:3, :3] = [[0.0, -1.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]]
        pose[:3, 3] = [3.0, 4.0, 5.0]
        result = MODULE.body_to_lidar_pose(pose, np.array([-0.05, 0.0, 0.055]))
        np.testing.assert_allclose(result[:3, 3], [3.0, 3.95, 5.055])

    def test_slerp_halfway_about_z(self) -> None:
        left = np.array([0.0, 0.0, 0.0, 1.0])
        right = np.array([0.0, 0.0, 1.0, 0.0])
        quaternion = MODULE.slerp(left, right, 0.5)
        rotation = MODULE.quaternion_to_rotation(quaternion)
        np.testing.assert_allclose(rotation @ [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], atol=1e-12)

    def test_boundary_pose_is_held(self) -> None:
        stamps = np.array([1.0, 2.0])
        positions = np.array([[1.0, 0.0, 0.0], [2.0, 0.0, 0.0]])
        quaternions = np.array([[0.0, 0.0, 0.0, 1.0], [0.0, 0.0, 0.0, 1.0]])
        pose, held = MODULE.interpolate_pose(0.5, stamps, positions, quaternions)
        self.assertTrue(held)
        np.testing.assert_allclose(pose[:3, 3], positions[0])


if __name__ == "__main__":
    unittest.main()
