#!/usr/bin/env python3

"""Convert KITTI velodyne .bin files into sequential PCD directory for pcd_dogfooding."""

from __future__ import annotations

import argparse
import csv
import struct
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert KITTI .bin velodyne scans to NNNNNNNN/cloud.pcd directory.",
    )
    parser.add_argument("--velodyne-dir", required=True, help="Directory containing KITTI .bin files.")
    parser.add_argument("--output-dir", required=True, help="Output directory for PCD sequence.")
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First frame index to export (default: 0).",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=-1,
        help="Maximum frames to export (-1 for all).",
    )
    return parser.parse_args()


def write_binary_pcd(path: Path, points: np.ndarray) -> None:
    """Write PCD v0.7 binary with FIELDS x y z intensity (matching extract_ros2_lidar_imu.py)."""
    field_names = ["x", "y", "z", "intensity"]
    sizes = " ".join(["4"] * len(field_names))
    types = " ".join(["F"] * len(field_names))
    counts = " ".join(["1"] * len(field_names))
    fields = " ".join(field_names)
    header = "\n".join(
        [
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
        ]
    ).encode("ascii")

    with path.open("wb") as handle:
        handle.write(header)
        handle.write(points.tobytes())


def load_kitti_bin(path: Path) -> np.ndarray:
    """Load KITTI .bin and filter by range [0.5, 120.0] (matching dataset_loader.h)."""
    raw = np.fromfile(str(path), dtype=np.float32).reshape(-1, 4)
    x, y, z = raw[:, 0], raw[:, 1], raw[:, 2]
    r = np.sqrt(x * x + y * y + z * z)
    mask = (r > 0.5) & (r < 120.0)
    return raw[mask]


def main() -> int:
    args = parse_args()
    velodyne_dir = Path(args.velodyne_dir)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    bin_files = sorted(velodyne_dir.glob("*.bin"))
    if not bin_files:
        raise RuntimeError(f"No .bin files found in {velodyne_dir}")

    end_frame = len(bin_files)
    if args.max_frames >= 0:
        end_frame = min(args.start_frame + args.max_frames, len(bin_files))

    selected = bin_files[args.start_frame:end_frame]
    if not selected:
        raise RuntimeError(f"No frames in range [{args.start_frame}, {end_frame})")

    timestamps_path = output_dir / "frame_timestamps.csv"
    with timestamps_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        for i, bin_path in enumerate(selected):
            points = load_kitti_bin(bin_path)

            # Pack as float32 structured array for PCD binary
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

            # timestamp = global frame index (matching loadKittiPoses: gp.timestamp = idx++)
            global_frame_idx = args.start_frame + i
            writer.writerow([i, global_frame_idx, len(packed)])

    print(f"[done] wrote {len(selected)} frames to {output_dir}")
    print(f"[done] frame range: [{args.start_frame}, {args.start_frame + len(selected)})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
