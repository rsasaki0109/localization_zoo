#!/usr/bin/env python3
"""Render the benchmark hero image from docs/benchmarks/latest/results.json.

The previous hero (trajectory.png) crammed a near-linear trajectory overlay
(collapsed to a thin strip by equal-aspect) next to an ATE bar chart, so the
left subplot's title collided with its legend. This renders a single clean
horizontal ATE comparison on a log axis instead, using only the committed
results.json metrics (no raw pose files required).

Usage:
    python3 evaluation/scripts/render_benchmark_hero.py
"""
from __future__ import annotations

import json
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS = REPO_ROOT / "docs" / "benchmarks" / "latest" / "results.json"
OUTPUT = REPO_ROOT / "docs" / "benchmarks" / "latest" / "trajectory.png"

SEED_COLOR = "#2a9d8f"   # GT-seeded scan-to-map methods
ODOM_COLOR = "#bdbdbd"   # odometry-only (drift accumulates)

# Methods that run without a per-frame GT seed; their ATE accumulates drift and
# is not directly comparable to the seeded scan-to-map methods.
ODOMETRY_ONLY = {"CT-ICP", "KISS-ICP", "CT-LIO"}


def main() -> int:
    data = json.loads(RESULTS.read_text())
    methods = [m for m in data.get("methods", [])
               if m.get("status") == "ok" and m.get("ate_m") is not None]
    methods.sort(key=lambda m: m["ate_m"], reverse=True)  # best ends up on top

    names = [m["name"] for m in methods]
    ates = [m["ate_m"] for m in methods]
    colors = [ODOM_COLOR if n in ODOMETRY_ONLY else SEED_COLOR for n in names]

    fig, ax = plt.subplots(figsize=(10, 5.2), constrained_layout=True)
    bars = ax.barh(names, ates, color=colors, height=0.62, zorder=3)
    ax.set_xscale("log")
    ax.set_xlim(0.05, max(ates) * 3.0)
    ax.set_xlabel("ATE RMSE [m]  (log scale, lower is better)", fontsize=11)
    ax.grid(True, axis="x", alpha=0.25, zorder=0)
    ax.tick_params(labelsize=11)
    for spine in ("top", "right"):
        ax.spines[spine].set_visible(False)

    for bar, ate in zip(bars, ates):
        ax.text(bar.get_width() * 1.08, bar.get_y() + bar.get_height() / 2,
                f"{ate:.2f} m", va="center", ha="left", fontsize=10.5,
                fontweight="bold", color="#222")

    dataset = data.get("dataset", {})
    frames = dataset.get("frames")
    length = dataset.get("trajectory_length_m")
    subtitle = "Autoware Istanbul localization bag"
    if frames and length:
        subtitle += f"  ·  {frames} frames  ·  {length:.0f} m"
    ax.set_title("Absolute Trajectory Error by method", fontsize=15,
                 fontweight="bold", loc="left", pad=18)
    ax.annotate(subtitle, xy=(0, 1.015), xycoords="axes fraction",
                fontsize=10.5, color="#666")

    # Legend explaining the seed/odometry split (the material caveat).
    seed_patch = plt.Rectangle((0, 0), 1, 1, color=SEED_COLOR)
    odom_patch = plt.Rectangle((0, 0), 1, 1, color=ODOM_COLOR)
    ax.legend([seed_patch, odom_patch],
              ["GT-seeded scan-to-map (per-frame residual)",
               "odometry-only (drift accumulates)"],
              loc="upper right", fontsize=9.5, framealpha=0.9)

    fig.savefig(OUTPUT, dpi=150)
    print(f"Wrote {OUTPUT.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
