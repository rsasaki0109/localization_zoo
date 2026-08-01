import argparse
import importlib.util
import sys
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_DIR = REPO_ROOT / "evaluation" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
SPEC = importlib.util.spec_from_file_location(
    "mad_icp_baseline_runner",
    SCRIPT_DIR / "run_mad_icp_baseline.py",
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load MAD-ICP runner")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class MadIcpBaselineRunnerTests(unittest.TestCase):
    def test_parse_odometry_time_ms(self) -> None:
        values = MODULE.parse_odometry_time_ms(
            "Time for odometry estimation [ms]:  12.5\n"
            "ignored\n"
            "Time for odometry estimation [ms]:  1.25e2\n"
        )
        self.assertEqual(values, [12.5, 125.0])

    def test_child_command_never_contains_reference_path(self) -> None:
        args = argparse.Namespace(
            mad_icp_executable="/venv/bin/mad_icp",
            data_path="/data/kitti/00",
            reference_csv="/secret/ground_truth.csv",
            dataset_config="kitti",
            mad_icp_params="default",
            num_cores=16,
            num_keyframes=16,
            realtime=False,
        )
        command = MODULE.build_odometry_command(args, Path("/results/00"))
        self.assertNotIn(args.reference_csv, command)
        self.assertEqual(command[0], args.mad_icp_executable)
        self.assertIn("--noviz", command)

    def test_mulran_uses_official_sensor_extrinsic(self) -> None:
        transform = MODULE.lidar_to_base_for_dataset("mulran")
        self.assertEqual(len(transform), 12)
        self.assertEqual(transform[:4], [-1.0, -0.0058, 0.0, 1.7042])
        self.assertEqual(transform[8:], [0.0, 0.0, 1.0, 1.8047])


if __name__ == "__main__":
    unittest.main()
