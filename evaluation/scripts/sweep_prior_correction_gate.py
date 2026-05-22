#!/usr/bin/env python3
"""Replay cubemap prior-correction gates from logged prior/search candidates."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np

from intensity_bev_odometry_demo import compute_metrics, load_gt_xyyaw, se2_matrix


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--input-json", required=True, type=Path)
    p.add_argument("--gt-csv", required=True, type=Path)
    p.add_argument("--output-json", type=Path)
    p.add_argument("--min-score-gains", default="0.02,0.03,0.04,0.05,0.06")
    p.add_argument("--max-translations", default="0.25,0.4,0.6,0.8")
    p.add_argument("--max-yaws-deg", default="0.5,1.0,2.0")
    p.add_argument("--min-overlap-ratio", type=float, default=0.90)
    return p.parse_args()


def parse_float_list(csv: str) -> list[float]:
    return [float(x.strip()) for x in csv.split(",") if x.strip()]


def accept(pair: dict, min_gain: float, max_translation: float, max_yaw_deg: float, min_overlap_ratio: float) -> bool:
    gain = pair["search_score"] - pair["prior_score"]
    if gain < min_gain:
        return False
    if pair["prior_correction_translation_delta_m"] > max_translation:
        return False
    if pair["prior_correction_yaw_delta_deg"] > max_yaw_deg:
        return False
    prior_overlap = pair.get("prior_overlap_pixels") or 0
    if prior_overlap > 0:
        if pair["search_overlap_pixels"] / float(prior_overlap) < min_overlap_ratio:
            return False
    return True


def candidate_from_pair(pair: dict, use_search: bool) -> tuple[float, float, float]:
    prefix = "search" if use_search else "prior"
    return (
        float(pair[f"{prefix}_dx_curr_to_prev_m"]),
        float(pair[f"{prefix}_dy_curr_to_prev_m"]),
        math.radians(float(pair[f"{prefix}_yaw_curr_to_prev_deg"])),
    )


def replay(pairs: list[dict], min_gain: float, max_translation: float, max_yaw_deg: float, min_overlap_ratio: float) -> tuple[np.ndarray, int]:
    pose = np.eye(3)
    poses = [[0.0, 0.0, 0.0]]
    accepted = 0
    for pair in pairs:
        use_search = accept(pair, min_gain, max_translation, max_yaw_deg, min_overlap_ratio)
        accepted += int(use_search)
        dx, dy, yaw = candidate_from_pair(pair, use_search)
        pose = pose @ se2_matrix(dx, dy, yaw)
        poses.append([float(pose[0, 2]), float(pose[1, 2]), float(math.atan2(pose[1, 0], pose[0, 0]))])
    return np.asarray(poses, dtype=np.float64), accepted


def main() -> int:
    args = parse_args()
    payload = json.loads(args.input_json.read_text())
    pairs = payload["pairs"]
    gt = load_gt_xyyaw(args.gt_csv)
    rows = []
    for min_gain in parse_float_list(args.min_score_gains):
        for max_translation in parse_float_list(args.max_translations):
            for max_yaw_deg in parse_float_list(args.max_yaws_deg):
                poses, accepted = replay(
                    pairs,
                    min_gain,
                    max_translation,
                    max_yaw_deg,
                    args.min_overlap_ratio,
                )
                metrics = compute_metrics(poses, gt)
                rows.append({
                    "min_score_gain": min_gain,
                    "max_translation": max_translation,
                    "max_yaw_deg": max_yaw_deg,
                    "min_overlap_ratio": args.min_overlap_ratio,
                    "accepted_corrections": accepted,
                    "metrics": metrics,
                })
    rows.sort(key=lambda r: (r["metrics"]["ate_xy_m"], r["metrics"]["step_length_rmse_m"]))
    for row in rows[:10]:
        m = row["metrics"]
        print(
            f"gain={row['min_score_gain']:.3f} trans={row['max_translation']:.2f} "
            f"yaw={row['max_yaw_deg']:.1f} accepted={row['accepted_corrections']} "
            f"ATE={m['ate_xy_m']:.3f} step={m['step_length_rmse_m']:.3f}"
        )
    if args.output_json is not None:
        args.output_json.parent.mkdir(parents=True, exist_ok=True)
        args.output_json.write_text(json.dumps({"input_json": str(args.input_json), "rows": rows}, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
