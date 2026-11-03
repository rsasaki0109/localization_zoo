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
from evaluate_external_kitti_odometry import (  # noqa: E402
    compute_metrics,
    load_kitti_poses,
    load_reference_csv,
    normalize_first_pose,
)

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


def load_poses(path: Path):
    """KITTI pose file, optionally gzip-compressed."""
    if path.suffix != ".gz":
        return load_kitti_poses(path)
    import gzip
    import tempfile
    with tempfile.TemporaryDirectory() as tmp:
        plain = Path(tmp) / path.stem
        plain.write_bytes(gzip.decompress(path.read_bytes()))
        return load_kitti_poses(plain)


def rte(poses, gt) -> float:
    return compute_metrics(poses, gt)["kitti_rte_trans_pct"]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--output", type=Path, default=Path(__file__).resolve().parent / "decomposition.json")
    parser.add_argument("--extra", action="append", default=[], metavar="LABEL:ESTIMATE:GT",
                        help="Also decompose another saved trajectory (KITTI pose files, .gz allowed); "
                             "repeatable. Written to the output under 'extra'.")
    args = parser.parse_args()
    rows = []
    for sequence in sorted(PAPER):
        run = REPO_ROOT / f"experiments/results/runs/l_lo_kitti_seq_{sequence}_full_paper_comp_matrix/seq07_tuned"
        row = {"sequence": sequence, "paper_rte_pct": PAPER[sequence]}
        # GT as the runner evaluates it: the committed reference CSV with the
        # first pose's inverse applied. Checked against a saved copy if present.
        csv_path = REPO_ROOT / f"experiments/reference_data/kitti_seq_{sequence}_full_gt.csv"
        gt = normalize_first_pose(load_reference_csv(csv_path))
        row["gt_source"] = str(csv_path.relative_to(REPO_ROOT))
        saved_gt = run / "evaluated_gt_kitti.txt"
        if saved_gt.is_file():
            saved = load_kitti_poses(saved_gt)
            worst = max(float(np.abs(a - b).max()) for a, b in zip(gt, saved))
            if len(saved) != len(gt) or worst > 1e-5:
                raise SystemExit(f"reference CSV and saved GT differ on {sequence}: {worst}")
            row["saved_gt_max_abs_difference"] = worst
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
    extra = []
    for spec in args.extra:
        label, est_path, gt_path = spec.split(":")
        est, gt = load_poses(Path(est_path)), load_poses(Path(gt_path))
        if len(est) != len(gt):
            raise SystemExit(f"pose count mismatch for {label}")
        ez = np.array([e[2, 3] for e in est])
        gz = np.array([g[2, 3] for g in gt])
        extra.append({"label": label, "estimate": est_path, "gt": gt_path,
                      "estimate_rte_pct": rte(est, gt),
                      "horizontal_only_rte_pct": rte(mix(est, gt, True), gt),
                      "vertical_only_rte_pct": rte(mix(est, gt, False), gt),
                      "estimate_height_range_m": float(ez.max() - ez.min()),
                      "gt_height_range_m": float(gz.max() - gz.min()),
                      "final_height_m": {"estimate": float(ez[-1]), "gt": float(gz[-1])}})
    args.output.write_text(json.dumps({"schema_version": 1, "method": "L-LO seq07_tuned",
                                       "metric": "official KITTI RTE translation [%]", "rows": rows,
                                       "extra": extra}, indent=2) + "\n")
    print("| Seq | Paper | Estimate | Horizontal-only error | Vertical-only error | GT planar (z=0, no pitch/roll) | GT roll-free | GT height range [m] |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|")
    for r in rows:
        f = lambda k: "-" if k not in r else f"{r[k]:.3f}"  # noqa: E731
        print(f"| {r['sequence']} | {r['paper_rte_pct']:.2f} | {f('estimate_rte_pct')} | {f('horizontal_only_rte_pct')} | "
              f"{f('vertical_only_rte_pct')} | {f('gt_planar_rte_pct')} | {f('gt_roll_free_rte_pct')} | {r['gt_height_range_m']:.1f} |")
    for e in extra:
        print(f"{e['label']}: estimate {e['estimate_rte_pct']:.3f}, horizontal-only {e['horizontal_only_rte_pct']:.3f}, "
              f"vertical-only {e['vertical_only_rte_pct']:.3f}, height range {e['estimate_height_range_m']:.1f} m "
              f"(GT {e['gt_height_range_m']:.1f} m), final height {e['final_height_m']['estimate']:.1f} m "
              f"(GT {e['final_height_m']['gt']:.1f} m)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
