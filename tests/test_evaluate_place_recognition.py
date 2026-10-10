import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "evaluation" / "scripts" / "evaluate_place_recognition.py"
SPEC = importlib.util.spec_from_file_location("evaluate_place_recognition", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load evaluate_place_recognition")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def write_gt(path: Path, positions: list[tuple[float, float, float]]) -> None:
    lines = ["timestamp,lidar_pose.x,lidar_pose.y,lidar_pose.z,"
             "lidar_pose.roll,lidar_pose.pitch,lidar_pose.yaw"]
    for i, (x, y, z) in enumerate(positions):
        lines.append(f"{i},{x},{y},{z},0,0,0")
    path.write_text("\n".join(lines) + "\n")


class RevisitMaskTests(unittest.TestCase):
    def test_respects_exclusion_window(self) -> None:
        # Frames 0..4 on a line 1 m apart, frame 5 returns to the origin.
        positions = np.array([[i, 0, 0] for i in range(5)] + [[0, 0, 0]], float)
        mask = MODULE.revisit_mask(positions, exclude=3, radius=0.5)
        self.assertEqual(mask.tolist(), [False] * 5 + [True])
        # Frame 4 is 1 m from frame 3, but frame 3 is inside the exclusion window.
        mask = MODULE.revisit_mask(positions, exclude=1, radius=1.5)
        self.assertTrue(mask[4])


class PrCurveTests(unittest.TestCase):
    def test_perfect_ranking(self) -> None:
        curve = MODULE.pr_curve(np.array([0.9, 0.8, 0.1]),
                                np.array([True, True, False]), num_revisits=2)
        self.assertAlmostEqual(curve["f1_max"], 1.0)
        self.assertAlmostEqual(curve["recall_at_100_precision"], 1.0)

    def test_confident_false_positive_zeroes_recall_at_full_precision(self) -> None:
        curve = MODULE.pr_curve(np.array([0.9, 0.8]),
                                np.array([False, True]), num_revisits=1)
        self.assertEqual(curve["recall_at_100_precision"], 0.0)
        self.assertAlmostEqual(curve["f1_max"], 2 * 0.5 * 1.0 / 1.5)

    def test_ties_are_cut_together(self) -> None:
        curve = MODULE.pr_curve(np.array([0.5, 0.5]),
                                np.array([True, False]), num_revisits=1)
        self.assertEqual(curve["precision"], [0.5])
        self.assertEqual(curve["recall_at_100_precision"], 0.0)

    def test_missed_revisits_bound_recall(self) -> None:
        curve = MODULE.pr_curve(np.array([1.0]), np.array([True]), num_revisits=4)
        self.assertAlmostEqual(curve["recall"][-1], 0.25)


class EndToEndTests(unittest.TestCase):
    def test_scores_wrong_top1_as_false_positive(self) -> None:
        positions = [(i * 10.0, 0, 0) for i in range(6)] + [(0.5, 0, 0), (20.2, 0, 0)]
        payload = {
            "frames": 8,
            "exclude_frames": 3,
            "methods": {
                "toy": {
                    "score": "test",
                    "seconds": 0.8,
                    # Frame 6 matches frame 0 (correct), frame 7 matches 0 (wrong;
                    # its true revisit is frame 2).
                    "queries": [[-1, None]] * 6 + [[0, 0.9], [0, 0.95]],
                }
            },
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            gt = root / "gt.csv"
            results = root / "results.json"
            write_gt(gt, positions)
            results.write_text(json.dumps(payload))
            report = MODULE.evaluate(results, gt, "toy_seq", radius=4.0)
        method = report["methods"]["toy"]
        self.assertEqual(report["revisit_queries"], 2)
        self.assertEqual(method["correct_top1"], 1)
        self.assertEqual(method["recall_at_100_precision"], 0.0)
        self.assertAlmostEqual(method["f1_max"], 0.5)
        self.assertAlmostEqual(method["ms_per_frame"], 100.0)


if __name__ == "__main__":
    unittest.main()
