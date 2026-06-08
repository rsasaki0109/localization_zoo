#!/usr/bin/env python3
"""Build VMSC + adaptive NHC training windows from KITTI Raw OXTS."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

OXTS_VF, OXTS_VL, OXTS_VU = 8, 9, 10
OXTS_AX, OXTS_AY, OXTS_AZ = 11, 12, 13
OXTS_WX, OXTS_WY, OXTS_WZ = 17, 18, 19


def load_oxts_rows(oxts_dir: Path) -> list[list[float]]:
    rows: list[list[float]] = []
    for path in sorted(oxts_dir.glob("*.txt")):
        parts = path.read_text().strip().split()
        rows.append([float(x) for x in parts[:-5]])
    return rows


def upsample_rows(rows: list[list[float]], ratio: int = 5) -> list[list[float]]:
    if not rows:
        return []
    out: list[list[float]] = []
    for i in range(len(rows) - 1):
        a, b = rows[i], rows[i + 1]
        for k in range(ratio):
            alpha = k / ratio
            out.append([(1 - alpha) * a[j] + alpha * b[j] for j in range(len(a))])
    out.append(rows[-1])
    return out


def motion_class(vf: float, vl: float, vu: float, wz: float) -> int:
    if abs(vf) < 0.5:
        return 0  # stop
    if abs(vl) > 0.25 or abs(vu) > 0.15:
        return 3  # slip / bounce
    if abs(wz) > 0.08:
        return 2  # turn
    return 1  # straight


def build_windows(rows: list[list[float]], window: int = 50):
    xs, y_cls, y_nhc = [], [], []
    for i in range(window - 1, len(rows)):
        win = []
        for j in range(i - window + 1, i + 1):
            r = rows[j]
            win.extend([r[OXTS_WX], r[OXTS_WY], r[OXTS_WZ], r[OXTS_AX], r[OXTS_AY], r[OXTS_AZ]])
        r = rows[i]
        xs.append(win)
        y_cls.append(motion_class(r[OXTS_VF], r[OXTS_VL], r[OXTS_VU], r[OXTS_WZ]))
        y_nhc.append([r[OXTS_VL], r[OXTS_VU]])
    return xs, y_cls, y_nhc


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--kitti-root", default="/media/sasaki/aiueo/ai_coding_ws/_kitti/2011_09_26")
    parser.add_argument("--output", default="papers/nhc_net/data/kitti_nhc_net_dataset.npz")
    parser.add_argument("--window", type=int, default=50)
    parser.add_argument("--val-drive", default="2011_09_26_drive_0056_sync")
    args = parser.parse_args()

    import numpy as np

    root = Path(args.kitti_root)
    train_x, train_cls, train_nhc = [], [], []
    val_x, val_cls, val_nhc = [], [], []
    for drive in sorted(root.glob("*_sync")):
        rows = upsample_rows(load_oxts_rows(drive / "oxts" / "data"))
        xs, yc, yn = build_windows(rows, args.window)
        if drive.name == args.val_drive:
            val_x.extend(xs); val_cls.extend(yc); val_nhc.extend(yn)
        else:
            train_x.extend(xs); train_cls.extend(yc); train_nhc.extend(yn)
        print(f"{drive.name}: {len(xs)} windows")

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    np.savez_compressed(
        out,
        train_x=np.asarray(train_x, dtype=np.float32),
        train_cls=np.asarray(train_cls, dtype=np.int64),
        train_nhc=np.asarray(train_nhc, dtype=np.float32),
        val_x=np.asarray(val_x, dtype=np.float32),
        val_cls=np.asarray(val_cls, dtype=np.int64),
        val_nhc=np.asarray(val_nhc, dtype=np.float32),
        window=np.int32(args.window),
        channels=np.int32(6),
        num_classes=np.int32(4),
    )
    print(f"[done] {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
