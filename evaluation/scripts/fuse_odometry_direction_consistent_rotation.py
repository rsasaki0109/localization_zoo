#!/usr/bin/env python3
"""Causally gate rotation fusion by cross-frontend travel-direction agreement."""

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
    parser.add_argument("--window-frames", type=int, default=20)
    parser.add_argument("--min-window-displacement-m", type=float, default=2.0)
    parser.add_argument("--min-direction-disagreement-deg", type=float, default=0.05)
    parser.add_argument("--low-speed-rotation-blend", type=float, default=1.0)
    parser.add_argument("--high-speed-rotation-blend", type=float, default=0.05)
    parser.add_argument("--low-speed-threshold-m", type=float, default=1.0)
    parser.add_argument("--strong-blend-max-disagreement-deg", type=float, default=0.1)
    parser.add_argument("--max-strong-blend-fraction", type=float, default=0.05)
    parser.add_argument("--max-increment-disagreement-deg", type=float, default=5.0)
    parser.add_argument("--max-cumulative-correction-deg", type=float, default=0.1)
    parser.add_argument("--manifest")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def signed_xy_direction_error(primary_delta: np.ndarray, reference_delta: np.ndarray) -> float:
    """Return the signed yaw taking primary XY travel toward reference travel."""
    cross = primary_delta[0] * reference_delta[1] - primary_delta[1] * reference_delta[0]
    dot = float(np.dot(primary_delta[:2], reference_delta[:2]))
    return math.atan2(float(cross), dot)


def clamp_to_primary(
    proposed: np.ndarray,
    primary: np.ndarray,
    max_correction_rad: float,
) -> tuple[np.ndarray, bool]:
    correction = log_so3(primary[:3, :3].T @ proposed[:3, :3])
    norm = float(np.linalg.norm(correction))
    if norm <= max_correction_rad:
        return proposed, False
    clamped = proposed.copy()
    clamped[:3, :3] = primary[:3, :3] @ exp_so3(
        correction * (max_correction_rad / norm)
    )
    return clamped, True


def fuse_direction_consistent_rotations(
    primary: list[np.ndarray],
    reference: list[np.ndarray],
    *,
    window_frames: int = 20,
    min_window_displacement_m: float = 2.0,
    min_direction_disagreement_rad: float = math.radians(0.05),
    low_speed_rotation_blend: float = 1.0,
    high_speed_rotation_blend: float = 0.05,
    low_speed_threshold_m: float = 1.0,
    strong_blend_max_disagreement_rad: float = math.radians(0.1),
    max_strong_blend_fraction: float = 0.05,
    max_increment_disagreement_rad: float = math.radians(5.0),
    max_cumulative_correction_rad: float = math.radians(0.1),
    reset_to_primary_on_reject: bool = True,
) -> tuple[list[np.ndarray], dict[str, int]]:
    if len(primary) != len(reference):
        raise RuntimeError("Primary and reference pose counts differ")
    if window_frames < 1:
        raise RuntimeError("window_frames must be positive")
    if min_window_displacement_m < 0.0:
        raise RuntimeError("min_window_displacement_m must be non-negative")
    if not 0.0 <= max_strong_blend_fraction <= 1.0:
        raise RuntimeError("max_strong_blend_fraction must be in [0, 1]")

    output = [primary[0].copy()]
    counters = {
        "strong_blend_candidates": 0,
        "strong_blend_increments": 0,
        "weak_blend_increments": 0,
        "rate_limited_increments": 0,
        "hard_gated_increments": 0,
        "direction_accepted_increments": 0,
        "direction_rejected_increments": 0,
        "direction_warmup_or_low_motion": 0,
        "direction_reject_orientation_resets": 0,
        "cumulative_correction_clamps": 0,
    }
    for index in range(1, len(primary)):
        primary_delta = np.linalg.inv(primary[index - 1]) @ primary[index]
        reference_delta = np.linalg.inv(reference[index - 1]) @ reference[index]
        disagreement = log_so3(primary_delta[:3, :3].T @ reference_delta[:3, :3])
        disagreement_norm = float(np.linalg.norm(disagreement))
        primary_step_m = float(np.linalg.norm(primary_delta[:3, 3]))
        strong_candidate = (
            primary_step_m <= low_speed_threshold_m
            and disagreement_norm <= strong_blend_max_disagreement_rad
        )
        if strong_candidate:
            counters["strong_blend_candidates"] += 1
        strong_budget = math.floor(max_strong_blend_fraction * index)
        if strong_candidate and counters["strong_blend_increments"] < strong_budget:
            blend = low_speed_rotation_blend
            counters["strong_blend_increments"] += 1
        else:
            blend = high_speed_rotation_blend
            counters["weak_blend_increments"] += 1
            if strong_candidate:
                counters["rate_limited_increments"] += 1
        if disagreement_norm > max_increment_disagreement_rad:
            blend = 0.0
            counters["hard_gated_increments"] += 1

        direction_signal: float | None = None
        if index >= window_frames:
            primary_travel = primary[index][:3, 3] - primary[index - window_frames][:3, 3]
            reference_travel = (
                reference[index][:3, 3] - reference[index - window_frames][:3, 3]
            )
            if (
                np.linalg.norm(primary_travel[:2]) >= min_window_displacement_m
                and np.linalg.norm(reference_travel[:2]) >= min_window_displacement_m
            ):
                direction_signal = signed_xy_direction_error(primary_travel, reference_travel)

        fused_delta = np.eye(4)
        fused_delta[:3, :3] = primary_delta[:3, :3] @ exp_so3(blend * disagreement)
        fused_delta[:3, 3] = primary_delta[:3, 3]
        proposed = output[-1] @ fused_delta
        proposed, clamped = clamp_to_primary(
            proposed, primary[index], max_cumulative_correction_rad
        )
        counters["cumulative_correction_clamps"] += int(clamped)
        proposed_correction = log_so3(
            primary[index][:3, :3].T @ proposed[:3, :3]
        )
        direction_consistent = (
            direction_signal is not None
            and abs(direction_signal) >= min_direction_disagreement_rad
            and float(proposed_correction[2]) * direction_signal > 0.0
        )
        if blend > 0.0 and direction_consistent:
            counters["direction_accepted_increments"] += 1
        elif blend > 0.0:
            if direction_signal is None:
                counters["direction_warmup_or_low_motion"] += 1
            else:
                counters["direction_rejected_increments"] += 1
            primary_only_delta = np.eye(4)
            primary_only_delta[:3, :3] = primary_delta[:3, :3]
            primary_only_delta[:3, 3] = primary_delta[:3, 3]
            proposed = output[-1] @ primary_only_delta
            if reset_to_primary_on_reject:
                proposed[:3, :3] = primary[index][:3, :3]
                counters["direction_reject_orientation_resets"] += 1
            else:
                proposed, clamped = clamp_to_primary(
                    proposed, primary[index], max_cumulative_correction_rad
                )
                counters["cumulative_correction_clamps"] += int(clamped)
        output.append(proposed)
    return output, counters


