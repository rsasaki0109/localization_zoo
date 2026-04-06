#!/usr/bin/env python3

"""Convert MCD pose_inW.csv (num,t,x,y,z,qx,qy,qz,qw) into pcd_dogfooding GT CSV."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert MCD pose_inW.csv into timestamp,lidar_pose.* CSV.",
    )
    parser.add_argument("--input", required=True, help="MCD pose_inW.csv path.")
    parser.add_argument("--output", required=True, help="Output GT CSV path.")
    parser.add_argument(
        "--frame-timestamps",
        default="",
        help="Optional frame_timestamps.csv to align GT poses with extracted PCD frames.",
    )
    parser.add_argument(
        "--start-index", type=int, default=0,
        help="First pose index to export (default: 0).",
    )
    parser.add_argument(
        "--max-frames", type=int, default=-1,
        help="Maximum poses to export (-1 for all).",
    )
    return parser.parse_args()


def quat_to_rpy(qx: float, qy: float, qz: float, qw: float) -> tuple[float, float, float]:
    """Quaternion to roll/pitch/yaw (ZYX Euler), matching pose_trace_to_gt_csv.py."""
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

    # Load MCD GT poses
    with input_path.open() as f:
        reader = csv.DictReader(f)
        all_poses = list(reader)

    if not all_poses:
        raise RuntimeError(f"No poses in {input_path}")

    # If frame_timestamps provided, align GT to extracted frames by nearest timestamp
    if args.frame_timestamps:
        ts_path = Path(args.frame_timestamps)
        with ts_path.open() as f:
            ts_reader = csv.DictReader(f)
            frame_timestamps = [float(row["timestamp"]) for row in ts_reader]

        gt_timestamps = [float(p["t"]) for p in all_poses]
        selected_poses = []
        for ft in frame_timestamps:
            # Find nearest GT pose
            best_idx = min(range(len(gt_timestamps)), key=lambda i: abs(gt_timestamps[i] - ft))
            selected_poses.append(all_poses[best_idx])
    else:
        end = len(all_poses)
        if args.max_frames >= 0:
            end = min(args.start_index + args.max_frames, len(all_poses))
        selected_poses = all_poses[args.start_index:end]

    if not selected_poses:
        raise RuntimeError("No poses selected after filtering.")

    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "timestamp",
            "lidar_pose.x",
            "lidar_pose.y",
            "lidar_pose.z",
            "lidar_pose.roll",
            "lidar_pose.pitch",
            "lidar_pose.yaw",
        ])
        for pose in selected_poses:
            t = float(pose["t"])
            x, y, z = float(pose["x"]), float(pose["y"]), float(pose["z"])
            qx, qy, qz, qw = float(pose["qx"]), float(pose["qy"]), float(pose["qz"]), float(pose["qw"])
            roll, pitch, yaw = quat_to_rpy(qx, qy, qz, qw)
            writer.writerow([
                f"{t:.9f}",
                f"{x:.10f}",
                f"{y:.10f}",
                f"{z:.10f}",
                f"{roll:.10f}",
                f"{pitch:.10f}",
                f"{yaw:.10f}",
            ])

    print(f"[done] wrote {output_path} ({len(selected_poses)} poses)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
