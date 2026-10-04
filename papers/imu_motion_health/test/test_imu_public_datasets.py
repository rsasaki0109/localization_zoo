#!/usr/bin/env python3
from __future__ import annotations
import importlib.util, json, pathlib, tempfile, unittest

ROOT = pathlib.Path(__file__).parents[1]
spec = importlib.util.spec_from_file_location("public_benchmark", ROOT / "evaluation" / "public_dataset_benchmark.py")
tool = importlib.util.module_from_spec(spec); spec.loader.exec_module(tool)
check_spec = importlib.util.spec_from_file_location("check_public_benchmark", ROOT / "evaluation" / "check_public_benchmark.py")
checker = importlib.util.module_from_spec(check_spec); check_spec.loader.exec_module(checker)

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
            baseline = json.loads(json.dumps(report)); baseline["passed"] = False
            baseline["groups"]["cgu_bes"]["adl_false_positive_rate"] = 0.5
            output = pathlib.Path(directory) / "report.html"; tool.render(report, output, baseline)
            text = output.read_text(encoding="utf-8")
            self.assertIn("Public IMU benchmark", text)
            self.assertIn("Baseline → tuned", text)
            self.assertNotIn("<script src=", text)

    def test_regression_check_accepts_expected_and_flags_drift(self):
        expected = json.loads(checker.EXPECTED.read_text())
        self.assertEqual(checker.compare_groups({"groups": expected["groups"], "passed": expected["passed"]}, expected), [])
        drifted = json.loads(json.dumps(expected["groups"]))
        drifted["cgu_bes"]["fall_sensitivity"] -= 0.01; drifted["uci_har"]["cli_failures"] = 1; del drifted["parkinson"]
        errors = checker.compare_groups({"groups": drifted, "passed": expected["passed"]}, expected)
        self.assertEqual(len(errors), 3)
        self.assertTrue(any("cgu_bes.fall_sensitivity" in e for e in errors))

    def test_regression_check_compares_tuned_values_not_comments(self):
        with tempfile.TemporaryDirectory() as directory:
            tuned = pathlib.Path(directory)
            frozen = checker.CONFIG_DIR / "wearable-public-v1.yaml"
            body = "\n".join(l for l in frozen.read_text().splitlines() if not l.startswith("#"))
            (tuned / "wearable-public-v1.yaml").write_text(body)
            policy = json.loads((checker.CONFIG_DIR / "wearable-public-v1-policy.json").read_text()); policy.pop("description")
            (tuned / "wearable-public-v1-policy.json").write_text(json.dumps(policy))
            self.assertEqual(checker.compare_tuning(tuned, checker.CONFIG_DIR), [])
            (tuned / "wearable-public-v1.yaml").write_text(body.replace("45.0", "40.0"))
            self.assertEqual(len(checker.compare_tuning(tuned, checker.CONFIG_DIR)), 1)

    def test_expected_metrics_match_documented_result(self):
        cgu = json.loads(checker.EXPECTED.read_text())["groups"]["cgu_bes"]
        self.assertEqual(f"{cgu['fall_sensitivity']:.1%}", "96.7%")
        self.assertEqual(f"{cgu['median_latency_s']:.2f}", "2.53")
        self.assertEqual(f"{cgu['adl_false_positive_rate']:.1%}", "0.7%")
        doc = (ROOT / "evaluation" / "public_datasets.md").read_text()
        self.assertIn("96.7% CGU", doc)
        self.assertIn("2% false-positive acceptance gate fails", doc)

    def test_posture_confirmation_is_causal(self):
        policy = {"minimum_posture_change_deg": 30.0, "pre_window_s": [1.5, 0.5], "post_window_s": [0.25, 0.75]}
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "r.csv"
            rows = ["timestamp,gx,gy,gz,ax,ay,az"]
            for i in range(600):  # 3 s at 200 Hz: upright, impact at 1.6 s, lying from 1.7 s
                t = i / 200.0
                accel = (0.0, 0.0, 9.8) if t < 1.7 else (9.8, 0.0, 0.0)
                rows.append(f"{t:.3f},0,0,0,{accel[0]},{accel[1]},{accel[2]}")
            path.write_text("\n".join(rows))
            self.assertAlmostEqual(tool.causal_confirmation(path, [1.6], policy), 2.35)
            # An impact too close to the end cannot be confirmed yet.
            self.assertIsNone(tool.causal_confirmation(path, [2.5], policy))
            # Same posture before and after: not a fall candidate.
            self.assertIsNone(tool.causal_confirmation(path, [0.9], {**policy, "post_window_s": [0.0, 0.5]}))

if __name__ == "__main__": unittest.main()
