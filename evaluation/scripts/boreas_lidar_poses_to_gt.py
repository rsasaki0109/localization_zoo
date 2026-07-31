#!/usr/bin/env python3
"""Align official Boreas lidar poses to a frozen scan window and write Zoo GT CSV."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lidar-poses", required=True)
    parser.add_argument("--frame-timestamps", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args()


def boreas_rotation(roll: float, pitch: float, yaw: float) -> np.ndarray:
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    roll_matrix = np.array([[1, 0, 0], [0, cr, sr], [0, -sr, cr]])
    pitch_matrix = np.array([[cp, 0, -sp], [0, 1, 0], [sp, 0, cp]])
    yaw_matrix = np.array([[cy, sy, 0], [-sy, cy, 0], [0, 0, 1]])
    return roll_matrix @ pitch_matrix @ yaw_matrix


def rotation_matrix_to_rpy(rotation: np.ndarray) -> tuple[float, float, float]:
    sin_pitch = -float(rotation[2, 0])
    if abs(sin_pitch) >= 1.0:
        pitch = math.copysign(math.pi / 2.0, sin_pitch)
        roll = math.atan2(-rotation[0, 1], rotation[1, 1])
        yaw = 0.0
    else:
        pitch = math.asin(sin_pitch)
        roll = math.atan2(rotation[2, 1], rotation[2, 2])
        yaw = math.atan2(rotation[1, 0], rotation[0, 0])
    return roll, pitch, yaw


def load_selected_timestamps(path: Path) -> list[int]:
    with path.open(newline="", encoding="utf-8") as handle:
        return [int(round(float(row["timestamp"]) * 1e6)) for row in csv.DictReader(handle)]


def load_lidar_poses(path: Path) -> dict[int, tuple[float, ...]]:
    poses: dict[int, tuple[float, ...]] = {}
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.reader(handle)
        next(reader, None)
        for line_number, row in enumerate(reader, start=2):
            if len(row) < 13:
                raise RuntimeError(f"{path}:{line_number}: expected 13 columns")
            values = tuple(float(value) for value in row[:13])
            poses[int(round(values[0]))] = values
    return poses


def main() -> int:
    args = parse_args()
    selected = load_selected_timestamps(Path(args.frame_timestamps))
    poses = load_lidar_poses(Path(args.lidar_poses))
    missing = [timestamp for timestamp in selected if timestamp not in poses]
    if missing:
        raise RuntimeError(f"Missing {len(missing)} selected poses; first={missing[0]}")
    output = Path(args.output).resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["timestamp", "lidar_pose.x", "lidar_pose.y", "lidar_pose.z", "lidar_pose.roll", "lidar_pose.pitch", "lidar_pose.yaw"])
        for timestamp in selected:
            _, x, y, z, _, _, _, roll, pitch, yaw, _, _, _ = poses[timestamp]
            converted = rotation_matrix_to_rpy(boreas_rotation(roll, pitch, yaw))
            writer.writerow([f"{timestamp * 1e-6:.6f}", f"{x:.12g}", f"{y:.12g}", f"{z:.12g}", *(f"{value:.12g}" for value in converted)])
    print(f"Wrote {len(selected)} Boreas poses to {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
