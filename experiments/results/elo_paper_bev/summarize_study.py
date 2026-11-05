#!/usr/bin/env python3
"""Tabulate the ELO paper-BEV study (protocol.md).

Reads experiments/results/runs/elo_kitti_seq_NN_full_paper_bev_matrix/<arm>/
(or --runs-root), the paper's ELO values from paper_reported_numbers.json
and the stored Table 6 results in
experiments/results/elo_kitti_seq_NN_full_paper_comp_matrix.json. For
each arm it also scores the scale-only and yaw-only channels of the saved poses,
as in ../gap_decomposition/decompose_table6.py, and reads the bad-length step
count from step_diagnostics.json. Writes summary.json and prints the tables
used in README.md.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import math
from pathlib import Path

STUDY = Path(__file__).resolve().parent
REPO_ROOT = STUDY.parents[2]
_spec = importlib.util.spec_from_file_location("gap", STUDY.parent / "gap_decomposition/decompose_table6.py")
gap = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(gap)
SEQUENCES = ("00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10")
DEVELOPMENT = ("07",)
REF, CAND = "dense_elevation_paper_normals_unbiased", "dense_elevation_paper_normals_unbiased_paper_bev"
ARMS = (REF, CAND)
COMPARED = ("kitti_rte_trans_pct", "rpe_trans_pct", "ate_m")
CHANNELS = ("scale", "yaw")


def geomean(values):
    return math.exp(sum(math.log(v) for v in values) / len(values))


def stored_reference(sequence: str) -> dict:
    path = f"experiments/results/elo_kitti_seq_{sequence}_full_paper_comp_matrix.json"
    aggregate = json.loads((REPO_ROOT / path).read_text())
    variant = next(v for v in aggregate["variants"] if v["id"] == REF)
    return {"aggregate": path, "frames": variant["frames"], **{k: variant[k] for k in COMPARED}}


def arm_row(runs_root: Path, sequence: str, arm: str, gt) -> dict:
    folder = runs_root / f"elo_kitti_seq_{sequence}_full_paper_bev_matrix" / arm
    method = json.loads((folder / "summary.json").read_text())["methods"][0]
    steps = json.loads((folder / "step_diagnostics.json").read_text())
    est = gap.normalize_first_pose(gap.load_kitti_poses(folder / "trajectory_kitti.txt"))
    return {"status": method["status"], "frames": method["frames"],
            **{k: method[k] for k in COMPARED},
            "bad_length_steps": steps["steps_length_off_over_threshold"],
            "moving_steps": steps["moving_step_count"],
            "path_length_ratio": steps["path_length_ratio"],
            **{f"{c}_only_rte_pct": gap.rte(gap.channel_trajectory(est, gt, c), gt) for c in CHANNELS}}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--runs-root", type=Path, default=REPO_ROOT / "experiments/results/runs")
    parser.add_argument("--output", type=Path, default=STUDY / "summary.json")
    args = parser.parse_args()
    paper_all = json.loads((REPO_ROOT / "evaluation/data/paper_reported_numbers.json").read_text())
    paper = {k.split("_")[1]: v for k, v in paper_all["methods"]["elo"]["reported_values"].items()}
    rows = []
    for sequence in SEQUENCES:
        gt = gap.normalize_first_pose(gap.load_reference_csv(
            REPO_ROOT / f"experiments/reference_data/kitti_seq_{sequence}_full_gt.csv"))
        arms = {arm: arm_row(args.runs_root, sequence, arm, gt) for arm in ARMS}
        ref, cand = arms[REF], arms[CAND]
        hist = stored_reference(sequence)
        rows.append({
            "sequence": sequence, "development": sequence in DEVELOPMENT,
            "paper_rte_pct": paper[sequence], "arms": arms, "stored_reference": hist,
            "fresh_reference_matches_stored": all(ref[k] == hist[k] for k in COMPARED) and ref["frames"] == hist["frames"],
            "ratio_to_paper": {arm: arms[arm]["kitti_rte_trans_pct"] / paper[sequence] for arm in ARMS},
            "candidate_over_reference": cand["kitti_rte_trans_pct"] / ref["kitti_rte_trans_pct"],
        })
    held_out = [r for r in rows if not r["development"]]
    aggregate = {
        "geometric_mean_ratio_to_paper": {arm: geomean([r["ratio_to_paper"][arm] for r in rows]) for arm in ARMS},
        "candidate_over_reference_geometric_mean": geomean([r["candidate_over_reference"] for r in rows]),
        "held_out_candidate_over_reference_geometric_mean": geomean([r["candidate_over_reference"] for r in held_out]),
        "held_out_lower": [r["sequence"] for r in held_out if r["candidate_over_reference"] < 1.0],
        "held_out_higher": [r["sequence"] for r in held_out if r["candidate_over_reference"] > 1.0],
        "adverse_over_5_percent": [r["sequence"] for r in rows if r["candidate_over_reference"] > 1.05],
        "above_twice_paper": {arm: [r["sequence"] for r in rows if r["ratio_to_paper"][arm] > 2.0] for arm in ARMS},
        "fresh_reference_matches_stored_all": all(r["fresh_reference_matches_stored"] for r in rows),
    }
    args.output.write_text(json.dumps({"schema_version": 1, "metric": "official KITTI RTE translation [%]",
                                       "paper_source": "ELO Table II, Frame-to-Model (proposed), t_rel (%)",
                                       "development_sequences": list(DEVELOPMENT),
                                       "rows": rows, "aggregate": aggregate}, indent=2) + "\n")
    print("| Seq | Paper | Reference | + paper BEV | Candidate / reference | "
          "Bad-length steps (ref -> cand) | Scale-only (ref -> cand) | Yaw-only (ref -> cand) | ATE ref / cand [m] |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|")
    for r in rows:
        a, b = r["arms"][REF], r["arms"][CAND]
        label = f"{r['sequence']} (dev)" if r["development"] else r["sequence"]
        print(f"| {label} | {r['paper_rte_pct']:.2f} | {a['kitti_rte_trans_pct']:.3f} ({r['ratio_to_paper'][REF]:.2f}x) | "
              f"{b['kitti_rte_trans_pct']:.3f} ({r['ratio_to_paper'][CAND]:.2f}x) | {r['candidate_over_reference']:.3f} | "
              f"{a['bad_length_steps']} -> {b['bad_length_steps']} | "
              f"{a['scale_only_rte_pct']:.3f} -> {b['scale_only_rte_pct']:.3f} | "
              f"{a['yaw_only_rte_pct']:.3f} -> {b['yaw_only_rte_pct']:.3f} | {a['ate_m']:.1f} / {b['ate_m']:.1f} |")
    g = aggregate["geometric_mean_ratio_to_paper"]
    print(f"\nGeometric mean vs paper: reference {g[REF]:.3f}x, candidate {g[CAND]:.3f}x; "
          f"candidate/reference {aggregate['candidate_over_reference_geometric_mean']:.3f}x "
          f"(held out {aggregate['held_out_candidate_over_reference_geometric_mean']:.3f}x); "
          f"held-out lower {aggregate['held_out_lower']}, higher {aggregate['held_out_higher']}; "
          f"adverse >5 % {aggregate['adverse_over_5_percent']}; above 2x {aggregate['above_twice_paper']}; "
          f"fresh reference == stored: {aggregate['fresh_reference_matches_stored_all']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
