#!/usr/bin/env python3
"""Convert Boreas [x,y,z,i,r,t] float32 scans to timestamped binary PCD."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sequence-dir", required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--identity-reference", required=True)
    return parser.parse_args()


def load_boreas_scan(path: Path) -> np.ndarray:
    values = np.fromfile(path, dtype=np.float32)
    if values.size == 0 or values.size % 6:
        raise RuntimeError(f"Invalid Boreas scan: {path}")
    return values.reshape((-1, 6))


def write_pcd(path: Path, points: np.ndarray) -> None:
    selected = np.ascontiguousarray(points[:, [0, 1, 2, 5]], dtype=np.float32)
    header = (
        "# .PCD v0.7 - Point Cloud Data file format\n"
        "VERSION 0.7\n"
        "FIELDS x y z time\n"
        "SIZE 4 4 4 4\n"
        "TYPE F F F F\n"
        "COUNT 1 1 1 1\n"
        f"WIDTH {selected.shape[0]}\n"
        "HEIGHT 1\n"
        "VIEWPOINT 0 0 0 1 0 0 0\n"
        f"POINTS {selected.shape[0]}\n"
        "DATA binary\n"
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as handle:
        handle.write(header.encode("ascii"))
        handle.write(selected.tobytes())


def main() -> int:
    args = parse_args()
    sequence_dir = Path(args.sequence_dir).resolve()
    scans = sorted((sequence_dir / "lidar").glob("*.bin"), key=lambda path: int(path.stem))
    if not scans:
        raise RuntimeError(f"No Boreas scans in {sequence_dir / 'lidar'}")
    output_dir = Path(args.output_dir).resolve()
    identity_path = Path(args.identity_reference).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    total_points = 0
    timestamps: list[int] = []
    for index, source in enumerate(scans):
        points = load_boreas_scan(source)
        write_pcd(output_dir / f"{index:08d}" / "cloud.pcd", points)
        total_points += points.shape[0]
        timestamps.append(int(source.stem))
        if index % 50 == 0:
            print(f"[boreas-pcd] {index}/{len(scans)}", flush=True)
    with (output_dir / "frame_timestamps.csv").open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["frame_idx", "timestamp", "points"])
        for index, timestamp in enumerate(timestamps):
            writer.writerow([index, f"{timestamp * 1e-6:.6f}", ""])
    identity_path.parent.mkdir(parents=True, exist_ok=True)
    with identity_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["timestamp", "lidar_pose.x", "lidar_pose.y", "lidar_pose.z", "lidar_pose.roll", "lidar_pose.pitch", "lidar_pose.yaw"])
        for timestamp in timestamps:
            writer.writerow([f"{timestamp * 1e-6:.6f}", 0, 0, 0, 0, 0, 0])
    payload = {
        "schema_version": 1,
        "source": str(sequence_dir),
        "output": str(output_dir),
        "frames": len(scans),
        "points": total_points,
        "timestamp_range_us": [timestamps[0], timestamps[-1]],
        "pcd_fields": ["x", "y", "z", "time"],
        "ground_truth_used": False,
    }
    (output_dir / "conversion_manifest.json").write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
