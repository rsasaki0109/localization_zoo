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

    def test_uses_frozen_fallback_when_correction_is_rejected(self) -> None:
        raw = Path("raw.txt")
        corrected = Path("corrected.txt")
        fallback = Path("fallback.txt")
        corroborated = Path("corroborated.txt")
        self.assertEqual(
            MODULE.select_source(raw, corrected, fallback, False), fallback
        )
        self.assertEqual(
            MODULE.select_source(raw, corrected, fallback, True), corrected
        )
        self.assertEqual(MODULE.select_source(raw, corrected, None, False), raw)
        self.assertEqual(
            MODULE.select_source(raw, corrected, fallback, False, corroborated, True),
            corroborated,
        )
        self.assertEqual(
            MODULE.select_source(raw, corrected, fallback, True, corroborated, True),
            corrected,
        )

    def test_rotation_correction_agreement_detects_same_hemisphere(self) -> None:
        raw = np.repeat(np.eye(4)[None, :, :], 2, axis=0)
        corrected = raw.copy()
        reference = raw.copy()
        for poses, angle_degrees in ((corrected, 2.0), (reference, 4.0)):
            angle = np.radians(angle_degrees)
            poses[1, :3, :3] = [
                [np.cos(angle), -np.sin(angle), 0.0],
                [np.sin(angle), np.cos(angle), 0.0],
                [0.0, 0.0, 1.0],
            ]
        agrees, active, cosine, positive_fraction = (
            MODULE.rotation_correction_agreement(raw, corrected, reference)
        )
        self.assertTrue(agrees)
        self.assertEqual(active, 1)
        self.assertAlmostEqual(cosine, 1.0)
        self.assertAlmostEqual(positive_fraction, 1.0)

        reference[1, :2, :2] = reference[1, :2, :2].T
        agrees, _, cosine, positive_fraction = MODULE.rotation_correction_agreement(
            raw, corrected, reference
        )
        self.assertFalse(agrees)
        self.assertAlmostEqual(cosine, -1.0)
        self.assertAlmostEqual(positive_fraction, 0.0)

    def test_overwhelming_loop_support_can_select_accumulated_correction(self) -> None:
        self.assertTrue(MODULE.correction_is_selected(True, True, False, False, True))
        self.assertFalse(MODULE.correction_is_selected(True, True, False, False))
        self.assertTrue(MODULE.correction_is_selected(True, False, True, True))
        self.assertFalse(MODULE.correction_is_selected(True, False, False, True))


if __name__ == "__main__":
    unittest.main()
