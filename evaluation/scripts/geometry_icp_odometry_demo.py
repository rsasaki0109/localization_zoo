#!/usr/bin/env python3
"""Lightweight geometry-backed pairwise ICP odometry demo.

This produces the same pair-motion JSON shape as the intensity odometry demos,
so its output can be passed to `intensity_cubemap_odometry_demo.py` via
`--pair-prior-json`.

Scope: 2D point-to-point ICP over filtered LiDAR points. This is not a full
KISS-ICP/GICP implementation, but it is a geometry-backed prior for testing
whether photometric cubemap refinement can improve on non-image odometry.
"""

from __future__ import annotations

import argparse
import json
import math
import time
from pathlib import Path

import numpy as np
from scipy.spatial import cKDTree

from intensity_bev_odometry_demo import (
    Candidate,
    Scan,
    compute_metrics,
    load_gt_xyyaw,
    preprocess_scan,
    read_binary_pcd_xyz_intensity,
    se2_inverse,
    se2_matrix,
)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--sequence-pcd-dir", required=True, type=Path)
    p.add_argument("--gt-csv", type=Path)
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument("--max-frames", type=int, default=60)
    p.add_argument("--min-range", type=float, default=2.0)
    p.add_argument("--max-range", type=float, default=70.0)
    p.add_argument("--z-min", type=float, default=-3.0)
    p.add_argument("--z-max", type=float, default=3.0)
    p.add_argument("--max-points", type=int, default=5000)
    p.add_argument("--voxel-size", type=float, default=0.75)
    p.add_argument("--max-iterations", type=int, default=20)
    p.add_argument("--max-correspondence-distance", type=float, default=1.5)
    p.add_argument("--min-correspondences", type=int, default=80)
    p.add_argument("--convergence-translation", type=float, default=1e-3)
    p.add_argument("--convergence-yaw-deg", type=float, default=0.02)
    p.add_argument("--max-step-translation", type=float, default=0.3)
    p.add_argument("--max-step-yaw-deg", type=float, default=6.0)
    p.add_argument(
        "--motion-prior",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="Initialize each pair ICP from the previous accepted pair motion.",
    )
    p.add_argument("--progress-every", type=int, default=10)
    return p.parse_args()


def voxel_downsample_xy(points: np.ndarray, voxel_size: float) -> np.ndarray:
    if voxel_size <= 0.0 or points.shape[0] == 0:
        return points
    keys = np.floor(points[:, :2] / voxel_size).astype(np.int64)
    _, first = np.unique(keys, axis=0, return_index=True)
    return points[np.sort(first)]


def preprocess_geometry_scan(scan: Scan, args: argparse.Namespace) -> np.ndarray:
    filtered = preprocess_scan(scan, args)
    points = filtered.xyz[:, :2]
    points = voxel_downsample_xy(points, args.voxel_size)
    if points.shape[0] > args.max_points:
        step = points.shape[0] / args.max_points
        idx = np.minimum((np.arange(args.max_points) * step).astype(np.int64), points.shape[0] - 1)
        points = points[idx]
    return points


def transform_points(points: np.ndarray, cand: Candidate) -> np.ndarray:
    c = math.cos(cand.yaw)
    s = math.sin(cand.yaw)
    out = np.empty_like(points)
    out[:, 0] = c * points[:, 0] - s * points[:, 1] + cand.dx
    out[:, 1] = s * points[:, 0] + c * points[:, 1] + cand.dy
    return out


def compose(a: Candidate, b: Candidate) -> Candidate:
    """Return transform b after a, both current-to-previous style SE2."""
    ta = se2_matrix(a.dx, a.dy, a.yaw)
    tb = se2_matrix(b.dx, b.dy, b.yaw)
    t = tb @ ta
    return Candidate(float(t[0, 2]), float(t[1, 2]), float(math.atan2(t[1, 0], t[0, 0])), -1.0, 0)


def estimate_rigid_2d(src: np.ndarray, dst: np.ndarray) -> Candidate:
    src_mean = src.mean(axis=0)
    dst_mean = dst.mean(axis=0)
    src_c = src - src_mean
    dst_c = dst - dst_mean
    h = src_c.T @ dst_c
    u, _, vt = np.linalg.svd(h)
    r = vt.T @ u.T
    if np.linalg.det(r) < 0.0:
        vt[-1, :] *= -1.0
        r = vt.T @ u.T
    t = dst_mean - r @ src_mean
    yaw = math.atan2(float(r[1, 0]), float(r[0, 0]))
    return Candidate(float(t[0]), float(t[1]), yaw, -1.0, int(src.shape[0]))


def passes_motion_gate(cand: Candidate, args: argparse.Namespace) -> bool:
    if math.hypot(cand.dx, cand.dy) > args.max_step_translation:
        return False
    if abs(math.degrees(cand.yaw)) > args.max_step_yaw_deg:
        return False
    return True


