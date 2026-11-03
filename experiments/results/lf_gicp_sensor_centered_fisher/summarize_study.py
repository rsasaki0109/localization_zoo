#!/usr/bin/env python3
"""Tabulate the LF-GICP sensor-centred Fisher study from its run summaries.

Reads experiments/results/runs/lf_gicp_kitti_seq_NN_full_sensor_fisher_matrix/<arm>/summary.json
(or the same layout under --runs-root), the paper's Table I values from
evaluation/data/paper_reported_numbers.json, the paper's vanilla VGICP column,
and the historical paper_default results recorded in
experiments/results/remaining_kitti_coverage/summary.json. Writes summary.json
and prints the Markdown tables used in README.md.
"""

from __future__ import annotations

import argparse
import json
import math
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
STUDY_DIR = Path(__file__).resolve().parent
SEQUENCES = [f"{i:02d}" for i in range(11)]
ARMS = ("paper_default", "sensor_centered_fisher", "no_mitigation")
# Paper Table I, "VGICP" column: the paper's backend without degeneracy handling.
PAPER_VGICP = {"00": 1.062, "01": 2.390, "02": 1.429, "03": 1.165, "04": 1.065, "05": 0.841,
               "06": 0.698, "07": 0.596, "08": 1.110, "09": 1.268, "10": 1.604}
COMPARED_FIELDS = ("kitti_rte_trans_pct", "rpe_trans_pct", "ate_m")


def geomean(values):
    return math.exp(sum(math.log(v) for v in values) / len(values))


def load_arm(runs_root: Path, sequence: str, arm: str) -> dict:
    path = runs_root / f"lf_gicp_kitti_seq_{sequence}_full_sensor_fisher_matrix" / arm / "summary.json"
    method = json.loads(path.read_text())["methods"][0]
    gated = re.search(r"degenerate_frames=(\d+)/(\d+)", method.get("note", ""))
    return {
        "summary": str(path.relative_to(REPO_ROOT)) if path.is_relative_to(REPO_ROOT) else str(path),
        "status": method["status"],
        "frames": method["frames"],
        "kitti_rte_trans_pct": method["kitti_rte_trans_pct"],
        "kitti_rte_rot_deg_per_100m": method["kitti_rte_rot_deg_per_100m"],
        "rpe_trans_pct": method["rpe_trans_pct"],
        "ate_m": method["ate_m"],
        "fps": method["fps"],
        "gated_frames": int(gated.group(1)) if gated else None,
        "registered_frames": int(gated.group(2)) if gated else None,
    }


