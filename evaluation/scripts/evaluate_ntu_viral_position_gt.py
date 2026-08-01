#!/usr/bin/env python3
"""Evaluate a TUM trajectory against NTU VIRAL position-only Leica GT."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trajectory", type=Path, required=True)
    parser.add_argument("--gt-csv", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--method", required=True)
    parser.add_argument("--lidar-frames", type=int, required=True)
    parser.add_argument("--runtime-seconds", type=float)
    parser.add_argument("--segment-length", type=float, default=100.0)
    parser.add_argument("--max-gt-gap", type=float, default=0.2)
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_tum_positions(path: Path) -> tuple[np.ndarray, np.ndarray]:
    stamps: list[float] = []
    positions: list[list[float]] = []
    with path.open(encoding="utf-8") as stream:
        for line_number, raw in enumerate(stream, start=1):
            tokens = raw.split()
            if not tokens:
                continue
            if len(tokens) != 8:
                raise ValueError(f"{path}:{line_number}: expected 8 TUM columns")
            values = [float(token) for token in tokens]
            if not all(math.isfinite(value) for value in values):
                raise ValueError(f"{path}:{line_number}: non-finite value")
            stamps.append(values[0])
            positions.append(values[1:4])
    if len(stamps) < 2:
        raise ValueError("trajectory must contain at least two poses")
    if any(right <= left for left, right in zip(stamps, stamps[1:])):
        raise ValueError("trajectory timestamps must be strictly increasing")
    return np.asarray(stamps), np.asarray(positions)


def load_gt_positions(path: Path) -> tuple[np.ndarray, np.ndarray]:
    stamps: list[float] = []
    positions: list[list[float]] = []
    with path.open(encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            stamps.append(float(row["timestamp"]))
            positions.append(
                [float(row[f"prism_{axis}"]) for axis in ("x", "y", "z")]
            )
    if len(stamps) < 2:
        raise ValueError("GT must contain at least two positions")
    return np.asarray(stamps), np.asarray(positions)


def interpolate_gt(
    estimate_stamps: np.ndarray,
    gt_stamps: np.ndarray,
    gt_positions: np.ndarray,
    max_gt_gap: float,
) -> tuple[np.ndarray, np.ndarray]:
    right = np.searchsorted(gt_stamps, estimate_stamps, side="left")
    valid = (right > 0) & (right < len(gt_stamps))
    valid_indices = np.flatnonzero(valid)
    left_indices = right[valid_indices] - 1
    right_indices = right[valid_indices]
    gaps = gt_stamps[right_indices] - gt_stamps[left_indices]
    keep = gaps <= max_gt_gap
    valid_indices = valid_indices[keep]
    left_indices = left_indices[keep]
    right_indices = right_indices[keep]
    gaps = gaps[keep]
    if len(valid_indices) < 2:
        raise ValueError("fewer than two estimates have bracketed Leica GT")
    alpha = (
        estimate_stamps[valid_indices] - gt_stamps[left_indices]
    ) / gaps
    interpolated = (
        gt_positions[left_indices] * (1.0 - alpha[:, None])
        + gt_positions[right_indices] * alpha[:, None]
    )
    return valid_indices, interpolated


def rmse(values: np.ndarray) -> float:
    return float(np.sqrt(np.mean(np.square(values))))


def segment_rpe_percent(
    estimated: np.ndarray, gt: np.ndarray, segment_length: float
) -> np.ndarray:
    cumulative = np.concatenate(
        ([0.0], np.cumsum(np.linalg.norm(np.diff(gt, axis=0), axis=1)))
    )
    errors: list[float] = []
    for start in range(len(gt) - 1):
        end = int(
            np.searchsorted(cumulative, cumulative[start] + segment_length)
        )
        if end >= len(gt):
            continue
        distance = cumulative[end] - cumulative[start]
        estimate_delta = estimated[end] - estimated[start]
        gt_delta = gt[end] - gt[start]
        errors.append(float(np.linalg.norm(estimate_delta - gt_delta) / distance * 100.0))
    if not errors:
        raise ValueError("trajectory is too short for the requested RPE segment")
    return np.asarray(errors)


def main() -> int:
    args = parse_args()
    estimate_stamps, estimate_positions = load_tum_positions(args.trajectory)
    gt_stamps, gt_positions = load_gt_positions(args.gt_csv)
    indices, interpolated_gt = interpolate_gt(
        estimate_stamps, gt_stamps, gt_positions, args.max_gt_gap
    )
    associated_estimate = estimate_positions[indices]
    associated_estimate = associated_estimate - associated_estimate[0]
    interpolated_gt = interpolated_gt - interpolated_gt[0]
    ate_errors = np.linalg.norm(associated_estimate - interpolated_gt, axis=1)
    rpe_errors = segment_rpe_percent(
        associated_estimate, interpolated_gt, args.segment_length
    )
    payload = {
        "schema_version": 1,
        "method": args.method,
        "trajectory": str(args.trajectory),
        "trajectory_sha256": sha256_file(args.trajectory),
        "gt_csv": str(args.gt_csv),
        "gt_sha256": sha256_file(args.gt_csv),
        "protocol": {
            "alignment": "first common position anchor only",
            "gt_interpolation": "linear between bracketing Leica timestamps",
            "max_gt_gap_s": args.max_gt_gap,
            "segment_length_m": args.segment_length,
            "rotation_rpe": "unavailable because Leica GT is position-only",
        },
        "metrics": {
            "trajectory_poses": len(estimate_stamps),
            "associated_poses": len(indices),
            "lidar_frames": args.lidar_frames,
            "tracking_success_rate": len(estimate_stamps) / args.lidar_frames,
            "ate_3d_m": rmse(ate_errors),
            "rpe_trans_pct": rmse(rpe_errors),
            "rpe_segments": len(rpe_errors),
            "rpe_rot_deg_per_m": None,
            "gt_path_length_m": float(
                np.sum(np.linalg.norm(np.diff(interpolated_gt, axis=0), axis=1))
            ),
            "runtime_seconds": args.runtime_seconds,
            "fps": (
                args.lidar_frames / args.runtime_seconds
                if args.runtime_seconds is not None
                else None
            ),
        },
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
