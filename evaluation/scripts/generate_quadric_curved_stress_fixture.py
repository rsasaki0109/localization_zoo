#!/usr/bin/env python3
"""Generate a deterministic curved-object/non-urban stress fixture for Quadric-LO."""

from __future__ import annotations

import argparse
import csv
import math
import shutil
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = REPO_ROOT / "evaluation" / "fixtures" / "quadric_curved_stress"


def terrain_z(x: np.ndarray | float, y: np.ndarray | float) -> np.ndarray | float:
    return 0.08 * np.sin(0.11 * x) + 0.06 * np.sin(0.17 * y + 0.02 * x)


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


def cylinder(cx: float, cy: float, radius: float, height: float, intensity: float) -> list[tuple[float, float, float, float]]:
    base = float(terrain_z(cx, cy))
    pts: list[tuple[float, float, float, float]] = []
    for z in np.arange(base, base + height, 0.18):
        for a in np.linspace(0.0, 2.0 * math.pi, 30, endpoint=False):
            wobble = 1.0 + 0.06 * math.sin(3.0 * a + 1.7 * z)
            pts.append((cx + radius * wobble * math.cos(a), cy + radius * wobble * math.sin(a), z, intensity))
    return pts


def ellipsoid(cx: float, cy: float, cz: float, rx: float, ry: float, rz: float, intensity: float) -> list[tuple[float, float, float, float]]:
    pts: list[tuple[float, float, float, float]] = []
    for u in np.linspace(0.10, math.pi - 0.10, 16):
        for v in np.linspace(0.0, 2.0 * math.pi, 28, endpoint=False):
            pts.append(
                (
                    cx + rx * math.sin(u) * math.cos(v),
                    cy + ry * math.sin(u) * math.sin(v),
                    cz + rz * math.cos(u),
                    intensity,
                )
            )
    return pts


def world_points(length: float) -> np.ndarray:
    pts: list[tuple[float, float, float, float]] = []

    # Sparse rolling ground so the scene is non-urban but not plane dominated.
    for x in np.arange(-8.0, length + 70.0, 1.1):
        for y in np.arange(-13.0, 13.1, 1.1):
            pts.append((x, y, float(terrain_z(x, y)), 0.20))

    # Curved trunks arranged irregularly on both sides of the path.
    for i, x in enumerate(np.arange(5.0, length + 58.0, 5.5)):
        for side in (-1.0, 1.0):
            y = side * (4.2 + 3.2 * ((i * 37) % 5) / 4.0)
            r = 0.22 + 0.07 * ((i * 17) % 4) / 3.0
            h = 3.2 + 0.7 * ((i * 13) % 3)
            pts.extend(cylinder(float(x), float(y), r, h, 0.72))

    # Boulders/curved shrubs near the path create non-cylindrical quadrics.
    for i, x in enumerate(np.arange(9.0, length + 54.0, 8.0)):
        y = (-1.0 if i % 2 == 0 else 1.0) * (2.0 + 0.8 * (i % 3))
        z = float(terrain_z(x, y)) + 0.55
        pts.extend(ellipsoid(float(x), float(y), z, 0.9, 0.65, 0.55, 0.55))

    return np.array(pts, dtype=np.float64)


def pose(frame: int, step: float, sensor_height: float) -> tuple[np.ndarray, float]:
    x = frame * step
    y = 0.85 * math.sin(0.055 * frame)
    yaw = 0.025 * math.sin(0.045 * frame)
    z = float(terrain_z(x, y)) + sensor_height
    return np.array([x, y, z], dtype=np.float64), yaw


def rotation_yaw(yaw: float) -> np.ndarray:
    c = math.cos(yaw)
    s = math.sin(yaw)
    return np.array([[c, -s, 0.0], [s, c, 0.0], [0.0, 0.0, 1.0]], dtype=np.float64)


def sensor_cloud(static_pts: np.ndarray, frame: int, step: float, sensor_height: float) -> np.ndarray:
    sensor, yaw = pose(frame, step, sensor_height)
    rel_xyz = (static_pts[:, :3] - sensor) @ rotation_yaw(yaw)
    rel = np.column_stack([rel_xyz, static_pts[:, 3]])
    ranges = np.linalg.norm(rel[:, :3], axis=1)
    mask = (
        (rel[:, 0] > 1.0)
        & (rel[:, 0] < 52.0)
        & (np.abs(rel[:, 1]) < 18.0)
        & (rel[:, 2] > -3.5)
        & (rel[:, 2] < 5.5)
        & (ranges > 1.0)
        & (ranges < 65.0)
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
    first_sensor, _ = pose(0, step, sensor_height)
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
            sensor, yaw = pose(i, step, sensor_height)
            rel = sensor - first_sensor
            writer.writerow([f"{i * dt:.6f}", f"{rel[0]:.6f}", f"{rel[1]:.6f}", f"{rel[2]:.6f}", "0", "0", f"{yaw:.9f}"])


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
    parser.add_argument("--step", type=float, default=0.34)
    parser.add_argument("--sensor-height", type=float, default=1.5)
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
