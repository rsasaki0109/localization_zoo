import json
import tempfile
import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module


class CheckFixtureGoldenTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module("check_fixture_golden", "evaluation/scripts/check_fixture_golden.py")

    def run_check(self, summaries: dict, golden: dict | None) -> tuple[int, dict]:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "s").mkdir()
            for name, methods in summaries.items():
                (root / "s" / f"{name}.json").write_text(json.dumps({"methods": methods}))
            if golden is not None:
                (root / "golden.json").write_text(json.dumps(golden))
            code = self.module.main(["--summaries", str(root / "s"), "--golden", str(root / "golden.json"),
                                     "--actual-out", str(root / "out" / "actual.json")])
            return code, json.loads((root / "out" / "actual.json").read_text())

    def method(self, ate: float) -> list[dict]:
        return [{"name": "KISS-ICP", "ate_m": ate, "rpe_trans_pct": 167.0, "frames": 3, "status": "ok"}]

    def test_matching_and_tolerated_noise_pass(self) -> None:
        code, actual = self.run_check({"kiss_icp": self.method(0.035614)}, None)
        self.assertEqual(code, 1)  # no golden yet, but actual values are written
        self.assertIn("kiss_icp/KISS-ICP", actual)
        code, _ = self.run_check({"kiss_icp": self.method(0.0356141)}, actual)
        self.assertEqual(code, 0)

    def test_real_change_missing_and_new_methods_fail(self) -> None:
        _, golden = self.run_check({"kiss_icp": self.method(0.035614)}, None)
        # The 2026-08-02 KISS-ICP search change moved this fixture by ~1.7 %.
        self.assertEqual(self.run_check({"kiss_icp": self.method(0.036234)}, golden)[0], 1)
        self.assertEqual(self.run_check({}, golden)[0], 1)
        both = {"kiss_icp": self.method(0.035614), "other": [{"name": "X", "ate_m": 1.0, "rpe_trans_pct": 1.0,
                                                               "frames": 3, "status": "ok"}]}
        self.assertEqual(self.run_check(both, golden)[0], 1)

    def test_status_and_nan_handling(self) -> None:
        _, golden = self.run_check({"m": [{"name": "M", "ate_m": float("nan"), "rpe_trans_pct": None,
                                           "frames": 3, "status": "invalid_metric"}]}, None)
        same = {"m": [{"name": "M", "ate_m": float("nan"), "rpe_trans_pct": None, "frames": 3,
                       "status": "invalid_metric"}]}
        self.assertEqual(self.run_check(same, golden)[0], 0)
        changed = {"m": [{"name": "M", "ate_m": 0.1, "rpe_trans_pct": None, "frames": 3, "status": "ok"}]}
        self.assertEqual(self.run_check(changed, golden)[0], 1)


if __name__ == "__main__":
    unittest.main()
