#!/usr/bin/env python3
"""Generate a synthetic 2D scan fixture with realistic slow motion (~0.08 m/frame).

Uses the same bounded box raycast as rf2o_smoke (closed walls on all sides) so scan
odometry has sufficient geometric constraints. Pure parallel-wall corridors without
near forward features are degenerate for 2D odometry — see SETUP_2D_SCAN_BENCHMARK.md.
"""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
OUT = REPO / "evaluation" / "fixtures" / "rf2o_corridor"
BEAMS = 360
DT = 0.1
FRAMES = 120
BOX_HALF = 10.0


def raycast_box(x: float, y: float, yaw: float, angle: float) -> float:
    wx = math.cos(yaw) * math.cos(angle) - math.sin(yaw) * math.sin(angle)
    wy = math.sin(yaw) * math.cos(angle) + math.cos(yaw) * math.sin(angle)
    t = 30.0
    if wx > 1e-6:
        t = min(t, (BOX_HALF - x) / wx)
    if wx < -1e-6:
        t = min(t, (-BOX_HALF - x) / wx)
    if wy > 1e-6:
        t = min(t, (BOX_HALF - y) / wy)
    if wy < -1e-6:
        t = min(t, (-BOX_HALF - y) / wy)
    return max(0.1, t)


def pose_at_frame(i: int) -> tuple[float, float, float]:
    """Slow mixed motion: smoke profile scaled to ~0.08 m/frame."""
    x = 0.08 * i
    y = 0.04 * math.sin(i * 0.2)
    yaw = 0.015 * i
    return x, y, yaw


def main() -> int:
    if OUT.exists():
        for child in OUT.iterdir():
            if child.is_dir() and len(child.name) == 8 and child.name.isdigit():
                for f in child.iterdir():
                    f.unlink()
                child.rmdir()
            elif child.is_file():
                child.unlink()
    OUT.mkdir(parents=True, exist_ok=True)
    meta = {
        "angle_min": -math.pi,
        "angle_max": math.pi,
        "angle_increment": (2 * math.pi) / (BEAMS - 1),
        "range_min": 0.1,
        "range_max": 30.0,
        "scan_rate_hz": 1.0 / DT,
    }
    (OUT / "scan_meta.json").write_text(json.dumps(meta, indent=2) + "\n")

    gt_rows = [["timestamp", "x", "y", "yaw"]]
    angle_min = meta["angle_min"]
    angle_inc = meta["angle_increment"]

    xs: list[float] = []
    ys: list[float] = []
    for i in range(FRAMES):
        t = i * DT
        x, y, yaw = pose_at_frame(i)
        xs.append(x)
        ys.append(y)
        gt_rows.append([f"{t:.3f}", f"{x:.6f}", f"{y:.6f}", f"{yaw:.6f}"])

        frame_dir = OUT / f"{i:08d}"
        frame_dir.mkdir(exist_ok=True)
        ranges = []
        for b in range(BEAMS):
            ang = angle_min + b * angle_inc
            ranges.append(f"{raycast_box(x, y, yaw, ang):.6f}")
        (frame_dir / "scan.csv").write_text(",".join(ranges) + "\n")

    with (OUT / "gt.csv").open("w", newline="") as f:
        csv.writer(f).writerows(gt_rows)

    length = sum(
        math.hypot(xs[j] - xs[j - 1], ys[j] - ys[j - 1]) for j in range(1, FRAMES)
    )
    print(f"wrote {FRAMES} frames to {OUT} (traj ~{length:.1f} m, slow mixed motion)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
