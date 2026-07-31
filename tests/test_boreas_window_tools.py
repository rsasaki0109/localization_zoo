import csv
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[1]


def load_module(name: str, relative: str):
    path = REPO_ROOT / relative
    script_dir = str(path.parent)
    if script_dir not in sys.path:
        sys.path.insert(0, script_dir)
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class BoreasWindowToolsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.fetch = load_module("test_fetch_boreas_window", "evaluation/scripts/fetch_boreas_window.py")
        cls.prepare = load_module("test_prepare_boreas_window", "evaluation/scripts/prepare_boreas_window.py")
        cls.gt = load_module("test_boreas_lidar_poses_to_gt", "evaluation/scripts/boreas_lidar_poses_to_gt.py")
        cls.pcd = load_module("test_official_pcd_loader", "evaluation/scripts/run_official_kiss_pcd_odometry.py")

    def test_selection_hash_is_order_sensitive_and_repeatable(self) -> None:
        items = [{"key": "a", "size": 1, "etag": "x"}, {"key": "b", "size": 2, "etag": "y"}]
        self.assertEqual(self.fetch.selection_sha256(items), self.fetch.selection_sha256(items))
        self.assertNotEqual(self.fetch.selection_sha256(items), self.fetch.selection_sha256(list(reversed(items))))

    def test_boreas_binary_to_timestamped_pcd(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            source = root / "scan.bin"
            values = np.array([[1, 2, 3, 4, 5, -0.05], [6, 7, 8, 9, 10, 0.05]], dtype=np.float32)
            values.tofile(source)
            output = root / "cloud.pcd"
            self.prepare.write_pcd(output, self.prepare.load_boreas_scan(source))
            xyz, timestamps = self.pcd.load_pcd_xyz_timestamps(output)
            np.testing.assert_allclose(xyz, values[:, :3])
            np.testing.assert_allclose(timestamps, [0.0, 1.0])

    def test_boreas_rotation_round_trip_to_standard_rpy(self) -> None:
        rotation = self.gt.boreas_rotation(0.1, -0.2, 0.3)
        roll, pitch, yaw = self.gt.rotation_matrix_to_rpy(rotation)
        cr, sr = np.cos(roll), np.sin(roll)
        cp, sp = np.cos(pitch), np.sin(pitch)
        cy, sy = np.cos(yaw), np.sin(yaw)
        standard = np.array([[cy * cp, cy * sp * sr - sy * cr, cy * sp * cr + sy * sr], [sy * cp, sy * sp * sr + cy * cr, sy * sp * cr - cy * sr], [-sp, cp * sr, cp * cr]])
        np.testing.assert_allclose(standard, rotation, atol=1e-12)

    def test_gt_alignment_requires_exact_selected_timestamps(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            path = Path(tmpdir) / "frames.csv"
            with path.open("w", newline="") as handle:
                writer = csv.writer(handle)
                writer.writerow(["frame_idx", "timestamp", "points"])
                writer.writerow([0, "123.456789", ""])
            self.assertEqual(self.gt.load_selected_timestamps(path), [123456789])


if __name__ == "__main__":
    unittest.main()
