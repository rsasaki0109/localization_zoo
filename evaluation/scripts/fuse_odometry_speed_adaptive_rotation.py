#!/usr/bin/env python3
"""Causally fuse odometry rotations with a primary-motion speed schedule."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import time
from pathlib import Path

import numpy as np

from fuse_odometry_incremental_rotation import (
    exp_so3,
    load_kitti_poses,
    log_so3,
    write_kitti_poses,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--primary-poses", required=True)
    parser.add_argument("--reference-poses", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--low-speed-rotation-blend", type=float, required=True)
    parser.add_argument("--high-speed-rotation-blend", type=float, default=0.05)
    parser.add_argument("--low-speed-threshold-m", type=float, default=1.0)
    parser.add_argument("--strong-blend-max-disagreement-deg", type=float, default=5.0)
    parser.add_argument("--max-strong-blend-fraction", type=float, default=1.0)
    parser.add_argument("--max-increment-disagreement-deg", type=float, default=5.0)
    parser.add_argument("--max-cumulative-correction-deg", type=float, default=180.0)
    parser.add_argument("--manifest")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def fuse_speed_adaptive_rotations(
    primary: list[np.ndarray],
    reference: list[np.ndarray],
    low_speed_rotation_blend: float,
    high_speed_rotation_blend: float,
    low_speed_threshold_m: float,
    strong_blend_max_disagreement_rad: float,
    max_strong_blend_fraction: float,
    max_disagreement_rad: float,
    max_cumulative_correction_rad: float = math.pi,
) -> tuple[list[np.ndarray], dict[str, int]]:
    if len(primary) != len(reference):
        raise RuntimeError("Primary and reference pose counts differ")
    if not 0.0 <= low_speed_rotation_blend <= 1.0:
        raise RuntimeError("low_speed_rotation_blend must be in [0, 1]")
    if not 0.0 <= high_speed_rotation_blend <= 1.0:
        raise RuntimeError("high_speed_rotation_blend must be in [0, 1]")
    if low_speed_threshold_m < 0.0:
        raise RuntimeError("low_speed_threshold_m must be non-negative")
    if not 0.0 <= max_strong_blend_fraction <= 1.0:
        raise RuntimeError("max_strong_blend_fraction must be in [0, 1]")
    if max_cumulative_correction_rad < 0.0:
        raise RuntimeError("max_cumulative_correction_rad must be non-negative")

    output = [primary[0].copy()]
    counters = {
        "strong_blend_candidates": 0,
        "low_speed_increments": 0,
        "high_speed_increments": 0,
        "rate_limited_increments": 0,
        "gated_increments": 0,
        "cumulative_correction_clamps": 0,
    }
    for index in range(1, len(primary)):
        primary_delta = np.linalg.inv(primary[index - 1]) @ primary[index]
        reference_delta = np.linalg.inv(reference[index - 1]) @ reference[index]
        disagreement = log_so3(primary_delta[:3, :3].T @ reference_delta[:3, :3])
        primary_step_m = float(np.linalg.norm(primary_delta[:3, 3]))
        disagreement_norm = float(np.linalg.norm(disagreement))
        strong_candidate = (
            primary_step_m <= low_speed_threshold_m
            and disagreement_norm <= strong_blend_max_disagreement_rad
        )
        if strong_candidate:
            counters["strong_blend_candidates"] += 1
        strong_budget = math.floor(max_strong_blend_fraction * index)
        if strong_candidate and counters["low_speed_increments"] < strong_budget:
            blend = low_speed_rotation_blend
            counters["low_speed_increments"] += 1
        else:
            blend = high_speed_rotation_blend
            counters["high_speed_increments"] += 1
            if strong_candidate:
                counters["rate_limited_increments"] += 1
        if disagreement_norm > max_disagreement_rad:
            blend = 0.0
            counters["gated_increments"] += 1
        fused_delta = np.eye(4)
        fused_delta[:3, :3] = primary_delta[:3, :3] @ exp_so3(blend * disagreement)
        fused_delta[:3, 3] = primary_delta[:3, 3]
        proposed = output[-1] @ fused_delta
        cumulative_correction = log_so3(
            primary[index][:3, :3].T @ proposed[:3, :3]
        )
        correction_norm = float(np.linalg.norm(cumulative_correction))
        if correction_norm > max_cumulative_correction_rad:
            proposed[:3, :3] = primary[index][:3, :3] @ exp_so3(
                cumulative_correction
                * (max_cumulative_correction_rad / correction_norm)
            )
            counters["cumulative_correction_clamps"] += 1
        output.append(proposed)
    return output, counters


def main() -> int:
    args = parse_args()
    primary_path = Path(args.primary_poses).resolve()
    reference_path = Path(args.reference_poses).resolve()
    output_path = Path(args.output).resolve()
    started = time.perf_counter()
    output, counters = fuse_speed_adaptive_rotations(
        load_kitti_poses(primary_path),
        load_kitti_poses(reference_path),
        args.low_speed_rotation_blend,
        args.high_speed_rotation_blend,
        args.low_speed_threshold_m,
        math.radians(args.strong_blend_max_disagreement_deg),
        args.max_strong_blend_fraction,
        math.radians(args.max_increment_disagreement_deg),
        math.radians(args.max_cumulative_correction_deg),
    )
    write_kitti_poses(output_path, output)
    elapsed = time.perf_counter() - started
    payload = {
        "schema_version": 1,
        "method": "causal_speed_adaptive_incremental_rotation_blend",
        "ground_truth_used": False,
        "frames": len(output),
        "low_speed_rotation_blend": args.low_speed_rotation_blend,
        "high_speed_rotation_blend": args.high_speed_rotation_blend,
        "low_speed_threshold_m": args.low_speed_threshold_m,
        "strong_blend_max_disagreement_deg": args.strong_blend_max_disagreement_deg,
        "max_strong_blend_fraction": args.max_strong_blend_fraction,
        "max_increment_disagreement_deg": args.max_increment_disagreement_deg,
        "max_cumulative_correction_deg": args.max_cumulative_correction_deg,
        **counters,
        "translation_policy": "copy primary relative translation only; never use reference translation",
        "causality": "speed schedule and output i use primary/reference increments only through i",
        "seconds_including_pose_io": elapsed,
        "fps_including_pose_io": len(output) / elapsed if elapsed > 0 else None,
        "primary_sha256": sha256_file(primary_path),
        "reference_sha256": sha256_file(reference_path),
        "output_sha256": sha256_file(output_path),
    }
    if args.manifest:
        manifest_path = Path(args.manifest).resolve()
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
