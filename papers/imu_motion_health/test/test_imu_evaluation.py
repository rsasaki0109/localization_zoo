#!/usr/bin/env python3
"""Unit and smoke tests for the standard-library IMU evaluation tools.

Run directly with ``python test_imu_evaluation.py`` or collect with pytest.
The CLI replay test is skipped when no built ``imu_motion_health_cli`` is
available; generator, evaluator, and dashboard tests remain dependency-free.
"""

from __future__ import annotations

import json
import os
import pathlib
import subprocess
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[3]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from papers.imu_motion_health.evaluation import (  # noqa: E402
    evaluate_fault_matrix,
    generate_fault_matrix,
    render_dashboard,
)


class EvaluationToolsTest(unittest.TestCase):
    def test_generator_is_deterministic_and_covers_fault_matrix(self) -> None:
        with tempfile.TemporaryDirectory(prefix="imu_fault_tools_") as raw:
            root = pathlib.Path(raw)
            first, second = root / "first", root / "second"
            generate_fault_matrix.generate(first, list(generate_fault_matrix.SCENARIOS),
                                           seed=17)
            generate_fault_matrix.generate(second, list(generate_fault_matrix.SCENARIOS),
                                           seed=17)
            self.assertEqual(set(generate_fault_matrix.SCENARIOS), {
                path.stem for path in first.glob("*.csv")
            })
            for name in generate_fault_matrix.SCENARIOS:
                self.assertEqual((first / f"{name}.csv").read_bytes(),
                                 (second / f"{name}.csv").read_bytes())
                self.assertEqual((first / f"{name}.truth.json").read_bytes(),
                                 (second / f"{name}.truth.json").read_bytes())
            manifest = json.loads((first / "manifest.json").read_text(encoding="utf-8"))
            self.assertEqual(manifest["schema"],
                             "imu_motion_health_fault_matrix_v1")
            self.assertEqual(len(manifest["scenarios"]), len(generate_fault_matrix.SCENARIOS))

    def test_evaluator_scores_snapshot_flags_and_states(self) -> None:
        truth = {
            "schema": "imu_motion_health_fault_truth_v1",
            "scenario": "synthetic",
            "observations": [
                {"kind": "flag", "field": "gap", "minimum": 1},
                {"kind": "counter", "field": "timestamp_gaps", "minimum": 1},
                {"kind": "state", "value": "impact", "minimum_samples": 1},
            ],
        }
        snapshots = [
            {"timestamp": 0.0, "motion_state": "stationary", "gap": False},
            {"timestamp": 1.0, "motion_state": "impact", "gap": True},
        ]
        summary = {"schema": "imu_motion_health_summary_v1",
                   "counters": {"timestamp_gaps": 1},
                   "final_state": {"relative_position": [1, 2, 3]}}
        report = evaluate_fault_matrix.evaluate_scenario(truth, snapshots, summary, [])
        self.assertTrue(report["passed"])
        self.assertEqual(report["relative_motion"]["relative_position"], [1, 2, 3])
        self.assertEqual(report["state_intervals"][-1]["state"], "impact")

    def test_dashboard_is_self_contained_and_replayable(self) -> None:
        with tempfile.TemporaryDirectory(prefix="imu_dashboard_") as raw:
            directory = pathlib.Path(raw)
            jsonl = directory / "trace.jsonl"
            summary = directory / "summary.json"
            truth = directory / "truth.json"
            output = directory / "dashboard.html"
            snapshots = [
                {"timestamp": 0.0, "motion_state": "stationary",
                 "health_state": "ready", "confidence": 0.9,
                 "relative_position": [0, 0, 0], "relative_velocity": [0, 0, 0]},
                {"timestamp": 0.1, "motion_state": "impact",
                 "health_state": "degraded", "confidence": 0.4,
                 "accel_norm": 40, "relative_position": [0.1, 0, 0],
                 "relative_velocity": [1, 0, 0], "impact": True},
            ]
            jsonl.write_text("\n".join(json.dumps(item) for item in snapshots) + "\n",
                             encoding="utf-8")
            summary.write_text(json.dumps({"schema": "imu_motion_health_summary_v1",
                                           "rows_read": 2, "duration_s": 0.1,
                                           "motion_state": "impact", "counters": {}}),
                               encoding="utf-8")
            truth.write_text(json.dumps({"scenario": "synthetic",
                                         "fault_intervals": [{"label": "impact",
                                                               "start_s": 0.1,
                                                               "end_s": 0.1}]}),
                             encoding="utf-8")
            render_dashboard.render(jsonl, output, summary_path=summary,
                                    truth_path=truth)
            text = output.read_text(encoding="utf-8")
            self.assertIn("imu_motion_health_dashboard_v1", text)
            self.assertIn("Detected events", text)
            self.assertIn("<svg", text)
            self.assertIn("Play", text)
            self.assertNotIn("https://", text)

    def test_cli_replay_matrix_when_available(self) -> None:
        cli_value = os.environ.get("IMU_MOTION_HEALTH_CLI")
        candidates = [pathlib.Path(cli_value)] if cli_value else []
        candidates.extend([
            ROOT / "build_imu_motion_health_verify" / "Release" / "imu_motion_health_cli.exe",
            ROOT / "build_imu_motion_health_standalone_vcpkg" / "Release" / "imu_motion_health_cli.exe",
            ROOT / "build" / "imu_motion_health" / "imu_motion_health_cli",
        ])
        cli = next((item for item in candidates if item.is_file()), None)
        if cli is None:
            self.skipTest("imu_motion_health_cli build not available")
        with tempfile.TemporaryDirectory(prefix="imu_fault_cli_") as raw:
            directory = pathlib.Path(raw)
            generate_fault_matrix.generate(directory, list(generate_fault_matrix.SCENARIOS))
            report_path = directory / "report.json"
            report = evaluate_fault_matrix.evaluate_manifest(
                directory / "manifest.json", cli=cli,
                output_dir=directory / "runs")
            report_path.write_text(json.dumps(report), encoding="utf-8")
            self.assertTrue(report["aggregate"]["pass"], report)
            impact = next(item for item in report["scenarios"]
                          if item["scenario"] == "impact")
            # Newer CLIs expose lifecycle events; the evaluator also supports
            # older binaries, so only assert the stronger contract when the
            # optional event artifact was actually produced.
            if impact["events"]:
                self.assertIn("impact", {event.get("type")
                                          for event in impact["events"]})
                self.assertIn("started", {event.get("phase")
                                           for event in impact["events"]})
                self.assertIn("ended", {event.get("phase")
                                         for event in impact["events"]})


if __name__ == "__main__":
    unittest.main()
