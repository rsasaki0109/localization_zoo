#!/usr/bin/env python3
"""CUBE-LIO-style cubemap photometric odometry demo.

This is a compact direct LiDAR odometry scaffold:

  1. project LiDAR reflectance to six cubemap faces,
  2. search an SE(2) current-to-previous transform,
  3. rasterize the transformed current scan into the previous cubemap frame,
  4. score candidates by image normalized cross-correlation,
  5. integrate accepted relative transforms.

It is closer to CUBE-LIO's reflectance-image front-end than the BEV demo, but
still omits IMU coupling, ESIKF, and full photometric optimization.
"""

from __future__ import annotations

import argparse
import json
import math
import time
from dataclasses import dataclass
from pathlib import Path

import numpy as np

from cube_lio_cubemap_demo import (
    convolve_separable,
    dir_to_cube_face_uv,
    directions,
    gaussian_kernel1d,
    rasterize_max_intensity,
    sobel_magnitude,
    uv_to_pixel,
)
from intensity_bev_odometry_demo import (
    Candidate,
    Scan,
    compute_metrics,
    load_gt_xyyaw,
    preprocess_scan,
    read_binary_pcd_xyz_intensity,
    se2_inverse,
    se2_matrix,
    values_around,
)


@dataclass
class Cubemap:
    intensity: np.ndarray
    igm: np.ndarray | None = None
    feature_mask: np.ndarray | None = None


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
    p.add_argument("--max-points", type=int, default=4000)
    p.add_argument("--face-size", type=int, default=64)
    p.add_argument("--gaussian-sigma", type=float, default=1.0)
    p.add_argument(
        "--score-channel",
        choices=("intensity", "igm", "hybrid", "semi_dense"),
        default="intensity",
        help="Image channel used for candidate scoring.",
    )
    p.add_argument(
        "--feature-quantile",
        type=float,
        default=0.90,
        help="IGM quantile used for semi_dense scoring mask on the reference cubemap.",
    )
    p.add_argument("--x-window", type=float, default=1.5)
    p.add_argument("--y-window", type=float, default=0.75)
    p.add_argument("--yaw-window-deg", type=float, default=4.0)
    p.add_argument("--x-step", type=float, default=0.5)
    p.add_argument("--y-step", type=float, default=0.5)
    p.add_argument("--yaw-step-deg", type=float, default=2.0)
    p.add_argument("--refine-levels", type=int, default=1)
    p.add_argument("--min-overlap-pixels", type=int, default=120)
    p.add_argument("--max-step-translation", type=float, default=0.75)
    p.add_argument("--max-step-yaw-deg", type=float, default=4.0)
    p.add_argument(
        "--local-opt-iters",
        type=int,
        default=0,
        help="Coordinate-descent photometric refinement iterations after grid search.",
    )
    p.add_argument("--local-opt-x-step", type=float, default=0.15)
    p.add_argument("--local-opt-y-step", type=float, default=0.15)
    p.add_argument("--local-opt-yaw-step-deg", type=float, default=0.5)
    p.add_argument(
        "--local-opt-regularization",
        type=float,
        default=0.25,
        help="Penalty on local-opt deviation from the grid-search candidate.",
    )
    p.add_argument(
        "--local-opt-min-score-gain",
        type=float,
        default=0.02,
        help="Minimum raw score improvement required to accept local optimization.",
    )
    p.add_argument(
        "--local-opt-max-translation-delta",
        type=float,
        default=0.25,
        help="Maximum local-opt translation deviation from grid candidate.",
    )
    p.add_argument(
        "--local-opt-max-yaw-delta-deg",
        type=float,
        default=1.0,
        help="Maximum local-opt yaw deviation from grid candidate.",
    )
    p.add_argument(
        "--local-opt-min-overlap-ratio",
        type=float,
        default=0.85,
        help="Minimum optimized/grid overlap ratio required for acceptance.",
    )
    p.add_argument(
        "--motion-prior",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="Center each pair search on the previous accepted relative transform.",
    )
    p.add_argument(
        "--pair-prior-json",
        type=Path,
        help="Optional prior pair motions from another odometry JSON, e.g. BEV gated output.",
    )
    p.add_argument(
        "--pair-prior-source",
        choices=("used", "raw"),
        default="used",
        help="Use accepted/used or raw pair motions from --pair-prior-json.",
    )
    p.add_argument(
        "--prior-correction-gate",
        action="store_true",
        help="When a pair prior is available, keep it unless photometric search passes a correction gate.",
    )
    p.add_argument(
        "--prior-correction-min-score-gain",
        type=float,
        default=0.04,
        help="Minimum score gain over the pair prior required to accept a photometric correction.",
    )
    p.add_argument(
        "--prior-correction-max-translation",
        type=float,
        default=0.25,
        help="Maximum photometric correction translation away from the pair prior.",
    )
    p.add_argument(
        "--prior-correction-max-yaw-deg",
        type=float,
        default=1.0,
        help="Maximum photometric correction yaw away from the pair prior.",
    )
    p.add_argument(
        "--prior-correction-min-overlap-ratio",
        type=float,
        default=0.90,
        help="Minimum correction/prior overlap ratio required to accept a photometric correction.",
    )
    p.add_argument("--progress-every", type=int, default=10)
    return p.parse_args()


