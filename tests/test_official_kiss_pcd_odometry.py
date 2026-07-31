import importlib.util
import sys
import unittest
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = REPO_ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "official_kiss_pcd_odometry",
    SCRIPT_DIR / "run_official_kiss_pcd_odometry.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load official KISS PCD runner")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class OfficialKissPcdOdometryTests(unittest.TestCase):
    def test_normalizes_timestamp_span(self) -> None:
        normalized = MODULE.normalize_timestamps(np.array([10.0, 15.0, 20.0]))
        np.testing.assert_allclose(normalized, [0.0, 0.5, 1.0])

    def test_constant_timestamps_disable_deskew(self) -> None:
        normalized = MODULE.normalize_timestamps(np.array([4.0, 4.0]))
        self.assertEqual(normalized.size, 0)

    def test_command_surface_has_no_ground_truth_argument(self) -> None:
        source = (SCRIPT_DIR / "run_official_kiss_pcd_odometry.py").read_text()
        self.assertNotIn("reference-csv", source)
        self.assertNotIn("ground_truth.csv", source)

    def test_timestamp_adaptive_policy_selects_four_without_timestamps(self) -> None:
        selected = MODULE.select_thread_cap(
            policy="timestamp_adaptive",
            fixed_cap=0,
            first_pcd_has_timestamps=False,
            first_pcd_point_count=126_085,
            timestamped_cap=8,
            untimestamped_cap=4,
        )
        self.assertEqual(selected, 4)

    def test_timestamp_adaptive_policy_selects_eight_with_timestamps(self) -> None:
        selected = MODULE.select_thread_cap(
            policy="timestamp_adaptive",
            fixed_cap=0,
            first_pcd_has_timestamps=True,
            first_pcd_point_count=40_013,
            timestamped_cap=8,
            untimestamped_cap=4,
        )
        self.assertEqual(selected, 8)

    def test_timestamp_adaptive_policy_selects_eight_for_small_scan(self) -> None:
        selected = MODULE.select_thread_cap(
            policy="timestamp_adaptive",
            fixed_cap=0,
            first_pcd_has_timestamps=False,
            first_pcd_point_count=24_309,
            timestamped_cap=8,
            untimestamped_cap=4,
        )
        self.assertEqual(selected, 8)

    def test_fixed_policy_preserves_existing_cap(self) -> None:
        selected = MODULE.select_thread_cap(
            policy="fixed",
            fixed_cap=12,
            first_pcd_has_timestamps=False,
            first_pcd_point_count=126_085,
            timestamped_cap=8,
            untimestamped_cap=4,
        )
        self.assertEqual(selected, 12)


if __name__ == "__main__":
    unittest.main()
