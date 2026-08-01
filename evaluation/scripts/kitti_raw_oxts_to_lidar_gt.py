#!/usr/bin/env python3
"""Convert KITTI Raw OXTS packets into initial-LiDAR-relative GT poses.

The odometry process must finish before this script is invoked.  The output
uses the Zoo CSV convention and matches scans by their native filename stem.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path

import numpy as np


EARTH_RADIUS_M = 6378137.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scan-dir", required=True)
    parser.add_argument("--oxts-dir", required=True)
    parser.add_argument("--calib-imu-to-velo", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--manifest")
    parser.add_argument("--source-archive")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def rotation_from_rpy(roll: float, pitch: float, yaw: float) -> np.ndarray:
    sr, cr = math.sin(roll), math.cos(roll)
    sp, cp = math.sin(pitch), math.cos(pitch)
    sy, cy = math.sin(yaw), math.cos(yaw)
    rx = np.array([[1, 0, 0], [0, cr, -sr], [0, sr, cr]], dtype=float)
    ry = np.array([[cp, 0, sp], [0, 1, 0], [-sp, 0, cp]], dtype=float)
    rz = np.array([[cy, -sy, 0], [sy, cy, 0], [0, 0, 1]], dtype=float)
    return rz @ ry @ rx


def rpy_from_rotation(rotation: np.ndarray) -> tuple[float, float, float]:
    pitch = math.asin(float(np.clip(-rotation[2, 0], -1.0, 1.0)))
    if abs(math.cos(pitch)) < 1e-9:
        roll = 0.0
        yaw = math.atan2(-rotation[0, 1], rotation[1, 1])
    else:
        roll = math.atan2(rotation[2, 1], rotation[2, 2])
        yaw = math.atan2(rotation[1, 0], rotation[0, 0])
    return roll, pitch, yaw


def parse_calibration(path: Path) -> np.ndarray:
    values: dict[str, list[float]] = {}
    for line in path.read_text().splitlines():
        if ":" not in line:
            continue
        key, raw = line.split(":", 1)
        key = key.strip()
        if key in {"R", "T"}:
            values[key] = [float(token) for token in raw.split()]
    if len(values.get("R", [])) != 9 or len(values.get("T", [])) != 3:
        raise RuntimeError(f"Missing R/T in {path}")
    transform = np.eye(4)
    transform[:3, :3] = np.asarray(values["R"]).reshape(3, 3)
    transform[:3, 3] = values["T"]
    return transform


def oxts_pose(values: list[float], scale: float) -> np.ndarray:
    if len(values) < 6:
        raise RuntimeError("OXTS packet must contain at least six values")
    lat, lon, alt, roll, pitch, yaw = values[:6]
    x = scale * lon * math.pi * EARTH_RADIUS_M / 180.0
    y = scale * EARTH_RADIUS_M * math.log(
        math.tan((90.0 + lat) * math.pi / 360.0)
    )
    pose = np.eye(4)
    pose[:3, :3] = rotation_from_rpy(roll, pitch, yaw)
    pose[:3, 3] = [x, y, alt]
    return pose


def build_lidar_poses(
    packets: list[list[float]], transform_velo_from_imu: np.ndarray
) -> list[np.ndarray]:
    if not packets:
        return []
    scale = math.cos(packets[0][0] * math.pi / 180.0)
    transform_imu_from_velo = np.linalg.inv(transform_velo_from_imu)
    world_from_velo = [
        oxts_pose(packet, scale) @ transform_imu_from_velo for packet in packets
    ]
    anchor = np.linalg.inv(world_from_velo[0])
    return [anchor @ pose for pose in world_from_velo]


def main() -> int:
    args = parse_args()
    scan_dir = Path(args.scan_dir).resolve()
    oxts_dir = Path(args.oxts_dir).resolve()
    calibration_path = Path(args.calib_imu_to_velo).resolve()
    output_path = Path(args.output).resolve()

    scan_paths = sorted(scan_dir.glob("*.bin"))
    if not scan_paths:
        raise RuntimeError(f"No scans in {scan_dir}")
    native_indices = [path.stem for path in scan_paths]
    oxts_paths = [oxts_dir / f"{index}.txt" for index in native_indices]
    missing = [str(path) for path in oxts_paths if not path.is_file()]
    if missing:
        raise RuntimeError(f"Missing {len(missing)} matching OXTS packets")

    packets = [list(map(float, path.read_text().split())) for path in oxts_paths]
    poses = build_lidar_poses(packets, parse_calibration(calibration_path))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle, lineterminator="\n")
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
        for index, pose in enumerate(poses):
            roll, pitch, yaw = rpy_from_rotation(pose[:3, :3])
            writer.writerow(
                [index, *[f"{value:.12g}" for value in pose[:3, 3]], roll, pitch, yaw]
            )

    payload = {
        "schema_version": 1,
        "dataset": "KITTI Raw",
        "frames": len(poses),
        "native_first": native_indices[0],
        "native_last": native_indices[-1],
        "pose_frame": "initial_velodyne",
        "calibration_direction": "T_velo_imu inverted to compose T_world_velo",
        "projection": "KITTI Mercator with scale cos(first latitude)",
        "output": str(output_path),
        "output_sha256": sha256_file(output_path),
        "calibration": str(calibration_path),
        "calibration_sha256": sha256_file(calibration_path),
    }
    if args.source_archive:
        archive = Path(args.source_archive).resolve()
        payload["source_archive"] = str(archive)
        payload["source_archive_sha256"] = sha256_file(archive)
    if args.manifest:
        manifest_path = Path(args.manifest).resolve()
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
