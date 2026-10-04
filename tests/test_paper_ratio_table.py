import json
import tempfile
import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module

REPO_ROOT = Path(__file__).resolve().parents[1]


class PaperRatioTableTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module(
            "generate_paper_ratio_table", "evaluation/scripts/generate_paper_ratio_table.py"
        )

    def test_best_excludes_gt_seeded_and_non_full_sequences(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            paper = root / "paper.json"
            paper.write_text(json.dumps({"methods": {"demo": {
                "reported_values": {"kitti_07": 0.5, "kitti_avg_00_10": 0.4},
                "reported_source": {"table": "Table I", "row": "demo"},
            }}}))
            results = root / "results"
            results.mkdir()

            def aggregate(name: str, pcd_dir: str, variants: list[dict]) -> None:
                (results / f"{name}_matrix.json").write_text(json.dumps({
                    "stable_interface": {"methods": "demo"},
                    "dataset": {"pcd_dir": pcd_dir},
                    "variants": variants,
                }))

            aggregate("full", "dogfooding_results/kitti_seq_07_full", [
                {"id": "pure", "rpe_trans_pct": 1.0, "note": "no GT seed"},
                {"id": "worse", "rpe_trans_pct": 2.0, "note": ""},
                {"id": "seeded", "rpe_trans_pct": 0.1, "note": "Uses GT-seeded scan-to-map initialization"},
                {"id": "missing", "rpe_trans_pct": None, "note": ""},
            ])
            aggregate("window", "dogfooding_results/kitti_seq_07_108", [
                {"id": "short", "rpe_trans_pct": 0.05, "note": ""},
            ])
            rows = self.module.collect_rows(paper, results)

        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual(row["repo_best_variant"], "pure")
        self.assertEqual(row["repo_best_ratio"], "2.00")
        self.assertEqual(row["repo_pool_size"], "2")
        self.assertEqual(row["repo_pool_median_rpe_pct"], "1.500")
        self.assertEqual(row["paper_source"], "Table I demo")
        self.assertEqual(row["repo_metric"], "rpe_100m")

    def test_kitti_rte_replaces_100m_rpe_when_recorded(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            paper = root / "paper.json"
            paper.write_text(json.dumps({"methods": {"demo": {"reported_values": {"kitti_00": 0.5}}}}))
            results = root / "results"
            results.mkdir()
            (results / "a_matrix.json").write_text(json.dumps({
                "stable_interface": {"methods": "demo"},
                "dataset": {"pcd_dir": "kitti_seq_00_full"},
                "variants": [
                    {"id": "old", "rpe_trans_pct": 0.1, "note": ""},
                    {"id": "new", "rpe_trans_pct": 2.0, "kitti_rte_trans_pct": 1.0, "note": ""},
                ],
            }))
            row = self.module.collect_rows(paper, results)[0]
        self.assertEqual(row["repo_metric"], "kitti_rte")
        self.assertEqual(row["repo_best_variant"], "new")
        self.assertEqual(row["repo_best_rpe_pct"], "1.000")
        self.assertEqual(row["repo_pool_size"], "1")

    def test_repo_variants_restrict_the_pool(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            paper = root / "paper.json"
            paper.write_text(json.dumps({"methods": {"demo": {
                "reported_values": {"kitti_00": 0.5}, "repo_variants": ["paper_default"]}}}))
            results = root / "results"
            results.mkdir()
            (results / "a_matrix.json").write_text(json.dumps({
                "stable_interface": {"methods": "demo"},
                "dataset": {"pcd_dir": "kitti_seq_00_full"},
                "variants": [
                    {"id": "paper_default", "rpe_trans_pct": 1.0, "kitti_rte_trans_pct": 0.9, "note": ""},
                    {"id": "ablation", "rpe_trans_pct": 0.5, "kitti_rte_trans_pct": 0.4, "note": ""},
                ],
            }))
            row = self.module.collect_rows(paper, results)[0]
        self.assertEqual(row["repo_best_variant"], "paper_default")
        self.assertEqual(row["selected_from_n"], "1")

    def test_every_kitti_paper_value_cites_its_source(self) -> None:
        methods = json.loads(
            (REPO_ROOT / "evaluation/data/paper_reported_numbers.json").read_text()
        )["methods"]
        for name, info in methods.items():
            if any(key.startswith("kitti_") for key in info.get("reported_values", {})):
                source = info.get("reported_source", {})
                self.assertTrue(source.get("arxiv") and source.get("table") and source.get("row"), name)

    def test_verified_paper_values_are_pinned(self) -> None:
        methods = json.loads(
            (REPO_ROOT / "evaluation/data/paper_reported_numbers.json").read_text()
        )["methods"]
        pick = lambda name: [methods[name]["reported_values"][f"kitti_{s}"] for s in ("00", "02", "05", "07", "08")]
        self.assertEqual(pick("litamin2"), [0.78, 0.95, 0.55, 0.48, 1.01])
        self.assertEqual(pick("ct_icp"), [0.49, 0.52, 0.25, 0.31, 0.81])
        self.assertEqual(methods["kiss_icp"]["reported_values"]["kitti_00"], 0.51)
        self.assertEqual(pick("mulls"), [0.51, 0.55, 0.28, 0.29, 0.80])
        self.assertEqual(pick("suma"), [0.7, 1.1, 0.5, 0.4, 1.0])
        self.assertEqual(pick("aloam"), [0.78, 0.92, 0.57, 0.63, 1.12])
        self.assertIn("secondary", methods["aloam"]["reported_source"]["arxiv"])


if __name__ == "__main__":
    unittest.main()