def transform_xyyaw(scan: Scan, cand: Candidate) -> Scan:
    c = math.cos(cand.yaw)
    s = math.sin(cand.yaw)
    xyz = scan.xyz.copy()
    x = c * scan.xyz[:, 0] - s * scan.xyz[:, 1] + cand.dx
    y = s * scan.xyz[:, 0] + c * scan.xyz[:, 1] + cand.dy
    xyz[:, 0] = x
    xyz[:, 1] = y
    return Scan(xyz=xyz, intensity=scan.intensity)


def make_cubemap(scan: Scan, args: argparse.Namespace, include_igm: bool) -> Cubemap:
    d = directions(scan.xyz)
    face, u, v = dir_to_cube_face_uv(d)
    row, col = uv_to_pixel(u, v, args.face_size)
    intensity = rasterize_max_intensity(face, row, col, scan.intensity, args.face_size)

    if not include_igm:
        return Cubemap(intensity=intensity)

    k = gaussian_kernel1d(args.gaussian_sigma)
    smoothed = np.empty_like(intensity)
    igm = np.empty_like(intensity)
    for i in range(6):
        smoothed[i] = convolve_separable(intensity[i], k) if k.size > 1 else intensity[i]
        igm[i] = sobel_magnitude(smoothed[i])
    feature_mask = None
    if args.score_channel == "semi_dense":
        valid = intensity > 0.0
        if np.any(valid):
            threshold = float(np.quantile(igm[valid], args.feature_quantile))
            feature_mask = valid & (igm >= threshold)
        else:
            feature_mask = np.zeros_like(intensity, dtype=bool)
    return Cubemap(intensity=intensity, igm=igm, feature_mask=feature_mask)


def ncc(a: np.ndarray, b: np.ndarray, mask: np.ndarray, min_overlap: int) -> tuple[float, int]:
    overlap = int(mask.sum())
    if overlap < min_overlap:
        return -1.0, overlap
    av = a[mask].astype(np.float64)
    bv = b[mask].astype(np.float64)
    av -= float(av.mean())
    bv -= float(bv.mean())
    denom = float(np.linalg.norm(av) * np.linalg.norm(bv))
    if denom <= 1e-12:
        return -1.0, overlap
    return float(av.dot(bv) / denom), overlap


def score_maps(ref: Cubemap, curr: Cubemap, args: argparse.Namespace) -> tuple[float, int]:
    mask_i = (ref.intensity > 0.0) & (curr.intensity > 0.0)
    score_i, overlap_i = ncc(ref.intensity, curr.intensity, mask_i, args.min_overlap_pixels)
    if args.score_channel == "intensity":
        return score_i, overlap_i

    if args.score_channel == "semi_dense":
        if ref.feature_mask is None:
            return score_i, overlap_i
        mask_s = mask_i & ref.feature_mask
        return ncc(ref.intensity, curr.intensity, mask_s, args.min_overlap_pixels)

    if ref.igm is None or curr.igm is None:
        return score_i, overlap_i

    mask_g = mask_i & (ref.igm > 0.0) & (curr.igm > 0.0)
    score_g, overlap_g = ncc(ref.igm, curr.igm, mask_g, args.min_overlap_pixels)
    if args.score_channel == "igm":
        return score_g, overlap_g

    if score_i < 0.0:
        return score_g, overlap_g
    if score_g < 0.0:
        return score_i, overlap_i
    return 0.65 * score_i + 0.35 * score_g, min(overlap_i, overlap_g)


