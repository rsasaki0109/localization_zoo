#!/usr/bin/env python3
"""Calibrate pair-prior quality from selector diagnostics."""

from __future__ import annotations

import argparse
import json
import math
from collections import Counter
from pathlib import Path

import numpy as np

from intensity_bev_odometry_demo import compute_metrics, load_gt_xyyaw, se2_matrix


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--diagnostic",
        action="append",
        required=True,
        help="Selector diagnostic JSON. Repeat for each sequence.",
    )
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument("--output-md", type=Path)
    p.add_argument("--ridge", type=float, default=1e-3)
    p.add_argument(
        "--feature-set",
        choices=("all", "no_method", "cubemap_only", "motion_cubemap", "shared", "shared_no_method"),
        default="all",
    )
    return p.parse_args()


def feature_names_for(method_names: list[str], feature_set: str) -> list[str]:
    names = ["bias", "cubemap_score"]
    if feature_set in ("all", "no_method", "shared", "shared_no_method"):
        names.extend(["geometry_score", "log_overlap", "motion_norm", "abs_yaw_deg"])
    elif feature_set == "motion_cubemap":
        names.extend(["motion_norm", "abs_yaw_deg"])
    if feature_set in ("shared", "shared_no_method"):
        names.extend([
            "shared_residual_score",
            "shared_residual_rmse",
            "shared_inlier_ratio",
            "shared_stability_score",
            "shared_stability_mean_delta",
            "shared_stability_min_delta",
        ])
    if feature_set in ("all", "shared"):
        names.extend(f"is_{name}" for name in method_names)
    return names


def feature_vector(method: str, row: dict, method_names: list[str], feature_set: str) -> list[float]:
    motion_norm = math.hypot(row["dx_curr_to_prev_m"], row["dy_curr_to_prev_m"])
    base = [1.0, float(row.get("cubemap_score") or -1.0)]
    if feature_set in ("all", "no_method", "shared", "shared_no_method"):
        base.extend([
            float(row.get("geometry_score") or 0.0),
            math.log1p(float(row.get("overlap") or 0.0)),
            motion_norm,
            abs(float(row["yaw_curr_to_prev_deg"])),
        ])
    elif feature_set == "motion_cubemap":
        base.extend([motion_norm, abs(float(row["yaw_curr_to_prev_deg"]))])
    if feature_set in ("shared", "shared_no_method"):
        shared_rmse = row.get("shared_residual_rmse_m")
        stability_score = row.get("shared_stability_score")
        stability_mean = row.get("shared_stability_mean_delta_m")
        stability_min = row.get("shared_stability_min_delta_m")
        base.extend([
            float(row.get("shared_residual_score") or -1.0),
            float(shared_rmse if shared_rmse is not None else 999.0),
            float(row.get("shared_residual_inlier_ratio") or 0.0),
            float(stability_score if stability_score is not None else -999.0),
            float(stability_mean if stability_mean is not None else -999.0),
            float(stability_min if stability_min is not None else -999.0),
        ])
    if feature_set in ("all", "shared"):
        base.extend(1.0 if method == name else 0.0 for name in method_names)
    return base


def load_diagnostic(path: Path) -> dict:
    payload = json.loads(path.read_text())
    payload["_path"] = str(path)
    payload["_seq"] = path.stem.replace("selector_diagnostics_", "").replace("_30", "")
    return payload


def build_rows(diag: dict) -> list[dict]:
    rows = []
    for pair in diag["pairs"]:
        for method, method_row in pair["methods"].items():
            rows.append({
                "seq": diag["_seq"],
                "pair_index": int(pair["index"]),
                "method": method,
                "row": method_row,
                "target_quality": -float(method_row["translation_error_m"]),
                "translation_error_m": float(method_row["translation_error_m"]),
                "is_oracle": method == pair["oracle_best"],
            })
    return rows


def fit_ridge(rows: list[dict], method_names: list[str], ridge: float, feature_set: str) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    x = np.asarray([feature_vector(r["method"], r["row"], method_names, feature_set) for r in rows], dtype=np.float64)
    y = np.asarray([r["target_quality"] for r in rows], dtype=np.float64)
    mean = x.mean(axis=0)
    scale = x.std(axis=0)
    mean[0] = 0.0
    scale[0] = 1.0
    scale[scale < 1e-9] = 1.0
    xs = (x - mean) / scale
    reg = ridge * np.eye(xs.shape[1])
    reg[0, 0] = 0.0
    weights = np.linalg.solve(xs.T @ xs + reg, xs.T @ y)
    return weights, mean, scale


def predict_quality(method: str, row: dict, method_names: list[str], feature_set: str, weights: np.ndarray, mean: np.ndarray, scale: np.ndarray) -> float:
    x = np.asarray(feature_vector(method, row, method_names, feature_set), dtype=np.float64)
    return float(((x - mean) / scale) @ weights)


