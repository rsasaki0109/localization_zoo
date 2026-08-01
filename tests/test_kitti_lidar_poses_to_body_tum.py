import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/kitti_lidar_poses_to_body_tum.py"
SPEC = importlib.util.spec_from_file_location("lidar_to_body", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load LiDAR-to-body converter")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class LidarToBodyPoseTests(unittest.TestCase):
    def test_removes_rotated_body_to_lidar_translation(self) -> None:
        pose = np.eye(4)
        pose[:3, :3] = [[0.0, -1.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]]
        pose[:3, 3] = [3.0, 4.0, 5.0]
        result = MODULE.lidar_to_body_pose(pose, np.array([-0.05, 0.0, 0.055]))
        np.testing.assert_allclose(result[:3, 3], [3.0, 4.05, 4.945])

    def test_identity_rotation_has_identity_quaternion(self) -> None:
        np.testing.assert_allclose(
            MODULE.rotation_to_quaternion(np.eye(3)), [0.0, 0.0, 0.0, 1.0]
        )


if __name__ == "__main__":
    unittest.main()
