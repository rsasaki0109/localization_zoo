#!/usr/bin/env python3
"""Select a pairwise geometry prior from multiple candidate odometry JSONs."""

from __future__ import annotations

import argparse
import json
import math
import time
from pathlib import Path

import numpy as np

from intensity_bev_odometry_demo import (
    Candidate,
    compute_metrics,
    load_gt_xyyaw,
    preprocess_scan,
    read_binary_pcd_xyz_intensity,
    se2_matrix,
)
from intensity_cubemap_odometry_demo import make_cubemap, score_candidate


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--sequence-pcd-dir", required=True, type=Path)
    p.add_argument("--gt-csv", type=Path)
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument(
        "--candidate",
        action="append",
        required=True,
        help="Candidate prior as name:path. Repeat for KISS, SmallGICP, etc.",
    )
    p.add_argument("--candidate-source", choices=("used", "raw"), default="used")
    p.add_argument(
        "--policy",
        choices=(
            "cubemap_score",
            "geometry_score",
            "hybrid_zscore",
            "prefer_first",
            "preferred_with_cubemap_margin",
            "preferred_with_stability_margin",
            "preferred_with_stability_motion_consistency",
            "shared_residual_score",
            "shared_residual_rmse",
            "shared_stability_score",
            "shared_stability_delta",
            "calibrated_quality",
        ),
        default="hybrid_zscore",
    )
    p.add_argument(
        "--shared-residual-json",
        type=Path,
        help="Shared residual JSON from evaluate_pair_prior_residuals.py.",
    )
    p.add_argument(
        "--calibration-json",
        type=Path,
        help="Calibration JSON from calibrate_pair_prior_quality.py for calibrated_quality.",
    )
    p.add_argument(
        "--preferred-name",
        default="small_gicp",
        help="Preferred candidate for preferred_with_*_margin policies.",
    )
    p.add_argument(
        "--score-margin",
        type=float,
        default=0.01,
        help="Score advantage required to override --preferred-name in margin policies.",
    )
    p.add_argument(
        "--motion-consistency-margin",
        type=float,
        default=0.04,
        help="Required distance-to-previous-motion improvement for motion consistency override.",
    )
    p.add_argument(
        "--motion-consistency-min-prev-step",
        type=float,
        default=0.0,
        help="Minimum previous selected translation norm for motion consistency gate.",
    )
    p.add_argument(
        "--motion-consistency-max-prev-step",
        type=float,
        default=0.30,
        help="Maximum previous selected translation norm for motion consistency gate.",
    )
    p.add_argument("--cubemap-weight", type=float, default=1.0)
    p.add_argument("--geometry-weight", type=float, default=1.0)
    p.add_argument("--max-frames", type=int, default=60)
    p.add_argument("--min-range", type=float, default=2.0)
    p.add_argument("--max-range", type=float, default=70.0)
    p.add_argument("--z-min", type=float, default=-3.0)
    p.add_argument("--z-max", type=float, default=3.0)
    p.add_argument("--max-points", type=int, default=4000)
    p.add_argument("--face-size", type=int, default=64)
    p.add_argument("--gaussian-sigma", type=float, default=1.0)
    p.add_argument(
        "--score-channel",
        choices=("intensity", "igm", "hybrid", "semi_dense"),
        default="intensity",
    )
    p.add_argument("--feature-quantile", type=float, default=0.90)
    p.add_argument("--min-overlap-pixels", type=int, default=120)
    p.add_argument("--progress-every", type=int, default=10)
    return p.parse_args()


