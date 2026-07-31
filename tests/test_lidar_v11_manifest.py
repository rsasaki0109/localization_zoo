import hashlib
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class LidarV11ManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.candidate = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_candidate_density_adaptive_reference_v11.json").read_text()
        )
        cls.external = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_v11_external_validation.json").read_text()
        )

    def test_runtime_only_selection_is_frozen(self) -> None:
        policy = self.candidate["development_policy"]
        self.assertTrue(self.candidate["configuration_frozen"])
        self.assertFalse(policy["ground_truth_loaded_by_profiler"])
        self.assertFalse(policy["v11_candidate_metrics_inspected_during_selection"])
        self.assertEqual(
            self.candidate["runtime_search"]["selected_max_input_points"], 80000
        )
        self.assertGreater(
            self.candidate["frozen_runtime_result"]["sequential_combined_algorithm_fps"],
            10.0,
        )

    def test_external_result_is_not_over_promoted(self) -> None:
        result = self.external["result"]
        self.assertEqual(result["accuracy"]["tracking_gate"], "pass_600_of_600")
        self.assertEqual(result["accuracy"]["accuracy_gate"], "pass")
        self.assertLess(result["accuracy"]["v11_vs_v10"]["ate_change_pct"], 0.0)
        self.assertLess(result["accuracy"]["v11_vs_v10"]["rpe_trans_change_pct"], 0.0)
        self.assertLess(
            result["runtime_before_gt_download"]["v11_sequential_combined_algorithm_fps"],
            10.0,
        )
        self.assertFalse(self.candidate["promoted"])
        self.assertIn("not_promoted", self.candidate["status"])
        self.assertFalse(result["post_result_tuning"])

    def test_trajectory_freeze_precedes_ground_truth(self) -> None:
        result = self.external["result"]
        self.assertFalse(result["real_ground_truth_present_during_trajectory_phase"])
        self.assertEqual(len(result["trajectory_sha256_before_gt_download"]), 5)
        self.assertIn("ground_truth_after_trajectory_freeze", result)

    def test_implementation_hashes_match(self) -> None:
        for relative, expected in self.candidate["implementation_sha256"].items():
            content = (ROOT / relative).read_text(encoding="utf-8")
            actual = hashlib.sha256(content.replace("\r\n", "\n").encode()).hexdigest()
            self.assertEqual(actual, expected, relative)


if __name__ == "__main__":
    unittest.main()
