import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from test_experiment_scripts import load_script_module


class ElevationCorrectionSummaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module(
            "summarize_kitti_elevation_correction",
            "evaluation/scripts/summarize_kitti_elevation_correction.py",
        )

    def test_pairs_each_variant_with_its_corrected_twin(self) -> None:
        aggregate = {
            "stable_interface": {"methods": "kiss_icp", "primary_method": "KISS-ICP"},
            "variants": [
                {"id": "dense", "status": "ok", "design_style": "reference",
                 "kitti_rte_trans_pct": 0.9, "rpe_trans_pct": 0.7, "ate_m": 2.0},
                {"id": "dense_elevation", "status": "ok", "design_style": "paper_input",
                 "kitti_rte_trans_pct": 0.6, "rpe_trans_pct": 0.6, "ate_m": 1.5},
                {"id": "lonely", "status": "ok", "design_style": "reference",
                 "kitti_rte_trans_pct": 1.0},
                {"id": "failed", "status": "failed", "design_style": "reference"},
                {"id": "failed_elevation", "status": "ok", "design_style": "paper_input",
                 "kitti_rte_trans_pct": 0.5},
            ],
        }
        with tempfile.TemporaryDirectory() as tmp:
            results = Path(tmp)
            (results / "kiss_icp_kitti_seq_00_full_elevation_matrix.json").write_text(
                json.dumps(aggregate))
            (results / "kiss_icp_kitti_seq_00_full_sweep_matrix.json").write_text(
                json.dumps(aggregate))
            with mock.patch.object(self.module, "RESULTS_DIR", results):
                rows = self.module.collect_pairs()
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual((row["sequence"], row["variant"]), ("00", "dense"))
        self.assertEqual(row["paper_rte_pct"], 0.51)
        self.assertAlmostEqual(row["rte_change_pct"], -100.0 / 3.0)
        self.assertEqual(row["corrected_design_style"], "paper_input")

    def test_ingest_replaces_the_same_sequence_and_mode(self) -> None:
        manifest = {
            "command": ["python", "x", "--max-threads", "2", "--_odometry-only"],
            "configuration": {"kitti_elevation_correction": "none", "config_sha256": "c"},
            "versions": {"kiss_icp": "1.3.0"},
            "artifacts": {"estimate_sha256": "e", "reference_sha256": "r"},
            "metrics": {"frames": 10, "kitti_rte_trans_pct": 0.9},
        }
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            manifest_path = root / "zoo_manifest.json"
            official = root / "official.json"
            with mock.patch.object(self.module, "OFFICIAL_PATH", official):
                manifest_path.write_text(json.dumps(manifest))
                self.module.ingest_official(manifest_path, "7")
                manifest["metrics"]["kitti_rte_trans_pct"] = 0.8
                manifest_path.write_text(json.dumps(manifest))
                self.module.ingest_official(manifest_path, "07")
                runs = json.loads(official.read_text())["runs"]
        self.assertEqual(len(runs), 1)
        self.assertEqual(runs[0]["sequence"], "07")
        self.assertFalse(runs[0]["elevation_correction"])
        self.assertEqual(runs[0]["max_threads"], 2)
        self.assertEqual(runs[0]["metrics"]["kitti_rte_trans_pct"], 0.8)


if __name__ == "__main__":
    unittest.main()
