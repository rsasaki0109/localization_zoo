#!/usr/bin/env python3
"""Extract GT-free KITTI-style scan bins from compact NTU VIRAL LiDAR data."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
from pathlib import Path

import numpy as np


SOURCE_POINT_DTYPE = np.dtype(
    [("x", "<f4"), ("y", "<f4"), ("z", "<f4"), ("intensity", "<f4"), ("t_ms", "<f4")]
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lidar-bin", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main() -> int:
    args = parse_args()
    scan_dir = args.output_dir / "velodyne"
    scan_dir.mkdir(parents=True, exist_ok=True)
    timestamps: list[float] = []
    total_points = 0
    with args.lidar_bin.open("rb") as stream:
        frame = 0
        while True:
            header = stream.read(16)
            if not header:
                break
            if len(header) != 16:
                raise ValueError("short LiDAR frame header")
            stamp, count, _ = struct.unpack("<dII", header)
            payload = stream.read(count * SOURCE_POINT_DTYPE.itemsize)
            if len(payload) != count * SOURCE_POINT_DTYPE.itemsize:
                raise ValueError("short LiDAR frame payload")
            source = np.frombuffer(payload, dtype=SOURCE_POINT_DTYPE)
            output = np.column_stack(
                (source["x"], source["y"], source["z"], source["intensity"])
            ).astype("<f4", copy=False)
            output.tofile(scan_dir / f"{frame:06d}.bin")
            timestamps.append(stamp)
            total_points += count
            frame += 1
    with (args.output_dir / "timestamps.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(["frame_id", "timestamp"])
        for frame, stamp in enumerate(timestamps):
            writer.writerow([frame, f"{stamp:.9f}"])
    manifest = {
        "schema_version": 1,
        "method": "ntu_viral_compact_lidar_to_kitti_bins",
        "ground_truth_used": False,
        "frames": len(timestamps),
        "total_points": total_points,
        "source_sha256": sha256(args.lidar_bin),
        "point_layout": "x y z intensity float32; per-point t_ms omitted for scan registration",
    }
    (args.output_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
