import json
import os
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from unittest.mock import patch

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

    def test_invalid_reference_metrics_cannot_match(self) -> None:
        valid = {"status": "ok", "rpe_trans_pct": 0.5, "ate_m": 1.0, "frames": 4}
        for key in ("rpe_trans_pct", "ate_m", "frames"):
            for value in (None, True, -1, float("nan"), float("inf")):
                with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                    self.module.compare_result({**valid, key: value}, valid)

    def test_incomplete_method_sequence_selection_is_rejected(self) -> None:
        rows = [{"method": "A", "sequence": "00"}, {"method": "A", "sequence": "07"},
                {"method": "B", "sequence": "07"}]
        with self.assertRaisesRegex(ValueError, "pairs"):
            self.module.select_cells(rows, {"A", "B"}, {"00", "07"})

    def test_leaderboard_does_not_verify_ate_or_frame_drift_with_identical_rpe(self) -> None:
        leaderboard = load_script_module("generate_leaderboard", "evaluation/scripts/generate_leaderboard.py")
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            rows = [
                {"aggregate_path": f"{name}_matrix.json", "variant_id": "v", "rerun_rpe_abs_delta": 0,
                 **fields}
                for name, fields in [("legacy", {}), ("same", {"audit_status": "ok", "drifted": False}),
                                     ("ate", {"audit_status": "drift", "drifted": True}),
                                     ("error", {"audit_status": "error"}),
                                     ("extra", {"rescore_extra_args": ["--different"]})]
            ]
            (root / "kitti_drift_audit.json").write_text(json.dumps({"rows": rows}))
            with patch.object(leaderboard, "RESULTS_DIR", root):
                self.assertEqual(leaderboard.verified_current(), {("legacy_matrix.json", "v"),
                                                                  ("same_matrix.json", "v")})


