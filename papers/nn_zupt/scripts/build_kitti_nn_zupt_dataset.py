#!/usr/bin/env python3
"""Build NN-ZUPT binary training windows from KITTI Raw OXTS."""

from __future__ import annotations

import argparse
from pathlib import Path

OXTS_VF = 8
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


def is_stop(vf: float, threshold: float = 0.5) -> int:
    return 1 if abs(vf) < threshold else 0


def build_windows(rows: list[list[float]], window: int = 50):
    xs, ys = [], []
    for i in range(window - 1, len(rows)):
        win = []
        for j in range(i - window + 1, i + 1):
            r = rows[j]
            win.extend([r[OXTS_WX], r[OXTS_WY], r[OXTS_WZ], r[OXTS_AX], r[OXTS_AY], r[OXTS_AZ]])
        xs.append(win)
        ys.append(is_stop(rows[i][OXTS_VF]))
    return xs, ys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--kitti-root", default="_kitti/2011_09_26")
    parser.add_argument("--output", default="papers/nn_zupt/data/kitti_nn_zupt_dataset.npz")
    parser.add_argument("--window", type=int, default=50)
    parser.add_argument("--stop-threshold", type=float, default=0.5)
    parser.add_argument("--val-drive", default="2011_09_26_drive_0056_sync")
    args = parser.parse_args()

    import numpy as np

    root = Path(args.kitti_root)
    train_x, train_y, val_x, val_y = [], [], [], []
    for drive in sorted(root.glob("*_sync")):
        rows = upsample_rows(load_oxts_rows(drive / "oxts" / "data"))
        xs, ys = build_windows(rows, args.window)
        if drive.name == args.val_drive:
            val_x.extend(xs)
            val_y.extend(ys)
        else:
            train_x.extend(xs)
            train_y.extend(ys)
        pos = sum(ys)
        print(f"{drive.name}: {len(xs)} windows stop={pos} ({100*pos/max(len(xs),1):.1f}%)")

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    np.savez_compressed(
        out,
        train_x=np.asarray(train_x, dtype=np.float32),
        train_y=np.asarray(train_y, dtype=np.float32),
        val_x=np.asarray(val_x, dtype=np.float32),
        val_y=np.asarray(val_y, dtype=np.float32),
        window=np.int32(args.window),
        channels=np.int32(6),
    )
    print(f"[done] {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
