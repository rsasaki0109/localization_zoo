import hashlib
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class LidarV10ManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_candidate_direction_consistent_rotation_v10.json").read_text()
        )
        cls.aggregate = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_v10_aggregate.json").read_text()
        )
        cls.suite = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_sota_suite.json").read_text()
        )

    def test_v10_is_promoted_suite_candidate(self) -> None:
        self.assertTrue(self.manifest["promoted"])
        self.assertEqual(self.manifest["status"], "promoted_fresh_heldout_pass")
        self.assertEqual(
            self.suite["promoted_candidate_manifest"],
            "evaluation/data/lidar_odometry_candidate_direction_consistent_rotation_v10.json",
        )

    def test_all_development_ate_rows_improve_and_rpe_guard_holds(self) -> None:
        rows = self.manifest["development_results"]
        self.assertGreaterEqual(len(rows), 7)
        self.assertTrue(all(row["ate_change_pct"] < 0.0 for row in rows))
        self.assertTrue(all(row["rpe_change_pct"] <= 1.0 for row in rows))

    def test_fresh_gates_pass_without_post_result_tuning(self) -> None:
        fresh = self.manifest["fresh_held_out_result"]
        self.assertEqual(fresh["ate_gate"], "pass")
        self.assertEqual(fresh["rpe_gate"], "pass")
        self.assertEqual(fresh["runtime_gate"], "pass")
        self.assertFalse(fresh["post_result_tuning"])
        self.assertLess(fresh["frozen_v10"]["ate_change_vs_v6_pct"], 0.0)
        self.assertLessEqual(fresh["frozen_v10"]["rpe_trans_change_vs_v6_pct"], 1.0)

    def test_same_input_v10_wins_ate_and_translational_rpe(self) -> None:
        rows = self.aggregate["same_input_kitti04"]
        candidate = next(row for row in rows if row["method"] == "direction_consistent_rotation_v10")
        self.assertEqual(candidate["ate_m"], min(row["ate_m"] for row in rows))
        self.assertEqual(
            candidate["rpe_trans_pct"], min(row["rpe_trans_pct"] for row in rows)
        )
        self.assertGreaterEqual(candidate["sequential_combined_algorithm_fps"], 10.0)

    def test_boreas_external_transfer_is_gt_isolated_and_not_overclaimed(self) -> None:
        boreas = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_boreas_external_validation.json").read_text()
        )
        result = boreas["result"]
        self.assertIn("completed_gt_isolated", boreas["status"])
        self.assertFalse(result["post_result_tuning"])
        self.assertLess(result["frozen_v10"]["ate_change_vs_v6_pct"], 0.0)
        self.assertLess(result["frozen_v10"]["rpe_trans_change_vs_v6_pct"], 0.0)
        self.assertLess(result["frozen_v10"]["sequential_combined_algorithm_fps"], 10.0)
        self.assertIn("limitation", result["runtime"])
        self.assertEqual(self.aggregate["cross_dataset_summary"]["datasets"], 9)
        self.assertEqual(len(self.aggregate["cross_dataset_summary"]["families"]), 5)

    def test_boreas_tool_hashes_match_the_frozen_manifest(self) -> None:
        boreas = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_boreas_external_validation.json").read_text()
        )
        for name, expected in boreas["implementation_sha256"].items():
            path = ROOT / "evaluation/scripts" / name
            actual = hashlib.sha256(path.read_bytes()).hexdigest()
            self.assertEqual(actual, expected, name)


if __name__ == "__main__":
    unittest.main()