def score_candidate(ref: Cubemap, curr_scan: Scan, cand: Candidate, args: argparse.Namespace) -> Candidate:
    curr_map = make_cubemap(
        transform_xyyaw(curr_scan, cand),
        args,
        include_igm=args.score_channel in ("igm", "hybrid"),
    )
    score, overlap = score_maps(ref, curr_map, args)
    return Candidate(cand.dx, cand.dy, cand.yaw, score, overlap)


def passes_motion_gate(cand: Candidate, args: argparse.Namespace) -> bool:
    if args.max_step_translation > 0.0:
        if math.hypot(cand.dx, cand.dy) > args.max_step_translation:
            return False
    if args.max_step_yaw_deg > 0.0:
        if abs(math.degrees(cand.yaw)) > args.max_step_yaw_deg:
            return False
    return True


def estimate_relative(
    prev_map: Cubemap,
    curr_scan: Scan,
    args: argparse.Namespace,
    prior: Candidate | None,
) -> Candidate:
    best = Candidate(0.0, 0.0, 0.0, -1.0, 0)
    if prior is not None:
        best = Candidate(prior.dx, prior.dy, prior.yaw, -1.0, 0)

    x_window = args.x_window
    y_window = args.y_window
    yaw_window = math.radians(args.yaw_window_deg)
    x_step = args.x_step
    y_step = args.y_step
    yaw_step = math.radians(args.yaw_step_deg)

    for _ in range(max(1, args.refine_levels)):
        anchor = score_candidate(prev_map, curr_scan, best, args)
        if anchor.score > best.score:
            best = anchor
        for yaw in values_around(best.yaw, yaw_window, yaw_step):
            for dx in values_around(best.dx, x_window, x_step):
                for dy in values_around(best.dy, y_window, y_step):
                    scored = score_candidate(
                        prev_map, curr_scan, Candidate(dx, dy, yaw, -1.0, 0), args
                    )
                    if scored.score > best.score:
                        best = scored
        x_window = max(x_step, x_window * 0.5)
        y_window = max(y_step, y_window * 0.5)
        yaw_window = max(yaw_step, yaw_window * 0.5)
        x_step *= 0.5
        y_step *= 0.5
        yaw_step *= 0.5
    return best


def local_optimize(
    prev_map: Cubemap,
    curr_scan: Scan,
    initial: Candidate,
    args: argparse.Namespace,
) -> Candidate:
    def objective(cand: Candidate) -> float:
        trans_dev = math.hypot(cand.dx - initial.dx, cand.dy - initial.dy)
        yaw_dev = abs(cand.yaw - initial.yaw)
        return cand.score - args.local_opt_regularization * (
            trans_dev + yaw_dev / math.radians(5.0)
        )

    best = score_candidate(prev_map, curr_scan, initial, args)
    best_obj = objective(best)
    dx_step = args.local_opt_x_step
    dy_step = args.local_opt_y_step
    yaw_step = math.radians(args.local_opt_yaw_step_deg)

    for _ in range(max(0, args.local_opt_iters)):
        improved = False
        for ddx, ddy, dyaw in (
            (dx_step, 0.0, 0.0),
            (-dx_step, 0.0, 0.0),
            (0.0, dy_step, 0.0),
            (0.0, -dy_step, 0.0),
            (0.0, 0.0, yaw_step),
            (0.0, 0.0, -yaw_step),
        ):
            cand = Candidate(
                best.dx + ddx,
                best.dy + ddy,
                best.yaw + dyaw,
                -1.0,
                0,
            )
            scored = score_candidate(prev_map, curr_scan, cand, args)
            scored_obj = objective(scored)
            if scored_obj > best_obj:
                best = scored
                best_obj = scored_obj
                improved = True
        if not improved:
            dx_step *= 0.5
            dy_step *= 0.5
            yaw_step *= 0.5
            if max(dx_step, dy_step, yaw_step) < 1e-4:
                break
    return best


