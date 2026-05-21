#!/usr/bin/env python3
"""Apply a temporal smoother to selected pair-prior odometry.

The smoother operates only on the selected pair motions. Ground truth is used
only for optional reporting, never for correction.
"""

from __future__ import annotations

import argparse
import json
import math
import time
from dataclasses import dataclass
from pathlib import Path

import numpy as np

from intensity_bev_odometry_demo import compute_metrics, load_gt_xyyaw, se2_matrix


@dataclass(frozen=True)
class SmootherParams:
    lambda_translation: float = 1.0
    lambda_yaw: float = 0.5
    robust_iterations: int = 3
    huber_translation_m: float = 0.08
    huber_yaw_deg: float = 1.0
    max_correction_translation_m: float = 0.12
    max_correction_yaw_deg: float = 1.5


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--input-json", required=True, type=Path)
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument("--gt-csv", type=Path, help="Overrides gt_csv from the input JSON.")
    p.add_argument("--lambda-translation", type=float, default=1.0)
    p.add_argument("--lambda-yaw", type=float, default=0.5)
    p.add_argument("--robust-iterations", type=int, default=3)
    p.add_argument("--huber-translation-m", type=float, default=0.08)
    p.add_argument("--huber-yaw-deg", type=float, default=1.0)
    p.add_argument("--max-correction-translation-m", type=float, default=0.12)
    p.add_argument("--max-correction-yaw-deg", type=float, default=1.5)
    return p.parse_args()


def wrap_angle_rad(value: np.ndarray | float) -> np.ndarray | float:
    return (value + math.pi) % (2.0 * math.pi) - math.pi


def pair_measurements(payload: dict) -> np.ndarray:
    rows = []
    for pair in payload.get("pairs", []):
        rows.append([
            float(pair.get("used_dx_curr_to_prev_m", pair.get("dx_curr_to_prev_m", 0.0))),
            float(pair.get("used_dy_curr_to_prev_m", pair.get("dy_curr_to_prev_m", 0.0))),
            math.radians(
                float(
                    pair.get(
                        "used_yaw_curr_to_prev_deg",
                        pair.get("yaw_curr_to_prev_deg", 0.0),
                    )
                )
            ),
        ])
    if not rows:
        return np.zeros((0, 3), dtype=np.float64)
    out = np.asarray(rows, dtype=np.float64)
    out[:, 2] = wrap_angle_rad(out[:, 2])
    return out


def solve_smooth_channel(
    measured: np.ndarray,
    smoothness_lambda: float,
    huber_threshold: float,
    robust_iterations: int,
) -> np.ndarray:
    n = measured.size
    if n <= 2 or smoothness_lambda <= 0.0:
        return measured.copy()

    diff_weights = np.ones(n - 1, dtype=np.float64)
    x = measured.copy()
    for _ in range(max(1, robust_iterations)):
        a = np.eye(n, dtype=np.float64)
        b = measured.copy()
        for i, weight in enumerate(diff_weights):
            value = smoothness_lambda * weight
            a[i, i] += value
            a[i + 1, i + 1] += value
            a[i, i + 1] -= value
            a[i + 1, i] -= value
        x = np.linalg.solve(a, b)
        diffs = np.abs(np.diff(x))
        diff_weights = np.where(
            diffs <= huber_threshold,
            1.0,
            np.maximum(huber_threshold / np.maximum(diffs, 1e-12), 1e-3),
        )
    return x


def cap_translation_correction(
    measured_xy: np.ndarray,
    smoothed_xy: np.ndarray,
    max_correction_m: float,
) -> np.ndarray:
    if max_correction_m <= 0.0:
        return smoothed_xy
    delta = smoothed_xy - measured_xy
    norms = np.linalg.norm(delta, axis=1)
    scale = np.ones_like(norms)
    over = norms > max_correction_m
    scale[over] = max_correction_m / np.maximum(norms[over], 1e-12)
    return measured_xy + delta * scale[:, None]


def cap_yaw_correction(
    measured_yaw: np.ndarray,
    smoothed_yaw: np.ndarray,
    max_correction_rad: float,
) -> np.ndarray:
    if max_correction_rad <= 0.0:
        return wrap_angle_rad(smoothed_yaw)
    delta = wrap_angle_rad(smoothed_yaw - measured_yaw)
    delta = np.clip(delta, -max_correction_rad, max_correction_rad)
    return wrap_angle_rad(measured_yaw + delta)


