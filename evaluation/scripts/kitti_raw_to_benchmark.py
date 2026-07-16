#!/usr/bin/env python3

"""Convert KITTI Raw drive into pcd_dogfooding benchmark format.

Reads velodyne_points/data/*.bin + oxts/data/*.txt and produces:
  - PCD directory (NNNNNNNN/cloud.pcd)
  - GT CSV (timestamp, lidar_pose.x/y/z/roll/pitch/yaw)
  - Optional imu.csv via ``--write-imu-csv`` (DLIO/CT-LIO; see kitti_oxts_imu_for_dogfooding.py).

Indexing: some KITTI Raw sync packages are missing individual Velodyne scans
(e.g. drive 2011_09_26_0009 is missing native frames 177-180 -- 443 Velodyne
files vs 447 OXTS samples) while the OXTS stream itself has no gaps. Frames
are therefore matched by **native frame number** parsed from each
``velodyne_points/data/NNNNNNNNNN.bin`` filename (and, symmetrically, from
each ``oxts/data/NNNNNNNNNN.txt`` filename) rather than by position in the
sorted file listing. A native index missing from either stream is dropped
from the export entirely -- output frame ``i`` (the ``i``-th row of
``frame_timestamps.csv`` / the GT CSV / ``imu.csv``) always corresponds to
the same native index across all three, so nothing silently shifts out of
alignment. This intentionally means a "200-frame window" over a drive with
gaps in range can yield fewer than 200 exported frames; the run log reports
exactly which native indices were dropped and why.
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
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="Native frame number (parsed from filenames) to start from, not a position.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=-1,
        help="Max native frame range width from --start-frame. Frames missing from "
        "either stream within that range are dropped, so the exported count can be "
        "smaller than --max-frames.",
    )
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


def parse_native_index(path: Path) -> int:
    """Parse the native KITTI frame number from a zero-padded filename stem."""
    return int(path.stem)


def load_velodyne_index(velodyne_dir: Path) -> dict[int, Path]:
    """Map native frame number -> .bin path for all Velodyne scans present."""
    files = sorted(velodyne_dir.glob("*.bin"))
    return {parse_native_index(f): f for f in files}


def load_oxts(oxts_dir: Path) -> dict[int, dict]:
    """Map native frame number -> {lat, lon, alt, roll, pitch, yaw} for all OXTS samples present."""
    files = sorted(oxts_dir.glob("*.txt"))
    poses: dict[int, dict] = {}
    for f in files:
        vals = list(map(float, f.read_text().strip().split()))
        poses[parse_native_index(f)] = {
            "lat": vals[0], "lon": vals[1], "alt": vals[2],
            "roll": vals[3], "pitch": vals[4], "yaw": vals[5],
        }
    return poses


def oxts_to_local(poses: dict[int, dict]) -> dict[int, dict]:
    """Convert GPS (lat/lon/alt) to local ENU coords relative to native frame 0."""
    if not poses:
        return {}
    origin_idx = min(poses)
    lat0 = math.radians(poses[origin_idx]["lat"])
    lon0 = math.radians(poses[origin_idx]["lon"])
    alt0 = poses[origin_idx]["alt"]

    result: dict[int, dict] = {}
    for idx, p in poses.items():
        lat = math.radians(p["lat"])
        lon = math.radians(p["lon"])
        # Mercator-like local projection
        x = R_EARTH * (lon - lon0) * math.cos(lat0)
        y = R_EARTH * (lat - lat0)
        z = p["alt"] - alt0
        result[idx] = {
            "x": x, "y": y, "z": z,
            "roll": p["roll"], "pitch": p["pitch"], "yaw": p["yaw"],
        }
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

    velodyne_index = load_velodyne_index(velodyne_dir)
    if not velodyne_index:
        raise RuntimeError(f"No .bin files in {velodyne_dir}")

    oxts_poses = load_oxts(oxts_dir)
    local_poses = oxts_to_local(oxts_poses)

    max_native = max(max(velodyne_index), max(local_poses) if local_poses else 0)
    end = max_native + 1
    if args.max_frames >= 0:
        end = args.start_frame + args.max_frames
    candidate_indices = range(args.start_frame, end)

    selected_native_indices = [
        idx for idx in candidate_indices
        if idx in velodyne_index and idx in local_poses
    ]
    dropped_no_velodyne = [
        idx for idx in candidate_indices if idx not in velodyne_index and idx in local_poses
    ]
    dropped_no_oxts = [
        idx for idx in candidate_indices if idx in velodyne_index and idx not in local_poses
    ]
    dropped_neither = [
        idx for idx in candidate_indices
        if idx not in velodyne_index and idx not in local_poses
    ]

    if not selected_native_indices:
        raise RuntimeError(
            f"No native frames with both Velodyne + OXTS data in range "
            f"[{args.start_frame}, {end})"
        )

    if dropped_no_velodyne:
        print(
            f"[warn] {len(dropped_no_velodyne)} native frame(s) have OXTS but no "
            f"Velodyne scan, dropped: {dropped_no_velodyne}"
        )
    if dropped_no_oxts:
        print(
            f"[warn] {len(dropped_no_oxts)} native frame(s) have a Velodyne scan but "
            f"no OXTS sample, dropped: {dropped_no_oxts}"
        )
    if dropped_neither:
        print(
            f"[info] {len(dropped_neither)} native frame(s) in range have neither "
            f"stream (out of drive bounds), dropped: {dropped_neither}"
        )

    # Write PCD files + frame_timestamps.csv. Output frame i ("{i:08d}") always
    # corresponds to GT CSV row i and imu.csv midpoints derived from the same
    # native index -- see kitti_oxts_imu_for_dogfooding.py, which reads the
    # "timestamp" column below as the native OXTS index.
    ts_path = output_dir / "frame_timestamps.csv"
    with ts_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        for i, native_idx in enumerate(selected_native_indices):
            points = load_kitti_bin(velodyne_index[native_idx])
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
            writer.writerow([i, native_idx, len(packed)])

            if (i + 1) % 50 == 0:
                print(f"  extracted {i + 1} frames...")

    # Write GT CSV (row i matches PCD frame i and frame_timestamps.csv row i above).
    with gt_csv_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "timestamp", "lidar_pose.x", "lidar_pose.y", "lidar_pose.z",
            "lidar_pose.roll", "lidar_pose.pitch", "lidar_pose.yaw",
        ])
        for native_idx in selected_native_indices:
            p = local_poses[native_idx]
            writer.writerow([
                f"{native_idx}",
                f"{p['x']:.10f}", f"{p['y']:.10f}", f"{p['z']:.10f}",
                f"{p['roll']:.10f}", f"{p['pitch']:.10f}", f"{p['yaw']:.10f}",
            ])

    n = len(selected_native_indices)
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