def run_pair_icp(prev_xy: np.ndarray, curr_xy: np.ndarray, prior: Candidate | None, args: argparse.Namespace) -> Candidate:
    if prev_xy.shape[0] < args.min_correspondences or curr_xy.shape[0] < args.min_correspondences:
        return Candidate(0.0, 0.0, 0.0, -1.0, 0)

    tree = cKDTree(prev_xy)
    estimate = prior if prior is not None else Candidate(0.0, 0.0, 0.0, -1.0, 0)
    max_dist = args.max_correspondence_distance
    rmse = float("inf")
    correspondences = 0

    for _ in range(args.max_iterations):
        transformed = transform_points(curr_xy, estimate)
        dists, indices = tree.query(transformed, k=1, distance_upper_bound=max_dist)
        valid = np.isfinite(dists) & (indices < prev_xy.shape[0])
        correspondences = int(valid.sum())
        if correspondences < args.min_correspondences:
            break

        src = transformed[valid]
        dst = prev_xy[indices[valid]]
        delta = estimate_rigid_2d(src, dst)
        estimate = compose(estimate, delta)
        rmse = float(np.sqrt(np.mean(dists[valid] ** 2)))
        if math.hypot(delta.dx, delta.dy) < args.convergence_translation and abs(math.degrees(delta.yaw)) < args.convergence_yaw_deg:
            break

    score = 1.0 / (1.0 + rmse) if math.isfinite(rmse) else -1.0
    return Candidate(estimate.dx, estimate.dy, estimate.yaw, score, correspondences)


def main() -> int:
    args = parse_args()
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    started_at = time.perf_counter()

    scans: list[np.ndarray] = []
    pcd_paths: list[Path] = []
    for i in range(args.max_frames):
        pcd_path = args.sequence_pcd_dir / f"{i:08d}" / "cloud.pcd"
        if not pcd_path.is_file():
            break
        scans.append(preprocess_geometry_scan(read_binary_pcd_xyz_intensity(pcd_path), args))
        pcd_paths.append(pcd_path)
    if len(scans) < 2:
        raise RuntimeError("Need at least two scans")

    gt = load_gt_xyyaw(args.gt_csv) if args.gt_csv is not None else None
    pose = np.eye(3)
    poses = [[0.0, 0.0, 0.0]]
    pairs = []
    prior: Candidate | None = None
    for i in range(1, len(scans)):
        cand = run_pair_icp(scans[i - 1], scans[i], prior if args.motion_prior else None, args)
        accepted = cand.score >= 0.0 and passes_motion_gate(cand, args)
        if accepted:
            pose_cand = cand
            prior = cand
        elif prior is not None and passes_motion_gate(prior, args):
            pose_cand = prior
        else:
            pose_cand = Candidate(0.0, 0.0, 0.0, cand.score, cand.overlap)

        pose = pose @ se2_matrix(pose_cand.dx, pose_cand.dy, pose_cand.yaw)
        poses.append([float(pose[0, 2]), float(pose[1, 2]), float(math.atan2(pose[1, 0], pose[0, 0]))])
        pairs.append({
            "index": i,
            "dx_curr_to_prev_m": cand.dx,
            "dy_curr_to_prev_m": cand.dy,
            "yaw_curr_to_prev_deg": math.degrees(cand.yaw),
            "accepted": accepted,
            "used_dx_curr_to_prev_m": pose_cand.dx,
            "used_dy_curr_to_prev_m": pose_cand.dy,
            "used_yaw_curr_to_prev_deg": math.degrees(pose_cand.yaw),
            "score": cand.score,
            "overlap": cand.overlap,
        })
        if args.progress_every > 0 and i % args.progress_every == 0:
            print(
                f"[geometry-icp] {i}/{len(scans)-1} "
                f"score={cand.score:.3f} corr={cand.overlap} accepted={accepted}"
            )

    pose_arr = np.asarray(poses, dtype=np.float64)
    metrics = compute_metrics(pose_arr, gt)
    payload = {
        "sequence_pcd_dir": str(args.sequence_pcd_dir),
        "gt_csv": str(args.gt_csv) if args.gt_csv is not None else None,
        "frames": len(scans),
        "method": "geometry_icp_pairwise_odometry",
        "parameters": {
            "max_points": args.max_points,
            "voxel_size": args.voxel_size,
            "max_iterations": args.max_iterations,
            "max_correspondence_distance": args.max_correspondence_distance,
            "min_correspondences": args.min_correspondences,
            "max_step_translation": args.max_step_translation,
            "max_step_yaw_deg": args.max_step_yaw_deg,
            "motion_prior": args.motion_prior,
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
