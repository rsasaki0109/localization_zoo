#!/usr/bin/env python3
"""Summarize the feature-class arms of protocol.md.

For each arm directory (summary.json, trace.jsonl, dogfooding_results/) this
computes official RTE/ATE from the run summary. It also computes per-frame
relative-yaw error against the evaluated GT at the S2S output and at the
final pose: mean (bias), standard deviation, sum, and the yaw-only official
RTE, as in ../gap_decomposition/inten_heading.py. A compact per-frame CSV
(frame, S2S and final yaw error in degrees) is written next to the results
so the traces themselves need not be committed.
"""
from __future__ import annotations

import argparse
import csv
import importlib.util
import json
import math
from pathlib import Path

import numpy as np

STUDY = Path(__file__).resolve().parent
REPO_ROOT = STUDY.parents[2]
_spec = importlib.util.spec_from_file_location("heading", STUDY.parent / "gap_decomposition/inten_heading.py")
heading = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(heading)
ARMS = ("reference", "no_edge", "no_facade", "no_intensity", "no_ground")


def arm_result(folder: Path, out_dir: Path) -> dict:
    summary = json.loads((folder / "summary.json").read_text())["methods"][0]
    gt = heading.normalize_first_pose(heading.load_kitti_poses(
        next((folder / "dogfooding_results").glob("*_evaluated_gt.txt"))))
    est = heading.normalize_first_pose(heading.load_kitti_poses(folder / "dogfooding_results/InTEn_LOAM.txt"))
    s2s_out = {}
    stops: dict[str, int] = {}
    net_yaw, last_yaw = [], []
    with (folder / "trace.jsonl").open() as handle:
        for line in handle:
            head = line[:40].replace(" ", "")
            if '"type":"frame"' not in head:
                continue
            record = json.loads(line)
            s2s = record["s2s"]
            if not s2s.get("called"):
                continue
            s2s_out[record["input_index"]] = np.asarray(s2s["output"], dtype=float).reshape(4, 4)
            stops[s2s["stop"]] = stops.get(s2s["stop"], 0) + 1
            # S2S yaw correction relative to its own prediction, and the yaw of
            # the last applied update (delta layout: translation, rotation).
            before = np.asarray(s2s["input"], dtype=float).reshape(4, 4)
            net_yaw.append(heading.wrap(heading.yaw(s2s_out[record["input_index"]]) - heading.yaw(before)))
            deltas = [it["delta"] for it in s2s["iterations"] if it.get("delta")]
            if deltas:
                last_yaw.append(deltas[-1][5])
    gt_rel, est_rel = heading.relative(gt), heading.relative(est)
    s2s_err, final_err, frames = [], [], []
    for k, (G, E) in enumerate(zip(gt_rel, est_rel), start=1):
        if k not in s2s_out:
            continue
        frames.append(k)
        s2s_err.append(heading.wrap(heading.yaw(s2s_out[k]) - heading.yaw(G)))
        final_err.append(heading.wrap(heading.yaw(E) - heading.yaw(G)))
    s2s_err, final_err = np.array(s2s_err), np.array(final_err)
    with (out_dir / "per_frame_yaw_error_deg.csv").open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["frame", "s2s_yaw_error_deg", "final_yaw_error_deg"])
        for k, a, b in zip(frames, np.degrees(s2s_err), np.degrees(final_err)):
            writer.writerow([k, f"{a:.9f}", f"{b:.9f}"])
    complete = len(frames) == len(gt) - 1
    result = {"official_rte_pct": summary["kitti_rte_trans_pct"], "rpe_100m_pct": summary["rpe_trans_pct"],
              "ate_m": summary["ate_m"], "frames": summary["frames"], "s2s_frames": len(frames),
              "s2s": heading.stats(np.degrees(s2s_err)), "final": heading.stats(np.degrees(final_err)),
              "s2s_stop_reasons": stops,
              "s2s_net_yaw_correction_deg": heading.stats(np.degrees(np.array(net_yaw))),
              "s2s_last_iteration_yaw_update_deg": heading.stats(np.degrees(np.array(last_yaw)))}
    if complete:
        result["s2s"]["yaw_only_rte_pct"] = heading.yaw_only_rte(s2s_err, gt)
        result["final"]["yaw_only_rte_pct"] = heading.yaw_only_rte(final_err, gt)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--work-root", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, default=STUDY / "results")
    args = parser.parse_args()
    args.output_dir.mkdir(exist_ok=True)
    results = {}
    for arm in ARMS:
        folder = args.work_root / arm
        if not (folder / "summary.json").is_file():
            results[arm] = {"status": "missing"}
            continue
        out = args.output_dir / arm
        out.mkdir(exist_ok=True)
        for name in ("summary.json", "run.log"):
            (out / name).write_bytes((folder / name).read_bytes())
        results[arm] = arm_result(folder, out)
    reference = results.get("reference", {})
    for arm, r in results.items():
        if "final" not in r or "final" not in reference:
            continue
        r["final_bias_ratio_to_reference"] = r["final"]["mean_deg"] / reference["final"]["mean_deg"]
        r["rte_ratio_to_reference"] = r["official_rte_pct"] / reference["official_rte_pct"]
        r["implicated_by_rule"] = (abs(r["final"]["mean_deg"]) <= 0.5 * abs(reference["final"]["mean_deg"])
                                   and r["rte_ratio_to_reference"] <= 2.0 and arm != "reference")
    (args.output_dir / "summary.json").write_text(json.dumps({"schema_version": 1, "arms": results}, indent=2) + "\n")
    print(f"{'arm':13s} {'RTE%':>8s} {'ATE m':>8s} | {'S2S bias':>9s} {'S2S yawRTE':>10s} | {'final bias':>10s} {'final std':>9s} {'final yawRTE':>12s} implicated")
    for arm, r in results.items():
        if "final" not in r:
            print(arm, r)
            continue
        print(f"{arm:13s} {r['official_rte_pct']:8.4f} {r['ate_m']:8.2f} | {r['s2s']['mean_deg']:+9.5f} "
              f"{r['s2s'].get('yaw_only_rte_pct', float('nan')):10.4f} | {r['final']['mean_deg']:+10.5f} "
              f"{r['final']['std_deg']:9.5f} {r['final'].get('yaw_only_rte_pct', float('nan')):12.4f} "
              f"{r.get('implicated_by_rule', '-')}")
    print("S2S stop reasons, net yaw correction vs its prediction, last-iteration yaw update [deg]:")
    for arm, r in results.items():
        if "s2s_stop_reasons" in r:
            print(f"  {arm:13s} {r['s2s_stop_reasons']}  net {r['s2s_net_yaw_correction_deg']['mean_deg']:+.6f}  "
                  f"last {r['s2s_last_iteration_yaw_update_deg']['mean_deg']:+.6f} "
                  f"(mean abs {r['s2s_last_iteration_yaw_update_deg']['mean_abs_deg']:.5f})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
