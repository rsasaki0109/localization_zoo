import importlib.util
import sys
import unittest
from dataclasses import dataclass
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/extract_rosbag_lidar_bins.py"
SPEC = importlib.util.spec_from_file_location("extract_rosbag_lidar", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load ROS bag LiDAR extractor")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


@dataclass
class Field:
    name: str
    offset: int
    datatype: int = 7
    count: int = 1


@dataclass
class Cloud:
    fields: list[Field]
    point_step: int
    row_step: int
    height: int
    width: int
    is_bigendian: bool
    data: bytes


class ExtractRosbagLidarBinsTests(unittest.TestCase):
    def test_extracts_xyzi_with_row_padding(self) -> None:
        fields = [
            Field("x", 0),
            Field("y", 4),
            Field("z", 8),
            Field("intensity", 12),
        ]
        left = np.array([1.0, 2.0, 3.0, 4.0], dtype="<f4").tobytes()
        right = np.array([5.0, 6.0, 7.0, 8.0], dtype="<f4").tobytes()
        cloud = Cloud(fields, 16, 20, 2, 1, False, left + b"pad!" + right)
        np.testing.assert_allclose(
            MODULE.cloud_xyzi(cloud),
            [[1.0, 2.0, 3.0, 4.0], [5.0, 6.0, 7.0, 8.0]],
        )

    def test_uses_reflectivity_and_drops_nonfinite_xyz(self) -> None:
        fields = [
            Field("x", 0),
            Field("y", 4),
            Field("z", 8),
            Field("reflectivity", 12, datatype=4),
        ]
        dtype = MODULE.cloud_dtype(fields, 16, False)
        points = np.zeros(2, dtype=dtype)
        points["x"] = [1.0, np.nan]
        points["y"] = 2.0
        points["z"] = 3.0
        points["reflectivity"] = [42, 7]
        cloud = Cloud(fields, 16, 32, 1, 2, False, points.tobytes())
        np.testing.assert_allclose(MODULE.cloud_xyzi(cloud), [[1.0, 2.0, 3.0, 42.0]])


if __name__ == "__main__":
    unittest.main()
