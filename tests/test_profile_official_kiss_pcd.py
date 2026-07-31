import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/profile_official_kiss_pcd.py"
SCRIPT_DIR = str(SCRIPT.parent)
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)
SPEC = importlib.util.spec_from_file_location("profile_official_kiss_pcd", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class OfficialKissProfilerTests(unittest.TestCase):
    def test_duration_summary(self) -> None:
        result = MODULE.summarize_durations([1_000_000, 3_000_000], 2)
        self.assertEqual(result["seconds"], 0.004)
        self.assertEqual(result["mean_ms"], 2.0)
        self.assertEqual(result["p50_ms"], 2.0)
        self.assertEqual(result["stage_fps"], 500.0)

    def test_count_summary(self) -> None:
        result = MODULE.summarize_counts([10, 20, 30])
        self.assertEqual(result["min"], 10)
        self.assertEqual(result["mean"], 20.0)
        self.assertEqual(result["max"], 30)

    def test_estimate_comparison_ignores_insignificant_last_digits(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            actual = Path(tmpdir) / "actual.txt"
            expected = Path(tmpdir) / "expected.txt"
            expected.write_text("1 0 0 2 0 1 0 3 0 0 1 4\n")
            actual.write_text("1 0 0 2.000000000001 0 1 0 3 0 0 1 4\n")
            result = MODULE.compare_estimates(actual, expected)
            self.assertTrue(result["same_shape"])
            self.assertTrue(result["numeric_equivalent_at_1e_10"])
            self.assertGreater(result["max_abs_matrix_delta"], 0.0)

    def test_density_cap_preserves_endpoints_and_timestamp_alignment(self) -> None:
        points = np.arange(30, dtype=np.float64).reshape(10, 3)
        timestamps = np.arange(10, dtype=np.float64) / 9.0
        selected, selected_timestamps = MODULE.density_cap(points, timestamps, 4)
        np.testing.assert_array_equal(selected, points[[0, 3, 6, 9]])
        np.testing.assert_array_equal(selected_timestamps, timestamps[[0, 3, 6, 9]])

    def test_density_cap_is_noop_below_limit(self) -> None:
        points = np.ones((3, 3))
        timestamps = np.arange(3)
        selected, selected_timestamps = MODULE.density_cap(points, timestamps, 4)
        self.assertIs(selected, points)
        self.assertIs(selected_timestamps, timestamps)


if __name__ == "__main__":
    unittest.main()
