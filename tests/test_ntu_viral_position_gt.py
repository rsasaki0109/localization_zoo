import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/evaluate_ntu_viral_position_gt.py"
SPEC = importlib.util.spec_from_file_location("ntu_position_gt", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load NTU position-only evaluator")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class NtuViralPositionGtTests(unittest.TestCase):
    def test_interpolates_only_bracketed_gt(self) -> None:
        indices, positions = MODULE.interpolate_gt(
            np.array([-1.0, 0.5, 1.5, 3.0]),
            np.array([0.0, 1.0, 2.0]),
            np.array([[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [2.0, 0.0, 0.0]]),
            1.1,
        )
        np.testing.assert_array_equal(indices, [1, 2])
        np.testing.assert_allclose(positions[:, 0], [0.5, 1.5])

    def test_exact_trajectory_has_zero_segment_rpe(self) -> None:
        positions = np.array([[float(i), 0.0, 0.0] for i in range(202)])
        errors = MODULE.segment_rpe_percent(positions, positions, 100.0)
        np.testing.assert_allclose(errors, 0.0)

    def test_scale_error_is_reported_as_percent(self) -> None:
        gt = np.array([[float(i), 0.0, 0.0] for i in range(202)])
        estimated = 1.01 * gt
        errors = MODULE.segment_rpe_percent(estimated, gt, 100.0)
        np.testing.assert_allclose(errors, 1.0)


if __name__ == "__main__":
    unittest.main()
