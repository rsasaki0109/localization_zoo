#!/usr/bin/env python3

"""Convert reference pose CSV into pcd_dogfooding ground-truth format."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, help="Input reference pose CSV.")
    parser.add_argument("--output", required=True, help="Output GT CSV path.")
    return parser.parse_args()


def quaternion_to_rpy(x: float, y: float, z: float, w: float):
    sinr_cosp = 2.0 * (w * x + y * z)
    cosr_cosp = 1.0 - 2.0 * (x * x + y * y)
    roll = math.atan2(sinr_cosp, cosr_cosp)

    sinp = 2.0 * (w * y - z * x)
    if abs(sinp) >= 1.0:
        pitch = math.copysign(math.pi / 2.0, sinp)
    else:
        pitch = math.asin(sinp)

    siny_cosp = 2.0 * (w * z + x * y)
    cosy_cosp = 1.0 - 2.0 * (y * y + z * z)
    yaw = math.atan2(siny_cosp, cosy_cosp)
    return roll, pitch, yaw


def main() -> int:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with input_path.open() as src, output_path.open("w", newline="") as dst:
        reader = csv.DictReader(src)
        writer = csv.writer(dst)
        writer.writerow(
            [
                "timestamp",
                "lidar_pose.x",
                "lidar_pose.y",
                "lidar_pose.z",
                "lidar_pose.roll",
                "lidar_pose.pitch",
                "lidar_pose.yaw",
            ]
        )

        for row in reader:
            roll, pitch, yaw = quaternion_to_rpy(
                float(row["orientation_x"]),
                float(row["orientation_y"]),
                float(row["orientation_z"]),
                float(row["orientation_w"]),
            )
            writer.writerow(
                [
                    row["stamp_sec"],
                    row["position_x"],
                    row["position_y"],
                    row["position_z"],
                    f"{roll:.12f}",
                    f"{pitch:.12f}",
                    f"{yaw:.12f}",
                ]
            )

    print(f"[done] output={output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
