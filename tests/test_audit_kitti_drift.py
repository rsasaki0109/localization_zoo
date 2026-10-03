import json
import tempfile
import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module


class AuditKittiDriftTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module("audit_kitti_drift", "evaluation/scripts/audit_kitti_drift.py")

    def test_cells_pick_lowest_stored_rpe_per_method_and_sequence(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            def write(name, pcd, variants, method="Demo"):
                (root / f"{name}_matrix.json").write_text(json.dumps({
                    "stable_interface": {"primary_method": method},
                    "dataset": {"pcd_dir": pcd}, "variants": variants}))
            write("a", "dogfooding_results/kitti_seq_07_full", [
                {"id": "slow", "status": "ok", "rpe_trans_pct": 0.9},
                {"id": "best", "status": "ok", "rpe_trans_pct": 0.5},
                {"id": "failed", "status": "timeout_budget", "rpe_trans_pct": 0.1},
                {"id": "nan", "status": "ok", "rpe_trans_pct": float("nan")}])
            write("b", "dogfooding_results/kitti_seq_07_full", [{"id": "other", "status": "ok", "rpe_trans_pct": 0.6}])
            write("c", "dogfooding_results/kitti_seq_07_108", [{"id": "short", "status": "ok", "rpe_trans_pct": 0.01}])
            write("d", "dogfooding_results/kitti_seq_07_full", [{"id": "x", "status": "ok", "rpe_trans_pct": 0.2}],
                  method="Other")
            cells = self.module.leaderboard_cells(root, {"Demo"})
        self.assertEqual(cells, [{"method": "Demo", "sequence": "07", "repo_best_variant": "best",
                                  "repo_best_aggregate": "experiments/results/a_matrix.json"}])


if __name__ == "__main__":
    unittest.main()
