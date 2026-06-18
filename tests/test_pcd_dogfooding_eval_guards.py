import subprocess
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
PCD_DOGFOODING = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"


def run_pcd_dogfooding(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(PCD_DOGFOODING), *args],
        cwd=REPO_ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )


def make_pcd_dirs(root: Path, count: int) -> None:
    for i in range(count):
        frame_dir = root / f"{i:08d}"
        frame_dir.mkdir(parents=True)
        (frame_dir / "cloud.pcd").write_text("", encoding="utf-8")


@unittest.skipUnless(PCD_DOGFOODING.exists(), "pcd_dogfooding binary not built")
class PcdDogfoodingEvalGuardsTest(unittest.TestCase):
    def test_rejects_malformed_kitti_gt_line(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            pcd_dir = tmp_path / "pcd"
            pcd_dir.mkdir()
            gt_path = tmp_path / "bad_gt.txt"
            gt_path.write_text(
                "1 0 0 0 0 1 0 0 0 0 1\n",
                encoding="utf-8",
            )

            result = run_pcd_dogfooding(str(pcd_dir), str(gt_path), "--methods", "kiss_icp")

            self.assertNotEqual(result.returncode, 0)
            self.assertIn("expected KITTI 3x4 pose with 12 values", result.stdout)
            self.assertNotIn("Segmentation fault", result.stdout)

    def test_rejects_mutable_dogfooding_gt_artifact_by_default(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            pcd_dir = tmp_path / "pcd"
            pcd_dir.mkdir()
            gt_dir = tmp_path / "dogfooding_results"
            gt_dir.mkdir()
            gt_path = gt_dir / "gt.txt"
            gt_path.write_text(
                "1 0 0 0 0 1 0 0 0 0 1 0\n",
                encoding="utf-8",
            )

            result = run_pcd_dogfooding(str(pcd_dir), str(gt_path), "--methods", "kiss_icp")

            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Refusing dogfooding_results/gt.txt as canonical GT", result.stdout)

    def test_rejects_integer_timeline_with_missing_frame_id(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            pcd_dir = tmp_path / "pcd"
            pcd_dir.mkdir()
            make_pcd_dirs(pcd_dir, 4)
            (pcd_dir / "frame_timestamps.csv").write_text(
                "frame_idx,timestamp,points\n"
                "0,0,0\n"
                "1,1,0\n"
                "2,2,0\n"
                "3,3,0\n",
                encoding="utf-8",
            )
            gt_path = tmp_path / "gt.csv"
            gt_path.write_text(
                "timestamp,lidar_pose.x,lidar_pose.y,lidar_pose.z,"
                "lidar_pose.roll,lidar_pose.pitch,lidar_pose.yaw\n"
                "0,0,0,0,0,0,0\n"
                "2,1,0,0,0,0,0\n"
                "4,2,0,0,0,0,0\n"
                "6,3,0,0,0,0,0\n",
                encoding="utf-8",
            )

            result = run_pcd_dogfooding(str(pcd_dir), str(gt_path), "--methods", "kiss_icp")

            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Missing GT pose for frame_id 1", result.stdout)
            self.assertIn("refusing silent GT sampling fallback", result.stdout)


if __name__ == "__main__":
    unittest.main()
