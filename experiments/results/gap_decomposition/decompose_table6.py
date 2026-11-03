#!/usr/bin/env python3
"""Attribute each Table 6 cell's official KITTI RTE to error channels.

For every row of docs/assets/paper/paper_ratio_table.csv this finds committed
saved poses of exactly the Table 6 variant, either the run's
trajectory_kitti.txt under experiments/results/runs/ or a shared_near_range
audit replay of the same aggregate/variant. It recomputes official RTE
from them and uses the cell only if that equals the Table 6 value to its
three printed decimals. GT is the committed reference CSV, normalized to the
first pose as the runner evaluates it.

Each channel replaces part of the estimate with GT and scores the result with
the official KITTI RTE (100-800 m) against the full GT:

- horizontal: estimate x, y, yaw; GT z, pitch, roll (world frame);
- vertical:   GT x, y, yaw; estimate z, pitch, roll;
- rotation:   estimated relative rotations, GT relative translations;
- yaw:        as rotation, but only each step's yaw comes from the estimate
              (GT pitch/roll increments), i.e. heading error alone;
- tilt:       as rotation, but only each step's pitch/roll come from the estimate;
- translation: GT relative rotations, estimated relative translations;
- scale:      GT relative motion, with each step's length taken from the estimate.

These are attributions, not parts of a sum: RTE is a mean of segment-end error
norms, so channels can overlap or cancel. They say which kind of error is large
enough on its own to exceed the paper, not why the estimator made it.
"""

from __future__ import annotations

import argparse
import csv
import importlib.util
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

_spec = importlib.util.spec_from_file_location(
    "l_lo_decompose", REPO_ROOT / "experiments/results/l_lo_vertical_decomposition/decompose.py")
_lld = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_lld)
zyx, compose = _lld.zyx, _lld.compose

TABLE = REPO_ROOT / "docs/assets/paper/paper_ratio_table.csv"
CHANNELS = ("horizontal", "vertical", "rotation", "yaw", "tilt", "translation", "scale")


def rte(poses, gt) -> float:
    return compute_metrics(poses, gt)["kitti_rte_trans_pct"]


def world_mix(est, gt, horizontal_from_est: bool):
    out = []
    for e, g in zip(est, gt):
        ey, ep, er = zyx(e[:3, :3])
        gy, gp, gr = zyx(g[:3, :3])
        if horizontal_from_est:
            out.append(compose(ey, gp, gr, (e[0, 3], e[1, 3], g[2, 3])))
        else:
            out.append(compose(gy, ep, er, (g[0, 3], g[1, 3], e[2, 3])))
    return out


def relative(poses):
    return [np.linalg.inv(a) @ b for a, b in zip(poses[:-1], poses[1:])]


def chain(steps):
    out = [np.eye(4)]
    for step in steps:
        out.append(out[-1] @ step)
    return out


def step_mix(est, gt, kind: str):
    steps = []
    for e, g in zip(relative(est), relative(gt)):
        s = np.eye(4)
        if kind == "rotation":
            s[:3, :3], s[:3, 3] = e[:3, :3], g[:3, 3]
        elif kind in ("yaw", "tilt"):
            ey, ep, er = zyx(e[:3, :3])
            gy, gp, gr = zyx(g[:3, :3])
            mixed = compose(ey, gp, gr, (0.0, 0.0, 0.0)) if kind == "yaw" else compose(gy, ep, er, (0.0, 0.0, 0.0))
            s[:3, :3], s[:3, 3] = mixed[:3, :3], g[:3, 3]
        elif kind == "translation":
            s[:3, :3], s[:3, 3] = g[:3, :3], e[:3, 3]
        elif kind == "scale":
            s[:3, :3] = g[:3, :3]
            gl = float(np.linalg.norm(g[:3, 3]))
            s[:3, 3] = g[:3, 3] * (float(np.linalg.norm(e[:3, 3])) / gl if gl > 1e-9 else 1.0)
        steps.append(s)
    return chain(steps)


def channel_trajectory(est, gt, channel):
    if channel == "horizontal":
        return world_mix(est, gt, True)
    if channel == "vertical":
        return world_mix(est, gt, False)
    return step_mix(est, gt, channel)


def find_poses(aggregate: str, variant: str) -> Path | None:
    stem = Path(aggregate).stem
    run = REPO_ROOT / "experiments/results/runs" / stem / variant / "trajectory_kitti.txt"
    if run.is_file():
        return run
    pattern = f"**/{stem}__{variant}_native/dogfooding_results/*.txt"
    for candidate in sorted((REPO_ROOT / "experiments/results/shared_near_range").glob(pattern)):
        if candidate.name == "gt.txt" or candidate.name.endswith("_evaluated_gt.txt"):
            continue
        return candidate
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--output", type=Path, default=Path(__file__).resolve().parent / "decomposition.json")
    parser.add_argument("--method", action="append", help="Restrict to these methods (repeatable).")
    args = parser.parse_args()
    rows = list(csv.DictReader(TABLE.open()))
    if args.method:
        rows = [r for r in rows if r["method"] in args.method]
    gt_cache: dict[str, list] = {}
    out = []
    for row in rows:
        method, sequence = row["method"], row["sequence"]
        paper = float(row["paper_rte_pct"])
        table_value = float(row["repo_best_rpe_pct"])
        entry = {"method": method, "sequence": sequence, "paper_rte_pct": paper,
                 "table6_rte_pct": table_value, "variant": row["repo_best_variant"],
                 "aggregate": row["repo_best_aggregate"]}
        poses_path = find_poses(row["repo_best_aggregate"], row["repo_best_variant"])
        if poses_path is None:
            entry["status"] = "no_committed_poses"
            out.append(entry)
            continue
        entry["poses"] = str(poses_path.relative_to(REPO_ROOT))
        if sequence not in gt_cache:
            gt_cache[sequence] = normalize_first_pose(load_reference_csv(
                REPO_ROOT / f"experiments/reference_data/kitti_seq_{sequence}_full_gt.csv"))
        gt = gt_cache[sequence]
        est = normalize_first_pose(load_kitti_poses(poses_path))
        if len(est) != len(gt):
            entry["status"] = f"pose_count_mismatch_{len(est)}_{len(gt)}"
            out.append(entry)
            continue
        estimate = rte(est, gt)
        entry["recomputed_rte_pct"] = estimate
        if abs(round(estimate, 3) - table_value) > 0.0015:
            entry["status"] = "poses_do_not_reproduce_table6"
            out.append(entry)
            continue
        entry["status"] = "ok"
        entry["channels_rte_pct"] = {c: rte(channel_trajectory(est, gt, c), gt) for c in CHANNELS}
        entry["ratio_to_paper"] = estimate / paper
        entry["channel_ratio_to_paper"] = {c: v / paper for c, v in entry["channels_rte_pct"].items()}
        out.append(entry)
        print(f"{method:11s} {sequence} {estimate:7.3f} ({estimate / paper:5.2f}x) " +
              " ".join(f"{c[:5]}={entry['channels_rte_pct'][c]:.3f}" for c in CHANNELS), flush=True)
    args.output.write_text(json.dumps({"schema_version": 1, "metric": "official KITTI RTE translation [%]",
                                       "channels": list(CHANNELS), "rows": out}, indent=2) + "\n")
    counts: dict[str, int] = {}
    for e in out:
        counts[e["status"]] = counts.get(e["status"], 0) + 1
    print("status counts:", counts)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