def accept_local_optimization(
    grid: Candidate,
    refined: Candidate,
    args: argparse.Namespace,
) -> bool:
    if refined.score < grid.score + args.local_opt_min_score_gain:
        return False
    if math.hypot(refined.dx - grid.dx, refined.dy - grid.dy) > args.local_opt_max_translation_delta:
        return False
    if abs(math.degrees(refined.yaw - grid.yaw)) > args.local_opt_max_yaw_delta_deg:
        return False
    if grid.overlap > 0:
        if refined.overlap / float(grid.overlap) < args.local_opt_min_overlap_ratio:
            return False
    return True


def accept_prior_correction(
    prior: Candidate,
    corrected: Candidate,
    args: argparse.Namespace,
) -> bool:
    if corrected.score < prior.score + args.prior_correction_min_score_gain:
        return False
    if math.hypot(corrected.dx - prior.dx, corrected.dy - prior.dy) > args.prior_correction_max_translation:
        return False
    if abs(math.degrees(corrected.yaw - prior.yaw)) > args.prior_correction_max_yaw_deg:
        return False
    if prior.overlap > 0:
        if corrected.overlap / float(prior.overlap) < args.prior_correction_min_overlap_ratio:
            return False
    return True


def load_pair_priors(path: Path | None, source: str) -> dict[int, Candidate]:
    if path is None:
        return {}
    payload = json.loads(path.read_text())
    priors: dict[int, Candidate] = {}
    for pair in payload.get("pairs", []):
        idx = int(pair["index"])
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
        priors[idx] = Candidate(dx, dy, math.radians(yaw_deg), score, overlap)
    return priors


