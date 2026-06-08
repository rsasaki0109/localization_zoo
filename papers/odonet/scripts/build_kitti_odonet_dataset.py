#!/usr/bin/env python3
"""Build OdoNet training windows from KITTI Raw OXTS packets.

Resamples OXTS to 50 Hz (paper default) and emits (window, speed) pairs using
forward velocity ``vf`` as label.
"""

from __future__ import annotations

import argparse
import csv
import json
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


def upsample_rows(rows: list[list[float]], target_hz: float = 50.0,
                  source_hz: float = 10.0) -> tuple[list[list[float]], list[float]]:
    if not rows:
        return [], []
    ratio = max(1, int(round(target_hz / source_hz)))
    out_rows: list[list[float]] = []
    stamps: list[float] = []
    for i in range(len(rows) - 1):
        a = rows[i]
        b = rows[i + 1]
        for k in range(ratio):
            alpha = k / ratio
            interp = [(1 - alpha) * a[j] + alpha * b[j] for j in range(len(a))]
            out_rows.append(interp)
            stamps.append(i + alpha / ratio)
    out_rows.append(rows[-1])
    stamps.append(float(len(rows) - 1))
    return out_rows, stamps


def build_windows(rows: list[list[float]], window: int = 50) -> tuple[list[list[float]], list[float]]:
    samples: list[list[float]] = []
    labels: list[float] = []
    for i in range(window - 1, len(rows)):
        win: list[float] = []
        for j in range(i - window + 1, i + 1):
            r = rows[j]
            win.extend([
                r[OXTS_WX], r[OXTS_WY], r[OXTS_WZ],
                r[OXTS_AX], r[OXTS_AY], r[OXTS_AZ],
            ])
        samples.append(win)
        labels.append(max(0.0, rows[i][OXTS_VF]))
    return samples, labels


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--kitti-root",
        default="/media/sasaki/aiueo/ai_coding_ws/_kitti/2011_09_26",
        help="KITTI raw date folder containing drive_*_sync dirs.",
    )
    parser.add_argument(
        "--output",
        default="papers/odonet/data/kitti_odonet_dataset.npz",
        help="Output NPZ path.",
    )
    parser.add_argument("--window", type=int, default=50)
    parser.add_argument("--val-drive", default="2011_09_26_drive_0056_sync")
    args = parser.parse_args()

    root = Path(args.kitti_root)
    drives = sorted(p for p in root.glob("*_sync") if p.is_dir())
    if not drives:
        raise SystemExit(f"No *_sync drives under {root}")

    import numpy as np

    train_x: list[list[float]] = []
    train_y: list[float] = []
    val_x: list[list[float]] = []
    val_y: list[float] = []

    for drive in drives:
        rows = load_oxts_rows(drive / "oxts" / "data")
        up_rows, _ = upsample_rows(rows)
        xs, ys = build_windows(up_rows, args.window)
        if drive.name == args.val_drive:
            val_x.extend(xs)
            val_y.extend(ys)
        else:
            train_x.extend(xs)
            train_y.extend(ys)
        print(f"[{drive.name}] windows={len(xs)} train_total={len(train_x)} val_total={len(val_x)}")

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
    meta = {
        "train_samples": len(train_x),
        "val_samples": len(val_x),
        "window": args.window,
        "channels": 6,
        "val_drive": args.val_drive,
    }
    out.with_suffix(".json").write_text(json.dumps(meta, indent=2) + "\n")
    print(f"[done] wrote {out} ({meta})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
