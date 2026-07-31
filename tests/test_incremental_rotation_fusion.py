import importlib.util
import sys
from pathlib import Path

import numpy as np


SCRIPT = Path(__file__).resolve().parents[1] / "evaluation" / "scripts" / "fuse_odometry_incremental_rotation.py"
SPEC = importlib.util.spec_from_file_location("incremental_rotation_fusion", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def pose(x: float, yaw: float = 0.0) -> np.ndarray:
    result = np.eye(4)
    result[:3, :3] = MODULE.exp_so3(np.array([0.0, 0.0, yaw]))
    result[0, 3] = x
    return result


def test_zero_blend_reproduces_primary() -> None:
    primary = [pose(0.0), pose(1.0, 0.1), pose(2.0, 0.2)]
    reference = [pose(5.0), pose(6.0, -0.1), pose(7.0, -0.2)]
    fused, gated = MODULE.fuse_incremental_rotations(primary, reference, 0.0, 1.0)
    assert gated == 0
    np.testing.assert_allclose(fused, primary, atol=1e-12)


def test_reference_translation_is_ignored() -> None:
    primary = [pose(0.0), pose(1.0), pose(2.0)]
    reference_a = [pose(0.0), pose(10.0, 0.1), pose(20.0, 0.2)]
    reference_b = [pose(100.0), pose(-50.0, 0.1), pose(999.0, 0.2)]
    fused_a, _ = MODULE.fuse_incremental_rotations(primary, reference_a, 0.5, 1.0)
    fused_b, _ = MODULE.fuse_incremental_rotations(primary, reference_b, 0.5, 1.0)
    np.testing.assert_allclose(fused_a, fused_b, atol=1e-12)


def test_large_rotation_disagreement_is_gated_to_primary() -> None:
    primary = [pose(0.0), pose(1.0, 0.01)]
    reference = [pose(0.0), pose(1.0, 1.0)]
    fused, gated = MODULE.fuse_incremental_rotations(
        primary, reference, 1.0, np.deg2rad(5.0)
    )
    assert gated == 1
    np.testing.assert_allclose(fused, primary, atol=1e-12)