def historical_rows() -> dict:
    coverage = json.loads((REPO_ROOT / "experiments/results/remaining_kitti_coverage/summary.json").read_text())
    rows = {}
    for row in coverage["methods"]["lf_gicp"]["rows"]:
        aggregate = json.loads((REPO_ROOT / row["aggregate"]).read_text())
        variant = next(v for v in aggregate["variants"] if v["id"] == "paper_default")
        rows[row["sequence"]] = {"aggregate": row["aggregate"],
                                 **{k: variant[k] for k in COMPARED_FIELDS},
                                 "frames": row["frames"]}
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--runs-root", type=Path, default=REPO_ROOT / "experiments/results/runs")
    parser.add_argument("--output", type=Path, default=STUDY_DIR / "summary.json")
    args = parser.parse_args()

    paper = json.loads((REPO_ROOT / "evaluation/data/paper_reported_numbers.json").read_text())
    paper_rte = {k.split("_")[1]: v for k, v in paper["methods"]["lf_gicp"]["reported_values"].items()}
    historical = historical_rows()

    rows = []
    for sequence in SEQUENCES:
        arms = {arm: load_arm(args.runs_root, sequence, arm) for arm in ARMS}
        hist = historical[sequence]
        ref = arms["paper_default"]
        reference_matches = (all(ref[k] == hist[k] for k in COMPARED_FIELDS)
                             and ref["frames"] == hist["frames"])
        cand = arms["sensor_centered_fisher"]["kitti_rte_trans_pct"]
        rows.append({
            "sequence": sequence,
            "paper_lf_gicp_rte_pct": paper_rte[sequence],
            "paper_vgicp_rte_pct": PAPER_VGICP[sequence],
            "arms": arms,
            "historical_paper_default": hist,
            "fresh_reference_matches_historical": reference_matches,
            "candidate_over_reference": cand / ref["kitti_rte_trans_pct"],
            "ratio_to_paper": {arm: arms[arm]["kitti_rte_trans_pct"] / paper_rte[sequence] for arm in ARMS},
            "no_mitigation_over_paper_vgicp": arms["no_mitigation"]["kitti_rte_trans_pct"] / PAPER_VGICP[sequence],
        })

    def count(arm_a, arm_b, cmp):
        return sum(1 for r in rows if cmp(r["arms"][arm_a]["kitti_rte_trans_pct"],
                                          r["arms"][arm_b]["kitti_rte_trans_pct"]))

    aggregate = {
        "geometric_mean_ratio_to_paper": {arm: geomean([r["ratio_to_paper"][arm] for r in rows]) for arm in ARMS},
        "mean_rte_pct": {arm: sum(r["arms"][arm]["kitti_rte_trans_pct"] for r in rows) / len(rows) for arm in ARMS},
        "paper_mean_rte_pct": {"lf_gicp": sum(paper_rte.values()) / len(paper_rte),
                               "vgicp": sum(PAPER_VGICP.values()) / len(PAPER_VGICP)},
        "candidate_over_reference_geometric_mean": geomean([r["candidate_over_reference"] for r in rows]),
        "candidate_vs_reference": {
            "lower": count("sensor_centered_fisher", "paper_default", lambda a, b: a < b),
            "higher": count("sensor_centered_fisher", "paper_default", lambda a, b: a > b),
            "equal": count("sensor_centered_fisher", "paper_default", lambda a, b: a == b)},
        "candidate_vs_no_mitigation": {
            "lower": count("sensor_centered_fisher", "no_mitigation", lambda a, b: a < b),
            "higher": count("sensor_centered_fisher", "no_mitigation", lambda a, b: a > b)},
        "above_twice_paper": {arm: [r["sequence"] for r in rows if r["ratio_to_paper"][arm] > 2.0] for arm in ARMS},
        "no_mitigation_over_paper_vgicp_geometric_mean": geomean([r["no_mitigation_over_paper_vgicp"] for r in rows]),
        "fresh_reference_matches_historical_all": all(r["fresh_reference_matches_historical"] for r in rows),
    }
    out = {"schema_version": 1, "study": "lf_gicp_sensor_centered_fisher",
           "metric": "official KITTI translational RTE [%] (100-800 m)",
           "paper_source": "arXiv 2608.19522 Table I (LF-GICP and VGICP columns)",
           "rows": rows, "aggregate": aggregate}
    args.output.write_text(json.dumps(out, indent=2) + "\n")

    print("| Seq | Paper LF-GICP | `paper_default` | Sensor-centred | Candidate / reference | Gated frames (ref / cand) | No mitigation | Paper VGICP |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|")
    for r in rows:
        a = r["arms"]
        print(f"| {r['sequence']} | {r['paper_lf_gicp_rte_pct']:.3f} | {a['paper_default']['kitti_rte_trans_pct']:.3f} "
              f"({r['ratio_to_paper']['paper_default']:.2f}x) | {a['sensor_centered_fisher']['kitti_rte_trans_pct']:.3f} "
              f"({r['ratio_to_paper']['sensor_centered_fisher']:.2f}x) | {r['candidate_over_reference']:.3f} | "
              f"{a['paper_default']['gated_frames']} / {a['sensor_centered_fisher']['gated_frames']} of "
              f"{a['paper_default']['registered_frames']} | {a['no_mitigation']['kitti_rte_trans_pct']:.3f} "
              f"({r['ratio_to_paper']['no_mitigation']:.2f}x) | {r['paper_vgicp_rte_pct']:.3f} |")
    g = aggregate["geometric_mean_ratio_to_paper"]
    print(f"\nGeometric mean ratio to paper: paper_default {g['paper_default']:.3f}x, "
          f"sensor-centred {g['sensor_centered_fisher']:.3f}x, no mitigation {g['no_mitigation']:.3f}x")
    print(f"Mean RTE: {aggregate['mean_rte_pct']}; paper {aggregate['paper_mean_rte_pct']}")
    print(f"Candidate vs reference: {aggregate['candidate_vs_reference']}, geometric mean "
          f"{aggregate['candidate_over_reference_geometric_mean']:.3f}x")
    print(f"Candidate vs no mitigation: {aggregate['candidate_vs_no_mitigation']}")
    print(f"Above twice paper: {aggregate['above_twice_paper']}")
    print(f"Fresh reference equals historical on every sequence: {aggregate['fresh_reference_matches_historical_all']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
