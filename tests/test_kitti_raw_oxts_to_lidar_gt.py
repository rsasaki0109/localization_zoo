import importlib.util
import sys
from pathlib import Path

import numpy as np


SCRIPT = Path(__file__).resolve().parents[1] / "evaluation" / "scripts" / "kitti_raw_oxts_to_lidar_gt.py"
SPEC = importlib.util.spec_from_file_location("kitti_raw_oxts_to_lidar_gt", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def test_rpy_round_trip() -> None:
    expected = (0.12, -0.21, 0.34)
    actual = MODULE.rpy_from_rotation(MODULE.rotation_from_rpy(*expected))
    np.testing.assert_allclose(actual, expected, atol=1e-12)


def test_first_lidar_pose_is_identity_with_nontrivial_extrinsic() -> None:
    packets = [
        [49.0, 8.0, 100.0, 0.01, -0.02, 0.03],
        [49.00001, 8.00002, 100.2, 0.02, -0.01, 0.04],
    ]
    velo_from_imu = np.eye(4)
    velo_from_imu[:3, :3] = MODULE.rotation_from_rpy(0.1, 0.2, -0.3)
    velo_from_imu[:3, 3] = [0.8, -0.2, 1.5]
    poses = MODULE.build_lidar_poses(packets, velo_from_imu)
    np.testing.assert_allclose(poses[0], np.eye(4), atol=1e-9)
    assert np.linalg.norm(poses[1][:3, 3]) > 0.5
    np.testing.assert_allclose(poses[1][:3, :3].T @ poses[1][:3, :3], np.eye(3), atol=1e-9)
