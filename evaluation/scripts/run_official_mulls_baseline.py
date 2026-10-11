#!/usr/bin/env python3
"""Run official MULLS (YuePanEdward/MULLS) LiDAR odometry on KITTI without exposing ground truth.

`mulls_slam` reads the KITTI .bin scans directly with the KITTI urban
profile (`script/config/lo_gflag_list_kitti_urban.txt`: loop closure off,
0.195 deg vertical-angle calibration), the paper's MULLS-LO setting, with
the viewer and map output off and no ground-truth or calibration file. The
LiDAR-frame poses (`pose_l_lo`) are scored with the Zoo protocol.

Upstream builds with BUILD_WITH_OPENCV/HDF5/CERES/TEASER=OFF. With PCL 1.14 and
GCC 13 it needs local patches: `enable_language(C)` and
`find_package(MPI COMPONENTS C CXX)` before `find_package(PCL)` (VTK's MPI
target), boost::make_shared/shared_ptr -> std::, pcl_isfinite ->
std::isfinite, `bfgs.testGradient(gradient_tol)` replaced by a gradient-norm
test in include/baseline_reg/gicp_omp_impl.hpp, and
CMAKE_CXX_FLAGS="-include cfloat -include cmath".
"""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import time
from pathlib import Path

from evaluate_external_kitti_odometry import compute_metrics, load_kitti_poses, load_reference_csv


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--velodyne-dir", required=True, help="KITTI sequences/<seq>/velodyne directory.")
    parser.add_argument("--reference-csv", required=True, help="Zoo lidar_pose GT CSV (metrics only).")
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--mulls-root", required=True, help="MULLS source tree containing bin/mulls_slam.")
    parser.add_argument("--config", default="script/config/lo_gflag_list_kitti_urban.txt",
                        help="gflags file, relative to --mulls-root.")
    parser.add_argument("--method", default="official_mulls")
    parser.add_argument("extra_flags", nargs="*", help="Additional gflags passed to mulls_slam (after --).")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    args = parse_args()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    mulls_root = Path(args.mulls_root).resolve()
    # mulls_slam reads "<folder>_filelist.txt" next to the scan folder, so link
    # the scans into the output directory instead of writing into the dataset.
    scans = output_dir / "scans"
    if scans.is_symlink() or scans.exists():
        scans.unlink()
    scans.symlink_to(Path(args.velodyne_dir).resolve(), target_is_directory=True)
    names = sorted(path.name for path in scans.glob("*.bin"))
    (output_dir / "scans_filelist.txt").write_text("".join(f"{name}\n" for name in names))
    for sub in ("log", "map"):
        (output_dir / sub).mkdir(exist_ok=True)

    estimate_path = output_dir / "pose_l_lo.txt"
    command = [
        str(mulls_root / "bin" / "mulls_slam"),
        "--colorlogtostderr=false", "-stderrthreshold", "2", f"-log_dir={output_dir / 'log'}", "--v=10",
        f"--point_cloud_folder={scans}", "--pc_format=.bin",
        "--gt_body_pose_file_path=", "--calib_file_path=",
        f"--output_adjacent_lo_pose_file_path={output_dir / 'Rt_lo.txt'}",
        f"--output_lo_lidar_pose_file_path={estimate_path}",
        f"--output_lo_body_pose_file_path={output_dir / 'pose_b_lo.txt'}",
        f"--output_gt_lidar_pose_file_path={output_dir / 'pose_l_gt.txt'}",
        f"--output_map_point_cloud_folder_path={output_dir / 'map'}",
        f"--lo_lidar_pose_point_cloud={output_dir / 'traj_l.pcd'}",
        f"--gt_lidar_pose_point_cloud={output_dir / 'traj_gt.pcd'}",
        f"--timing_report_file={output_dir / 'timing.txt'}",
        "--frame_num_begin=0", "--frame_num_end=99999", "--frame_step=1",
        f"--flagfile={args.config}",
        "--real_time_viewer_on=0", "--gt_in_lidar_frame=0", "--gt_oxts_format=0",
        "--write_out_map_on=0", "--write_out_gt_map_on=0", "--write_map_each_frame=0",
        *args.extra_flags,
    ]
    log_path = output_dir / "official_mulls.log"
    start = time.perf_counter()
    with log_path.open("w") as log:
        completed = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, cwd=mulls_root, check=False)
    wall_seconds = time.perf_counter() - start
    if completed.returncode != 0:
        raise subprocess.CalledProcessError(completed.returncode, command)

    # Ground truth is opened only after the odometry process exits.
    reference_path = Path(args.reference_csv).resolve()
    estimates = load_kitti_poses(estimate_path)
    metrics = compute_metrics(estimates, load_reference_csv(reference_path))
    metrics["end_to_end_fps"] = len(estimates) / wall_seconds if wall_seconds > 0 else None
    config_path = mulls_root / args.config
    payload = {
        "schema_version": 1,
        "method": args.method,
        "gt_usage": "metrics_only_after_odometry_process_exit",
        "first_pose_anchor_only": True,
        "command": command,
        "configuration": {
            "config": str(config_path),
            "config_sha256": sha256_file(config_path),
            "extra_flags": args.extra_flags,
        },
        "artifacts": {
            "velodyne_dir": str(Path(args.velodyne_dir).resolve()),
            "estimate": str(estimate_path),
            "estimate_sha256": sha256_file(estimate_path),
            "reference_csv": str(reference_path),
            "reference_sha256": sha256_file(reference_path),
            "log": str(log_path),
        },
        "timing": {"wall_seconds": wall_seconds},
        "metrics": metrics,
    }
    (output_dir / "zoo_manifest.json").write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(metrics, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
