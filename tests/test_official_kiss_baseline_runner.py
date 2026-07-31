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
    "official_kiss_baseline_runner",
    SCRIPT_DIR / "run_official_kiss_baseline.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load official KISS-ICP runner")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class OfficialKissBaselineRunnerTests(unittest.TestCase):
    def test_child_command_never_contains_reference_path(self) -> None:
        args = argparse.Namespace(
            official_python="/venv/bin/python",
            dataset_root="/data/kitti-no-gt",
            sequence="10",
            reference_csv="/secret/ground_truth.csv",
        )
        command = MODULE.build_odometry_command(args, Path("/results/kiss"))
        self.assertNotIn(args.reference_csv, command)
        self.assertEqual(command[0], args.official_python)
        self.assertEqual(command[-1], "--_odometry-only")

    def test_rejects_gt_in_dataset_tree(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            dataset_root = Path(directory)
            poses = dataset_root / "poses"
            poses.mkdir()
            (poses / "10.txt").write_text("secret")
            with self.assertRaisesRegex(RuntimeError, "Refusing to expose"):
                MODULE.assert_no_dataset_ground_truth(dataset_root, "10")

    def test_accepts_dataset_tree_without_gt(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            MODULE.assert_no_dataset_ground_truth(Path(directory), "10")

    def test_thread_cap_is_forwarded_to_isolated_child(self) -> None:
        args = argparse.Namespace(
            official_python="/venv/bin/python",
            dataset_root="/data/kitti-no-gt",
            sequence="03",
            reference_csv="/secret/ground_truth.csv",
            max_threads=4,
        )
        command = MODULE.build_odometry_command(args, Path("/results/kiss"))
        self.assertIn("--max-threads", command)
        self.assertEqual(command[command.index("--max-threads") + 1], "4")
        self.assertNotIn(args.reference_csv, command)


if __name__ == "__main__":
    unittest.main()