def main() -> int:
    args = parse_args()
    primary_path = Path(args.primary_poses).resolve()
    reference_path = Path(args.reference_poses).resolve()
    output_path = Path(args.output).resolve()
    started = time.perf_counter()
    output, counters = fuse_direction_consistent_rotations(
        load_kitti_poses(primary_path),
        load_kitti_poses(reference_path),
        window_frames=args.window_frames,
        min_window_displacement_m=args.min_window_displacement_m,
        min_direction_disagreement_rad=math.radians(args.min_direction_disagreement_deg),
        low_speed_rotation_blend=args.low_speed_rotation_blend,
        high_speed_rotation_blend=args.high_speed_rotation_blend,
        low_speed_threshold_m=args.low_speed_threshold_m,
        strong_blend_max_disagreement_rad=math.radians(args.strong_blend_max_disagreement_deg),
        max_strong_blend_fraction=args.max_strong_blend_fraction,
        max_increment_disagreement_rad=math.radians(args.max_increment_disagreement_deg),
        max_cumulative_correction_rad=math.radians(args.max_cumulative_correction_deg),
    )
    write_kitti_poses(output_path, output)
    elapsed = time.perf_counter() - started
    payload = {
        "schema_version": 1,
        "method": "causal_direction_consistent_rotation_fusion",
        "ground_truth_used": False,
        "frames": len(output),
        "window_frames": args.window_frames,
        "min_window_displacement_m": args.min_window_displacement_m,
        "min_direction_disagreement_deg": args.min_direction_disagreement_deg,
        "low_speed_rotation_blend": args.low_speed_rotation_blend,
        "high_speed_rotation_blend": args.high_speed_rotation_blend,
        "low_speed_threshold_m": args.low_speed_threshold_m,
        "strong_blend_max_disagreement_deg": args.strong_blend_max_disagreement_deg,
        "max_strong_blend_fraction": args.max_strong_blend_fraction,
        "max_increment_disagreement_deg": args.max_increment_disagreement_deg,
        "max_cumulative_correction_deg": args.max_cumulative_correction_deg,
        **counters,
        "translation_policy": "primary relative translation only; reference translation is a causal gate signal only",
        "causality": "output i uses primary/reference poses only through i",
        "seconds_including_pose_io": elapsed,
        "fps_including_pose_io": len(output) / elapsed if elapsed > 0.0 else None,
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