@unittest.skipIf(os.name == "nt", "fake replay binary uses a shebang")
class AuditKittiDriftCliTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        scripts = self.root / "evaluation/scripts"
        scripts.mkdir(parents=True)
        source = Path(__file__).resolve().parents[1] / "evaluation/scripts"
        for name in ("audit_kitti_drift.py", "rescore_kitti_rte.py", "capture_benchmark_environment.py"):
            shutil.copyfile(source / name, scripts / name)
        self.script = scripts / "audit_kitti_drift.py"
        self.results = self.root / "experiments/results"
        self.results.mkdir(parents=True)
        self.binary = self.root / "replay"
        self.binary.write_text(f"#!{sys.executable}\n" + textwrap.dedent('''\
            import json, pathlib, sys
            argv = sys.argv
            scenario = argv[argv.index('--scenario') + 1]
            output = pathlib.Path(argv[argv.index('--summary-json') + 1])
            name = 'Demo' if argv[argv.index('--methods') + 1] == 'demo' else 'Other'
            result = {'name': name, 'status': 'ok', 'frames': 4, 'rpe_trans_pct': 0.5,
                      'ate_m': 1.0, 'kitti_rte_trans_pct': 0.7,
                      'kitti_rte_rot_deg_per_100m': 0.2, 'fps': 10.0}
            if scenario == 'rpe': result['rpe_trans_pct'] = 0.6
            if scenario == 'ate': result['ate_m'] = 1.1
            if scenario == 'frames': result['frames'] = 3
            if scenario == 'nan': result['rpe_trans_pct'] = float('nan')
            if scenario == 'nan_official': result['kitti_rte_trans_pct'] = float('nan')
            if scenario == 'null': result['ate_m'] = None
            if scenario == 'status': result['status'] = 'failed'
            print('replay stdout', flush=True)
            print('replay stderr', file=sys.stderr, flush=True)
            if scenario == 'missing': sys.exit(0)
            if scenario == 'malformed':
                output.write_text('{')
            else:
                output.write_text(json.dumps({'methods': [result], 'argv': argv}))
            if scenario == 'exit': sys.exit(7)
        '''))
        self.binary.chmod(0o755)
        (self.root / "gt.csv").write_text("fixture ground truth\n")
        self.write_cell("Demo", "00")
        self.write_cell("Demo", "07")
        self.write_cell("Other", "07")

    def write_cell(self, method: str, sequence: str, scenario: str = "same") -> None:
        dataset = f"dogfooding_results/kitti_seq_{sequence}_full"
        (self.root / dataset).mkdir(parents=True, exist_ok=True)
        aggregate = {
            "stable_interface": {"methods": method.lower(), "primary_method": method},
            "dataset": {"pcd_dir": dataset, "gt_csv": "gt.csv", "extra_args": ["--no-gt-seed"]},
            "variants": [{"id": "v", "status": "ok", "args": ["--scenario", scenario],
                          "frames": 4, "rpe_trans_pct": 0.5, "ate_m": 1.0}],
        }
        (self.results / f"{method}_{sequence}_matrix.json").write_text(json.dumps(aggregate))

    def run_cli(self, *arguments: str, name: str = "run") -> subprocess.CompletedProcess:
        return subprocess.run([sys.executable, str(self.script), "--binary", str(self.binary),
                               "--work-dir", str(self.root / name),
                               "--output", str(self.root / f"{name}.json"), *arguments],
                              cwd=self.root, capture_output=True, text=True, timeout=20)

    def report(self, name: str = "run") -> dict:
        return json.loads((self.root / f"{name}.json").read_text())

    def test_sequence_filter_preserves_command_log_and_other_audit(self) -> None:
        historical = self.results / "kitti_drift_audit.json"
        historical.write_text("historical\n")
        process = self.run_cli("--sequence", "00")
        self.assertEqual(process.returncode, 0, process.stderr + process.stdout)
        report = self.report()
        self.assertEqual(report["exit_code"], 0)
        self.assertEqual(report["scope"]["selected_cell_count"], 1)
        self.assertEqual(report["scope"]["sequences"], ["00"])
        row = report["rows"][0]
        summary = json.loads(Path(row["summary_path"]).read_text())
        self.assertEqual(row["command"], summary["argv"])
        self.assertEqual(row["command"][-3:], ["--no-gt-seed", "--scenario", "same"])
        self.assertEqual(row["process_exit_code"], 0)
        log = Path(row["log_path"]).read_text()
        self.assertIn("replay stdout", log)
        self.assertIn("replay stderr", log)
        self.assertEqual(historical.read_text(), "historical\n")

    def test_each_drift_metric_causes_exit_one(self) -> None:
        for scenario, metric in (("rpe", "rpe_trans_pct"), ("ate", "ate_m"), ("frames", "frames")):
            with self.subTest(scenario=scenario):
                self.write_cell("Demo", "00", scenario)
                process = self.run_cli("--sequence", "00", name=scenario)
                self.assertEqual(process.returncode, 1, process.stderr + process.stdout)
                row = self.report(scenario)["rows"][0]
                self.assertEqual(row["drift_metrics"], [metric])
                self.assertEqual(row["audit_status"], "drift")

    def test_failed_or_invalid_replay_does_not_discard_other_cells(self) -> None:
        for scenario in ("exit", "missing", "malformed", "nan", "nan_official", "null", "status"):
            with self.subTest(scenario=scenario):
                self.write_cell("Demo", "07", scenario)
                process = self.run_cli("--sequence", "07", "--jobs", "2", name=scenario)
                self.assertEqual(process.returncode, 2, process.stderr + process.stdout)
                report = self.report(scenario)
                self.assertEqual(report["exit_code"], 2)
                self.assertEqual([r["audit_status"] for r in report["rows"]], ["error", "ok"])
                failed = report["rows"][0]
                self.assertEqual(failed["process_exit_code"], 7 if scenario == "exit" else 0)
                self.assertIn("error", failed)

    def test_missing_inputs_and_missing_binary_are_recorded(self) -> None:
        (self.root / "dogfooding_results/kitti_seq_00_full").rmdir()
        process = self.run_cli("--sequence", "00", name="data")
        self.assertEqual(process.returncode, 2, process.stderr + process.stdout)
        self.assertIn("missing scan directory", self.report("data")["rows"][0]["error"])
        self.binary.unlink()
        process = self.run_cli("--sequence", "07", name="binary")
        self.assertEqual(process.returncode, 2, process.stderr + process.stdout)
        self.assertIsNone(self.report("binary")["binary_sha256"])
        self.assertTrue(all(r["audit_status"] == "error" for r in self.report("binary")["rows"]))

    def test_empty_or_unknown_selection_fails_without_replay(self) -> None:
        for arguments in (("--sequence", "01"), ("--method", "unknown"),
                          ("--sequence", "00", "--method", "Other"), ("--jobs", "0")):
            with self.subTest(arguments=arguments):
                process = self.run_cli(*arguments)
                self.assertEqual(process.returncode, 2, process.stderr + process.stdout)
                self.assertFalse((self.root / "run").exists())
                self.assertFalse((self.root / "run.json").exists())

    def test_filtered_run_requires_explicit_output_and_fresh_work_directory(self) -> None:
        process = subprocess.run([sys.executable, str(self.script), "--sequence", "07",
                                  "--work-dir", str(self.root / "new")],
                                 capture_output=True, text=True, timeout=20)
        self.assertEqual(process.returncode, 2)
        self.assertIn("require --output", process.stderr)
        old = self.root / "run"
        old.mkdir()
        (old / "previous.json").write_text("keep\n")
        process = self.run_cli()
        self.assertEqual(process.returncode, 2)
        self.assertEqual((old / "previous.json").read_text(), "keep\n")

    def test_list_can_repeat_sequences_without_creating_outputs(self) -> None:
        process = subprocess.run([sys.executable, str(self.script), "--list", "--method", "Demo",
                                  "--sequence", "07", "--sequence", "00", "--sequence", "07"],
                                 capture_output=True, text=True, timeout=20)
        self.assertEqual(process.returncode, 0, process.stderr)
        report = json.loads(process.stdout)
        self.assertEqual([r["sequence"] for r in report["rows"]], ["00", "07"])
        self.assertFalse((self.results / "kitti_drift_audit.json").exists())


if __name__ == "__main__":
    unittest.main()
