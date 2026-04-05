#!/usr/bin/env python3

"""Convert public pose_trace.csv exports into pcd_dogfooding trajectory CSVs."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert pose_trace.csv into timestamp,lidar_pose.* CSV.",
    )
    parser.add_argument("--input", required=True, help="Input pose_trace.csv path.")
    parser.add_argument("--output", required=True, help="Output CSV path.")
    parser.add_argument(
        "--timestamp-window-sec",
        type=float,
        default=3600.0,
        help="Keep rows whose timestamps stay within this window around the median timestamp.",
    )
    parser.add_argument(
        "--min-index",
        type=int,
        default=1,
        help="Minimum message_index to keep. The default skips the common zero-initialized first row.",
    )
    return parser.parse_args()


def quat_to_rpy(qx: float, qy: float, qz: float, qw: float) -> tuple[float, float, float]:
    sinr_cosp = 2.0 * (qw * qx + qy * qz)
    cosr_cosp = 1.0 - 2.0 * (qx * qx + qy * qy)
    roll = math.atan2(sinr_cosp, cosr_cosp)

    sinp = 2.0 * (qw * qy - qz * qx)
    if abs(sinp) >= 1.0:
        pitch = math.copysign(math.pi / 2.0, sinp)
    else:
        pitch = math.asin(sinp)

    siny_cosp = 2.0 * (qw * qz + qx * qy)
    cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz)
    yaw = math.atan2(siny_cosp, cosy_cosp)
    return roll, pitch, yaw


def main() -> int:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with input_path.open() as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)

    if not rows:
        raise RuntimeError(f"No rows found in {input_path}")

    filtered = [
        row for row in rows if int(float(row["message_index"])) >= args.min_index
    ]
    if not filtered:
        raise RuntimeError("All rows were filtered out by min-index.")

    timestamps = sorted(float(row["stamp_sec"]) for row in filtered)
    median_ts = timestamps[len(timestamps) // 2]
    window = float(args.timestamp_window_sec)
    filtered = [
        row
        for row in filtered
        if abs(float(row["stamp_sec"]) - median_ts) <= window
    ]
    if not filtered:
        raise RuntimeError("All rows were filtered out by the timestamp window.")

    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
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
        for row in filtered:
            roll, pitch, yaw = quat_to_rpy(
                float(row["orientation_x"]),
                float(row["orientation_y"]),
                float(row["orientation_z"]),
                float(row["orientation_w"]),
            )
            writer.writerow(
                [
                    f"{float(row['stamp_sec']):.9f}",
                    f"{float(row['position_x']):.10f}",
                    f"{float(row['position_y']):.10f}",
                    f"{float(row['position_z']):.10f}",
                    f"{roll:.10f}",
                    f"{pitch:.10f}",
                    f"{yaw:.10f}",
                ]
            )

    print(f"[done] wrote {output_path}")
    print(f"[done] kept_rows={len(filtered)} median_ts={median_ts:.9f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