def main() -> int:
    args = parse_args()
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    started_at = time.perf_counter()

    scans: list[Scan] = []
    pcd_paths: list[Path] = []
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
    external_priors = load_pair_priors(args.pair_prior_json, args.pair_prior_source)

    pose = np.eye(3)
    poses = [[0.0, 0.0, 0.0]]
    pairs = []
    prior: Candidate | None = None
    for i in range(1, len(scans)):
        pair_prior = external_priors.get(i, prior if args.motion_prior else None)
        scored_pair_prior = (
            score_candidate(cubemaps[i - 1], scans[i], pair_prior, args)
            if pair_prior is not None
            else None
        )
        cand = estimate_relative(
            cubemaps[i - 1],
            scans[i],
            args,
            pair_prior,
        )
        searched_cand = cand
        grid_cand = cand
        prior_correction_accepted = False
        if args.prior_correction_gate and scored_pair_prior is not None:
            prior_correction_accepted = accept_prior_correction(
                scored_pair_prior, searched_cand, args
            )
            if not prior_correction_accepted:
                cand = scored_pair_prior
                grid_cand = scored_pair_prior
        local_opt_accepted = False
        if args.local_opt_iters > 0 and cand.score >= 0.0:
            refined = local_optimize(cubemaps[i - 1], scans[i], cand, args)
            if accept_local_optimization(cand, refined, args):
                cand = refined
                local_opt_accepted = True
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
            "grid_dx_curr_to_prev_m": grid_cand.dx,
            "grid_dy_curr_to_prev_m": grid_cand.dy,
            "grid_yaw_curr_to_prev_deg": math.degrees(grid_cand.yaw),
            "grid_score": grid_cand.score,
            "grid_overlap_pixels": grid_cand.overlap,
            "search_dx_curr_to_prev_m": searched_cand.dx,
            "search_dy_curr_to_prev_m": searched_cand.dy,
            "search_yaw_curr_to_prev_deg": math.degrees(searched_cand.yaw),
            "search_score": searched_cand.score,
            "search_overlap_pixels": searched_cand.overlap,
            "external_prior_used": i in external_priors,
            "prior_dx_curr_to_prev_m": scored_pair_prior.dx if scored_pair_prior is not None else None,
            "prior_dy_curr_to_prev_m": scored_pair_prior.dy if scored_pair_prior is not None else None,
            "prior_yaw_curr_to_prev_deg": (
                math.degrees(scored_pair_prior.yaw)
                if scored_pair_prior is not None
                else None
            ),
            "prior_score": scored_pair_prior.score if scored_pair_prior is not None else None,
            "prior_overlap_pixels": scored_pair_prior.overlap if scored_pair_prior is not None else None,
            "prior_correction_score_gain": (
                searched_cand.score - scored_pair_prior.score
                if scored_pair_prior is not None
                else None
            ),
            "prior_correction_translation_delta_m": (
                math.hypot(searched_cand.dx - scored_pair_prior.dx,
                           searched_cand.dy - scored_pair_prior.dy)
                if scored_pair_prior is not None
                else None
            ),
            "prior_correction_yaw_delta_deg": (
                abs(math.degrees(searched_cand.yaw - scored_pair_prior.yaw))
                if scored_pair_prior is not None
                else None
            ),
            "prior_correction_accepted": prior_correction_accepted,
            "local_opt_accepted": local_opt_accepted,
            "accepted": accepted,
            "used_dx_curr_to_prev_m": pose_cand.dx,
            "used_dy_curr_to_prev_m": pose_cand.dy,
            "used_yaw_curr_to_prev_deg": math.degrees(pose_cand.yaw),
            "score": cand.score,
            "overlap_pixels": cand.overlap,
        })
        if args.progress_every > 0 and i % args.progress_every == 0:
            print(
                f"[cubemap-odom] {i}/{len(scans)-1} "
                f"score={cand.score:.3f} overlap={cand.overlap} accepted={accepted}"
            )

    pose_arr = np.asarray(poses, dtype=np.float64)
    metrics = compute_metrics(pose_arr, gt)
    payload = {
        "sequence_pcd_dir": str(args.sequence_pcd_dir),
        "gt_csv": str(args.gt_csv) if args.gt_csv is not None else None,
        "frames": len(scans),
        "method": "intensity_cubemap_direct_odometry",
        "parameters": {
            "face_size": args.face_size,
            "gaussian_sigma": args.gaussian_sigma,
            "score_channel": args.score_channel,
            "feature_quantile": args.feature_quantile,
            "max_points": args.max_points,
            "x_window": args.x_window,
            "y_window": args.y_window,
            "yaw_window_deg": args.yaw_window_deg,
            "x_step": args.x_step,
            "y_step": args.y_step,
            "yaw_step_deg": args.yaw_step_deg,
            "refine_levels": args.refine_levels,
            "min_overlap_pixels": args.min_overlap_pixels,
            "max_step_translation": args.max_step_translation,
            "max_step_yaw_deg": args.max_step_yaw_deg,
            "local_opt_iters": args.local_opt_iters,
            "local_opt_x_step": args.local_opt_x_step,
            "local_opt_y_step": args.local_opt_y_step,
            "local_opt_yaw_step_deg": args.local_opt_yaw_step_deg,
            "local_opt_regularization": args.local_opt_regularization,
            "local_opt_min_score_gain": args.local_opt_min_score_gain,
            "local_opt_max_translation_delta": args.local_opt_max_translation_delta,
            "local_opt_max_yaw_delta_deg": args.local_opt_max_yaw_delta_deg,
            "local_opt_min_overlap_ratio": args.local_opt_min_overlap_ratio,
            "motion_prior": args.motion_prior,
            "pair_prior_json": str(args.pair_prior_json) if args.pair_prior_json else None,
            "pair_prior_source": args.pair_prior_source,
            "prior_correction_gate": args.prior_correction_gate,
            "prior_correction_min_score_gain": args.prior_correction_min_score_gain,
            "prior_correction_max_translation": args.prior_correction_max_translation,
            "prior_correction_max_yaw_deg": args.prior_correction_max_yaw_deg,
            "prior_correction_min_overlap_ratio": args.prior_correction_min_overlap_ratio,
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
