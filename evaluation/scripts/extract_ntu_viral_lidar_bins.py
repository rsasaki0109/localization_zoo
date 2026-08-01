#!/usr/bin/env python3
"""Extract xyz/intensity LiDAR scans from an NTU sensor-only ROS bag."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

import numpy as np


FLOAT32_DATATYPE = 7


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bag", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--topic", default="/os1_cloud_node1/points")
    return parser.parse_args()


def pointcloud_dtype(fields: object, point_step: int) -> np.dtype:
    by_name = {field.name: field for field in fields}
    required = ("x", "y", "z", "intensity")
    missing = [name for name in required if name not in by_name]
    if missing:
        raise ValueError(f"point cloud fields missing: {missing}")
    for name in required:
        if by_name[name].datatype != FLOAT32_DATATYPE:
            raise ValueError(f"{name} must be FLOAT32")
    return np.dtype(
        {
            "names": list(required),
            "formats": ["<f4"] * 4,
            "offsets": [by_name[name].offset for name in required],
            "itemsize": point_step,
        }
    )


def main() -> int:
    args = parse_args()
    import rosbag

    scans_dir = args.output_dir / "velodyne"
    scans_dir.mkdir(parents=True, exist_ok=True)
    timestamps_path = args.output_dir / "timestamps.csv"
    frames = 0
    total_points = 0
    with timestamps_path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(("frame_id", "timestamp"))
        with rosbag.Bag(str(args.bag), "r") as bag:
            for _, message, _ in bag.read_messages(topics=[args.topic]):
                if message.is_bigendian:
                    raise ValueError("big-endian PointCloud2 is unsupported")
                dtype = pointcloud_dtype(message.fields, message.point_step)
                points = np.frombuffer(message.data, dtype=dtype)
                xyzi = np.column_stack(
                    tuple(points[name] for name in ("x", "y", "z", "intensity"))
                ).astype("<f4", copy=False)
                xyzi = xyzi[np.isfinite(xyzi).all(axis=1)]
                xyzi.tofile(scans_dir / f"{frames:06d}.bin")
                writer.writerow((frames, f"{message.header.stamp.to_sec():.9f}"))
                total_points += len(xyzi)
                frames += 1
    if frames == 0:
        raise RuntimeError(f"no point clouds found on {args.topic}")
    manifest = {
        "schema_version": 1,
        "method": "ntu_viral_sensor_only_lidar_bin_extraction",
        "ground_truth_used": False,
        "topic": args.topic,
        "frames": frames,
        "total_points": total_points,
        "point_format": "little-endian float32 x y z intensity",
    }
    (args.output_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(manifest, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