def candidate_from_pair(pair: dict, source: str) -> Candidate:
    if source == "used":
        dx = float(pair.get("used_dx_curr_to_prev_m", pair["dx_curr_to_prev_m"]))
        dy = float(pair.get("used_dy_curr_to_prev_m", pair["dy_curr_to_prev_m"]))
        yaw_deg = float(pair.get("used_yaw_curr_to_prev_deg", pair["yaw_curr_to_prev_deg"]))
    else:
        dx = float(pair["dx_curr_to_prev_m"])
        dy = float(pair["dy_curr_to_prev_m"])
        yaw_deg = float(pair["yaw_curr_to_prev_deg"])
    score = float(pair.get("score", 0.0))
    overlap = int(pair.get("overlap", pair.get("overlap_pixels", 0)))
    return Candidate(dx, dy, math.radians(yaw_deg), score, overlap)


def load_candidates(specs: list[str], source: str) -> dict[str, dict[int, Candidate]]:
    loaded: dict[str, dict[int, Candidate]] = {}
    for spec in specs:
        if ":" not in spec:
            raise ValueError(f"--candidate must be name:path, got {spec}")
        name, path_str = spec.split(":", 1)
        payload = json.loads(Path(path_str).read_text())
        pairs: dict[int, Candidate] = {}
        for pair in payload.get("pairs", []):
            pairs[int(pair["index"])] = candidate_from_pair(pair, source)
        loaded[name] = pairs
    return loaded


def load_shared_residuals(path: Path | None) -> dict[int, dict[str, dict]]:
    if path is None:
        return {}
    payload = json.loads(path.read_text())
    return {
        int(pair["index"]): pair.get("methods", {})
        for pair in payload.get("pairs", [])
    }


def load_calibration(path: Path | None) -> dict | None:
    if path is None:
        return None
    payload = json.loads(path.read_text())
    model = payload.get("full_model", {})
    required = ("weights", "feature_mean", "feature_scale")
    if not all(key in model for key in required):
        raise ValueError(f"calibration JSON missing full_model fields: {required}")
    return payload


def zscore(values: dict[str, float]) -> dict[str, float]:
    vals = np.asarray(list(values.values()), dtype=np.float64)
    sigma = float(vals.std())
    if sigma < 1e-9:
        return {k: 0.0 for k in values}
    mean = float(vals.mean())
    return {k: (v - mean) / sigma for k, v in values.items()}


def finite_or(value: object, default: float) -> float:
    try:
        out = float(value)
    except (TypeError, ValueError):
        return default
    return out if math.isfinite(out) else default


def calibrated_feature_value(
    feature_name: str,
    name: str,
    cand: Candidate,
    scored: Candidate,
    shared: dict,
) -> float:
    if feature_name == "bias":
        return 1.0
    if feature_name == "cubemap_score":
        return finite_or(scored.score, -1.0)
    if feature_name == "geometry_score":
        return finite_or(cand.score, 0.0)
    if feature_name == "log_overlap":
        return math.log1p(max(0.0, finite_or(cand.overlap, 0.0)))
    if feature_name == "motion_norm":
        return math.hypot(scored.dx, scored.dy)
    if feature_name == "abs_yaw_deg":
        return abs(math.degrees(scored.yaw))
    if feature_name == "shared_residual_score":
        return finite_or(shared.get("shared_residual_score"), -1.0)
    if feature_name == "shared_residual_rmse":
        return finite_or(shared.get("symmetric_trimmed_plane_rmse_m"), 999.0)
    if feature_name == "shared_inlier_ratio":
        return finite_or(shared.get("symmetric_inlier_ratio"), 0.0)
    if feature_name == "shared_stability_score":
        return finite_or(shared.get("stability_score"), -999.0)
    if feature_name == "shared_stability_mean_delta":
        return finite_or(shared.get("stability_mean_delta_m"), -999.0)
    if feature_name == "shared_stability_min_delta":
        return finite_or(shared.get("stability_min_delta_m"), -999.0)
    if feature_name.startswith("is_"):
        return 1.0 if name == feature_name[3:] else 0.0
    raise ValueError(f"unsupported calibration feature {feature_name!r}")


