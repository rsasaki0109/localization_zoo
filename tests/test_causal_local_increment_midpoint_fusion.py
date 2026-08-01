import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = REPO_ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "causal_local_increment_midpoint",
    SCRIPT_DIR / "fuse_odometry_causal_local_increment_midpoint.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load causal local-increment midpoint fusion")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def pose(x: float, y: float = 0.0, yaw_quadrants: int = 0) -> np.ndarray:
    result = np.eye(4)
    angle = yaw_quadrants * np.pi / 2.0
    result[:2, :2] = [[np.cos(angle), -np.sin(angle)], [np.sin(angle), np.cos(angle)]]
    result[:3, 3] = [x, y, 0.0]
    return result


class CausalLocalIncrementMidpointFusionTests(unittest.TestCase):
    def test_local_increment_midpoint_is_integrated_by_published_rotation(self) -> None:
        output = MODULE.fuse_causal_local_increment_midpoint(
            [pose(0.0), pose(2.0)],
            [pose(0.0), pose(4.0)],
            [pose(0.0, yaw_quadrants=1), pose(0.0, yaw_quadrants=1)],
        )
        np.testing.assert_allclose(output[1][:3, 3], [0.0, 3.0, 0.0], atol=1e-12)

    def test_rotation_is_copied_from_rotation_source(self) -> None:
        rotation = [pose(0.0), pose(0.0, yaw_quadrants=1)]
        output = MODULE.fuse_causal_local_increment_midpoint(
            [pose(0.0), pose(1.0)], [pose(0.0), pose(1.0)], rotation
        )
        np.testing.assert_allclose(output[1][:3, :3], rotation[1][:3, :3])

    def test_future_change_does_not_change_output_prefix(self) -> None:
        primary = [pose(0.0), pose(1.0), pose(2.0)]
        reference_a = [pose(0.0), pose(2.0), pose(3.0)]
        reference_b = [pose(0.0), pose(2.0), pose(300.0)]
        rotation = [pose(0.0), pose(0.0), pose(0.0)]
        output_a = MODULE.fuse_causal_local_increment_midpoint(
            primary, reference_a, rotation
        )
        output_b = MODULE.fuse_causal_local_increment_midpoint(
            primary, reference_b, rotation
        )
        np.testing.assert_allclose(output_a[0], output_b[0])
        np.testing.assert_allclose(output_a[1], output_b[1])

    def test_pose_count_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "pose counts must match"):
            MODULE.fuse_causal_local_increment_midpoint(
                [pose(0.0)], [pose(0.0), pose(1.0)], [pose(0.0)]
            )


if __name__ == "__main__":
    unittest.main()
