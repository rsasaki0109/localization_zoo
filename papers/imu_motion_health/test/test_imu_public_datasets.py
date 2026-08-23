#!/usr/bin/env python3
from __future__ import annotations
import importlib.util, json, pathlib, tempfile, unittest

ROOT = pathlib.Path(__file__).parents[1]
spec = importlib.util.spec_from_file_location("public_benchmark", ROOT / "evaluation" / "public_dataset_benchmark.py")
tool = importlib.util.module_from_spec(spec); spec.loader.exec_module(tool)

class PublicDatasetTest(unittest.TestCase):
    def test_registry_pins_rights_and_hashes(self):
        data = tool.registry()
        self.assertEqual(set(data), {"cgu_bes", "uci_har", "parkinson"})
        self.assertTrue(all(item["license"] == "CC BY 4.0" for item in data.values()))
        self.assertEqual(data["cgu_bes"]["md5"], "2849b34a3a31bf299fef1f500187ac09")
        self.assertEqual(data["uci_har"]["md5"], "96924e20fe5b8c9ffc2196eec7d5048f")

    def test_fixed_public_metrics_separate_subjects_and_reject_jump(self):
        fixture = json.loads((ROOT / "test/data/public_imu/cgu_bes_metrics_fixture.json").read_text())
        rows = fixture["recordings"]
        self.assertFalse({r["subject"] for r in rows if r["split"] == "train"} &
                         {r["subject"] for r in rows if r["split"] == "test"})
        predicts = lambda r: r["max_accel_norm"] >= 45 and r["max_gyro_norm"] >= 1 and r["posture_change_deg"] >= 30
        self.assertTrue(all(predicts(r) for r in rows if r["label"] == "fall"))
        self.assertTrue(all(not predicts(r) for r in rows if r["label"] == "adl"))

    def test_cgu_conversion_units_and_manifest(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory); source = root / "raw"; source.mkdir()
            lines = ["Subject01_ForwardFall", "FALL", "ForwardFall", "AccX, AccY, AccZ, GyroX, GyroY, GyroZ"]
            lines += [f"0, 0, 1, 0, 0, 0" for _ in range(200)]
            lines += ["3, 0, 1, 2, 0, 0", "0, 1, 0, 0, 0, 0"] * 100
            (source / "Subject01_ForwardFall.txt").write_text("\n".join(lines), encoding="utf-8")
            entries = tool.convert_cgu(source, root / "out")
            self.assertEqual(entries[0]["split"], "train")
            self.assertEqual(entries[0]["label"], "fall")
            csv_text = (root / "out" / entries[0]["path"]).read_text()
            self.assertIn("9.806650000", csv_text)

    def test_tuning_uses_training_split_and_writes_policy(self):
        fixture = json.loads((ROOT / "test/data/public_imu/cgu_bes_metrics_fixture.json").read_text())
        with tempfile.TemporaryDirectory() as directory:
            result = tool.tune(fixture["recordings"], pathlib.Path(directory))
            self.assertEqual(result["training_split"], "CGU-BES Subject01-09")
            self.assertTrue((pathlib.Path(directory) / "wearable-public-v1-policy.json").exists())
            self.assertIn("impact_requires_accel_and_gyro: true", (pathlib.Path(directory) / "wearable-public-v1.yaml").read_text())

    def test_self_contained_report(self):
        report = {"passed": True, "groups": {"cgu_bes": {"recordings": 1,
                  "fall_sensitivity": 1.0, "adl_false_positive_rate": 0.0,
                  "median_latency_s": 0.4}}, "results": [], "acceptance": {}}
        with tempfile.TemporaryDirectory() as directory:
            output = pathlib.Path(directory) / "report.html"; tool.render(report, output)
            text = output.read_text(encoding="utf-8")
            self.assertIn("Public IMU benchmark", text)
            self.assertNotIn("<script src=", text)

if __name__ == "__main__": unittest.main()