def calibrated_quality_scores(
    scored: dict[str, Candidate],
    raw: dict[str, Candidate],
    shared_by_name: dict[str, dict],
    calibration: dict,
) -> dict[str, float]:
    feature_names = calibration["feature_names"]
    model = calibration["full_model"]
    weights = np.asarray(model["weights"], dtype=np.float64)
    mean = np.asarray(model["feature_mean"], dtype=np.float64)
    scale = np.asarray(model["feature_scale"], dtype=np.float64)
    if not (len(feature_names) == weights.size == mean.size == scale.size):
        raise ValueError("calibration feature vector length mismatch")
    scale = np.where(np.abs(scale) < 1e-9, 1.0, scale)
    scores = {}
    for name, scored_cand in scored.items():
        features = np.asarray([
            calibrated_feature_value(
                feature_name,
                name,
                raw[name],
                scored_cand,
                shared_by_name.get(name, {}),
            )
            for feature_name in feature_names
        ], dtype=np.float64)
        scores[name] = float(((features - mean) / scale) @ weights)
    return scores


def choose_candidate(
    scored: dict[str, Candidate],
    raw: dict[str, Candidate],
    shared: dict[str, dict],
    calibration: dict | None,
    policy: str,
    cubemap_weight: float,
    geometry_weight: float,
    preferred_name: str,
    score_margin: float,
) -> tuple[str, dict[str, float]]:
    if policy == "prefer_first":
        name = next(iter(scored))
        return name, {k: 0.0 for k in scored}

    cubemap_scores = {k: v.score for k, v in scored.items()}
    geometry_scores = {k: v.overlap for k, v in scored.items()}
    if policy == "preferred_with_cubemap_margin":
        if preferred_name not in scored:
            raise ValueError(f"preferred candidate {preferred_name!r} not found")
        preferred_score = cubemap_scores[preferred_name]
        values = {k: cubemap_scores[k] - preferred_score for k in scored}
        selected = preferred_name
        for name, value in values.items():
            if name != preferred_name and value >= score_margin:
                selected = name
                break
        return selected, values
    if policy in ("preferred_with_stability_margin", "preferred_with_stability_motion_consistency"):
        if preferred_name not in scored:
            raise ValueError(f"preferred candidate {preferred_name!r} not found")
        stability_scores = {
            k: finite_or(shared.get(k, {}).get("stability_mean_delta_m"), -float("inf"))
            for k in scored
        }
        preferred_score = stability_scores[preferred_name]
        values = {k: stability_scores[k] - preferred_score for k in scored}
        selected = preferred_name
        for name, value in values.items():
            if name != preferred_name and value >= score_margin:
                selected = name
                break
        return selected, values
    if policy == "cubemap_score":
        values = cubemap_scores
    elif policy == "geometry_score":
        values = geometry_scores
    elif policy == "shared_residual_score":
        values = {
            k: float(shared.get(k, {}).get("shared_residual_score", -float("inf")))
            for k in scored
        }
    elif policy == "shared_residual_rmse":
        values = {
            k: -float(shared.get(k, {}).get("symmetric_trimmed_plane_rmse_m", float("inf")))
            for k in scored
        }
    elif policy == "shared_stability_score":
        values = {
            k: float(shared.get(k, {}).get("stability_score", -float("inf")))
            for k in scored
        }
    elif policy == "shared_stability_delta":
        values = {
            k: float(shared.get(k, {}).get("stability_mean_delta_m", -float("inf")))
            for k in scored
        }
    elif policy == "calibrated_quality":
        if calibration is None:
            raise ValueError("--policy calibrated_quality requires --calibration-json")
        values = calibrated_quality_scores(scored, raw, shared, calibration)
    else:
        z_cubemap = zscore(cubemap_scores)
        z_geometry = zscore(geometry_scores)
        values = {
            k: cubemap_weight * z_cubemap[k] + geometry_weight * z_geometry[k]
            for k in scored
        }
    return max(values, key=values.get), values


