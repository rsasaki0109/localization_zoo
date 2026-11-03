#!/usr/bin/env python3
"""Tabulate the L-LO ground-height-map study (protocol.md).

Reads experiments/results/runs/l_lo_kitti_seq_NN_full_ground_map_matrix/<arm>/
(or --runs-root), the paper's L-LO values from paper_reported_numbers.json and
the stored seq07_tuned results listed in
experiments/results/remaining_kitti_coverage/summary.json. For each arm it also
scores the horizontal-only and vertical-only channels of the saved poses, as
in ../gap_decomposition/decompose_table6.py. Writes summary.json and prints
the tables used in README.md.
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
SEQUENCES = ("00", "02", "03", "04", "05", "06", "07", "08", "09", "10")
ARMS = ("seq07_tuned", "seq07_tuned_ground_map")
COMPARED = ("kitti_rte_trans_pct", "rpe_trans_pct", "ate_m")


def geomean(values):
    return math.exp(sum(math.log(v) for v in values) / len(values))


def stored_reference() -> dict:
    coverage = json.loads((REPO_ROOT / "experiments/results/remaining_kitti_coverage/summary.json").read_text())
    out = {}
    for row in coverage["methods"]["l_lo"]["rows"]:
        aggregate = json.loads((REPO_ROOT / row["aggregate"]).read_text())
        variant = next(v for v in aggregate["variants"] if v["id"] == "seq07_tuned")
        out[row["sequence"]] = {"aggregate": row["aggregate"], "frames": row["frames"],
                                **{k: variant[k] for k in COMPARED}}
    return out


def arm_row(runs_root: Path, sequence: str, arm: str, gt) -> dict:
    folder = runs_root / f"l_lo_kitti_seq_{sequence}_full_ground_map_matrix" / arm
    method = json.loads((folder / "summary.json").read_text())["methods"][0]
    est = gap.normalize_first_pose(gap.load_kitti_poses(folder / "trajectory_kitti.txt"))
    return {"status": method["status"], "frames": method["frames"],
            **{k: method[k] for k in COMPARED},
            "horizontal_only_rte_pct": gap.rte(gap.channel_trajectory(est, gt, "horizontal"), gt),
            "vertical_only_rte_pct": gap.rte(gap.channel_trajectory(est, gt, "vertical"), gt)}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--runs-root", type=Path, default=REPO_ROOT / "experiments/results/runs")
    parser.add_argument("--output", type=Path, default=STUDY / "summary.json")
    args = parser.parse_args()
    paper_all = json.loads((REPO_ROOT / "evaluation/data/paper_reported_numbers.json").read_text())
    paper = {k.split("_")[1]: v for k, v in paper_all["methods"]["l_lo"]["reported_values"].items()}
    stored = stored_reference()
    rows = []
    for sequence in SEQUENCES:
        gt = gap.normalize_first_pose(gap.load_reference_csv(
            REPO_ROOT / f"experiments/reference_data/kitti_seq_{sequence}_full_gt.csv"))
        arms = {arm: arm_row(args.runs_root, sequence, arm, gt) for arm in ARMS}
        ref, cand = arms["seq07_tuned"], arms["seq07_tuned_ground_map"]
        hist = stored[sequence]
        rows.append({
            "sequence": sequence, "paper_rte_pct": paper[sequence], "arms": arms,
            "stored_seq07_tuned": hist,
            "fresh_reference_matches_stored": all(ref[k] == hist[k] for k in COMPARED) and ref["frames"] == hist["frames"],
            "ratio_to_paper": {arm: arms[arm]["kitti_rte_trans_pct"] / paper[sequence] for arm in ARMS},
            "candidate_over_reference": cand["kitti_rte_trans_pct"] / ref["kitti_rte_trans_pct"],
        })
    lower = [r["sequence"] for r in rows if r["candidate_over_reference"] < 1.0]
    higher = [r["sequence"] for r in rows if r["candidate_over_reference"] > 1.0]
    aggregate = {
        "geometric_mean_ratio_to_paper": {arm: geomean([r["ratio_to_paper"][arm] for r in rows]) for arm in ARMS},
        "candidate_over_reference_geometric_mean": geomean([r["candidate_over_reference"] for r in rows]),
        "lower": lower, "higher": higher,
        "above_twice_paper": {arm: [r["sequence"] for r in rows if r["ratio_to_paper"][arm] > 2.0] for arm in ARMS},
        "fresh_reference_matches_stored_all": all(r["fresh_reference_matches_stored"] for r in rows),
    }
    args.output.write_text(json.dumps({"schema_version": 1, "metric": "official KITTI RTE translation [%]",
                                       "paper_source": "arXiv 2312.16787 Table I (L-LO)",
                                       "rows": rows, "aggregate": aggregate}, indent=2) + "\n")
    print("| Seq | Paper | `seq07_tuned` | + ground map | Candidate / reference | Vertical-only (ref -> cand) | Horizontal-only (ref -> cand) | ATE ref / cand [m] |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|")
    for r in rows:
        a, b = r["arms"]["seq07_tuned"], r["arms"]["seq07_tuned_ground_map"]
        print(f"| {r['sequence']} | {r['paper_rte_pct']:.2f} | {a['kitti_rte_trans_pct']:.3f} ({r['ratio_to_paper']['seq07_tuned']:.2f}x) | "
              f"{b['kitti_rte_trans_pct']:.3f} ({r['ratio_to_paper']['seq07_tuned_ground_map']:.2f}x) | {r['candidate_over_reference']:.3f} | "
              f"{a['vertical_only_rte_pct']:.3f} -> {b['vertical_only_rte_pct']:.3f} | "
              f"{a['horizontal_only_rte_pct']:.3f} -> {b['horizontal_only_rte_pct']:.3f} | {a['ate_m']:.1f} / {b['ate_m']:.1f} |")
    g = aggregate["geometric_mean_ratio_to_paper"]
    print(f"\nGeometric mean vs paper: seq07_tuned {g['seq07_tuned']:.3f}x, ground map {g['seq07_tuned_ground_map']:.3f}x; "
          f"candidate/reference {aggregate['candidate_over_reference_geometric_mean']:.3f}x; lower {lower}, higher {higher}; "
          f"above 2x {aggregate['above_twice_paper']}; fresh reference == stored: {aggregate['fresh_reference_matches_stored_all']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
