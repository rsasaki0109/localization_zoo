import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/select_gt_free_pose_graph.py"
SPEC = importlib.util.spec_from_file_location("pose_graph_selector", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load GT-free pose graph selector")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class PoseGraphSelectorTests(unittest.TestCase):
    def test_measures_translation_and_rotation_correction_envelope(self) -> None:
        raw = np.repeat(np.eye(4)[None, :, :], 2, axis=0)
        corrected = raw.copy()
        corrected[1, 0, 3] = 0.25
        angle = np.radians(3.0)
        corrected[1, :3, :3] = [
            [np.cos(angle), -np.sin(angle), 0.0],
            [np.sin(angle), np.cos(angle), 0.0],
            [0.0, 0.0, 1.0],
        ]
        maximum_t, final_t, maximum_r, final_r = MODULE.correction_envelope(
            raw, corrected
        )
        self.assertAlmostEqual(maximum_t, 0.25)
        self.assertAlmostEqual(final_t, 0.25)
        self.assertAlmostEqual(maximum_r, 3.0)
        self.assertAlmostEqual(final_r, 3.0)

    def test_rejects_mismatched_pose_counts(self) -> None:
        with self.assertRaisesRegex(ValueError, "counts must match"):
            MODULE.correction_envelope(
                np.repeat(np.eye(4)[None, :, :], 2, axis=0),
                np.repeat(np.eye(4)[None, :, :], 3, axis=0),
            )


if __name__ == "__main__":
    unittest.main()
