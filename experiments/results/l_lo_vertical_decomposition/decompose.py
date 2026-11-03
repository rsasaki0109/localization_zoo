#!/usr/bin/env python3
"""Split L-LO's official KITTI RTE into horizontal and vertical channels.

Every trajectory below is scored with the official KITTI RTE (100-800 m)
against the full 6-DoF evaluated GT that the run saved. Poses are decomposed
as T = (R = Rz(yaw) Ry(pitch) Rx(roll), t = (x, y, z)) in the sensor frame
(x forward, y left, z up):

- estimate: the saved L-LO trajectory as it is;
- horizontal_only: estimate x, y, yaw with GT z, pitch, roll
  (only the horizontal channel carries estimation error);
- vertical_only: GT x, y, yaw with estimate z, pitch, roll
  (only the vertical channel carries estimation error);
- gt_planar: GT x, y, yaw with z = 0 and zero pitch/roll (the paper's
  null-roll model without any height or pitch);
- gt_roll_free: GT position and GT yaw/pitch, roll = 0.

The two channel-isolated trajectories do not add up to the estimate: RTE is a
norm over segment end-point errors, so these are attributions, not a sum.
No KITTI scans are needed; only committed poses are read.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import numpy as np

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT / "evaluation" / "scripts"))
from evaluate_external_kitti_odometry import compute_metrics, load_kitti_poses  # noqa: E402

PAPER = {"00": 0.91, "02": 2.35, "03": 2.92, "04": 2.28, "05": 0.82, "06": 0.66,
         "07": 0.69, "08": 1.37, "09": 1.89, "10": 1.90}


def zyx(rotation: np.ndarray) -> tuple[float, float, float]:
    pitch = -math.asin(max(-1.0, min(1.0, rotation[2, 0])))
    roll = math.atan2(rotation[2, 1], rotation[2, 2])
    yaw = math.atan2(rotation[1, 0], rotation[0, 0])
    return yaw, pitch, roll


def compose(yaw: float, pitch: float, roll: float, t) -> np.ndarray:
    cz, sz = math.cos(yaw), math.sin(yaw)
    cy, sy = math.cos(pitch), math.sin(pitch)
    cx, sx = math.cos(roll), math.sin(roll)
    rz = np.array([[cz, -sz, 0], [sz, cz, 0], [0, 0, 1]])
    ry = np.array([[cy, 0, sy], [0, 1, 0], [-sy, 0, cy]])
    rx = np.array([[1, 0, 0], [0, cx, -sx], [0, sx, cx]])
    pose = np.eye(4)
    pose[:3, :3] = rz @ ry @ rx
    pose[:3, 3] = t
    return pose


def mix(est, gt, horizontal_from_est: bool):
    out = []
    for e, g in zip(est, gt):
        ey, ep, er = zyx(e[:3, :3])
        gy, gp, gr = zyx(g[:3, :3])
        if horizontal_from_est:
            out.append(compose(ey, gp, gr, (e[0, 3], e[1, 3], g[2, 3])))
        else:
            out.append(compose(gy, ep, er, (g[0, 3], g[1, 3], e[2, 3])))
    return out


def gt_planar(gt):
    return [compose(zyx(g[:3, :3])[0], 0.0, 0.0, (g[0, 3], g[1, 3], 0.0)) for g in gt]


def gt_roll_free(gt):
    out = []
    for g in gt:
        yaw, pitch, _ = zyx(g[:3, :3])
        out.append(compose(yaw, pitch, 0.0, g[:3, 3]))
    return out


def rte(poses, gt) -> float:
    return compute_metrics(poses, gt)["kitti_rte_trans_pct"]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--output", type=Path, default=Path(__file__).resolve().parent / "decomposition.json")
    args = parser.parse_args()
    rows = []
    for sequence in sorted(PAPER):
        run = REPO_ROOT / f"experiments/results/runs/l_lo_kitti_seq_{sequence}_full_paper_comp_matrix/seq07_tuned"
        row = {"sequence": sequence, "paper_rte_pct": PAPER[sequence]}
        gt_path = run / "evaluated_gt_kitti.txt"
        if not gt_path.is_file():
            # Older aggregates kept no saved poses; GT-only baselines still apply.
            gt_path = None
            for candidate in sorted((REPO_ROOT / "experiments/results/runs").glob(f"*kitti_seq_{sequence}_full*/*/evaluated_gt_kitti.txt")):
                gt_path = candidate
                break
        if gt_path is None:
            continue
        gt = load_kitti_poses(gt_path)
        row["gt_source"] = str(gt_path.relative_to(REPO_ROOT))
        row["gt_planar_rte_pct"] = rte(gt_planar(gt), gt)
        row["gt_roll_free_rte_pct"] = rte(gt_roll_free(gt), gt)
        z = np.array([g[2, 3] for g in gt])
        row["gt_height_range_m"] = float(z.max() - z.min())
        est_path = run / "trajectory_kitti.txt"
        if est_path.is_file():
            est = load_kitti_poses(est_path)
            if len(est) != len(gt):
                raise SystemExit(f"pose count mismatch on {sequence}")
            row["estimate_source"] = str(est_path.relative_to(REPO_ROOT))
            row["estimate_rte_pct"] = rte(est, gt)
            row["horizontal_only_rte_pct"] = rte(mix(est, gt, True), gt)
            row["vertical_only_rte_pct"] = rte(mix(est, gt, False), gt)
            ez = np.array([e[2, 3] for e in est])
            row["estimate_height_range_m"] = float(ez.max() - ez.min())
        rows.append(row)
    args.output.write_text(json.dumps({"schema_version": 1, "method": "L-LO seq07_tuned",
                                       "metric": "official KITTI RTE translation [%]", "rows": rows},
                                      indent=2) + "\n")
    print("| Seq | Paper | Estimate | Horizontal-only error | Vertical-only error | GT planar (z=0, no pitch/roll) | GT roll-free | GT height range [m] |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|")
    for r in rows:
        f = lambda k: "-" if k not in r else f"{r[k]:.3f}"
        print(f"| {r['sequence']} | {r['paper_rte_pct']:.2f} | {f('estimate_rte_pct')} | {f('horizontal_only_rte_pct')} | "
              f"{f('vertical_only_rte_pct')} | {f('gt_planar_rte_pct')} | {f('gt_roll_free_rte_pct')} | {r['gt_height_range_m']:.1f} |")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
