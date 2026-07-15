#!/usr/bin/env python3
"""Top-down trajectory comparison: GT vs pure dead reckoning vs ESKF.

Two panels per dataset:
  left  - full extent: pure DR diverges by orders of magnitude (the honest
          lower-bound catastrophe), GT/ESKF collapse to a dot.
  right - zoomed to the GT+ESKF extent: the ESKF tracks the real trajectory.

Usage:
  plot_trajectory.py <out.png> <label>:<traj.csv>[:nhc] [<label>:<traj.csv> ...]

Each traj.csv is the output of the `dump_trajectory` tool
(frame,gt_x,gt_y,dr_x,dr_y,eskf_x,eskf_y).
"""
import sys
import csv

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def load(path):
    cols = {k: [] for k in
            ("gt_x", "gt_y", "dr_x", "dr_y", "eskf_x", "eskf_y")}
    with open(path) as f:
        for row in csv.DictReader(f):
            for k in cols:
                cols[k].append(float(row[k]))
    return {k: np.array(v) for k, v in cols.items()}


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    out = sys.argv[1]
    specs = []
    for a in sys.argv[2:]:
        parts = a.split(":")
        specs.append((parts[0], parts[1]))

    n = len(specs)
    fig, axes = plt.subplots(n, 2, figsize=(11, 5.2 * n), squeeze=False)
    fig.suptitle("IMU-DR trajectory: ground truth vs. pure dead reckoning vs. "
                 "ESKF", fontsize=14, fontweight="bold")

    C_GT, C_DR, C_EK = "#111111", "#d1495b", "#2e86ab"
    for r, (label, path) in enumerate(specs):
        d = load(path)
        axL, axR = axes[r][0], axes[r][1]

        # Left: full extent (pure DR runs away).
        axL.plot(d["dr_x"], d["dr_y"], color=C_DR, lw=1.0,
                 label="pure dead reckoning")
        axL.plot(d["gt_x"], d["gt_y"], color=C_GT, lw=1.6, label="ground truth")
        axL.plot(d["eskf_x"], d["eskf_y"], color=C_EK, lw=1.6, label="ESKF")
        dr_span = max(np.ptp(d["dr_x"]), np.ptp(d["dr_y"]))
        axL.set_title(f"{label} — full extent\n"
                      f"(pure DR diverges ~{dr_span/1000:.0f} km)", fontsize=10)
        axL.set_aspect("equal", adjustable="datalim")
        axL.legend(loc="best", fontsize=8)
        axL.grid(True, alpha=0.25)
        axL.set_xlabel("x (m)"); axL.set_ylabel("y (m)")

        # Right: zoom to the GT + ESKF extent (pure DR off-screen).
        xs = np.concatenate([d["gt_x"], d["eskf_x"]])
        ys = np.concatenate([d["gt_y"], d["eskf_y"]])
        mx = 0.08 * max(np.ptp(xs), np.ptp(ys), 1.0)
        axR.plot(d["dr_x"], d["dr_y"], color=C_DR, lw=1.0, alpha=0.4, zorder=1)
        axR.plot(d["eskf_x"], d["eskf_y"], color=C_EK, lw=1.2, alpha=0.8,
                 zorder=2, label="ESKF")
        axR.plot(d["gt_x"], d["gt_y"], color=C_GT, lw=2.2, zorder=3,
                 label="ground truth")
        axR.scatter([d["gt_x"][0]], [d["gt_y"][0]], c="#2a9d8f", s=55,
                    zorder=5, label="start")
        rmse = float(np.sqrt(np.mean((d["eskf_x"] - d["gt_x"]) ** 2 +
                                     (d["eskf_y"] - d["gt_y"]) ** 2)))
        axR.set_xlim(xs.min() - mx, xs.max() + mx)
        axR.set_ylim(ys.min() - mx, ys.max() + mx)
        axR.set_title(f"{label} — zoom to GT scale\n"
                      f"(ESKF XY RMSE {rmse:.0f} m)", fontsize=10)
        axR.set_aspect("equal", adjustable="box")
        axR.legend(loc="best", fontsize=8)
        axR.grid(True, alpha=0.25)
        axR.set_xlabel("x (m)"); axR.set_ylabel("y (m)")

    fig.tight_layout(rect=(0, 0, 1, 0.97))
    fig.savefig(out, dpi=130)
    print("wrote", out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
