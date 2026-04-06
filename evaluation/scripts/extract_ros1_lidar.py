#!/usr/bin/env python3

"""Extract sequential PCDs from ROS1 .bag files using rosbags library."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import numpy as np
from rosbags.rosbag1 import Reader
from rosbags.typesys import Stores, get_typestore


FIELD_SPECS = {
    "x": "<f4",
    "y": "<f4",
    "z": "<f4",
    "intensity": "<f4",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract PointCloud2 from ROS1 .bag to sequential PCD directory.",
    )
    parser.add_argument("--bag", required=True, help="ROS1 .bag file path.")
    parser.add_argument(
        "--pointcloud-topic",
        required=True,
        help="PointCloud2 topic to export.",
    )
    parser.add_argument("--output-dir", required=True, help="Output directory.")
    parser.add_argument(
        "--start-frame", type=int, default=0,
        help="First lidar frame index to export (default: 0).",
    )
    parser.add_argument(
        "--max-frames", type=int, default=-1,
        help="Maximum frames to export (-1 for all).",
    )
    return parser.parse_args()


def write_binary_pcd(path: Path, points: np.ndarray, field_names: list[str]) -> None:
    """Write PCD v0.7 binary (matching extract_ros2_lidar_imu.py)."""
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


def extract_points_from_msg(msg) -> tuple[np.ndarray, list[str]]:
    """Parse deserialized PointCloud2 message into structured numpy array."""
    offsets = {field.name: field.offset for field in msg.fields}
    names = [name for name in ("x", "y", "z", "intensity") if name in offsets]
    if not {"x", "y", "z"}.issubset(set(names)):
        raise RuntimeError("PointCloud2 must contain x/y/z fields.")

    source_dtype = np.dtype(
        {
            "names": names,
            "formats": [FIELD_SPECS[name] for name in names],
            "offsets": [offsets[name] for name in names],
            "itemsize": msg.point_step,
        }
    )
    count = int(msg.width) * int(msg.height)
    source = np.frombuffer(msg.data, dtype=source_dtype, count=count)

    packed_dtype = np.dtype([(name, FIELD_SPECS[name]) for name in names])
    packed = np.empty(count, dtype=packed_dtype)
    for name in names:
        packed[name] = source[name]

    # Range filter matching dataset_loader.h (0.5 < r < 120.0)
    x, y, z = packed["x"], packed["y"], packed["z"]
    r = np.sqrt(x * x + y * y + z * z)
    mask = (r > 0.5) & (r < 120.0) & np.isfinite(r)
    return packed[mask], names


def main() -> int:
    args = parse_args()
    bag_path = Path(args.bag)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    typestore = get_typestore(Stores.ROS1_NOETIC)

    timestamps_path = output_dir / "frame_timestamps.csv"
    written = 0
    seen = 0

    with timestamps_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        with Reader(bag_path) as reader:
            for connection, timestamp, rawdata in reader.messages():
                if connection.topic != args.pointcloud_topic:
                    continue

                if seen < args.start_frame:
                    seen += 1
                    continue
                if args.max_frames >= 0 and written >= args.max_frames:
                    break

                msg = typestore.deserialize_ros1(rawdata, connection.msgtype)
                points, field_names = extract_points_from_msg(msg)

                frame_dir = output_dir / f"{written:08d}"
                frame_dir.mkdir(exist_ok=True)
                write_binary_pcd(frame_dir / "cloud.pcd", points, field_names)

                ts_sec = timestamp * 1e-9
                writer.writerow([written, f"{ts_sec:.9f}", len(points)])
                written += 1
                seen += 1

                if written % 50 == 0:
                    print(f"  extracted {written} frames...")

    print(f"[done] wrote {written} frames to {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
