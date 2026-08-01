import importlib.util
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "evaluation/data/lidar_odometry_sota_head_to_head_v12_protocol.json"
STAGING = ROOT / "evaluation/data/kitti_ssd_ext4_staging_v12.json"
SCRIPT = ROOT / "evaluation/scripts/evaluate_sota_head_to_head.py"


def load_module():
    spec = importlib.util.spec_from_file_location("sota_head_to_head", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def row(candidate_ate=0.8, candidate_rpe=0.8, candidate_fps=12.0):
    return {
        "candidate": {
            "ate_m": candidate_ate,
            "rpe_trans_pct": candidate_rpe,
            "tracking_success_rate": 1.0,
            "fps": candidate_fps,
            "peak_rss_mb": 500.0,
        },
        "comparator": {
            "ate_m": 1.0,
            "rpe_trans_pct": 1.0,
            "tracking_success_rate": 1.0,
            "fps": 8.0,
            "peak_rss_mb": 600.0,
        },
    }


class SotaHeadToHeadTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.protocol = json.loads(PROTOCOL.read_text(encoding="utf-8"))
        cls.module = load_module()

    def test_protocol_is_frozen_before_candidate_search(self):
        self.assertEqual(self.protocol["status"], "frozen_before_v12_candidate_search")
        self.assertFalse(self.protocol["candidate_search_started"])
        self.assertEqual(
            set(self.protocol["cube_lio_track"]["win_gate"]["required_rows"]),
            {"kitti_odometry_00", "kitti_odometry_07"},
        )
        self.assertEqual(
            set(self.protocol["fast_livo2_track"]["datasets_declared_before_download"]),
            {"ntu_viral_eee_01", "ntu_viral_eee_02", "ntu_viral_eee_03"},
        )
        provenance = self.protocol["fast_livo2_track"]["dataset_provenance"]
        self.assertIn("GT quarantine", provenance["archive_handling"])
        self.assertIn("sensor_only", provenance["candidate_sensor_root"])
        for dataset in self.protocol["fast_livo2_track"][
            "datasets_declared_before_download"
        ].values():
            self.assertTrue(dataset["archive_url"].startswith("https://"))

    def test_completed_cube_rerun_records_required_metrics(self):
        rerun = self.protocol["cube_lio_track"]["datasets"][
            "kitti_odometry_07"
        ]["direct_rerun"]
        self.assertEqual(
            rerun["status"],
            "complete_isolated_ssd_ext4_warm_cache_three_run_median",
        )
        self.assertGreaterEqual(rerun["fps"], 10.0)
        for key in (
            "ate_m",
            "rpe_trans_pct",
            "rpe_rot_deg_per_m",
            "tracking_success_rate",
            "fps",
            "peak_rss_mb",
        ):
            self.assertIsInstance(rerun[key], (int, float))

    def test_external_ssd_ext4_staging_is_byte_identical(self):
        staging = json.loads(STAGING.read_text(encoding="utf-8"))
        self.assertEqual(staging["status"], "byte_identical_verified")
        self.assertEqual(staging["verification"]["exit_code"], 0)
        self.assertFalse(staging["benchmark_policy"]["ground_truth_inside_image"])
        self.assertEqual(
            self.protocol["storage_staging"]["evidence"],
            "evaluation/data/kitti_ssd_ext4_staging_v12.json",
        )
        for sequence, frames in (("00", 4541), ("07", 1101)):
            record = staging["verification"]["sequences"][sequence]
            self.assertEqual(record["lidar_frames"], frames)
            self.assertEqual(record["manifest_files"], frames + 1)
            self.assertTrue(record["byte_identical"])
            self.assertEqual(
                record["source_manifest_sha256"],
                record["staged_manifest_sha256"],
            )

    def test_all_gates_pass_for_strict_win(self):
        results = {
            "cube_lio_track": {
                "rows": {"kitti_odometry_00": row(), "kitti_odometry_07": row()}
            },
            "fast_livo2_track": {
                "rows": {f"ntu_viral_eee_0{i}": row() for i in range(1, 4)}
            },
        }
        report = self.module.evaluate(self.protocol, results)
        self.assertTrue(report["promotion_passed"])

    def test_cube_requires_both_accuracy_metrics_on_every_row(self):
        results = {
            "cube_lio_track": {
                "rows": {
                    "kitti_odometry_00": row(candidate_ate=1.01),
                    "kitti_odometry_07": row(),
                }
            },
            "fast_livo2_track": {
                "rows": {f"ntu_viral_eee_0{i}": row() for i in range(1, 4)}
            },
        }
        report = self.module.evaluate(self.protocol, results)
        self.assertFalse(report["cube_lio_track"]["passed"])
        self.assertFalse(report["promotion_passed"])

    def test_fast_livo2_rejects_any_row_over_three_percent_regression(self):
        fast_rows = {f"ntu_viral_eee_0{i}": row() for i in range(1, 4)}
        fast_rows["ntu_viral_eee_03"] = row(candidate_rpe=1.031)
        results = {
            "cube_lio_track": {
                "rows": {"kitti_odometry_00": row(), "kitti_odometry_07": row()}
            },
            "fast_livo2_track": {"rows": fast_rows},
        }
        report = self.module.evaluate(self.protocol, results)
        self.assertFalse(report["fast_livo2_track"]["passed"])
        self.assertFalse(report["promotion_passed"])

    def test_memory_is_mandatory_even_without_a_promotion_threshold(self):
        invalid = row()
        del invalid["candidate"]["peak_rss_mb"]
        with self.assertRaisesRegex(ValueError, "peak_rss_mb"):
            self.module.evaluate_cube(
                self.protocol,
                {"kitti_odometry_00": invalid, "kitti_odometry_07": row()},
            )


if __name__ == "__main__":
    unittest.main()
