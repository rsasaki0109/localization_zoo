import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
V14 = "evaluation/data/lidar_odometry_candidate_runtime_feasible_reference_v14.json"


class LidarV14ManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = json.loads((ROOT / V14).read_text())
        cls.audit = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_v14_promotion_audit.json").read_text()
        )
        cls.suite = json.loads(
            (ROOT / "evaluation/data/lidar_odometry_sota_suite.json").read_text()
        )
        cls.readme = (ROOT / "README.md").read_text()

    def test_v14_is_promoted_suite_candidate(self) -> None:
        self.assertTrue(self.manifest["promoted"])
        self.assertTrue(self.audit["promotion_allowed"])
        self.assertTrue(self.audit["all_gates_passed"])
        self.assertEqual(self.suite["promoted_candidate_manifest"], V14)

    def test_readme_headline_matches_audit(self) -> None:
        self.assertIn("## Promoted LiDAR odometry v14", self.readme)
        self.assertIn(V14, self.readme)
        cube = self.audit["requirements"]["cube_lio_same_input_kitti_00_and_07"]["rows"]
        external = self.audit["requirements"]["untouched_normal_environment_external"]
        for value in (
            cube["kitti_odometry_00"]["candidate_ate_m"],
            cube["kitti_odometry_07"]["candidate_ate_m"],
            cube["kitti_odometry_07"]["cube_lio_ate_m"],
        ):
            self.assertIn(f"{value:.3f} m", self.readme)
        self.assertIn(f"{external['candidate_fps']:.1f} FPS", self.readme)


if __name__ == "__main__":
    unittest.main()
