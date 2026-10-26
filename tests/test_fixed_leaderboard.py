"""Contracts for fixed-configuration ranking and failure preservation."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("fixed_board", ROOT / "evaluation/scripts/generate_fixed_leaderboard.py")
board = importlib.util.module_from_spec(spec)
spec.loader.exec_module(board)


class FixedLeaderboardTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.manifest = {
            "schema_version": 1, "evidence_ref": "main", "sequences": {"00": 2, "01": 3},
            "metric": "kitti_rte_trans_pct", "aggregation": "mean",
            "methods": [{
                "method": "sample", "name": "Sample", "variant": "fixed",
                "args": ["--no-gt-seed"], "dataset_extra_args": [],
                "selection_sequences": ["00"], "selection_note": "Selected on 00",
                "selection_source": "papers/sample/README.md",
                "results": {"00": "results/00.json", "01": "results/01.json"}
            }]
        }
        path = self.root / board.MANIFEST
        path.parent.mkdir(parents=True)
        path.write_text(json.dumps(self.manifest))
        for sequence, frames in self.manifest["sequences"].items():
            self.write(sequence, {
                "stable_interface": {"methods": "sample"},
                "dataset": {"pcd_dir": "kitti_seq_" + sequence + "_full", "extra_args": []},
                "variants": [{"id": "fixed", "args": ["--no-gt-seed"], "note": "no GT seed",
                              "status": "ok", "frames": frames, "kitti_rte_trans_pct": 1 if sequence == "00" else 9}]
            })

    def write(self, seq, data):
        path = self.root / ("results/" + seq + ".json")
        path.parent.mkdir(exist_ok=True)
        path.write_text(json.dumps(data))

    def mutate(self, seq, **changes):
        path = self.root / ("results/" + seq + ".json")
        data = json.loads(path.read_text())
        data["variants"][0].update(changes)
        self.write(seq, data)

    def row(self):
        return board.collect(self.root)["methods"][0]

    def test_sequence_columns_are_sorted(self):
        path = self.root / board.MANIFEST
        manifest = json.loads(path.read_text())
        manifest["sequences"] = {"01": 3, "00": 2}
        path.write_text(json.dumps(manifest))
        data = board.collect(self.root)
        self.assertEqual(list(data["sequences"]), ["00", "01"])
        self.assertEqual(list(data["methods"][0]["cells"]), ["00", "01"])

    def test_mean_includes_large_error(self):
        self.assertEqual(self.row()["mean"], 5)
        self.assertEqual(self.row()["rank"], 1)

    def test_does_not_select_better_variant(self):
        path = self.root / "results/01.json"
        data = json.loads(path.read_text())
        data["variants"].append({**data["variants"][0], "id": "tuned", "kitti_rte_trans_pct": 0})
        self.write("01", data)
        self.assertEqual(self.row()["mean"], 5)

    def test_missing_is_unranked(self):
        (self.root / "results/01.json").unlink()
        self.assertIsNone(self.row()["rank"])
        self.assertIsNone(self.row()["mean"])
        self.assertEqual(self.row()["cells"]["01"]["status"], "missing")

    def test_failed_is_preserved(self):
        self.mutate("01", status="failed")
        self.assertIsNone(self.row()["rank"])
        self.assertEqual(self.row()["cells"]["01"]["status"], "failed")

    def test_partial_run_is_unranked(self):
        self.mutate("01", frames=2)
        self.assertEqual(self.row()["cells"]["01"]["status"], "incomplete")
        self.assertIsNone(self.row()["mean"])

    def test_invalid_metric_has_no_rpe_fallback(self):
        for value in (None, True, float("nan"), float("inf"), -1):
            with self.subTest(value=value):
                self.mutate("01", kitti_rte_trans_pct=value, rpe_trans_pct=0.01)
                self.assertEqual(self.row()["cells"]["01"]["status"], "metric_missing")
                self.assertIsNone(self.row()["rank"])

    def test_changed_configuration_rejected(self):
        self.mutate("01", args=["--no-gt-seed", "--tuned"])
        with self.assertRaisesRegex(ValueError, "Configuration mismatch"):
            self.row()

    def test_gt_seeded_rejected(self):
        self.mutate("01", note="GT-seeded")
        with self.assertRaisesRegex(ValueError, "Odometry evidence"):
            self.row()

    def test_zero_is_valid(self):
        self.mutate("01", kitti_rte_trans_pct=0)
        self.assertEqual(self.row()["mean"], 0.5)

    def test_audit_requires_evidence_and_rejects_gt_override(self):
        path = self.root / board.MANIFEST
        manifest = json.loads(path.read_text())
        self.mutate("01", note="First pose anchor")
        manifest["methods"][0]["odometry_audit"] = {"policy": "first_pose_anchor_only"}
        path.write_text(json.dumps(manifest))
        with self.assertRaisesRegex(ValueError, "Odometry evidence"):
            self.row()
        manifest["methods"][0]["odometry_audit"].update(
            source="evaluation/src/pcd_dogfooding.cpp", function="runSample", reason="Reviewed first-pose anchor", line=1)
        path.write_text(json.dumps(manifest))
        self.assertEqual(self.row()["mean"], 5)
        self.mutate("01", note="GT-seeded")
        with self.assertRaisesRegex(ValueError, "Odometry evidence"):
            self.row()
        for sequence in ("00", "01"):
            self.mutate(sequence, args=["--ct-icp-gt-seed"], note="First pose anchor")
        manifest["methods"][0]["args"] = ["--ct-icp-gt-seed"]
        path.write_text(json.dumps(manifest))
        with self.assertRaisesRegex(ValueError, "Odometry evidence"):
            self.row()

    def test_baselines_use_fixed_profiles_and_keep_mulls_highway_failure(self):
        data = board.collect()
        rows = {r["method"]: r for r in data["methods"]}
        for method in ("kiss_icp", "ct_icp", "mulls"):
            self.assertEqual(rows[method]["coverage"], 11)
            self.assertIsNotNone(rows[method]["rank"])
            self.assertTrue(rows[method]["odometry_audit"]["reason"])
        mulls = rows["mulls"]
        self.assertEqual(mulls["variant"], "mulls_lo_elevation")
        self.assertEqual(mulls["cells"]["01"]["value"], 5.974304)
        self.assertIn("3 registrations stopped", mulls["cells"]["01"]["recorded_note"])
        self.assertNotIn("--mulls-lo-highway", mulls["args"])
        self.assertIn("unresolved", rows["ct_icp"]["selection_note"])

    def test_repository_evidence_and_generated_outputs(self):
        data = board.collect()
        self.assertEqual(len(data["methods"]), 9)
        incomplete = next(r for r in data["methods"] if r["method"] == "l_lo")
        self.assertEqual(incomplete["coverage"], 10)
        self.assertIsNone(incomplete["rank"])
        page = (ROOT / "docs/index.html").read_text()
        self.assertIn(board.render(data), page)
        self.assertEqual(json.loads((ROOT / "docs/fixed_configuration_leaderboard.json").read_text()), data)

if __name__ == "__main__":
    unittest.main()
