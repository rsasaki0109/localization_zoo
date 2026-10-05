import importlib.util
import json
import pathlib
import tempfile
import unittest
from unittest import mock

REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "papers/attitude_estimation/evaluation/broad_benchmark.py"


def load_module():
    spec = importlib.util.spec_from_file_location("broad_benchmark", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class BroadBenchmarkTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_module()

    def test_grid_variant_expands_to_one_run_per_value(self) -> None:
        variant = {"id": "m", "args": ["--method", "madgwick"],
                   "grid": {"flag": "--beta", "values": [0.05, 0.1]}}
        self.assertEqual(self.module.expand(variant), [
            ("m@0.05", ["--method", "madgwick", "--beta", "0.05"]),
            ("m@0.1", ["--method", "madgwick", "--beta", "0.1"]),
        ])
        self.assertEqual(self.module.expand({"id": "x", "args": ["a"]}), [("x", ["a"])])

    def test_report_selects_tagp_compares_reference_and_check_detects_drift(self) -> None:
        def errors(total, incl):
            return {"total_rmse_deg": total, "heading_rmse_deg": total, "inclination_rmse_deg": incl}

        with tempfile.TemporaryDirectory() as tmp:
            tmp = pathlib.Path(tmp)
            (tmp / "csv").mkdir()
            (tmp / "csv" / "trials.json").write_text(json.dumps({
                "groups": ["all_trials", "slow"],
                "trials": {"t1": {"groups": ["all_trials", "slow"]},
                           "t2": {"groups": ["all_trials"]}},
            }))
            variants = {"variants": [
                {"id": "grid9", "method": "madgwick", "mode": "9d", "args": ["--method", "madgwick"],
                 "grid": {"flag": "--beta", "values": [0.1, 0.2]}, "intent": "-"},
                {"id": "fixed6", "method": "mahony", "mode": "6d", "args": ["--method", "mahony"],
                 "reference": "ref6", "reference_impl": "ref6", "intent": "-"},
            ]}
            reference = {"values": {"ref6": {
                "label": "L", "source": "S", "metric": "inclination_rmse_deg", "all_trials": 2.0,
                "trials": {"t1": 1.0, "t2": 3.5}}}}
            runs = {
                "grid9@0.1": {"t1": errors(5, 1), "t2": errors(7, 1)},
                "grid9@0.2": {"t1": errors(4, 9), "t2": errors(4, 9)},  # lower total wins in 9D
                "fixed6": {"t1": errors(30, 1.0), "t2": errors(30, 3.0)},
            }
            (tmp / "variants.json").write_text(json.dumps(variants))
            (tmp / "reference.json").write_text(json.dumps(reference))
            (tmp / "runs.json").write_text(json.dumps(runs))
            out_json, out_md = tmp / "out.json", tmp / "out.md"
            with mock.patch.object(self.module, "VARIANTS", tmp / "variants.json"), \
                    mock.patch.object(self.module, "REFERENCE", tmp / "reference.json"):
                self.module.report(tmp, tmp / "runs.json", out_json, out_md)

            rows = {row["id"]: row for row in json.loads(out_json.read_text())["variants"]}
            self.assertEqual(rows["grid9"]["tagp"]["value"], 0.2)
            self.assertEqual(rows["grid9"]["groups"]["all_trials"]["total_rmse_deg"], 4.0)
            self.assertEqual(rows["grid9"]["groups"]["slow"]["total_rmse_deg"], 4.0)
            self.assertEqual(rows["fixed6"]["reference"]["ratio"], 1.0)
            self.assertEqual(rows["fixed6"]["reference"]["max_abs_trial_diff_deg"], 0.5)
            self.assertIn("TAGP of 2", out_md.read_text())
            self.assertEqual(rows["fixed6"]["reference_impl"]["max_abs_trial_diff_deg"], 0.5)
            self.assertIn("0.5000 (L)", out_md.read_text())

            expected = tmp / "expected.json"
            expected.write_text(json.dumps({"variants": {
                "grid9": {"total_rmse_deg": 4.0}, "fixed6": {"inclination_rmse_deg": 2.1}}}))
            with mock.patch.object(self.module, "EXPECTED", expected):
                self.assertEqual(self.module.check(out_json, 1e-3), 1)
                expected.write_text(json.dumps({"variants": {"grid9": {"total_rmse_deg": 4.0}}}))
                self.assertEqual(self.module.check(out_json, 1e-3), 0)


if __name__ == "__main__":
    unittest.main()
