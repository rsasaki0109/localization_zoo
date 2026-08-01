import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "evaluation/scripts/prepare_boreas_xyzi_bin.py"
SPEC = importlib.util.spec_from_file_location("prepare_boreas_xyzi_bin", SCRIPT)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("Cannot load Boreas XYZI converter")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class PrepareBoreasXyziBinTests(unittest.TestCase):
    def test_preserves_xyzi_values_and_source_order(self) -> None:
        source_points = np.asarray(
            [
                [1.0, 2.0, 3.0, 4.0, 9.0, 0.1],
                [5.0, 6.0, 7.0, 8.0, 10.0, 0.2],
            ],
            dtype=np.float32,
        )
        with tempfile.TemporaryDirectory() as temp_dir:
            source = Path(temp_dir) / "source.bin"
            output = Path(temp_dir) / "output.bin"
            source_points.tofile(source)
            count = MODULE.convert_scan(source, output)
            converted = np.fromfile(output, dtype=np.float32).reshape(-1, 4)
        self.assertEqual(count, 2)
        np.testing.assert_array_equal(converted, source_points[:, :4])


if __name__ == "__main__":
    unittest.main()
