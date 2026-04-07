#!/usr/bin/env python3

"""Convert KITTI Raw drive into pcd_dogfooding benchmark format.

Reads velodyne_points/data/*.bin + oxts/data/*.txt and produces:
  - PCD directory (NNNNNNNN/cloud.pcd)
  - GT CSV (timestamp, lidar_pose.x/y/z/roll/pitch/yaw)
  - Optional imu.csv via ``--write-imu-csv`` (DLIO/CT-LIO; see kitti_oxts_imu_for_dogfooding.py).
"""

from __future__ import annotations

import argparse
import csv
import math
import subprocess
import sys
from pathlib import Path

import numpy as np


R_EARTH = 6378137.0  # WGS84 semi-major axis


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert KITTI Raw drive to pcd_dogfooding format.",
    )
    parser.add_argument("--drive-dir", required=True, help="KITTI Raw drive_XXXX_sync directory.")
    parser.add_argument("--output-dir", required=True, help="Output PCD directory.")
    parser.add_argument("--gt-csv", required=True, help="Output GT CSV path.")
    parser.add_argument("--start-frame", type=int, default=0)
    parser.add_argument("--max-frames", type=int, default=-1)
    parser.add_argument(
        "--write-imu-csv",
        action="store_true",
        help="After export, run kitti_oxts_imu_for_dogfooding.py into output-dir.",
    )
    return parser.parse_args()


def write_binary_pcd(path: Path, points: np.ndarray) -> None:
    """Write PCD v0.7 binary with FIELDS x y z intensity."""
    field_names = ["x", "y", "z", "intensity"]
    sizes = " ".join(["4"] * len(field_names))
    types = " ".join(["F"] * len(field_names))
    counts = " ".join(["1"] * len(field_names))
    fields = " ".join(field_names)
    header = "\n".join([
        "# .PCD v0.7 - Point Cloud Data file format",
        "VERSION 0.7",
        f"FIELDS {fields}",
        f"SIZE {sizes}",
        f"TYPE {types}",
        f"COUNT {counts}",
        f"WIDTH {len(points)}",
        "HEIGHT 1",
        "VIEWPOINT 0 0 0 1 0 0 0",
        f"POINTS {len(points)}",
        "DATA binary",
        "",
    ]).encode("ascii")
    with path.open("wb") as handle:
        handle.write(header)
        handle.write(points.tobytes())


def load_kitti_bin(path: Path) -> np.ndarray:
    """Load KITTI .bin and filter by range [0.5, 120.0]."""
    raw = np.fromfile(str(path), dtype=np.float32).reshape(-1, 4)
    x, y, z = raw[:, 0], raw[:, 1], raw[:, 2]
    r = np.sqrt(x * x + y * y + z * z)
    mask = (r > 0.5) & (r < 120.0)
    return raw[mask]


def load_oxts(oxts_dir: Path) -> list[dict]:
    """Load all oxts data files, return list of {lat, lon, alt, roll, pitch, yaw}."""
    files = sorted(oxts_dir.glob("*.txt"))
    poses = []
    for f in files:
        vals = list(map(float, f.read_text().strip().split()))
        poses.append({
            "lat": vals[0], "lon": vals[1], "alt": vals[2],
            "roll": vals[3], "pitch": vals[4], "yaw": vals[5],
        })
    return poses


def oxts_to_local(poses: list[dict]) -> list[dict]:
    """Convert GPS (lat/lon/alt) to local ENU coordinates relative to first pose."""
    if not poses:
        return []
    lat0 = math.radians(poses[0]["lat"])
    lon0 = math.radians(poses[0]["lon"])
    alt0 = poses[0]["alt"]

    result = []
    for p in poses:
        lat = math.radians(p["lat"])
        lon = math.radians(p["lon"])
        # Mercator-like local projection
        x = R_EARTH * (lon - lon0) * math.cos(lat0)
        y = R_EARTH * (lat - lat0)
        z = p["alt"] - alt0
        result.append({
            "x": x, "y": y, "z": z,
            "roll": p["roll"], "pitch": p["pitch"], "yaw": p["yaw"],
        })
    return result


def main() -> int:
    args = parse_args()
    drive_dir = Path(args.drive_dir)
    output_dir = Path(args.output_dir)
    gt_csv_path = Path(args.gt_csv)
    output_dir.mkdir(parents=True, exist_ok=True)
    gt_csv_path.parent.mkdir(parents=True, exist_ok=True)

    velodyne_dir = drive_dir / "velodyne_points" / "data"
    oxts_dir = drive_dir / "oxts" / "data"

    bin_files = sorted(velodyne_dir.glob("*.bin"))
    if not bin_files:
        raise RuntimeError(f"No .bin files in {velodyne_dir}")

    oxts_poses = load_oxts(oxts_dir)
    local_poses = oxts_to_local(oxts_poses)

    end = len(bin_files)
    if args.max_frames >= 0:
        end = min(args.start_frame + args.max_frames, len(bin_files))
    selected_range = range(args.start_frame, end)

    if not selected_range:
        raise RuntimeError(f"No frames in range [{args.start_frame}, {end})")

    # Write PCD files + frame_timestamps.csv
    ts_path = output_dir / "frame_timestamps.csv"
    with ts_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        for i, global_idx in enumerate(selected_range):
            points = load_kitti_bin(bin_files[global_idx])
            packed = np.empty(len(points), dtype=np.dtype([
                ("x", "<f4"), ("y", "<f4"), ("z", "<f4"), ("intensity", "<f4"),
            ]))
            packed["x"] = points[:, 0]
            packed["y"] = points[:, 1]
            packed["z"] = points[:, 2]
            packed["intensity"] = points[:, 3]

            frame_dir = output_dir / f"{i:08d}"
            frame_dir.mkdir(exist_ok=True)
            write_binary_pcd(frame_dir / "cloud.pcd", packed)
            writer.writerow([i, global_idx, len(packed)])

            if (i + 1) % 50 == 0:
                print(f"  extracted {i + 1} frames...")

    # Write GT CSV
    with gt_csv_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "timestamp", "lidar_pose.x", "lidar_pose.y", "lidar_pose.z",
            "lidar_pose.roll", "lidar_pose.pitch", "lidar_pose.yaw",
        ])
        for i, global_idx in enumerate(selected_range):
            p = local_poses[global_idx]
            writer.writerow([
                f"{global_idx}",
                f"{p['x']:.10f}", f"{p['y']:.10f}", f"{p['z']:.10f}",
                f"{p['roll']:.10f}", f"{p['pitch']:.10f}", f"{p['yaw']:.10f}",
            ])

    n = len(selected_range)
    print(f"[done] wrote {n} PCD frames to {output_dir}")
    print(f"[done] wrote {n} GT poses to {gt_csv_path}")

    if args.write_imu_csv:
        helper = Path(__file__).resolve().parent / "kitti_oxts_imu_for_dogfooding.py"
        subprocess.run(
            [
                sys.executable,
                str(helper),
                "--drive-dir",
                str(drive_dir.resolve()),
                "--pcd-dir",
                str(output_dir.resolve()),
            ],
            check=True,
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
