import importlib.util
import sys
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "evaluation/scripts/select_odometry_by_loop_support.py"
SPEC = importlib.util.spec_from_file_location("loop_support_selector", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load loop-support odometry selector")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class LoopSupportOdometrySelectorTests(unittest.TestCase):
    def test_selects_graph_candidate_with_two_gt_free_clusters(self) -> None:
        manifest = {
            "ground_truth_used": False,
            "loop_clusters": 2,
            "correction_enabled": True,
        }
        self.assertEqual(
            MODULE.select_by_loop_support(manifest, 2), "graph_supported"
        )

    def test_falls_back_when_loop_support_is_insufficient(self) -> None:
        manifest = {
            "ground_truth_used": False,
            "loop_clusters": 1,
            "correction_enabled": False,
        }
        self.assertEqual(MODULE.select_by_loop_support(manifest, 2), "fallback")

    def test_rejects_manifest_without_explicit_gt_free_provenance(self) -> None:
        with self.assertRaisesRegex(ValueError, "explicitly be GT-free"):
            MODULE.select_by_loop_support(
                {"loop_clusters": 3, "correction_enabled": True}, 2
            )


if __name__ == "__main__":
    unittest.main()
