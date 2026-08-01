import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = ROOT / "evaluation/scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "three_vote_rotation_fallback",
    SCRIPT_DIR / "build_three_vote_rotation_fallback.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load three-vote rotation fallback")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def pose(x: float, yaw: float) -> np.ndarray:
    result = np.eye(4)
    result[:3, :3] = MODULE.exp_so3(np.array([0.0, 0.0, yaw]))
    result[0, 3] = x
    return result


class ThreeVoteRotationFallbackTests(unittest.TestCase):
    def test_reference_receives_exactly_two_of_three_rotation_votes(self) -> None:
        primary = [pose(0.0, 0.0), pose(1.0, 0.0)]
        reference = [pose(0.0, 0.0), pose(9.0, 0.03)]
        output, gated = MODULE.build_three_vote_rotation_fallback(primary, reference)
        rotation_vector = MODULE.log_so3(output[1][:3, :3])
        self.assertEqual(gated, 0)
        np.testing.assert_allclose(rotation_vector, [0.0, 0.0, 0.02], atol=1e-12)
        np.testing.assert_allclose(output[1][:3, 3], [1.0, 0.0, 0.0])


if __name__ == "__main__":
    unittest.main()
