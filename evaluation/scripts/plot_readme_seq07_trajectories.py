#!/usr/bin/env python3
"""Render the README hero trajectory comparison for KITTI seq07.

This script uses only trajectory text files whose pose count exactly matches the
seq07 full ground truth. dogfooding_results/ is a mutable last-run workspace, so
the count check prevents accidentally mixing seq00/seq08/108-frame trajectories
into the README hero.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_GT = REPO_ROOT / "dogfooding_results/kitti_seq_07_full_evaluated_gt.txt"
DEFAULT_TRAJ_DIR = REPO_ROOT / "dogfooding_results"
DEFAULT_OUT = REPO_ROOT / "docs/assets/grid_seq07.png"
EXPECTED_FRAMES = 1101

METHODS = [
    ("KC_LO", "KC-LO", "docs/benchmarks/kitti_full_new_methods/seq07_kc_lo_no_anneal.json"),
    ("D2_LIO", "D2-LIO", "docs/benchmarks/kitti_full_new_methods/seq07_d2lio.json"),
    ("DegenSense", "DegenSense", "docs/benchmarks/kitti_full_new_methods/seq07_degen_sense.json"),
    ("Adaptive_ICP", "Adaptive-ICP", "docs/benchmarks/kitti_full_new_methods/seq07_adaptive_icp.json"),
    ("I_LOAM", "I-LOAM", "docs/benchmarks/kitti_full_new_methods/seq07_i_loam.json"),
    ("MCC_LO", "MCC-LO", "docs/benchmarks/kitti_full_new_methods/seq07_mcc_lo.json"),
    ("Intensity_Flow", "Intensity-Flow", "docs/benchmarks/kitti_full_new_methods/seq07_intensity_flow.json"),
    ("LiDAR_IBA", "LiDAR-IBA", "docs/benchmarks/kitti_full_new_methods/seq07_lidar_iba.json"),
    ("GMM_LO", "GMM-LO", "docs/benchmarks/kitti_full_new_methods/seq07_gmm_lo.json"),
    ("TrICP_LO", "TrICP-LO", "docs/benchmarks/kitti_full_new_methods/seq07_tricp_lo.json"),
    ("IMLS_SLAM", "IMLS-SLAM", "docs/benchmarks/kitti_full_new_methods/seq07_imls_slam.json"),
    ("SuMa", "SuMa", "docs/benchmarks/kitti_full_new_methods/seq07_suma.json"),
    ("GNC_LO", "GNC-LO", "docs/benchmarks/kitti_full_new_methods/seq07_gnc_lo.json"),
    ("Spectral_LO", "Spectral-LO", "docs/benchmarks/kitti_full_new_methods/seq07_spectral_lo.json"),
]

REQUIRED = {"KC-LO", "D2-LIO", "DegenSense", "Adaptive-ICP", "I-LOAM"}

BG = "#0d1117"
FG = "#f0f6fc"
MUTED = "#8b949e"
SPINE = "#30363d"
GT_COL = "#6e7681"
HIGHLIGHT = "#2dd4bf"


def load_xy(path: Path) -> tuple[np.ndarray, np.ndarray]:
    data = np.loadtxt(path)
    if data.size == 0 or data.size % 12 != 0:
        raise SystemExit(f"{path}: expected KITTI poses (12 values/line), got size {data.size}")
    poses = data.reshape(-1, 3, 4)
    return poses[:, 0, 3], poses[:, 1, 3]


def load_metric(artifact: str) -> tuple[float, float]:
    data = json.loads((REPO_ROOT / artifact).read_text())
    methods = data.get("methods") or []
    if len(methods) != 1:
        raise SystemExit(f"{artifact}: expected exactly one method row")
    row = methods[0]
    if row.get("status") != "ok":
        raise SystemExit(f"{artifact}: method status is {row.get('status')!r}")
    return float(row["rpe_trans_pct"]), float(row["ate_m"])


def per_frame_error(gx: np.ndarray, gy: np.ndarray, ex: np.ndarray, ey: np.ndarray) -> np.ndarray:
    if len(gx) != len(ex):
        raise SystemExit(f"frame count mismatch: gt={len(gx)} estimate={len(ex)}")
    return np.hypot(ex - gx, ey - gy)


def collect(gt_path: Path, traj_dir: Path) -> tuple[np.ndarray, np.ndarray, list[dict[str, object]]]:
    gx, gy = load_xy(gt_path)
    if len(gx) != EXPECTED_FRAMES:
        raise SystemExit(f"{gt_path}: expected {EXPECTED_FRAMES} seq07 frames, got {len(gx)}")

    rows: list[dict[str, object]] = []
    missing_required: list[str] = []
    for basename, label, artifact in METHODS:
        path = traj_dir / f"{basename}.txt"
        if not path.exists():
            if label in REQUIRED:
                missing_required.append(f"{label} ({path})")
            continue
        ex, ey = load_xy(path)
        if len(ex) != len(gx):
            if label in REQUIRED:
                missing_required.append(f"{label} ({path}, {len(ex)} frames)")
            continue
        rpe, ate = load_metric(artifact)
        rows.append(
            {
                "label": label,
                "x": ex,
                "y": ey,
                "err": per_frame_error(gx, gy, ex, ey),
                "rpe": rpe,
                "ate": ate,
            }
        )

    if missing_required:
        joined = "\n  - ".join(missing_required)
        raise SystemExit(f"required seq07 trajectories are missing or stale:\n  - {joined}")
    rows.sort(key=lambda row: float(row["rpe"]))
    return gx, gy, rows


def render(
    gx: np.ndarray,
    gy: np.ndarray,
    rows: list[dict[str, object]],
    out: Path,
    cols: int,
    vmax: float,
) -> None:
    if not rows:
        raise SystemExit("no trajectories to render")

    rows_count = (len(rows) + cols - 1) // cols
    panel_w = 2.55
    fig, axes = plt.subplots(rows_count, cols, figsize=(cols * panel_w, rows_count * panel_w * 1.22), dpi=135)
    fig.patch.set_facecolor(BG)
    axes = np.atleast_1d(axes).ravel()

    pad = 0.08 * max(np.ptp(gx), np.ptp(gy))
    xlim = (float(np.min(gx) - pad), float(np.max(gx) + pad))
    ylim = (float(np.min(gy) - pad), float(np.max(gy) + pad))
    cmap = plt.get_cmap("turbo")
    norm = plt.Normalize(0.0, vmax)
    sm = None

    for index, ax in enumerate(axes):
        ax.set_facecolor(BG)
        ax.set_xticks([])
        ax.set_yticks([])
        for spine in ax.spines.values():
            spine.set_color(SPINE)
        if index >= len(rows):
            ax.axis("off")
            continue

        row = rows[index]
        ex = row["x"]
        ey = row["y"]
        err = row["err"]
        label = str(row["label"])
        rpe = float(row["rpe"])
        ate = float(row["ate"])

        ax.set_aspect("equal")
        ax.set_xlim(*xlim)
        ax.set_ylim(*ylim)
        ax.plot(gx, gy, color=GT_COL, lw=1.1, alpha=0.45, zorder=1)

        points = np.array([ex, ey]).T.reshape(-1, 1, 2)
        segments = np.concatenate([points[:-1], points[1:]], axis=1)
        line = LineCollection(segments, cmap=cmap, norm=norm, zorder=3)
        line.set_array(np.clip(err[:-1], 0.0, vmax))
        line.set_linewidth(2.0)
        sm = ax.add_collection(line)

        title_color = HIGHLIGHT if index < 5 else FG
        ax.set_title(label, color=title_color, fontsize=10.2, fontweight="bold", pad=2)
        ax.text(
            0.5,
            0.02,
            f"{rpe:.3f}%/100m  ·  {ate:.1f}m ATE",
            color=MUTED,
            fontsize=7.4,
            ha="center",
            va="bottom",
            transform=ax.transAxes,
        )

    fig.suptitle(
        "Localization Zoo - KITTI seq07 trajectories (full sequence, no GT seed)",
        color=FG,
        fontsize=14.5,
        fontweight="bold",
        y=0.990,
    )
    fig.text(
        0.5,
        0.958,
        "path color = position error from ground truth; cool = on track, warm = drift; ranked by RPE",
        color=MUTED,
        fontsize=9.3,
        ha="center",
    )
    fig.tight_layout(rect=[0.005, 0.075, 0.995, 0.915], h_pad=1.55, w_pad=0.65)

    if sm is not None:
        cax = fig.add_axes([0.30, 0.030, 0.40, 0.016])
        colorbar = fig.colorbar(sm, cax=cax, orientation="horizontal")
        colorbar.set_label(f"position error (m), clipped at {vmax:g} m", color=MUTED, fontsize=8.5, labelpad=2)
        colorbar.ax.xaxis.set_label_position("top")
        colorbar.ax.xaxis.set_tick_params(color=MUTED, labelsize=8)
        plt.setp(plt.getp(colorbar.ax.axes, "xticklabels"), color=MUTED)
        colorbar.outline.set_edgecolor(SPINE)

    fig.text(
        0.985,
        0.027,
        "github.com/rsasaki0109/localization_zoo",
        color=MUTED,
        fontsize=8.2,
        ha="right",
        va="bottom",
    )
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, facecolor=BG)
    print(f"wrote {out.relative_to(REPO_ROOT)} ({len(rows)} trajectories)")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gt", type=Path, default=DEFAULT_GT)
    parser.add_argument("--traj-dir", type=Path, default=DEFAULT_TRAJ_DIR)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--cols", type=int, default=5)
    parser.add_argument("--vmax", type=float, default=3.0)
    args = parser.parse_args()

    gx, gy, rows = collect(args.gt, args.traj_dir)
    render(gx, gy, rows, args.out, args.cols, args.vmax)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
