import importlib.util
import json
import math
import pathlib
import tempfile
import unittest

REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "papers/allan_variance/evaluation/tumvi_allan.py"


def load_module():
    spec = importlib.util.spec_from_file_location("tumvi_allan", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TumviAllanTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_module()

    def test_fixed_slope_fit_recovers_ideal_lines(self) -> None:
        tau = [10 ** (i / 10) for i in range(-30, 50)]
        white = [2e-4 / math.sqrt(t) for t in tau]
        walk = [3e-6 * math.sqrt(t / 3) for t in tau]
        value, points = self.module.fixed_slope_fit(tau, white, -0.5, 0.02, 1.0, 1.0)
        self.assertAlmostEqual(value, 2e-4, places=12)
        self.assertEqual(points, 17)  # tau = 10^(i/10), i = -16..0
        value, _ = self.module.fixed_slope_fit(tau, walk, 0.5, 1000.0, 6000.0, 3.0)
        self.assertAlmostEqual(value / 3e-6, 1.0, places=9)
        with self.assertRaises(ValueError):
            self.module.fixed_slope_fit(tau, white, -0.5, 1e6, 1e7, 1.0)

    def test_report_applies_tum_protocol_and_check_detects_drift(self) -> None:
        tau = [10 ** (i / 10) for i in range(-30, 50)]

        def curve(name, wn, rw):
            adev = [math.hypot(wn / math.sqrt(t), rw * math.sqrt(t / 3)) for t in tau]
            return {"name": name, "tau": tau, "adev": adev, "white_noise_density": wn,
                    "bias_random_walk": rw, "bias_instability": 0.0, "bias_instability_tau": 1.0}

        # gx deliberately differs in random walk: the TUM gyro walk fit uses only y and z.
        columns = [curve("gx", 8e-5, 9e-6), curve("gy", 8e-5, 2.2e-6), curve("gz", 8e-5, 2.2e-6),
                   curve("ax", 1.4e-3, 8.6e-5), curve("ay", 1.4e-3, 8.6e-5), curve("az", 1.4e-3, 8.6e-5)]
        with tempfile.TemporaryDirectory() as tmp:
            tmp = pathlib.Path(tmp)
            run_json = tmp / "run.json"
            run_json.write_text(json.dumps({"samples": 720000, "sampling_rate": 200.0, "columns": columns}))
            out_json, out_md, out_yaml = tmp / "r.json", tmp / "r.md", tmp / "imu.yaml"
            self.module.report(run_json, out_json, out_md, None, out_yaml)
            p = json.loads(out_json.read_text())["tum_protocol"]
            self.assertEqual(p["gyro_bias_random_walk"]["channels"], ["gy", "gz"])
            self.assertAlmostEqual(p["gyro_bias_random_walk"]["ratio"], 1.0, delta=0.01)
            self.assertAlmostEqual(p["gyro_white_noise_density"]["ratio"], 1.0, delta=0.01)
            self.assertAlmostEqual(p["accel_bias_random_walk"]["ratio"], 1.0, delta=0.01)
            self.assertIn("gyroscope_noise_density:", out_yaml.read_text())
            self.assertIn("| gyro bias random walk", out_md.read_text())

            expected = tmp / "expected.json"
            expected.write_text(json.dumps({"tum_protocol": {
                k: v["value"] for k, v in p.items()}}))
            self.assertEqual(self.module.check(out_json, expected, 1e-6), 0)
            expected.write_text(json.dumps({"tum_protocol": {"gyro_white_noise_density": 1.0}}))
            self.assertEqual(self.module.check(out_json, expected, 1e-6), 1)


if __name__ == "__main__":
    unittest.main()
