import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/reproduce_lidar_v11.py"
SPEC = importlib.util.spec_from_file_location("reproduce_lidar_v11", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class ReproduceLidarV11Tests(unittest.TestCase):
    def test_failed_runtime_gate_requires_non_promotion(self) -> None:
        candidate = {"promoted": False}
        external = {
            "result": {
                "accuracy": {"tracking_gate": "pass_600_of_600", "accuracy_gate": "pass"},
                "runtime_before_gt_download": {"v11_runtime_gate": "fail_below_10_fps"},
                "promotion": "rejected_runtime_gate_failed",
                "post_result_tuning": False,
            }
        }
        policy = MODULE.promotion_policy(candidate, external)
        self.assertFalse(policy["runtime_gate_passed"])
        self.assertTrue(policy["not_promoted"])
        self.assertTrue(policy["rejection_recorded"])


if __name__ == "__main__":
    unittest.main()
