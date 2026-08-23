#!/usr/bin/env python3

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).parents[1]


def load(name: str, path: pathlib.Path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


cal = load("calibration_validation", ROOT / "evaluation" / "calibration_validation.py")
demo = load("calibration_demo", ROOT / "demo" / "run_calibration_validation_demo.py")
CLI: pathlib.Path | None = None


class CalibrationValidationTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self.temp.name)
        demo.write_session(self.root / "cold.csv", "stationary", 10.0)
        demo.write_session(self.root / "hot.csv", "stationary", 45.0)

    def tearDown(self):
        self.temp.cleanup()

    def test_temperature_fit_and_apply(self):
        artifact = cal.fit_calibration([self.root / "cold.csv", self.root / "hot.csv"])
        self.assertEqual(artifact["schema"], "imu_calibration_v1")
        self.assertGreater(artifact["quality"]["temperature_span_c"], 30.0)
        self.assertAlmostEqual(artifact["gyro_temperature_coefficient"][0], 0.0008, places=5)
        corrected = cal.apply_calibration(cal.read_rows(self.root / "hot.csv"), artifact)
        self.assertLess(cal.session_metrics(corrected)["gyro_rms_rad_s"], 0.002)

    def test_record_validate_and_html(self):
        source = self.root / "cold.csv"
        metadata = cal.record_stream(str(source), self.root / "recorded.csv", self.root / "metadata.json",
                                     "test-imu", "stationary")
        self.assertEqual(metadata["metrics"]["samples"], 301)
        calibration = cal.fit_calibration([source, self.root / "hot.csv"])
        calibration_path = self.root / "calibration.json"
        calibration_path.write_text(json.dumps(calibration), encoding="utf-8")
        demo.write_session(self.root / "walking.csv", "walking", 25.0)
        demo.write_session(self.root / "vehicle.csv", "vehicle", 32.0)
        manifest = {"schema": "imu_validation_manifest_v1", "sessions": [
            {"name": "cold", "activity": "stationary", "path": "cold.csv"},
            {"name": "walking", "activity": "walking", "path": "walking.csv"},
            {"name": "vehicle", "activity": "vehicle", "path": "vehicle.csv"}]}
        manifest_path = self.root / "manifest.json"
        manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
        report = cal.validate_manifest(manifest_path, calibration_path, CLI, self.root / "runs", 50.0, 0.03)
        self.assertTrue(report["passed"])
        output = self.root / "report.html"
        cal.render_report(report, calibration, output)
        self.assertIn("IMU calibration & validation", output.read_text(encoding="utf-8"))
        ros_yaml = self.root / "calibration.yaml"
        cal.export_ros_parameters(calibration, ros_yaml, "/raw", "/calibrated", "/temperature")
        yaml_text = ros_yaml.read_text(encoding="utf-8")
        self.assertIn("gyro_temperature_coefficient:", yaml_text)
        self.assertIn('input_topic: "/raw"', yaml_text)

    def test_ros2_watchdog_detects_healthy_and_early_exit(self):
        script = ROOT / "evaluation" / "ros2_endurance.py"
        healthy = subprocess.run([sys.executable, str(script), "--duration-s", "0.2", "--output",
                                  str(self.root / "healthy.json"), "--", sys.executable, "-c",
                                  "import time; time.sleep(2)"], check=False)
        self.assertEqual(healthy.returncode, 0)
        failed = subprocess.run([sys.executable, str(script), "--duration-s", "0.2", "--output",
                                 str(self.root / "failed.json"), "--", sys.executable, "-c", "pass"], check=False)
        self.assertEqual(failed.returncode, 1)

    def test_cli_endurance_replay(self):
        if CLI is None:
            self.skipTest("built CLI not supplied")
        calibration = cal.fit_calibration([self.root / "cold.csv", self.root / "hot.csv"])
        calibration_path = self.root / "calibration.json"
        calibration_path.write_text(json.dumps(calibration), encoding="utf-8")
        result = cal.endurance_replay(self.root / "cold.csv", calibration_path, CLI, 10.0,
                                      self.root / "endurance.json")
        self.assertTrue(result["passed"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--cli", type=pathlib.Path)
    args, remaining = parser.parse_known_args()
    CLI = args.cli
    unittest.main(argv=[sys.argv[0], *remaining])
