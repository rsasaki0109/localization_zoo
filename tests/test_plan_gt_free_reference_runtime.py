import importlib.util
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "plan_gt_free_reference_runtime",
    SCRIPT_DIR / "plan_gt_free_reference_runtime.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load GT-free reference runtime planner")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class GtFreeReferenceRuntimePlannerTests(unittest.TestCase):
    def test_runs_reference_at_frozen_point_limit(self) -> None:
        self.assertTrue(MODULE.reference_is_runtime_feasible(150_000, 150_000))

    def test_skips_reference_above_frozen_point_limit(self) -> None:
        self.assertFalse(MODULE.reference_is_runtime_feasible(150_001, 150_000))

    def test_rejects_invalid_limits(self) -> None:
        with self.assertRaisesRegex(ValueError, "positive"):
            MODULE.reference_is_runtime_feasible(1, 0)

    def test_rejects_invalid_point_counts(self) -> None:
        with self.assertRaisesRegex(ValueError, "non-negative"):
            MODULE.reference_is_runtime_feasible(-1, 150_000)


if __name__ == "__main__":
    unittest.main()
