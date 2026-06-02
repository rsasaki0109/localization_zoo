#!/usr/bin/env python3
"""Replay keyframe anchor corrections on top of an existing pair-prior JSON."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from keyframe_correction_component import (
    KeyframeCorrectionConfig,
    apply_keyframe_corrections,
)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--prior-json", required=True, type=Path)
    p.add_argument("--keyframe-json", required=True, type=Path)
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument("--gt-csv", type=Path)
    p.add_argument("--recipe-json", type=Path)
    p.add_argument("--max-keyframe-correction", type=float, default=0.5)
    p.add_argument("--max-keyframe-yaw-deg", type=float, default=5.0)
    p.add_argument("--max-keyframe-rmse", type=float, default=1.2)
    p.add_argument("--min-keyframe-correspondences", type=int, default=6000)
    return p.parse_args()


def main() -> int:
    args = parse_args()
    prior = json.loads(args.prior_json.read_text())
    keyframes = json.loads(args.keyframe_json.read_text())
    if args.recipe_json:
        recipe = json.loads(args.recipe_json.read_text())
        config = KeyframeCorrectionConfig.from_recipe(recipe)
    else:
        config = KeyframeCorrectionConfig(
            max_keyframe_correction_m=args.max_keyframe_correction,
            max_keyframe_yaw_deg=args.max_keyframe_yaw_deg,
            max_keyframe_rmse_m=args.max_keyframe_rmse,
            min_keyframe_correspondences=args.min_keyframe_correspondences,
        )
    payload = apply_keyframe_corrections(
        prior,
        keyframes,
        config,
        source_prior_json=str(args.prior_json),
        source_keyframe_json=str(args.keyframe_json),
        gt_csv=args.gt_csv,
    )
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    metrics = payload.get("metrics")
    correction_counts = payload["correction_counts"]
    if metrics:
        print(
            f"[done] ATE_xy={metrics['ate_xy_m']:.3f}m "
            f"step_rmse={metrics['step_length_rmse_m']:.3f}m corrections={correction_counts} "
            f"-> {args.output_json}"
        )
    else:
        print(f"[done] corrections={correction_counts} -> {args.output_json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