def candidate_matrix(row: dict) -> np.ndarray:
    return se2_matrix(
        float(row["dx_curr_to_prev_m"]),
        float(row["dy_curr_to_prev_m"]),
        math.radians(float(row["yaw_curr_to_prev_deg"])),
    )


def integrate_selected(diag: dict, selected: dict[int, str]) -> np.ndarray:
    pose = np.eye(3)
    poses = [[0.0, 0.0, 0.0]]
    for pair in diag["pairs"]:
        method = selected[int(pair["index"])]
        pose = pose @ candidate_matrix(pair["methods"][method])
        poses.append([float(pose[0, 2]), float(pose[1, 2]), float(math.atan2(pose[1, 0], pose[0, 0]))])
    return np.asarray(poses, dtype=np.float64)


def evaluate_diag(diag: dict, train_rows: list[dict], method_names: list[str], ridge: float, feature_set: str) -> dict:
    weights, mean, scale = fit_ridge(train_rows, method_names, ridge, feature_set)
    selected: dict[int, str] = {}
    hits = 0
    counts: Counter[str] = Counter()
    margins = []
    for pair in diag["pairs"]:
        scores = {
            method: predict_quality(method, row, method_names, feature_set, weights, mean, scale)
            for method, row in pair["methods"].items()
        }
        ordered = sorted(scores.items(), key=lambda kv: kv[1], reverse=True)
        selected_name = ordered[0][0]
        selected[int(pair["index"])] = selected_name
        counts[selected_name] += 1
        hits += int(selected_name == pair["oracle_best"])
        if len(ordered) > 1:
            margins.append(float(ordered[0][1] - ordered[1][1]))
    gt = load_gt_xyyaw(Path(diag["gt_csv"]))
    metrics = compute_metrics(integrate_selected(diag, selected), gt)
    return {
        "seq": diag["_seq"],
        "selector_hits": hits,
        "selector_total": len(diag["pairs"]),
        "selected_counts": dict(counts),
        "metrics": metrics,
        "mean_margin": float(np.mean(margins)) if margins else None,
        "weights": weights.tolist(),
        "feature_mean": mean.tolist(),
        "feature_scale": scale.tolist(),
    }


def main() -> int:
    args = parse_args()
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    if args.output_md is not None:
        args.output_md.parent.mkdir(parents=True, exist_ok=True)

    diagnostics = [load_diagnostic(Path(p)) for p in args.diagnostic]
    all_rows = [row for diag in diagnostics for row in build_rows(diag)]
    method_names = sorted({row["method"] for row in all_rows})
    feature_names = feature_names_for(method_names, args.feature_set)

    loo = []
    for diag in diagnostics:
        train_rows = [row for row in all_rows if row["seq"] != diag["_seq"]]
        loo.append(evaluate_diag(diag, train_rows, method_names, args.ridge, args.feature_set))

    full_weights, full_mean, full_scale = fit_ridge(all_rows, method_names, args.ridge, args.feature_set)
    payload = {
        "diagnostics": args.diagnostic,
        "method_names": method_names,
        "feature_names": feature_names,
        "ridge": args.ridge,
        "feature_set": args.feature_set,
        "leave_one_sequence_out": loo,
        "full_model": {
            "weights": full_weights.tolist(),
            "feature_mean": full_mean.tolist(),
            "feature_scale": full_scale.tolist(),
        },
    }
    args.output_json.write_text(json.dumps(payload, indent=2) + "\n")

    if args.output_md is not None:
        lines = [
            "# Pair Prior Quality Calibration",
            "",
            "Ridge model target: negative GT pair translation error.",
            "",
            "## Leave-One-Sequence-Out",
            "",
            "| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |",
            "| --- | ---: | ---: | ---: | --- |",
        ]
        for row in loo:
            m = row["metrics"]
            lines.append(
                f"| {row['seq']} | {row['selector_hits']} / {row['selector_total']} | "
                f"{m['ate_xy_m']:.3f} | {m['step_length_rmse_m']:.3f} | {row['selected_counts']} |"
            )
        lines.extend([
            "",
            "## Full-Model Weights",
            "",
            "| Feature | Weight |",
            "| --- | ---: |",
        ])
        for name, weight in zip(feature_names, full_weights, strict=True):
            lines.append(f"| {name} | {weight:.6f} |")
        args.output_md.write_text("\n".join(lines) + "\n")

    for row in loo:
        m = row["metrics"]
        print(
            f"[loo] {row['seq']} hits={row['selector_hits']}/{row['selector_total']} "
            f"ATE={m['ate_xy_m']:.3f} step={m['step_length_rmse_m']:.3f} "
            f"counts={row['selected_counts']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
