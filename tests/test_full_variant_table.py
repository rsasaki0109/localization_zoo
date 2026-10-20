import csv
import json
import tempfile
import unittest
from collections import Counter
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
            with (root / "out.csv").open() as stream:
                parsed = list(csv.DictReader(stream))
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
        with (repo / "docs/assets/paper/full_variant_results.csv").open() as stream:
            rows = list(csv.DictReader(stream))
        # A completed experiment can deliberately adopt no variant. Check the
        # actual declared defaults, including their identities and multiplicity.
        expected = Counter(
            (problem["problem_id"], problem["current_default"])
            for problem in index["problems"]
            if problem["status"] == "ready" and problem.get("current_default") is not None
        )
        actual = Counter(
            (row["problem_id"], row["variant_id"])
            for row in rows if row["is_current_default"] == "true"
        )
        self.assertEqual(actual, expected)

    def test_ready_experiment_without_adoption_keeps_every_variant(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            index_path = self.write_fixture(root)
            index = json.loads(index_path.read_text())
            index["problems"][0]["current_default"] = None
            index_path.write_text(json.dumps(index))
            aggregate_path = root / "agg.json"
            aggregate = json.loads(aggregate_path.read_text())
            for variant in aggregate["variants"]:
                variant.update(status="ok", ate_m=0.5, fps=None, frames=120,
                               decision="Experimental; default off")
            aggregate_path.write_text(json.dumps(aggregate))
            rows = self.module.collect_rows(index_path, root)

        self.assertEqual([row["variant_id"] for row in rows], ["dense_x", "fast"])
        self.assertTrue(all(row["is_current_default"] == "false" for row in rows))
        self.assertTrue(all(row["fps"] == "" for row in rows))


if __name__ == "__main__":
    unittest.main()
