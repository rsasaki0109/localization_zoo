#!/usr/bin/env python3
"""Where does InTEn-LOAM's heading error on KITTI 03/05/10 come from?

1. Per-stage yaw on sequence 10, from the committed passive trace of the
   unchanged paper_odometry_plane_check run
   (experiments/results/inten_natural_traces/sequences/10/on). For each frame
   it compares the yaw of the S2S prediction (s2s.input), the S2S output and the
   final relative pose (after S2M) with the GT relative yaw. It also scores
   "yaw-only" trajectories that take the yaw increment from one stage and
   everything else from GT, using the official KITTI RTE.
2. Per-frame relative-yaw error of every method's Table 6 poses on 03/05/10
   (the same poses as decomposition.json), as mean (bias), standard deviation
   and sum.

Reads committed files only.
"""

from __future__ import annotations

import gzip
import importlib.util
import json
import math
import sys
from pathlib import Path

import numpy as np

STUDY = Path(__file__).resolve().parent
REPO_ROOT = STUDY.parents[2]
sys.path.insert(0, str(REPO_ROOT / "evaluation" / "scripts"))
from evaluate_external_kitti_odometry import (  # noqa: E402
    compute_metrics,
    load_kitti_poses,
    load_reference_csv,
    normalize_first_pose,
)

_spec = importlib.util.spec_from_file_location("gap", STUDY / "decompose_table6.py")
gap = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(gap)
TRACE = REPO_ROOT / "experiments/results/inten_natural_traces/sequences/10/on"
STAGES = ("prediction", "s2s", "final")
SPEED_BINS = ((0.0, 0.3), (0.3, 0.6), (0.6, 0.9), (0.9, 1.2), (1.2, 2.0))


def wrap(angle: float) -> float:
    return (angle + math.pi) % (2.0 * math.pi) - math.pi


def yaw(T: np.ndarray) -> float:
    return gap.zyx(T[:3, :3])[0]


def relative(poses):
    return [np.linalg.inv(a) @ b for a, b in zip(poses[:-1], poses[1:])]


def trace_frames() -> dict[int, dict]:
    # The parts are consecutive slices of one gzip-compressed JSONL stream.
    raw = b"".join(gzip.decompress(p.read_bytes())
                   for p in sorted((TRACE / "trace.jsonl.parts").glob("part-*.gz")))
    frames = {}
    for line in raw.decode().splitlines():
        record = json.loads(line)
        if record.get("type") == "frame":
            frames[record["input_index"]] = record
    return frames


def stats(errors_deg: np.ndarray) -> dict:
    return {"mean_deg": float(errors_deg.mean()), "std_deg": float(errors_deg.std()),
            "sum_deg": float(errors_deg.sum()), "mean_abs_deg": float(np.abs(errors_deg).mean())}


def yaw_only_rte(stage_errors: np.ndarray, gt) -> float:
    steps = []
    for err, G in zip(stage_errors, relative(gt)):
        gy, gp, gr = gap.zyx(G[:3, :3])
        s = gap.compose(gy + err, gp, gr, (0.0, 0.0, 0.0))
        s[:3, 3] = G[:3, 3]
        steps.append(s)
    return compute_metrics(gap.chain(steps), gt)["kitti_rte_trans_pct"]


def stage_analysis() -> dict:
    frames = trace_frames()
    gt = normalize_first_pose(load_kitti_poses(TRACE / "dogfooding_results/velodyne_evaluated_gt.txt"))
    est = normalize_first_pose(load_kitti_poses(TRACE / "dogfooding_results/InTEn_LOAM.txt"))
    final_rel = relative(est)
    gt_rel = relative(gt)
    as_matrix = lambda flat: np.asarray(flat, dtype=float).reshape(4, 4)  # noqa: E731
    errors = {s: [] for s in STAGES}
    step_m, gt_yaw = [], []
    for k, G in enumerate(gt_rel, start=1):
        f = frames[k]
        errors["prediction"].append(wrap(yaw(as_matrix(f["s2s"]["input"])) - yaw(G)))
        errors["s2s"].append(wrap(yaw(as_matrix(f["s2s"]["output"])) - yaw(G)))
        errors["final"].append(wrap(yaw(final_rel[k - 1]) - yaw(G)))
        step_m.append(float(np.linalg.norm(G[:3, 3])))
        gt_yaw.append(math.degrees(yaw(G)))
    step_m, gt_yaw = np.array(step_m), np.array(gt_yaw)
    out = {"source": str(TRACE.relative_to(REPO_ROOT)), "frames": len(gt), "stages": {}}
    for stage in STAGES:
        e = np.array(errors[stage])
        entry = stats(np.degrees(e))
        entry["yaw_only_rte_pct"] = yaw_only_rte(e, gt)
        straight = np.abs(gt_yaw) < 0.2
        entry["straight_bias_by_step_m"] = []
        for lo, hi in SPEED_BINS:
            mask = straight & (step_m >= lo) & (step_m < hi)
            if mask.sum() >= 20:
                entry["straight_bias_by_step_m"].append(
                    {"step_m": [lo, hi], "frames": int(mask.sum()),
                     "mean_deg": float(np.degrees(e[mask]).mean())})
        X = np.column_stack([np.ones_like(step_m), step_m, gt_yaw])
        coef, *_ = np.linalg.lstsq(X, np.degrees(e), rcond=None)
        fitted = X @ coef
        y = np.degrees(e)
        entry["regression_deg"] = {"intercept": float(coef[0]), "per_step_m": float(coef[1]),
                                   "per_gt_yaw_deg": float(coef[2]),
                                   "r2": float(1 - ((y - fitted) ** 2).sum() / ((y - y.mean()) ** 2).sum())}
        out["stages"][stage] = entry
    return out


def cross_method() -> dict:
    decomposition = json.loads((STUDY / "decomposition.json").read_text())
    out = {}
    for sequence in ("03", "05", "10"):
        gt = normalize_first_pose(load_reference_csv(
            REPO_ROOT / f"experiments/reference_data/kitti_seq_{sequence}_full_gt.csv"))
        gt_rel = relative(gt)
        rows = []
        for r in decomposition["rows"]:
            if r["status"] != "ok" or r["sequence"] != sequence:
                continue
            est_rel = relative(normalize_first_pose(load_kitti_poses(REPO_ROOT / r["poses"])))
            e = np.degrees([wrap(yaw(a) - yaw(b)) for a, b in zip(est_rel, gt_rel)])
            rows.append({"method": r["method"], **stats(np.array(e)),
                         "yaw_channel_ratio": r["channel_ratio_to_paper"]["yaw"],
                         "total_ratio": r["ratio_to_paper"]})
        out[sequence] = sorted(rows, key=lambda x: -x["yaw_channel_ratio"])
    return out


def main() -> int:
    result = {"schema_version": 1, "stage_analysis_10": stage_analysis(), "cross_method": cross_method()}
    (STUDY / "inten_heading.json").write_text(json.dumps(result, indent=2) + "\n")
    s = result["stage_analysis_10"]["stages"]
    for stage in STAGES:
        print(f"{stage:10s} mean {s[stage]['mean_deg']:+.4f} std {s[stage]['std_deg']:.4f} "
              f"sum {s[stage]['sum_deg']:+7.2f} deg  yaw-only RTE {s[stage]['yaw_only_rte_pct']:.3f} %")
    for sequence, rows in result["cross_method"].items():
        print(f"seq {sequence}: " + ", ".join(f"{r['method']} {r['mean_deg']:+.4f}/{r['std_deg']:.3f}" for r in rows))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
