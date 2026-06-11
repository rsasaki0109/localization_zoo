#!/usr/bin/env python3
"""Generate a deterministic 3D dynamic-object stress fixture for pcd_dogfooding.

The scene is intentionally small and dataset-free: a forward-moving LiDAR sees a
static corridor while two box-shaped foreground objects cross in front of the
static background. GT motion is ego motion only; dynamic objects are observation
outliers that should exercise ID-LIO/RF-LIO dynamic filtering.
"""

from __future__ import annotations

import argparse
import csv
import math
import shutil
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = REPO_ROOT / "evaluation" / "fixtures" / "dynamic_object_stress"


def write_binary_pcd(path: Path, points: np.ndarray) -> None:
    field_names = ["x", "y", "z", "intensity"]
    header = "\n".join(
        [
            "# .PCD v0.7 - Point Cloud Data file format",
            "VERSION 0.7",
            f"FIELDS {' '.join(field_names)}",
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


def static_world_points(length: float) -> np.ndarray:
    pts: list[tuple[float, float, float, float]] = []

    # Side walls with vertical texture.
    for x in np.arange(-8.0, length + 65.0, 0.85):
        for y in (-9.0, 9.0):
            for z in np.arange(-1.4, 2.9, 0.45):
                wiggle = 0.08 * math.sin(0.37 * x + 0.9 * z)
                pts.append((x, y + math.copysign(wiggle, y), z, 0.35))

    # Ground grid and low curb lines.
    for x in np.arange(-8.0, length + 65.0, 1.0):
        for y in np.arange(-7.5, 7.6, 1.0):
            pts.append((x, y, -1.55 + 0.04 * math.sin(0.2 * x), 0.18))
        for y in (-4.2, 4.2):
            pts.append((x, y, -1.05, 0.42))

    # Poles break symmetry and keep the corridor observable.
    for x in np.arange(6.0, length + 50.0, 7.5):
        for y in (-5.8, 5.8):
            for z in np.arange(-1.4, 3.2, 0.35):
                for a in np.linspace(0.0, 2.0 * math.pi, 8, endpoint=False):
                    pts.append((x + 0.18 * math.cos(a), y + 0.18 * math.sin(a), z, 0.75))

    return np.array(pts, dtype=np.float64)


def box_surface(center: np.ndarray, dims: tuple[float, float, float], intensity: float) -> np.ndarray:
    lx, ly, lz = dims
    xs = np.arange(-0.5 * lx, 0.5 * lx + 1e-6, 0.28)
    ys = np.arange(-0.5 * ly, 0.5 * ly + 1e-6, 0.28)
    zs = np.arange(-0.5 * lz, 0.5 * lz + 1e-6, 0.28)
    pts: list[tuple[float, float, float, float]] = []
    for x in xs:
        for z in zs:
            pts.append((center[0] + x, center[1] - 0.5 * ly, center[2] + z, intensity))
            pts.append((center[0] + x, center[1] + 0.5 * ly, center[2] + z, intensity))
    for y in ys:
        for z in zs:
            pts.append((center[0] - 0.5 * lx, center[1] + y, center[2] + z, intensity))
            pts.append((center[0] + 0.5 * lx, center[1] + y, center[2] + z, intensity))
    for x in xs:
        for y in ys:
            pts.append((center[0] + x, center[1] + y, center[2] + 0.5 * lz, intensity))
    return np.array(pts, dtype=np.float64)


def sensor_cloud(
    static_pts: np.ndarray,
    frame: int,
    frames: int,
    step: float,
    include_dynamic_objects: bool,
) -> np.ndarray:
    sensor_x = frame * step
    sensor = np.array([sensor_x, 0.0, 1.35], dtype=np.float64)
    rel = static_pts.copy()
    rel[:, :3] -= sensor

    xyz = rel[:, :3]
    ranges = np.linalg.norm(xyz, axis=1)
    mask = (
        (xyz[:, 0] > 1.2)
        & (xyz[:, 0] < 58.0)
        & (np.abs(xyz[:, 1]) < 18.0)
        & (xyz[:, 2] > -3.4)
        & (xyz[:, 2] < 4.0)
        & (ranges > 1.0)
        & (ranges < 70.0)
    )
    visible = rel[mask]

    points = visible
    if include_dynamic_objects:
        phase = frame / max(frames - 1, 1)
        obj1_center = np.array([sensor_x + 18.0, -6.5 + 13.0 * phase, 0.0])
        obj2_center = np.array([sensor_x + 31.0, 5.0 - 10.0 * phase, -0.15])
        dynamic = np.vstack(
            [
                box_surface(obj1_center, (4.2, 2.2, 2.4), 0.95),
                box_surface(obj2_center, (3.0, 1.7, 2.0), 0.85),
            ]
        )
        dynamic[:, :3] -= sensor
        dyn_ranges = np.linalg.norm(dynamic[:, :3], axis=1)
        dynamic = dynamic[(dyn_ranges > 1.0) & (dyn_ranges < 70.0)]
        points = np.vstack([visible, dynamic])
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


def write_gt(path: Path, frames: int, dt: float, step: float) -> None:
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
            writer.writerow([f"{i * dt:.6f}", f"{i * step:.6f}", "0", "0", "0", "0", "0"])


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
    parser.add_argument("--step", type=float, default=0.38)
    parser.add_argument(
        "--no-dynamic-objects",
        action="store_true",
        help="Generate the same ego-motion scene without crossing foreground boxes.",
    )
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    out = Path(args.output_dir)
    if out.exists():
        if not args.force:
            raise SystemExit(f"{out} exists; pass --force to overwrite")
        shutil.rmtree(out)
    out.mkdir(parents=True)

    static_pts = static_world_points(args.frames * args.step)
    counts: list[int] = []
    for i in range(args.frames):
        cloud = sensor_cloud(
            static_pts,
            i,
            args.frames,
            args.step,
            include_dynamic_objects=not args.no_dynamic_objects,
        )
        counts.append(len(cloud))
        write_binary_pcd(out / f"{i:08d}" / "cloud.pcd", cloud)

    write_gt(out / "gt.csv", args.frames, args.dt, args.step)
    write_timestamps(out / "frame_timestamps.csv", args.frames, args.dt, counts)
    print(f"wrote {args.frames} frames to {out}")
    print(f"points/frame min={min(counts)} mean={sum(counts) / len(counts):.1f} max={max(counts)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
