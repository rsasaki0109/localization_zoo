#!/usr/bin/env python3
"""Plot top-down GT trajectories for committed public 2D scan fixtures."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def load_gt_xy(path: Path) -> tuple[list[float], list[float]]:
    xs: list[float] = []
    ys: list[float] = []
    with path.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            xs.append(float(row["x"]))
            ys.append(float(row["y"]))
    return xs, ys


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="Repository root.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Output PNG (default: docs/assets/scan2d_public_gt.png).",
    )
    args = parser.parse_args()
    root = args.root
    out = args.output or (root / "docs/assets/scan2d_public_gt.png")
    out.parent.mkdir(parents=True, exist_ok=True)

    fixtures = [
        ("intel_val_73", "Intel val"),
        ("fr079_val_384", "fr079 val"),
        ("mit_val_33", "MIT val"),
    ]

    fig, axes = plt.subplots(1, 3, figsize=(12, 4), constrained_layout=True)
    for ax, (name, title) in zip(axes, fixtures):
        fix = root / "evaluation/fixtures" / name
        meta = json.loads((fix / "scan_meta.json").read_text())
        xs, ys = load_gt_xy(fix / "gt.csv")
        ax.plot(xs, ys, color="#2563eb", linewidth=1.5)
        ax.set_title(title)
        ax.set_xlabel("x [m]")
        ax.set_ylabel("y [m]")
        ax.set_aspect("equal", adjustable="box")
        ax.grid(True, alpha=0.3)
        n = len(xs)
        traj_m = sum(
            ((xs[i] - xs[i - 1]) ** 2 + (ys[i] - ys[i - 1]) ** 2) ** 0.5
            for i in range(1, n)
        )
        ax.text(
            0.02,
            0.98,
            f"{n} fr · {meta.get('dataset', '?')} · ~{traj_m:.0f} m",
            transform=ax.transAxes,
            va="top",
            fontsize=8,
            bbox={"facecolor": "white", "alpha": 0.8, "edgecolor": "none"},
        )

    fig.suptitle("Public 2D scan fixtures — dataset odometry (GT proxy)", fontsize=11)
    fig.savefig(out, dpi=150)
    print(f"Wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
