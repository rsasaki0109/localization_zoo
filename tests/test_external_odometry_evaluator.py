import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "evaluation" / "scripts" / "evaluate_external_kitti_odometry.py"
SPEC = importlib.util.spec_from_file_location("external_odometry_evaluator", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"Cannot load {SCRIPT}")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def pose(x: float, y: float = 0.0) -> np.ndarray:
    result = np.eye(4)
    result[0, 3] = x
    result[1, 3] = y
    return result


class ExternalOdometryEvaluatorTests(unittest.TestCase):
    def test_tum_loader_uses_xyzw_quaternion_order(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "trajectory.tum"
            # 90 degrees about Z in TUM's qx qy qz qw order.
            path.write_text(
                "# timestamp tx ty tz qx qy qz qw\n"
                "1.25 1 2 3 0 0 0.7071067811865476 0.7071067811865476\n"
            )
            poses = MODULE.load_tum_poses(path)
        self.assertEqual(len(poses), 1)
        np.testing.assert_allclose(poses[0][:3, 3], [1, 2, 3])
        np.testing.assert_allclose(
            poses[0][:3, :3],
            [[0, -1, 0], [1, 0, 0], [0, 0, 1]],
            atol=1e-12,
        )

    def test_perfect_relative_trajectory_ignores_first_world_pose(self) -> None:
        estimates = [pose(0), pose(50), pose(100)]
        references = [pose(10, 4), pose(60, 4), pose(110, 4)]
        metrics = MODULE.compute_metrics(estimates, references, 100.0)
        self.assertAlmostEqual(metrics["ate_m"], 0.0)
        self.assertAlmostEqual(metrics["rpe_trans_pct"], 0.0)
        self.assertAlmostEqual(metrics["rpe_rot_deg_per_m"], 0.0)
        self.assertEqual(metrics["rpe_segments"], 1)

    def test_undo_conjugation_restores_sensor_poses(self) -> None:
        extrinsic = np.eye(4)
        extrinsic[:3, :3] = MODULE.rpy_matrix(0.1, -0.2, 0.3)
        extrinsic[:3, 3] = [0.4, -0.1, 1.2]
        sensor_poses = [pose(0), pose(3, 1)]
        output_poses = [
            extrinsic @ sensor_pose @ np.linalg.inv(extrinsic)
            for sensor_pose in sensor_poses
        ]
        restored = MODULE.undo_conjugation(output_poses, extrinsic)
        for actual, expected in zip(restored, sensor_poses):
            np.testing.assert_allclose(actual, expected, atol=1e-12)


if __name__ == "__main__":
    unittest.main()