def smooth_measurements(measured: np.ndarray, params: SmootherParams) -> np.ndarray:
    if measured.shape[0] == 0:
        return measured.copy()

    dx = solve_smooth_channel(
        measured[:, 0],
        params.lambda_translation,
        params.huber_translation_m,
        params.robust_iterations,
    )
    dy = solve_smooth_channel(
        measured[:, 1],
        params.lambda_translation,
        params.huber_translation_m,
        params.robust_iterations,
    )
    yaw = solve_smooth_channel(
        np.unwrap(measured[:, 2]),
        params.lambda_yaw,
        math.radians(params.huber_yaw_deg),
        params.robust_iterations,
    )

    xy = cap_translation_correction(
        measured[:, :2],
        np.column_stack([dx, dy]),
        params.max_correction_translation_m,
    )
    yaw = cap_yaw_correction(
        measured[:, 2],
        yaw,
        math.radians(params.max_correction_yaw_deg),
    )
    return np.column_stack([xy[:, 0], xy[:, 1], yaw])


def integrate_pairs(pairs: np.ndarray) -> np.ndarray:
    pose = np.eye(3, dtype=np.float64)
    poses = [[0.0, 0.0, 0.0]]
    for dx, dy, yaw in pairs:
        pose = pose @ se2_matrix(float(dx), float(dy), float(yaw))
        poses.append([float(pose[0, 2]), float(pose[1, 2]), float(math.atan2(pose[1, 0], pose[0, 0]))])
    return np.asarray(poses, dtype=np.float64)


def smooth_payload(
    payload: dict,
    params: SmootherParams,
    gt_csv: Path | None = None,
    input_json: Path | None = None,
) -> dict:
    started_at = time.perf_counter()
    measured = pair_measurements(payload)
    smoothed = smooth_measurements(measured, params)
    poses = integrate_pairs(smoothed)

    effective_gt_csv = gt_csv
    if effective_gt_csv is None and payload.get("gt_csv"):
        effective_gt_csv = Path(str(payload["gt_csv"]))
    gt = load_gt_xyyaw(effective_gt_csv) if effective_gt_csv is not None else None
    metrics = compute_metrics(poses, gt)

    pairs = []
    for i, (pair, raw, corrected) in enumerate(
        zip(payload.get("pairs", []), measured, smoothed, strict=True),
        start=1,
    ):
        translation_correction = float(np.linalg.norm(corrected[:2] - raw[:2]))
        yaw_correction = float(abs(wrap_angle_rad(corrected[2] - raw[2])))
        out_pair = dict(pair)
        out_pair.update({
            "index": int(pair.get("index", i)),
            "original_used_dx_curr_to_prev_m": float(raw[0]),
            "original_used_dy_curr_to_prev_m": float(raw[1]),
            "original_used_yaw_curr_to_prev_deg": math.degrees(float(raw[2])),
            "used_dx_curr_to_prev_m": float(corrected[0]),
            "used_dy_curr_to_prev_m": float(corrected[1]),
            "used_yaw_curr_to_prev_deg": math.degrees(float(corrected[2])),
            "temporal_smoother": {
                "translation_correction_m": translation_correction,
                "yaw_correction_deg": math.degrees(yaw_correction),
            },
        })
        pairs.append(out_pair)

    return {
        "sequence_pcd_dir": payload.get("sequence_pcd_dir"),
        "gt_csv": str(effective_gt_csv) if effective_gt_csv is not None else None,
        "frames": int(poses.shape[0]),
        "method": "temporal_smoothed_pair_prior_odometry",
        "source_method": payload.get("method"),
        "source_json": str(input_json) if input_json is not None else payload.get("source_json"),
        "parameters": {
            "lambda_translation": params.lambda_translation,
            "lambda_yaw": params.lambda_yaw,
            "robust_iterations": params.robust_iterations,
            "huber_translation_m": params.huber_translation_m,
            "huber_yaw_deg": params.huber_yaw_deg,
            "max_correction_translation_m": params.max_correction_translation_m,
            "max_correction_yaw_deg": params.max_correction_yaw_deg,
        },
        "runtime_s": time.perf_counter() - started_at,
        "metrics": metrics,
        "poses_xyyaw": [
            {"index": i, "x_m": float(p[0]), "y_m": float(p[1]), "yaw_deg": math.degrees(float(p[2]))}
            for i, p in enumerate(poses)
        ],
        "pairs": pairs,
    }


def main() -> int:
    args = parse_args()
    params = SmootherParams(
        lambda_translation=args.lambda_translation,
        lambda_yaw=args.lambda_yaw,
        robust_iterations=args.robust_iterations,
        huber_translation_m=args.huber_translation_m,
        huber_yaw_deg=args.huber_yaw_deg,
        max_correction_translation_m=args.max_correction_translation_m,
        max_correction_yaw_deg=args.max_correction_yaw_deg,
    )
    payload = json.loads(args.input_json.read_text())
    out = smooth_payload(payload, params, args.gt_csv, args.input_json)
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(out, indent=2) + "\n", encoding="utf-8")
    metrics = out.get("metrics")
    if metrics:
        print(
            f"[done] frames={out['frames']} ATE_xy={metrics['ate_xy_m']:.3f}m "
            f"step_rmse={metrics['step_length_rmse_m']:.3f}m -> {args.output_json}"
        )
    else:
        print(f"[done] frames={out['frames']} -> {args.output_json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
