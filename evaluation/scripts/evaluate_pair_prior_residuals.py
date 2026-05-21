#!/usr/bin/env python3
"""Evaluate candidate pair priors with a shared scan-to-scan residual."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np
from scipy.spatial import cKDTree

from geometry_icp_odometry_demo import preprocess_geometry_scan, transform_points
from intensity_bev_odometry_demo import Candidate, read_binary_pcd_xyz_intensity, se2_inverse, se2_matrix
from select_pair_prior import candidate_from_pair


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--sequence-pcd-dir", required=True, type=Path)
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument("--output-md", type=Path)
    p.add_argument("--max-frames", type=int, default=60)
    p.add_argument(
        "--candidate",
        action="append",
        required=True,
        help="Candidate prior as name:path. Repeat for KISS, SmallGICP, etc.",
    )
    p.add_argument("--candidate-source", choices=("used", "raw"), default="used")
    p.add_argument("--min-range", type=float, default=2.0)
    p.add_argument("--max-range", type=float, default=70.0)
    p.add_argument("--z-min", type=float, default=-3.0)
    p.add_argument("--z-max", type=float, default=3.0)
    p.add_argument("--max-points", type=int, default=5000)
    p.add_argument("--voxel-size", type=float, default=0.75)
    p.add_argument("--max-correspondence-distance", type=float, default=1.5)
    p.add_argument(
        "--normal-k",
        type=int,
        default=12,
        help="Neighbors used to estimate 2D target normals for point-to-line residual.",
    )
    p.add_argument(
        "--min-linearity",
        type=float,
        default=0.35,
        help="Minimum local line-likeness weight used by normal-aware residual.",
    )
    p.add_argument(
        "--trim-fraction",
        type=float,
        default=0.80,
        help="Fraction of smallest inlier distances used for trimmed residual.",
    )
    p.add_argument(
        "--stability-translation-step",
        type=float,
        default=0.20,
        help="Translation perturbation in meters for local residual stability scoring.",
    )
    p.add_argument(
        "--stability-yaw-step-deg",
        type=float,
        default=1.0,
        help="Yaw perturbation in degrees for local residual stability scoring.",
    )
    p.add_argument("--progress-every", type=int, default=10)
    return p.parse_args()


def load_candidate_sets(specs: list[str], source: str) -> dict[str, dict[int, Candidate]]:
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


def inverse_candidate(cand: Candidate) -> Candidate:
    mat = se2_inverse(se2_matrix(cand.dx, cand.dy, cand.yaw))
    return Candidate(
        float(mat[0, 2]),
        float(mat[1, 2]),
        float(math.atan2(mat[1, 0], mat[0, 0])),
        cand.score,
        cand.overlap,
    )


def estimate_normals_xy(points: np.ndarray, k: int) -> tuple[np.ndarray, np.ndarray]:
    normals = np.zeros_like(points, dtype=np.float64)
    linearity = np.zeros(points.shape[0], dtype=np.float64)
    if points.shape[0] < 3:
        normals[:, 0] = 1.0
        return normals, linearity
    k = min(max(3, k), points.shape[0])
    tree = cKDTree(points)
    _, indices = tree.query(points, k=k)
    if indices.ndim == 1:
        indices = indices[:, None]
    for i, idx in enumerate(indices):
        local = points[idx]
        centered = local - local.mean(axis=0)
        cov = centered.T @ centered / max(1, centered.shape[0] - 1)
        vals, vecs = np.linalg.eigh(cov)
        order = np.argsort(vals)
        vals = vals[order]
        vecs = vecs[:, order]
        normal = vecs[:, 0]
        norm = float(np.linalg.norm(normal))
        normals[i] = normal / max(norm, 1e-12)
        denom = max(float(vals[1]), 1e-12)
        linearity[i] = float(np.clip((vals[1] - vals[0]) / denom, 0.0, 1.0))
    return normals, linearity


def directed_residual(
    target_xy: np.ndarray,
    source_xy: np.ndarray,
    cand_source_to_target: Candidate,
    max_dist: float,
    trim_fraction: float,
    target_normals: np.ndarray,
    target_linearity: np.ndarray,
    min_linearity: float,
    target_tree: cKDTree | None = None,
) -> dict[str, float | int]:
    if target_xy.shape[0] == 0 or source_xy.shape[0] == 0:
        return {
            "rmse_m": float("inf"),
            "trimmed_rmse_m": float("inf"),
            "median_m": float("inf"),
            "p95_m": float("inf"),
            "plane_rmse_m": float("inf"),
            "trimmed_plane_rmse_m": float("inf"),
            "plane_median_m": float("inf"),
            "linearity_mean": 0.0,
            "inlier_ratio": 0.0,
            "correspondences": 0,
        }
    tree = target_tree if target_tree is not None else cKDTree(target_xy)
    transformed = transform_points(source_xy, cand_source_to_target)
    dists, indices = tree.query(transformed, k=1, distance_upper_bound=max_dist)
    valid = np.isfinite(dists) & (indices < target_xy.shape[0])
    if not np.any(valid):
        return {
            "rmse_m": float("inf"),
            "trimmed_rmse_m": float("inf"),
            "median_m": float("inf"),
            "p95_m": float("inf"),
            "plane_rmse_m": float("inf"),
            "trimmed_plane_rmse_m": float("inf"),
            "plane_median_m": float("inf"),
            "linearity_mean": 0.0,
            "inlier_ratio": 0.0,
            "correspondences": 0,
        }
    valid_dists = dists[valid]
    valid_indices = indices[valid]
    deltas = transformed[valid] - target_xy[valid_indices]
    plane = np.abs(np.sum(deltas * target_normals[valid_indices], axis=1))
    weights = np.maximum(target_linearity[valid_indices], min_linearity)
    weighted_plane_sq = weights * plane * plane
    trim_fraction = float(np.clip(trim_fraction, 0.05, 1.0))
    trim_count = max(1, int(math.ceil(valid_dists.size * trim_fraction)))
    trimmed = np.partition(valid_dists, trim_count - 1)[:trim_count]
    trimmed_plane = np.partition(plane, trim_count - 1)[:trim_count]
    return {
        "rmse_m": float(np.sqrt(np.mean(valid_dists * valid_dists))),
        "trimmed_rmse_m": float(np.sqrt(np.mean(trimmed * trimmed))),
        "median_m": float(np.median(valid_dists)),
        "p95_m": float(np.percentile(valid_dists, 95.0)),
        "plane_rmse_m": float(np.sqrt(np.mean(weighted_plane_sq))),
        "trimmed_plane_rmse_m": float(np.sqrt(np.mean(trimmed_plane * trimmed_plane))),
        "plane_median_m": float(np.median(plane)),
        "linearity_mean": float(np.mean(target_linearity[valid_indices])),
        "inlier_ratio": float(valid.sum() / max(1, source_xy.shape[0])),
        "correspondences": int(valid.sum()),
    }


def residual_metrics(
    prev_xy: np.ndarray,
    curr_xy: np.ndarray,
    cand: Candidate,
    max_dist: float,
    trim_fraction: float,
    normal_k: int,
    min_linearity: float,
) -> dict[str, float | int]:
    prev_normals, prev_linearity = estimate_normals_xy(prev_xy, normal_k)
    curr_normals, curr_linearity = estimate_normals_xy(curr_xy, normal_k)
    return residual_metrics_precomputed(
        prev_xy,
        curr_xy,
        cand,
        max_dist,
        trim_fraction,
        min_linearity,
        prev_normals,
        prev_linearity,
        curr_normals,
        curr_linearity,
        cKDTree(prev_xy) if prev_xy.shape[0] else None,
        cKDTree(curr_xy) if curr_xy.shape[0] else None,
    )


def residual_metrics_precomputed(
    prev_xy: np.ndarray,
    curr_xy: np.ndarray,
    cand: Candidate,
    max_dist: float,
    trim_fraction: float,
    min_linearity: float,
    prev_normals: np.ndarray,
    prev_linearity: np.ndarray,
    curr_normals: np.ndarray,
    curr_linearity: np.ndarray,
    prev_tree: cKDTree | None,
    curr_tree: cKDTree | None,
) -> dict[str, float | int]:
    forward = directed_residual(
        prev_xy,
        curr_xy,
        cand,
        max_dist,
        trim_fraction,
        prev_normals,
        prev_linearity,
        min_linearity,
        prev_tree,
    )
    backward = directed_residual(
        curr_xy,
        prev_xy,
        inverse_candidate(cand),
        max_dist,
        trim_fraction,
        curr_normals,
        curr_linearity,
        min_linearity,
        curr_tree,
    )
    return {
        "rmse_m": forward["rmse_m"],
        "trimmed_rmse_m": forward["trimmed_rmse_m"],
        "median_m": forward["median_m"],
        "p95_m": forward["p95_m"],
        "inlier_ratio": forward["inlier_ratio"],
        "correspondences": forward["correspondences"],
        "backward_rmse_m": backward["rmse_m"],
        "backward_trimmed_rmse_m": backward["trimmed_rmse_m"],
        "backward_plane_rmse_m": backward["plane_rmse_m"],
        "backward_trimmed_plane_rmse_m": backward["trimmed_plane_rmse_m"],
        "backward_inlier_ratio": backward["inlier_ratio"],
        "backward_correspondences": backward["correspondences"],
        "symmetric_rmse_m": 0.5 * (float(forward["rmse_m"]) + float(backward["rmse_m"])),
        "symmetric_trimmed_rmse_m": 0.5 * (
            float(forward["trimmed_rmse_m"]) + float(backward["trimmed_rmse_m"])
        ),
        "symmetric_plane_rmse_m": 0.5 * (
            float(forward["plane_rmse_m"]) + float(backward["plane_rmse_m"])
        ),
        "symmetric_trimmed_plane_rmse_m": 0.5 * (
            float(forward["trimmed_plane_rmse_m"]) + float(backward["trimmed_plane_rmse_m"])
        ),
        "symmetric_linearity_mean": 0.5 * (
            float(forward["linearity_mean"]) + float(backward["linearity_mean"])
        ),
        "symmetric_inlier_ratio": 0.5 * (
            float(forward["inlier_ratio"]) + float(backward["inlier_ratio"])
        ),
    }


def score_from_residual(metrics: dict[str, float | int]) -> float:
    rmse = float(metrics["symmetric_trimmed_plane_rmse_m"])
    inlier_ratio = float(metrics["symmetric_inlier_ratio"])
    if not math.isfinite(rmse):
        return -float("inf")
    return inlier_ratio / (1.0 + rmse)


def perturb_candidate(cand: Candidate, dx: float, dy: float, dyaw: float) -> Candidate:
    return Candidate(cand.dx + dx, cand.dy + dy, cand.yaw + dyaw, cand.score, cand.overlap)


def stability_metrics(
    prev_xy: np.ndarray,
    curr_xy: np.ndarray,
    cand: Candidate,
    base_metrics: dict[str, float | int],
    max_dist: float,
    trim_fraction: float,
    min_linearity: float,
    prev_normals: np.ndarray,
    prev_linearity: np.ndarray,
    curr_normals: np.ndarray,
    curr_linearity: np.ndarray,
    prev_tree: cKDTree | None,
    curr_tree: cKDTree | None,
    translation_step: float,
    yaw_step_deg: float,
) -> dict[str, float]:
    base = float(base_metrics["symmetric_trimmed_plane_rmse_m"])
    if not math.isfinite(base):
        return {
            "stability_min_delta_m": float("-inf"),
            "stability_mean_delta_m": float("-inf"),
            "stability_worst_delta_m": float("-inf"),
            "stability_score": -float("inf"),
        }

    yaw_step = math.radians(yaw_step_deg)
    perturbations = [
        (translation_step, 0.0, 0.0),
        (-translation_step, 0.0, 0.0),
        (0.0, translation_step, 0.0),
        (0.0, -translation_step, 0.0),
        (0.0, 0.0, yaw_step),
        (0.0, 0.0, -yaw_step),
    ]
    deltas = []
    for dx, dy, dyaw in perturbations:
        perturbed = perturb_candidate(cand, dx, dy, dyaw)
        metrics = residual_metrics_precomputed(
            prev_xy,
            curr_xy,
            perturbed,
            max_dist,
            trim_fraction,
            min_linearity,
            prev_normals,
            prev_linearity,
            curr_normals,
            curr_linearity,
            prev_tree,
            curr_tree,
        )
        value = float(metrics["symmetric_trimmed_plane_rmse_m"])
        if math.isfinite(value):
            deltas.append(value - base)

    if not deltas:
        return {
            "stability_min_delta_m": float("-inf"),
            "stability_mean_delta_m": float("-inf"),
            "stability_worst_delta_m": float("-inf"),
            "stability_score": -float("inf"),
        }

    arr = np.asarray(deltas, dtype=np.float64)
    mean_delta = float(arr.mean())
    min_delta = float(arr.min())
    worst_delta = float(arr.max())
    score = mean_delta / max(base, 1e-6)
    return {
        "stability_min_delta_m": min_delta,
        "stability_mean_delta_m": mean_delta,
        "stability_worst_delta_m": worst_delta,
        "stability_score": score,
    }


def main() -> int:
    args = parse_args()
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    if args.output_md is not None:
        args.output_md.parent.mkdir(parents=True, exist_ok=True)

    candidate_sets = load_candidate_sets(args.candidate, args.candidate_source)
    names = list(candidate_sets)
    scans = []
    pcd_paths = []
    for i in range(args.max_frames):
        pcd_path = args.sequence_pcd_dir / f"{i:08d}" / "cloud.pcd"
        if not pcd_path.is_file():
            break
        scans.append(preprocess_geometry_scan(read_binary_pcd_xyz_intensity(pcd_path), args))
        pcd_paths.append(str(pcd_path))
    if len(scans) < 2:
        raise RuntimeError("Need at least two scans")

    pairs = []
    selected_counts: dict[str, int] = {name: 0 for name in names}
    for i in range(1, len(scans)):
        prev_normals, prev_linearity = estimate_normals_xy(scans[i - 1], args.normal_k)
        curr_normals, curr_linearity = estimate_normals_xy(scans[i], args.normal_k)
        prev_tree = cKDTree(scans[i - 1]) if scans[i - 1].shape[0] else None
        curr_tree = cKDTree(scans[i]) if scans[i].shape[0] else None
        methods = {}
        scores = {}
        for name in names:
            cand = candidate_sets[name].get(i, Candidate(0.0, 0.0, 0.0, -1.0, 0))
            metrics = residual_metrics_precomputed(
                scans[i - 1],
                scans[i],
                cand,
                args.max_correspondence_distance,
                args.trim_fraction,
                args.min_linearity,
                prev_normals,
                prev_linearity,
                curr_normals,
                curr_linearity,
                prev_tree,
                curr_tree,
            )
            stability = stability_metrics(
                scans[i - 1],
                scans[i],
                cand,
                metrics,
                args.max_correspondence_distance,
                args.trim_fraction,
                args.min_linearity,
                prev_normals,
                prev_linearity,
                curr_normals,
                curr_linearity,
                prev_tree,
                curr_tree,
                args.stability_translation_step,
                args.stability_yaw_step_deg,
            )
            score = score_from_residual(metrics)
            scores[name] = score
            methods[name] = {
                "dx_curr_to_prev_m": cand.dx,
                "dy_curr_to_prev_m": cand.dy,
                "yaw_curr_to_prev_deg": math.degrees(cand.yaw),
                "geometry_score": cand.score,
                "overlap": cand.overlap,
                "shared_residual_score": score,
                **metrics,
                **stability,
            }
        selected = max(scores, key=scores.get)
        selected_counts[selected] += 1
        pairs.append({
            "index": i,
            "selected_by_shared_residual": selected,
            "methods": methods,
        })
        if args.progress_every > 0 and i % args.progress_every == 0:
            print(f"[shared-residual] {i}/{len(scans)-1} selected={selected}")

    payload = {
        "sequence_pcd_dir": str(args.sequence_pcd_dir),
        "frames": len(scans),
        "candidate_source": args.candidate_source,
        "parameters": {
            "max_frames": args.max_frames,
            "voxel_size": args.voxel_size,
            "max_points": args.max_points,
            "max_correspondence_distance": args.max_correspondence_distance,
            "trim_fraction": args.trim_fraction,
            "normal_k": args.normal_k,
            "min_linearity": args.min_linearity,
            "stability_translation_step": args.stability_translation_step,
            "stability_yaw_step_deg": args.stability_yaw_step_deg,
            "candidates": args.candidate,
        },
        "selected_counts": selected_counts,
        "pairs": pairs,
        "pcd_files": pcd_paths,
    }
    args.output_json.write_text(json.dumps(payload, indent=2) + "\n")

    if args.output_md is not None:
        lines = [
            "# Shared Pair Residuals",
            "",
            f"- Sequence: `{args.sequence_pcd_dir}`",
            f"- Selected counts: {selected_counts}",
            "",
            "| Pair | Selected | " + " | ".join(f"{name} sym-plane / inlier" for name in names) + " |",
            "| ---: | --- | " + " | ".join("---:" for _ in names) + " |",
        ]
        for pair in pairs:
            cells = []
            for name in names:
                m = pair["methods"][name]
                cells.append(f"{m['symmetric_trimmed_plane_rmse_m']:.3f} / {m['symmetric_inlier_ratio']:.3f}")
            lines.append(
                f"| {pair['index']} | {pair['selected_by_shared_residual']} | "
                + " | ".join(cells)
                + " |"
            )
        args.output_md.write_text("\n".join(lines) + "\n")

    print(f"[done] selected_counts={selected_counts} -> {args.output_json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
