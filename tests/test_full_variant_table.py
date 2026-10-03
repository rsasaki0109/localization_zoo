import csv
import json
import tempfile
import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module


class FullVariantTableTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module(
            "generate_full_variant_table", "evaluation/scripts/generate_full_variant_table.py"
        )

    def write_fixture(self, root: Path) -> Path:
        aggregate = {
            "problem": {"id": "demo_reference", "title": "Demo"},
            "stable_interface": {"methods": "demo"},
            "dataset": {"pcd_dir": "dogfooding_results/demo_120", "gt_csv": "gt.csv"},
            "variants": [
                {"id": "fast", "label": "Fast", "status": "ok", "ate_m": 0.5, "fps": 20.0,
                 "frames": 120, "decision": "Adopt as current default"},
                {"id": "dense_x", "label": "Dense", "status": "TIMED_OUT", "ate_m": None, "fps": None,
                 "frames": 0, "decision": "Rejected for this run", "rpe_trans_pct": None},
            ],
        }
        (root / "agg.json").write_text(json.dumps(aggregate))
        index = {"problems": [{"problem_id": "demo_reference", "status": "ready",
                               "aggregate_path": "agg.json", "current_default": "fast"}]}
        index_path = root / "index.json"
        index_path.write_text(json.dumps(index))
        return index_path

    def test_collects_every_variant_including_failed_runs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            rows = self.module.collect_rows(self.write_fixture(root), root)
            self.module.write_csv(rows, root / "out.csv")
            parsed = list(csv.DictReader((root / "out.csv").open()))
            tex = self.module.render_tex(rows)

        self.assertEqual([row["variant_id"] for row in parsed], ["dense_x", "fast"])
        fast = parsed[1]
        self.assertEqual(fast["run_status"], "OK")
        self.assertEqual(fast["is_current_default"], "true")
        self.assertEqual(fast["contract_type"], "reference-based")
        self.assertEqual(fast["dataset"], "demo_120")
        self.assertEqual(parsed[0]["ate_m"], "")
        self.assertIn(r"dense\_x & -- & -- & -- & rejected (TIMED\_OUT) \\", tex)
        self.assertIn(r"fast & 0.500 & -- & 20.0 & default \\", tex)

    def test_committed_csv_covers_every_ready_default(self) -> None:
        repo = Path(__file__).resolve().parents[1]
        index = json.loads((repo / "experiments/results/index.json").read_text())
        rows = list(csv.DictReader((repo / "docs/assets/paper/full_variant_results.csv").open()))
        ready = sum(1 for problem in index["problems"] if problem["status"] == "ready")
        self.assertEqual(sum(row["is_current_default"] == "true" for row in rows), ready)


if __name__ == "__main__":
    unittest.main()
