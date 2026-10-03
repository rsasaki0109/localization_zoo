import json
import tempfile
import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module


class PaperParetoTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module("export_paper_assets_pareto", "evaluation/scripts/export_paper_assets.py")

    def test_front_keeps_only_non_dominated_points(self) -> None:
        P = self.module.OdometryPoint
        points = [P("a", "fast", 0.54, 70.0, ""), P("a", "best", 0.55, 107.0, ""),
                  P("b", "slow", 0.53, 4.0, ""), P("c", "dominated", 2.0, 20.0, ""),
                  P("b", "accurate", 0.52, 3.0, "")]
        front = self.module.pareto_front(points)
        self.assertEqual([p.variant_id for p in front], ["accurate", "slow", "fast", "best"])

    def test_loader_drops_seeded_nan_and_other_datasets(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "a_matrix.json").write_text(json.dumps({
                "stable_interface": {"methods": "demo"},
                "dataset": {"pcd_dir": "dogfooding_results/kitti_seq_07_full"},
                "variants": [
                    {"id": "ok", "rpe_trans_pct": 0.6, "fps": 10.0, "note": ""},
                    {"id": "nan", "rpe_trans_pct": float("nan"), "fps": 10.0, "note": ""},
                    {"id": "seeded", "rpe_trans_pct": 0.1, "fps": 10.0, "note": "Uses GT-seeded init"},
                    {"id": "nofps", "rpe_trans_pct": 0.6, "fps": 0.0, "note": ""},
                ],
            }))
            (root / "b_matrix.json").write_text(json.dumps({
                "stable_interface": {"methods": "demo"},
                "dataset": {"pcd_dir": "dogfooding_results/kitti_seq_00_full"},
                "variants": [{"id": "other", "rpe_trans_pct": 0.6, "fps": 10.0, "note": ""}],
            }))
            points = self.module.load_pareto_points(root)
        self.assertEqual([p.variant_id for p in points], ["ok"])


if __name__ == "__main__":
    unittest.main()
