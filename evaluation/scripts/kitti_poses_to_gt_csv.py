#!/usr/bin/env python3

"""Convert KITTI poses.txt (12 float/line = 3x4 matrix) into pcd_dogfooding GT CSV."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert KITTI poses.txt into timestamp,lidar_pose.* CSV.",
    )
    parser.add_argument("--poses-file", required=True, help="KITTI poses.txt path.")
    parser.add_argument("--output", required=True, help="Output GT CSV path.")
    parser.add_argument(
        "--calib-file",
        default="",
        help=(
            "KITTI sequence calib.txt path. When provided, the Tr row (LiDAR-to-cam0 "
            "transform) is applied so the exported pose is the LiDAR pose in the "
            "cam0[0] world frame: T_world_lidar = poses[i] * Tr. Required for "
            "paper-comparable pure-odometry on KITTI Odometry data; without it, the "
            "tool's lidar-frame points are aligned against cam-frame GT poses and "
            "ICP cannot reconcile the constant ~90 deg rotation offset under the "
            "refinement-acceptance gate."
        ),
    )
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


def load_calib_tr(calib_path: Path) -> np.ndarray:
    with calib_path.open() as f:
        for line in f:
            if line.startswith("Tr:") or line.startswith("Tr "):
                vals = [float(v) for v in line.split()[1:]]
                if len(vals) != 12:
                    raise ValueError(f"Tr row in {calib_path} has {len(vals)} floats, expected 12.")
                Tr = np.eye(4)
                Tr[0, :4] = vals[0:4]
                Tr[1, :4] = vals[4:8]
                Tr[2, :4] = vals[8:12]
                return Tr
    raise ValueError(f"No Tr row found in {calib_path}.")


def rotation_matrix_to_rpy(R: np.ndarray) -> tuple[float, float, float]:
    """Decompose rotation matrix to roll, pitch, yaw (ZYX Euler convention).

    Matches pcd_dogfooding.cpp: pose = rz * ry * rx
    and pose_trace_to_gt_csv.py quat_to_rpy convention.
    """
    # pitch from R[2,0]
    sinp = -R[2, 0]
    if abs(sinp) >= 1.0:
        pitch = math.copysign(math.pi / 2.0, sinp)
        # gimbal lock: roll = 0, yaw absorbs remaining rotation
        roll = 0.0
        yaw = math.atan2(-R[0, 1], R[0, 2])
    else:
        pitch = math.asin(sinp)
        roll = math.atan2(R[2, 1], R[2, 2])
        yaw = math.atan2(R[1, 0], R[0, 0])
    return roll, pitch, yaw


def main() -> int:
    args = parse_args()
    poses_path = Path(args.poses_file)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Load all poses (KITTI poses.txt is cam0-frame: T_world_cam0)
    all_poses = []
    with poses_path.open() as f:
        for line in f:
            vals = list(map(float, line.strip().split()))
            if len(vals) != 12:
                continue
            T = np.eye(4)
            T[0, :4] = vals[0:4]
            T[1, :4] = vals[4:8]
            T[2, :4] = vals[8:12]
            all_poses.append(T)

    # Optionally apply calib Tr to convert poses to LiDAR frame:
    # T_world_lidar[i] = T_world_cam0[i] * Tr (where Tr = T_cam0_lidar).
    if args.calib_file:
        Tr = load_calib_tr(Path(args.calib_file))
        all_poses = [T @ Tr for T in all_poses]

    if not all_poses:
        raise RuntimeError(f"No poses found in {poses_path}")

    end_frame = len(all_poses)
    if args.max_frames >= 0:
        end_frame = min(args.start_frame + args.max_frames, len(all_poses))

    selected = all_poses[args.start_frame:end_frame]
    if not selected:
        raise RuntimeError(f"No poses in range [{args.start_frame}, {end_frame})")

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
        for i, T in enumerate(selected):
            global_idx = args.start_frame + i
            R = T[:3, :3]
            t = T[:3, 3]
            roll, pitch, yaw = rotation_matrix_to_rpy(R)
            writer.writerow([
                f"{global_idx}",
                f"{t[0]:.10f}",
                f"{t[1]:.10f}",
                f"{t[2]:.10f}",
                f"{roll:.10f}",
                f"{pitch:.10f}",
                f"{yaw:.10f}",
            ])

    print(f"[done] wrote {output_path}")
    print(f"[done] poses={len(selected)} frame_range=[{args.start_frame}, {args.start_frame + len(selected)})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
