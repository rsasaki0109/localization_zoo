import argparse
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = REPO_ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "mola_baseline_runner",
    SCRIPT_DIR / "run_mola_baseline.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load MOLA runner")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class MolaBaselineRunnerTests(unittest.TestCase):
    def test_child_command_never_contains_reference_path(self) -> None:
        args = argparse.Namespace(
            mola_executable="/opt/ros/humble/bin/mola-lidar-odometry-cli",
            pipeline="/opt/ros/humble/share/mola/pipeline.yaml",
            state_estimator_params="/opt/ros/humble/share/mola/state.yaml",
            sequence="07",
            reference_csv="/secret/ground_truth.csv",
            only_first_n=50,
        )
        command = MODULE.build_odometry_command(
            args, Path("/results/trajectory.tum")
        )
        self.assertNotIn(args.reference_csv, command)
        self.assertEqual(command[0], args.mola_executable)
        self.assertEqual(command[-2:], ["--only-first-n", "50"])

    def test_rejects_gt_in_mola_dataset_tree(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            dataset_root = Path(directory)
            poses = dataset_root / "poses"
            poses.mkdir()
            (poses / "07.txt").write_text("secret")
            with self.assertRaisesRegex(RuntimeError, "Refusing to expose"):
                MODULE.assert_no_dataset_ground_truth(dataset_root, "07")

    def test_accepts_dataset_tree_without_gt(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            MODULE.assert_no_dataset_ground_truth(Path(directory), "07")

    def test_partial_run_limits_tracking_denominator(self) -> None:
        references = list(range(100))
        self.assertEqual(
            MODULE.limit_references_for_run(references, 20), list(range(20))
        )
        self.assertIs(
            MODULE.limit_references_for_run(references, None), references
        )


if __name__ == "__main__":
    unittest.main()
