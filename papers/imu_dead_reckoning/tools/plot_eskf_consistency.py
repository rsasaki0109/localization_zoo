#!/usr/bin/env python3
"""Plot ESKF consistency (NEES) from eskf_consistency.csv outputs.

Compares the default (point-accuracy-optimal) tuning against a consistency-tuned
variant, showing that the default filter's covariance is far too small (position
error escapes its 3-sigma envelope; per-frame NEES sits far above the chi2_3 95%
line) while the retuned filter is statistically consistent.

Usage:
  plot_eskf_consistency.py <default.csv> <tuned.csv> <out.png>

Each CSV is the per-frame output of the `eskf_consistency` tool with columns
frame,err_m,nees,sigma_pos_m.
"""
import sys
import csv

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def load(path):
    frame, err, nees, sigma = [], [], [], []
    with open(path) as f:
        r = csv.DictReader(f)
        for row in r:
            frame.append(float(row["frame"]))
            err.append(float(row["err_m"]))
            nees.append(float(row["nees"]))
            sigma.append(float(row["sigma_pos_m"]))
    return (np.array(frame), np.array(err), np.array(nees), np.array(sigma))


def main():
    if len(sys.argv) != 4:
        print(__doc__)
        return 2
    d = load(sys.argv[1])
    t = load(sys.argv[2])
    out = sys.argv[3]

    CHI2_3_95 = 7.815  # chi-square 3 DOF 95th percentile.
    IDEAL_MEAN = 3.0

    fig, (ax0, ax1) = plt.subplots(2, 1, figsize=(9, 7), sharex=True)
    fig.suptitle(
        "IMU-DR ESKF consistency — NCLT 2013-01-10 full (5105 frames)",
        fontsize=13, fontweight="bold")

    # Panel A: position error vs the filter's own 3-sigma envelope.
    fd, ed, _, sd = d
    ft, et, _, st = t
    ax0.plot(fd, ed, color="#111", lw=1.2, label="actual position error")
    ax0.plot(fd, 3 * sd, color="#d1495b", lw=1.4,
             label="default tuning  3σ (overconfident)")
    ax0.plot(ft, 3 * st, color="#2e86ab", lw=1.4,
             label="consistency-tuned  3σ")
    ax0.set_yscale("log")
    ax0.set_ylabel("position (m, log)")
    ax0.set_title("Actual error vs. reported 3σ — default 3σ is orders of "
                  "magnitude too small", fontsize=10)
    ax0.legend(loc="lower right", fontsize=9)
    ax0.grid(True, which="both", alpha=0.25)

    # Panel B: per-frame NEES vs the chi2 consistency band.
    ax1.plot(fd, d[2], color="#d1495b", lw=0.7, alpha=0.8,
             label=f"default NEES (mean {d[2].mean():.0f})")
    ax1.plot(ft, t[2], color="#2e86ab", lw=0.7, alpha=0.8,
             label=f"consistency-tuned NEES (mean {t[2].mean():.2f})")
    ax1.axhline(CHI2_3_95, color="#666", ls="--", lw=1.2,
                label=f"chi²₃ 95% = {CHI2_3_95}")
    ax1.axhline(IDEAL_MEAN, color="#2a9d8f", ls=":", lw=1.2,
                label="ideal mean = 3.0")
    ax1.set_yscale("log")
    ax1.set_ylabel("NEES (log)")
    ax1.set_xlabel("frame")
    ax1.set_title("Per-frame NEES — consistent filter sits around 3, near the "
                  "chi² band", fontsize=10)
    ax1.legend(loc="upper right", fontsize=9, ncol=2)
    ax1.grid(True, which="both", alpha=0.25)

    fig.tight_layout(rect=(0, 0, 1, 0.97))
    fig.savefig(out, dpi=130)
    print("wrote", out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
