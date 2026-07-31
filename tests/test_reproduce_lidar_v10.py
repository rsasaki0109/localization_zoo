import importlib.util
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


def load_module():
    path = REPO_ROOT / "evaluation/scripts/reproduce_lidar_v10.py"
    spec = importlib.util.spec_from_file_location("test_reproduce_lidar_v10_module", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class ReproduceLidarV10Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_module()

    def test_hash_check_accepts_exact_file(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            path = Path(tmpdir) / "artifact.txt"
            path.write_bytes(b"v10\n")
            expected = self.module.sha256_file(path)
            self.assertTrue(self.module.check_hash(path, expected)["passed"])

    def test_hash_check_rejects_missing_file(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            result = self.module.check_hash(Path(tmpdir) / "missing", "0" * 64)
            self.assertFalse(result["exists"])
            self.assertFalse(result["passed"])

    def test_metric_comparison_is_strict_but_tolerates_serialization_noise(self) -> None:
        actual = {"ate_m": 1.0 + 1e-10, "rpe_trans_pct": 0.5, "rpe_rot_deg_per_m": 0.01}
        expected = {"ate_m": 1.0, "rpe_trans_pct": 0.5, "rpe_rot_deg_per_m": 0.01}
        result = self.module.compare_metrics(actual, expected)
        self.assertTrue(all(item["passed"] for item in result.values()))

    def test_metric_comparison_rejects_material_change(self) -> None:
        result = self.module.compare_metrics({"ate_m": 1.01}, {"ate_m": 1.0})
        self.assertFalse(result["ate_m"]["passed"])


if __name__ == "__main__":
    unittest.main()