def apply_motion_consistency_gate(
    selected_name: str,
    scored: dict[str, Candidate],
    previous: Candidate | None,
    policy_values: dict[str, float],
    margin: float,
    min_prev_step: float,
    max_prev_step: float,
) -> tuple[str, dict[str, float], dict[str, float | str | bool]]:
    if previous is None or len(scored) < 2:
        return selected_name, policy_values, {"applied": False, "reason": "no_previous_motion"}
    previous_step = math.hypot(previous.dx, previous.dy)
    if previous_step < min_prev_step or previous_step > max_prev_step:
        return selected_name, policy_values, {
            "applied": False,
            "reason": "previous_step_out_of_range",
            "previous_step_m": previous_step,
        }

    distances = {
        name: math.hypot(cand.dx - previous.dx, cand.dy - previous.dy)
        for name, cand in scored.items()
    }
    alternate_name = min(
        (name for name in scored if name != selected_name),
        key=lambda name: distances[name],
    )
    base_distance = distances[selected_name]
    alternate_distance = distances[alternate_name]
    out_values = dict(policy_values)
    for name, value in distances.items():
        out_values[f"motion_consistency_distance_{name}"] = value
    if alternate_distance + margin < base_distance:
        return alternate_name, out_values, {
            "applied": True,
            "reason": "alternate_closer_to_previous_motion",
            "previous_step_m": previous_step,
            "base_selected": selected_name,
            "override_selected": alternate_name,
            "base_distance_m": base_distance,
            "alternate_distance_m": alternate_distance,
            "margin_m": margin,
        }
    return selected_name, out_values, {
        "applied": False,
        "reason": "base_motion_consistent",
        "previous_step_m": previous_step,
        "base_selected": selected_name,
        "best_alternate": alternate_name,
        "base_distance_m": base_distance,
        "alternate_distance_m": alternate_distance,
        "margin_m": margin,
    }


