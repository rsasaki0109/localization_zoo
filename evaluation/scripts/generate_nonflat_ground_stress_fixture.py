#!/usr/bin/env python3
"""Generate a deterministic non-flat ground stress fixture for pcd_dogfooding."""

from __future__ import annotations

import argparse
import csv
import math
import shutil
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = REPO_ROOT / "evaluation" / "fixtures" / "nonflat_ground_stress"


def terrain_z(x: np.ndarray | float, y: np.ndarray | float) -> np.ndarray | float:
    return (
        0.22 * np.sin(0.085 * x)
        + 0.08 * np.sin(0.19 * y + 0.03 * x)
        + 0.0035 * x
    )


def write_binary_pcd(path: Path, points: np.ndarray) -> None:
    header = "\n".join(
        [
            "# .PCD v0.7 - Point Cloud Data file format",
            "VERSION 0.7",
            "FIELDS x y z intensity",
            "SIZE 4 4 4 4",
            "TYPE F F F F",
            "COUNT 1 1 1 1",
            f"WIDTH {len(points)}",
            "HEIGHT 1",
            "VIEWPOINT 0 0 0 1 0 0 0",
            f"POINTS {len(points)}",
            "DATA binary",
            "",
        ]
    ).encode("ascii")
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as handle:
        handle.write(header)
        handle.write(points.tobytes())


def world_points(length: float) -> np.ndarray:
    pts: list[tuple[float, float, float, float]] = []

    # Dense rolling ground with mild cross-slope and longitudinal ramp.
    for x in np.arange(-10.0, length + 70.0, 0.65):
        for y in np.arange(-12.0, 12.1, 0.65):
            z = float(terrain_z(x, y))
            pts.append((x, y, z, 0.25))

    # Low banks/curbs follow the terrain and create multiple ground-like patches.
    for x in np.arange(-10.0, length + 70.0, 0.55):
        for y, h in [(-6.2, 0.32), (-3.0, 0.16), (3.0, 0.16), (6.2, 0.32)]:
            pts.append((x, y, float(terrain_z(x, y)) + h, 0.45))

    # Static non-ground landmarks keep yaw/x-y observable without dominating z.
    for x in np.arange(7.0, length + 55.0, 9.0):
        for y in (-8.0, 8.0):
            base_z = float(terrain_z(x, y))
            for z in np.arange(base_z, base_z + 3.4, 0.35):
                for a in np.linspace(0.0, 2.0 * math.pi, 10, endpoint=False):
                    pts.append((x + 0.22 * math.cos(a), y + 0.22 * math.sin(a), z, 0.75))
    for x in np.arange(15.0, length + 55.0, 18.0):
        for y in (-10.5, 10.5):
            base_z = float(terrain_z(x, y))
            for z in np.arange(base_z + 0.2, base_z + 2.7, 0.5):
                for dx in np.arange(-1.1, 1.2, 0.35):
                    pts.append((x + dx, y, z, 0.65))

    return np.array(pts, dtype=np.float64)


def sensor_pose(frame: int, step: float, sensor_height: float) -> tuple[np.ndarray, float]:
    x = frame * step
    y = 1.2 * math.sin(0.035 * frame)
    z = float(terrain_z(x, y)) + sensor_height
    return np.array([x, y, z], dtype=np.float64), 0.018 * math.sin(0.04 * frame)


def rotation_yaw(yaw: float) -> np.ndarray:
    c = math.cos(yaw)
    s = math.sin(yaw)
    return np.array([[c, -s, 0.0], [s, c, 0.0], [0.0, 0.0, 1.0]], dtype=np.float64)


def sensor_cloud(static_pts: np.ndarray, frame: int, step: float, sensor_height: float) -> np.ndarray:
    sensor, yaw = sensor_pose(frame, step, sensor_height)
    rot = rotation_yaw(yaw)
    rel_xyz = (static_pts[:, :3] - sensor) @ rot
    rel = np.column_stack([rel_xyz, static_pts[:, 3]])
    ranges = np.linalg.norm(rel[:, :3], axis=1)
    mask = (
        (rel[:, 0] > 1.2)
        & (rel[:, 0] < 58.0)
        & (np.abs(rel[:, 1]) < 20.0)
        & (rel[:, 2] > -4.0)
        & (rel[:, 2] < 5.0)
        & (ranges > 1.0)
        & (ranges < 70.0)
    )
    points = rel[mask]
    order = np.lexsort((points[:, 2], points[:, 1], points[:, 0]))
    points = points[order]

    packed = np.empty(
        len(points),
        dtype=np.dtype(
            [("x", "<f4"), ("y", "<f4"), ("z", "<f4"), ("intensity", "<f4")]
        ),
    )
    packed["x"] = points[:, 0]
    packed["y"] = points[:, 1]
    packed["z"] = points[:, 2]
    packed["intensity"] = points[:, 3]
    return packed


def write_gt(path: Path, frames: int, dt: float, step: float, sensor_height: float) -> None:
    first_sensor, _ = sensor_pose(0, step, sensor_height)
    with path.open("w", newline="") as handle:
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
        for i in range(frames):
            sensor, yaw = sensor_pose(i, step, sensor_height)
            rel = sensor - first_sensor
            writer.writerow(
                [
                    f"{i * dt:.6f}",
                    f"{rel[0]:.6f}",
                    f"{rel[1]:.6f}",
                    f"{rel[2]:.6f}",
                    "0",
                    "0",
                    f"{yaw:.9f}",
                ]
            )


def write_timestamps(path: Path, frames: int, dt: float, counts: list[int]) -> None:
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["frame_idx", "timestamp", "points"])
        for i, count in enumerate(counts):
            writer.writerow([i, f"{i * dt:.6f}", count])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", default=str(DEFAULT_OUT))
    parser.add_argument("--frames", type=int, default=96)
    parser.add_argument("--dt", type=float, default=0.1)
    parser.add_argument("--step", type=float, default=0.36)
    parser.add_argument("--sensor-height", type=float, default=1.6)
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    out = Path(args.output_dir)
    if out.exists():
        if not args.force:
            raise SystemExit(f"{out} exists; pass --force to overwrite")
        shutil.rmtree(out)
    out.mkdir(parents=True)

    static_pts = world_points(args.frames * args.step)
    counts: list[int] = []
    for i in range(args.frames):
        cloud = sensor_cloud(static_pts, i, args.step, args.sensor_height)
        counts.append(len(cloud))
        write_binary_pcd(out / f"{i:08d}" / "cloud.pcd", cloud)

    write_gt(out / "gt.csv", args.frames, args.dt, args.step, args.sensor_height)
    write_timestamps(out / "frame_timestamps.csv", args.frames, args.dt, counts)
    print(f"wrote {args.frames} frames to {out}")
    print(f"points/frame min={min(counts)} mean={sum(counts) / len(counts):.1f} max={max(counts)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
