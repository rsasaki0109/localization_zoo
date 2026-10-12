import csv
import io
import json
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

from test_experiment_scripts import load_script_module

COLUMNS = ["method", "sequence", "paper_rte_pct", "paper_source", "repo_metric", "repo_best_rpe_pct",
           "repo_best_ratio", "repo_pool_median_rpe_pct", "repo_pool_size", "selected_from_n",
           "repo_best_variant", "repo_best_aggregate"]


class ReproducePaperRowTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module("reproduce_paper_row", "evaluation/scripts/reproduce_paper_row.py")

    def write_fixture(self, root: Path) -> Path:
        aggregate = root / "x_kitti_seq_07_full_matrix.json"
        aggregate.write_text(json.dumps({
            "stable_interface": {"methods": "litamin2"},
            "variants": [{"id": "paper_icp_cov", "args": ["--no-gt-seed", "--litamin2-paper-odometry"]},
                         {"id": "other", "args": ["--no-gt-seed"]}],
        }))
        table = root / "table.csv"
        with table.open("w", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=COLUMNS)
            writer.writeheader()
            writer.writerow({"method": "litamin2", "sequence": "07", "paper_rte_pct": "0.480",
                             "paper_source": "Table III", "repo_metric": "kitti_rte", "repo_best_rpe_pct": "0.477",
                             "repo_best_variant": "paper_icp_cov", "repo_best_aggregate": str(aggregate)})
            writer.writerow({"method": "litamin2", "sequence": "01", "paper_rte_pct": "2.100",
                             "paper_source": "Table III"})
        return table

    def test_list_shows_only_rows_with_a_repo_run(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            table = self.write_fixture(Path(tmp))
            out = io.StringIO()
            with redirect_stdout(out):
                self.module.main(["--list", "--table", str(table)])
            lines = out.getvalue().splitlines()
            self.assertEqual(len(lines), 2)
            self.assertIn("paper_icp_cov", lines[1])

    def test_dry_run_builds_the_recorded_command(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            table = self.write_fixture(root)
            (root / "kitti" / "dataset" / "sequences" / "07" / "velodyne").mkdir(parents=True)
            out = io.StringIO()
            with redirect_stdout(out):
                self.module.main(["--method", "litamin2", "--sequence", "07", "--table", str(table),
                                  "--kitti-root", str(root / "kitti"), "--binary", "zoo", "--dry-run"])
            command = out.getvalue().strip().splitlines()[0]
            self.assertTrue(command.startswith("$ zoo "))
            self.assertIn("--methods litamin2 --no-gt-seed --litamin2-paper-odometry", command)
            self.assertIn("kitti_seq_07_full_gt.csv", command)

    def test_missing_scans_without_fetch_stops(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            table = self.write_fixture(root)
            with self.assertRaises(SystemExit):
                self.module.main(["--method", "litamin2", "--sequence", "07", "--table", str(table),
                                  "--kitti-root", str(root / "none")])

    def test_unknown_row_stops(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            table = self.write_fixture(Path(tmp))
            with self.assertRaises(SystemExit):
                self.module.main(["--method", "litamin2", "--sequence", "01", "--table", str(table)])


if __name__ == "__main__":
    unittest.main()