def main() -> int:
    args = parse_args()
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    started_at = time.perf_counter()

    candidate_sets = load_candidates(args.candidate, args.candidate_source)
    shared_residuals = load_shared_residuals(args.shared_residual_json)
    calibration = load_calibration(args.calibration_json)
    candidate_names = list(candidate_sets)
    if len(candidate_names) < 2:
        raise RuntimeError("Need at least two --candidate entries")

    scans = []
    pcd_paths = []
    for i in range(args.max_frames):
        pcd_path = args.sequence_pcd_dir / f"{i:08d}" / "cloud.pcd"
        if not pcd_path.is_file():
            break
        scans.append(preprocess_scan(read_binary_pcd_xyz_intensity(pcd_path), args))
        pcd_paths.append(pcd_path)
    if len(scans) < 2:
        raise RuntimeError("Need at least two scans")

    gt = load_gt_xyyaw(args.gt_csv) if args.gt_csv is not None else None
    include_igm = args.score_channel in ("igm", "hybrid", "semi_dense")
    cubemaps = [make_cubemap(scan, args, include_igm=include_igm) for scan in scans]

    pose = np.eye(3)
    poses = [[0.0, 0.0, 0.0]]
    pairs = []
    previous_selected: Candidate | None = None
    for i in range(1, len(scans)):
        scored: dict[str, Candidate] = {}
        raw: dict[str, Candidate] = {}
        for name in candidate_names:
            cand = candidate_sets[name].get(i, Candidate(0.0, 0.0, 0.0, -1.0, 0))
            raw[name] = cand
            scored[name] = score_candidate(cubemaps[i - 1], scans[i], cand, args)
            # Keep geometry score/correspondence count available for policy use.
            scored[name] = Candidate(
                scored[name].dx,
                scored[name].dy,
                scored[name].yaw,
                scored[name].score,
                raw[name].overlap,
            )
        selected_name, policy_values = choose_candidate(
            scored,
            raw,
            shared_residuals.get(i, {}),
            calibration,
            args.policy,
            args.cubemap_weight,
            args.geometry_weight,
            args.preferred_name,
            args.score_margin,
        )
        motion_consistency = {"applied": False, "reason": "policy_disabled"}
        if args.policy == "preferred_with_stability_motion_consistency":
            selected_name, policy_values, motion_consistency = apply_motion_consistency_gate(
                selected_name,
                scored,
                previous_selected,
                policy_values,
                args.motion_consistency_margin,
                args.motion_consistency_min_prev_step,
                args.motion_consistency_max_prev_step,
            )
        selected = scored[selected_name]
        previous_selected = selected
        pose = pose @ se2_matrix(selected.dx, selected.dy, selected.yaw)
        poses.append([
            float(pose[0, 2]),
            float(pose[1, 2]),
            float(math.atan2(pose[1, 0], pose[0, 0])),
        ])

        pairs.append({
            "index": i,
            "selected": selected_name,
            "dx_curr_to_prev_m": selected.dx,
            "dy_curr_to_prev_m": selected.dy,
            "yaw_curr_to_prev_deg": math.degrees(selected.yaw),
            "accepted": True,
            "used_dx_curr_to_prev_m": selected.dx,
            "used_dy_curr_to_prev_m": selected.dy,
            "used_yaw_curr_to_prev_deg": math.degrees(selected.yaw),
            "score": selected.score,
            "overlap": selected.overlap,
            "policy_values": policy_values,
            "motion_consistency": motion_consistency,
            "candidates": {
                name: {
                    "dx_curr_to_prev_m": scored[name].dx,
                    "dy_curr_to_prev_m": scored[name].dy,
                    "yaw_curr_to_prev_deg": math.degrees(scored[name].yaw),
                    "cubemap_score": scored[name].score,
                    "geometry_score": raw[name].score,
                    "overlap": raw[name].overlap,
                    "shared_residual": shared_residuals.get(i, {}).get(name),
                }
                for name in candidate_names
            },
        })
        if args.progress_every > 0 and i % args.progress_every == 0:
            print(f"[prior-select] {i}/{len(scans)-1} selected={selected_name}")

    pose_arr = np.asarray(poses, dtype=np.float64)
    metrics = compute_metrics(pose_arr, gt)
    payload = {
        "sequence_pcd_dir": str(args.sequence_pcd_dir),
        "gt_csv": str(args.gt_csv) if args.gt_csv is not None else None,
        "frames": len(scans),
        "method": "selected_pair_prior_odometry",
        "parameters": {
            "candidate_source": args.candidate_source,
            "policy": args.policy,
            "preferred_name": args.preferred_name,
            "score_margin": args.score_margin,
            "cubemap_weight": args.cubemap_weight,
            "geometry_weight": args.geometry_weight,
            "motion_consistency_margin": args.motion_consistency_margin,
            "motion_consistency_min_prev_step": args.motion_consistency_min_prev_step,
            "motion_consistency_max_prev_step": args.motion_consistency_max_prev_step,
            "score_channel": args.score_channel,
            "face_size": args.face_size,
            "min_overlap_pixels": args.min_overlap_pixels,
            "shared_residual_json": str(args.shared_residual_json)
            if args.shared_residual_json is not None
            else None,
            "calibration_json": str(args.calibration_json)
            if args.calibration_json is not None
            else None,
            "candidates": args.candidate,
        },
        "runtime_s": time.perf_counter() - started_at,
        "metrics": metrics,
        "poses_xyyaw": [
            {"index": i, "x_m": float(p[0]), "y_m": float(p[1]), "yaw_deg": math.degrees(float(p[2]))}
            for i, p in enumerate(pose_arr)
        ],
        "pairs": pairs,
        "pcd_files": [str(p) for p in pcd_paths],
    }
    args.output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")

    if metrics is not None:
        print(
            f"[done] frames={len(scans)} ATE_xy={metrics['ate_xy_m']:.3f}m "
            f"step_rmse={metrics['step_length_rmse_m']:.3f}m -> {args.output_json}"
        )
    else:
        print(f"[done] frames={len(scans)} -> {args.output_json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
