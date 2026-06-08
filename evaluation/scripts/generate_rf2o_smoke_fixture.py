#!/usr/bin/env python3
"""Generate a synthetic 2D laser scan fixture for scan_dogfooding / RF2O smoke tests."""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
OUT = REPO / "evaluation" / "fixtures" / "rf2o_smoke"
BEAMS = 360
DT = 0.1
FRAMES = 60


def raycast_box(x: float, y: float, yaw: float, angle: float) -> float:
    wx = math.cos(yaw) * math.cos(angle) - math.sin(yaw) * math.sin(angle)
    wy = math.sin(yaw) * math.cos(angle) + math.cos(yaw) * math.sin(angle)
    t = 30.0
    if wx > 1e-6:
        t = min(t, (10.0 - x) / wx)
    if wx < -1e-6:
        t = min(t, (-10.0 - x) / wx)
    if wy > 1e-6:
        t = min(t, (10.0 - y) / wy)
    if wy < -1e-6:
        t = min(t, (-10.0 - y) / wy)
    return max(0.1, t)


def main() -> int:
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

    for i in range(FRAMES):
        t = i * DT
        x = 0.3 * i
        y = 0.05 * math.sin(i * 0.2)
        yaw = 0.02 * i
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

    print(f"wrote {FRAMES} frames to {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
