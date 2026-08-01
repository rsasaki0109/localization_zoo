#!/usr/bin/env python3
"""Convert KITTI T_W_L poses to timestamped T_W_B TUM poses."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--poses", type=Path, required=True)
    parser.add_argument("--timestamps", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--body-to-lidar-translation", default="-0.05,0,0.055")
    return parser.parse_args()


def rotation_to_quaternion(rotation: np.ndarray) -> tuple[float, float, float, float]:
    trace = float(np.trace(rotation))
    if trace > 0.0:
        scale = math.sqrt(trace + 1.0) * 2.0
        qw = 0.25 * scale
        qx = (rotation[2, 1] - rotation[1, 2]) / scale
        qy = (rotation[0, 2] - rotation[2, 0]) / scale
        qz = (rotation[1, 0] - rotation[0, 1]) / scale
    else:
        diagonal = np.diag(rotation)
        index = int(np.argmax(diagonal))
        if index == 0:
            scale = math.sqrt(1.0 + rotation[0, 0] - rotation[1, 1] - rotation[2, 2]) * 2.0
            qw = (rotation[2, 1] - rotation[1, 2]) / scale
            qx = 0.25 * scale
            qy = (rotation[0, 1] + rotation[1, 0]) / scale
            qz = (rotation[0, 2] + rotation[2, 0]) / scale
        elif index == 1:
            scale = math.sqrt(1.0 + rotation[1, 1] - rotation[0, 0] - rotation[2, 2]) * 2.0
            qw = (rotation[0, 2] - rotation[2, 0]) / scale
            qx = (rotation[0, 1] + rotation[1, 0]) / scale
            qy = 0.25 * scale
            qz = (rotation[1, 2] + rotation[2, 1]) / scale
        else:
            scale = math.sqrt(1.0 + rotation[2, 2] - rotation[0, 0] - rotation[1, 1]) * 2.0
            qw = (rotation[1, 0] - rotation[0, 1]) / scale
            qx = (rotation[0, 2] + rotation[2, 0]) / scale
            qy = (rotation[1, 2] + rotation[2, 1]) / scale
            qz = 0.25 * scale
    quaternion = np.asarray([qx, qy, qz, qw])
    quaternion /= np.linalg.norm(quaternion)
    return tuple(float(value) for value in quaternion)


def lidar_to_body_pose(pose: np.ndarray, body_to_lidar: np.ndarray) -> np.ndarray:
    result = pose.copy()
    result[:3, 3] = pose[:3, 3] - pose[:3, :3] @ body_to_lidar
    return result


def main() -> int:
    args = parse_args()
    translation = np.asarray(
        [float(value) for value in args.body_to_lidar_translation.split(",")]
    )
    if translation.shape != (3,):
        raise ValueError("body-to-lidar translation must contain three values")
    poses: list[np.ndarray] = []
    with args.poses.open(encoding="utf-8") as stream:
        for line_number, raw in enumerate(stream, start=1):
            values = [float(token) for token in raw.split()]
            if len(values) != 12:
                raise ValueError(f"{args.poses}:{line_number}: expected 12 values")
            pose = np.eye(4)
            pose[:3, :] = np.asarray(values).reshape(3, 4)
            poses.append(lidar_to_body_pose(pose, translation))
    with args.timestamps.open(encoding="utf-8") as stream:
        timestamps = [float(row["timestamp"]) for row in csv.DictReader(stream)]
    if len(poses) != len(timestamps):
        raise ValueError("pose and timestamp counts must match")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8") as stream:
        for stamp, pose in zip(timestamps, poses):
            qx, qy, qz, qw = rotation_to_quaternion(pose[:3, :3])
            x, y, z = pose[:3, 3]
            stream.write(
                f"{stamp:.9f} {x:.15g} {y:.15g} {z:.15g} "
                f"{qx:.15g} {qy:.15g} {qz:.15g} {qw:.15g}\n"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
