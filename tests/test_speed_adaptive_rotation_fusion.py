import importlib.util
import sys
from pathlib import Path

import numpy as np


SCRIPT = Path(__file__).resolve().parents[1] / "evaluation" / "scripts" / "fuse_odometry_speed_adaptive_rotation.py"
SCRIPT_DIR = str(SCRIPT.parent)
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)
SPEC = importlib.util.spec_from_file_location("speed_adaptive_rotation_fusion", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def pose(x: float, yaw: float = 0.0) -> np.ndarray:
    result = np.eye(4)
    result[:3, :3] = MODULE.exp_so3(np.array([0.0, 0.0, yaw]))
    result[0, 3] = x
    return result


def test_speed_schedule_selects_both_branches_causally() -> None:
    primary = [pose(0.0), pose(0.5), pose(2.5)]
    reference = [pose(100.0), pose(-100.0, 0.2), pose(999.0, 0.4)]
    fused, counters = MODULE.fuse_speed_adaptive_rotations(
        primary, reference, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0
    )
    assert counters["low_speed_increments"] == 1
    assert counters["high_speed_increments"] == 1
    np.testing.assert_allclose(fused[1][:3, :3], reference[1][:3, :3], atol=1e-12)
    np.testing.assert_allclose(fused[2][:3, :3], fused[1][:3, :3], atol=1e-12)


def test_reference_translation_is_ignored() -> None:
    primary = [pose(0.0), pose(0.5), pose(1.0)]
    reference_a = [pose(0.0), pose(10.0, 0.1), pose(20.0, 0.2)]
    reference_b = [pose(100.0), pose(-50.0, 0.1), pose(999.0, 0.2)]
    args = (0.2, 0.05, 1.0, 1.0, 1.0, 1.0)
    fused_a, _ = MODULE.fuse_speed_adaptive_rotations(primary, reference_a, *args)
    fused_b, _ = MODULE.fuse_speed_adaptive_rotations(primary, reference_b, *args)
    np.testing.assert_allclose(fused_a, fused_b, atol=1e-12)


def test_disagreement_gate_overrides_speed_schedule() -> None:
    primary = [pose(0.0), pose(0.5, 0.01)]
    reference = [pose(0.0), pose(0.5, 1.0)]
    fused, counters = MODULE.fuse_speed_adaptive_rotations(
        primary, reference, 1.0, 1.0, 1.0, 1.0, 1.0, np.deg2rad(5.0)
    )
    assert counters["gated_increments"] == 1
    np.testing.assert_allclose(fused, primary, atol=1e-12)


def test_strong_blend_requires_small_disagreement() -> None:
    primary = [pose(0.0), pose(0.5)]
    reference = [pose(0.0), pose(0.5, 0.2)]
    fused, counters = MODULE.fuse_speed_adaptive_rotations(
        primary, reference, 1.0, 0.0, 1.0, 0.1, 1.0, 1.0
    )
    assert counters["low_speed_increments"] == 0
    assert counters["high_speed_increments"] == 1
    np.testing.assert_allclose(fused, primary, atol=1e-12)


def test_strong_blend_is_causally_rate_limited() -> None:
    primary = [pose(float(i) * 0.1) for i in range(11)]
    reference = [pose(float(i) * 10.0, float(i) * 0.01) for i in range(11)]
    _, counters = MODULE.fuse_speed_adaptive_rotations(
        primary, reference, 1.0, 0.0, 1.0, 1.0, 0.2, 1.0
    )
    assert counters["strong_blend_candidates"] == 10
    assert counters["low_speed_increments"] == 2
    assert counters["rate_limited_increments"] == 8


def test_cumulative_orientation_correction_is_clamped() -> None:
    primary = [pose(0.0), pose(0.5)]
    reference = [pose(0.0), pose(0.5, 0.2)]
    max_correction = 0.05
    fused, counters = MODULE.fuse_speed_adaptive_rotations(
        primary,
        reference,
        1.0,
        0.0,
        1.0,
        1.0,
        1.0,
        1.0,
        max_correction,
    )
    correction = MODULE.log_so3(
        primary[1][:3, :3].T @ fused[1][:3, :3]
    )
    np.testing.assert_allclose(np.linalg.norm(correction), max_correction, atol=1e-12)
    assert counters["cumulative_correction_clamps"] == 1
