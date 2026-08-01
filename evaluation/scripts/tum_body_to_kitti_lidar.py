#!/usr/bin/env python3
"""Resample TUM T_W_B poses at LiDAR stamps and write KITTI T_W_L poses."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trajectory", type=Path, required=True)
    parser.add_argument("--timestamps", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--body-to-lidar-translation", default="-0.05,0,0.055")
    parser.add_argument(
        "--body-to-lidar-rotation",
        default="1,0,0,0,1,0,0,0,1",
        help="row-major R_B_L from LiDAR coordinates into body coordinates",
    )
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def quaternion_to_rotation(quaternion: np.ndarray) -> np.ndarray:
    x, y, z, w = quaternion / np.linalg.norm(quaternion)
    return np.asarray(
        [
            [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
        ]
    )


def slerp(left: np.ndarray, right: np.ndarray, fraction: float) -> np.ndarray:
    left = left / np.linalg.norm(left)
    right = right / np.linalg.norm(right)
    dot = float(np.dot(left, right))
    if dot < 0.0:
        right = -right
        dot = -dot
    dot = float(np.clip(dot, -1.0, 1.0))
    if dot > 0.9995:
        result = left + fraction * (right - left)
        return result / np.linalg.norm(result)
    angle = np.arccos(dot)
    return (
        np.sin((1.0 - fraction) * angle) * left
        + np.sin(fraction * angle) * right
    ) / np.sin(angle)


def interpolate_pose(
    stamp: float,
    source_stamps: np.ndarray,
    positions: np.ndarray,
    quaternions: np.ndarray,
) -> tuple[np.ndarray, bool]:
    if stamp <= source_stamps[0]:
        return make_pose(positions[0], quaternions[0]), stamp < source_stamps[0]
    if stamp >= source_stamps[-1]:
        return make_pose(positions[-1], quaternions[-1]), stamp > source_stamps[-1]
    right = int(np.searchsorted(source_stamps, stamp, side="right"))
    left = right - 1
    fraction = (stamp - source_stamps[left]) / (source_stamps[right] - source_stamps[left])
    position = positions[left] + fraction * (positions[right] - positions[left])
    quaternion = slerp(quaternions[left], quaternions[right], float(fraction))
    return make_pose(position, quaternion), False


def make_pose(position: np.ndarray, quaternion: np.ndarray) -> np.ndarray:
    pose = np.eye(4)
    pose[:3, :3] = quaternion_to_rotation(quaternion)
    pose[:3, 3] = position
    return pose


def body_to_lidar_pose(
    pose: np.ndarray,
    body_to_lidar: np.ndarray,
    body_to_lidar_rotation: np.ndarray | None = None,
) -> np.ndarray:
    extrinsic = np.eye(4)
    extrinsic[:3, :3] = (
        np.eye(3) if body_to_lidar_rotation is None else body_to_lidar_rotation
    )
    extrinsic[:3, 3] = body_to_lidar
    return pose @ extrinsic


def main() -> int:
    args = parse_args()
    body_to_lidar = np.asarray(
        [float(value) for value in args.body_to_lidar_translation.split(",")]
    )
    if body_to_lidar.shape != (3,):
        raise ValueError("body-to-lidar translation must contain three values")
    body_to_lidar_rotation = np.asarray(
        [float(value) for value in args.body_to_lidar_rotation.split(",")]
    )
    if body_to_lidar_rotation.shape != (9,):
        raise ValueError("body-to-lidar rotation must contain nine values")
    body_to_lidar_rotation = body_to_lidar_rotation.reshape(3, 3)
    rows = []
    with args.trajectory.open(encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, start=1):
            values = [float(token) for token in line.split()]
            if len(values) != 8:
                raise ValueError(f"{args.trajectory}:{line_number}: expected 8 values")
            rows.append(values)
    source = np.asarray(rows)
    if len(source) < 2 or np.any(np.diff(source[:, 0]) <= 0):
        raise ValueError("trajectory timestamps must be strictly increasing")
    with args.timestamps.open(encoding="utf-8") as stream:
        target_stamps = [float(row["timestamp"]) for row in csv.DictReader(stream)]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    boundary_holds = 0
    with args.output.open("w", encoding="utf-8") as stream:
        for stamp in target_stamps:
            pose, held = interpolate_pose(
                stamp, source[:, 0], source[:, 1:4], source[:, 4:8]
            )
            boundary_holds += int(held)
            pose = body_to_lidar_pose(
                pose, body_to_lidar, body_to_lidar_rotation
            )
            stream.write(" ".join(f"{value:.15g}" for value in pose[:3, :].reshape(-1)))
            stream.write("\n")
    manifest = {
        "schema_version": 1,
        "method": "tum_body_to_kitti_lidar_timestamp_resample",
        "ground_truth_used": False,
        "source_poses": len(source),
        "output_poses": len(target_stamps),
        "boundary_hold_poses": boundary_holds,
        "interpolation": "linear translation and quaternion SLERP; nearest boundary hold",
        "body_to_lidar_translation": body_to_lidar.tolist(),
        "body_to_lidar_rotation": body_to_lidar_rotation.tolist(),
        "source_sha256": sha256(args.trajectory),
        "output_sha256": sha256(args.output),
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
