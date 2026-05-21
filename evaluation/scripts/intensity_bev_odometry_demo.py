#!/usr/bin/env python3
"""Direct reflectance-image LiDAR odometry demo.

This is a compact reproduction scaffold for intensity-based LiDAR odometry:

  1. rasterize each scan into a bird's-eye-view max-reflectance image,
  2. search an SE(2) frame-to-frame transform,
  3. score candidates by normalized cross-correlation of reflectance values,
  4. integrate the best relative transforms into an odometry trajectory.

It is intentionally small and dependency-light. It does not implement the full
CUBE-LIO backend, IMU coupling, or photometric bundle adjustment.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import time
from dataclasses import dataclass
from pathlib import Path

import numpy as np


@dataclass
class Scan:
    xyz: np.ndarray
    intensity: np.ndarray


@dataclass
class Candidate:
    dx: float
    dy: float
    yaw: float
    score: float
    overlap: int


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--sequence-pcd-dir", required=True, type=Path)
    p.add_argument("--gt-csv", type=Path)
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument(
        "--preset",
        choices=("default", "fast", "pyramid"),
        default="default",
        help="Use fast for quick smoke sweeps; pyramid adds coarse-to-fine search.",
    )
    p.add_argument("--max-frames", type=int, default=60)
    p.add_argument("--start-frame", type=int, default=0)
    p.add_argument("--min-range", type=float, default=2.0)
    p.add_argument("--max-range", type=float, default=70.0)
    p.add_argument("--z-min", type=float, default=-3.0)
    p.add_argument("--z-max", type=float, default=3.0)
    p.add_argument("--max-points", type=int, default=16000)
    p.add_argument("--grid-resolution", type=float, default=0.35)
    p.add_argument("--grid-radius", type=float, default=45.0)
    p.add_argument("--x-window", type=float, default=3.0)
    p.add_argument("--y-window", type=float, default=1.5)
    p.add_argument("--yaw-window-deg", type=float, default=4.0)
    p.add_argument("--x-step", type=float, default=0.5)
    p.add_argument("--y-step", type=float, default=0.5)
    p.add_argument("--yaw-step-deg", type=float, default=1.0)
    p.add_argument("--refine-levels", type=int, default=2)
    p.add_argument(
        "--pyramid-levels",
        type=int,
        default=1,
        help="Coarse-to-fine BEV levels. 1 disables pyramid search.",
    )
    p.add_argument("--min-overlap", type=int, default=400)
    p.add_argument(
        "--max-step-translation",
        type=float,
        default=0.0,
        help="Reject pairwise estimates above this translation norm. <=0 disables.",
    )
    p.add_argument(
        "--max-step-yaw-deg",
        type=float,
        default=0.0,
        help="Reject pairwise estimates above this absolute yaw. <=0 disables.",
    )
    p.add_argument(
        "--motion-prior",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="Center each pair search on the previous best relative transform.",
    )
    p.add_argument("--progress-every", type=int, default=10)
    args = p.parse_args()
    if args.preset in ("fast", "pyramid"):
        args.max_points = min(args.max_points, 6000)
        args.grid_resolution = max(args.grid_resolution, 0.5)
        args.x_step = max(args.x_step, 1.0)
        args.y_step = max(args.y_step, 0.5)
        args.yaw_step_deg = max(args.yaw_step_deg, 2.0)
        args.refine_levels = min(args.refine_levels, 1)
        args.min_overlap = max(args.min_overlap, 200)
        if args.max_step_translation <= 0.0:
            args.max_step_translation = 0.75
        if args.max_step_yaw_deg <= 0.0:
            args.max_step_yaw_deg = 4.0
    if args.preset == "pyramid":
        args.pyramid_levels = max(args.pyramid_levels, 3)
    return args


def read_binary_pcd_xyz_intensity(path: Path) -> Scan:
    raw = path.read_bytes()
    header_end = raw.find(b"DATA binary\n")
    if header_end < 0:
        raise RuntimeError(f"{path}: expected binary PCD with x y z intensity")
    header_end += len(b"DATA binary\n")
    header = raw[:header_end].decode("ascii", errors="replace")

    fields: list[str] = []
    sizes: list[int] = []
    types: list[str] = []
    counts: list[int] = []
    points = 0
    for line in header.splitlines():
        parts = line.split()
        if not parts:
            continue
        if parts[0] == "FIELDS":
            fields = parts[1:]
        elif parts[0] == "SIZE":
            sizes = [int(v) for v in parts[1:]]
        elif parts[0] == "TYPE":
            types = parts[1:]
        elif parts[0] == "COUNT":
            counts = [int(v) for v in parts[1:]]
        elif parts[0] == "POINTS":
            points = int(parts[1])

    if not fields or points <= 0:
        raise RuntimeError(f"{path}: could not parse PCD header")
    if not counts:
        counts = [1] * len(fields)

    offsets: dict[str, int] = {}
    offset = 0
    dtype_fields = []
    for name, size, typ, count in zip(fields, sizes, types, counts, strict=True):
        if count != 1:
            raise RuntimeError(f"{path}: unsupported COUNT {count} for field {name}")
        if typ != "F" or size != 4:
            raise RuntimeError(f"{path}: only float32 fields are supported")
        offsets[name] = offset
        dtype_fields.append((name, "<f4"))
        offset += size
    for required in ("x", "y", "z", "intensity"):
        if required not in offsets:
            raise RuntimeError(f"{path}: missing {required} field")

    body = raw[header_end:]
    point_step = sum(s * c for s, c in zip(sizes, counts, strict=True))
    need = point_step * points
    if len(body) < need:
        raise RuntimeError(f"{path}: truncated binary body")
    structured = np.frombuffer(body[:need], dtype=np.dtype(dtype_fields), count=points)
    xyz = np.column_stack([structured["x"], structured["y"], structured["z"]]).astype(np.float64)
    intensity = structured["intensity"].astype(np.float64)
    return Scan(xyz=xyz, intensity=intensity)


def load_gt_xyyaw(path: Path) -> np.ndarray:
    with path.open(newline="") as f:
        rows = list(csv.DictReader(f))
    mats = []
    for row in rows:
        x = float(row["lidar_pose.x"])
        y = float(row["lidar_pose.y"])
        z = float(row["lidar_pose.z"])
        roll = float(row["lidar_pose.roll"])
        pitch = float(row["lidar_pose.pitch"])
        yaw = float(row["lidar_pose.yaw"])
        cr, sr = math.cos(roll), math.sin(roll)
        cp, sp = math.cos(pitch), math.sin(pitch)
        cy, sy = math.cos(yaw), math.sin(yaw)
        rx = np.array([[1.0, 0.0, 0.0], [0.0, cr, -sr], [0.0, sr, cr]])
        ry = np.array([[cp, 0.0, sp], [0.0, 1.0, 0.0], [-sp, 0.0, cp]])
        rz = np.array([[cy, -sy, 0.0], [sy, cy, 0.0], [0.0, 0.0, 1.0]])
        mat = np.eye(4)
        mat[:3, :3] = rz @ ry @ rx
        mat[:3, 3] = [x, y, z]
        mats.append(mat)
    if not mats:
        return np.zeros((0, 3), dtype=np.float64)
    anchor_inv = np.linalg.inv(mats[0])
    poses = []
    for mat in mats:
        rel = anchor_inv @ mat
        poses.append([rel[0, 3], rel[1, 3], math.atan2(rel[1, 0], rel[0, 0])])
    return np.asarray(poses, dtype=np.float64)


def se2_matrix(x: float, y: float, yaw: float) -> np.ndarray:
    c = math.cos(yaw)
    s = math.sin(yaw)
    return np.array([[c, -s, x], [s, c, y], [0.0, 0.0, 1.0]], dtype=np.float64)


def se2_inverse(t: np.ndarray) -> np.ndarray:
    r = t[:2, :2]
    trans = t[:2, 2]
    out = np.eye(3)
    out[:2, :2] = r.T
    out[:2, 2] = -r.T @ trans
    return out


def preprocess_scan(scan: Scan, args: argparse.Namespace) -> Scan:
    xyz = scan.xyz
    intensity = scan.intensity
    rng = np.linalg.norm(xyz, axis=1)
    mask = (
        np.isfinite(xyz).all(axis=1)
        & np.isfinite(intensity)
        & (rng >= args.min_range)
        & (rng <= args.max_range)
        & (xyz[:, 2] >= args.z_min)
        & (xyz[:, 2] <= args.z_max)
    )
    xyz = xyz[mask]
    intensity = intensity[mask]
    if xyz.shape[0] > args.max_points:
        step = xyz.shape[0] / args.max_points
        idx = np.minimum((np.arange(args.max_points) * step).astype(np.int64), xyz.shape[0] - 1)
        xyz = xyz[idx]
        intensity = intensity[idx]

    if intensity.size:
        lo, hi = np.percentile(intensity, [1.0, 99.0])
        denom = max(float(hi - lo), 1e-9)
        intensity = np.clip((intensity - lo) / denom, 0.0, 1.0)
    return Scan(xyz=xyz, intensity=intensity)


def rasterize_bev(scan: Scan, args: argparse.Namespace, grid_resolution: float) -> np.ndarray:
    n = int(math.ceil(2.0 * args.grid_radius / grid_resolution))
    img = np.zeros((n, n), dtype=np.float64)
    x = scan.xyz[:, 0]
    y = scan.xyz[:, 1]
    col = np.floor((x + args.grid_radius) / grid_resolution).astype(np.int64)
    row = np.floor((y + args.grid_radius) / grid_resolution).astype(np.int64)
    valid = (row >= 0) & (row < n) & (col >= 0) & (col < n)
    np.maximum.at(img, (row[valid], col[valid]), scan.intensity[valid])
    return img


def score_candidate(
    ref_img: np.ndarray,
    curr: Scan,
    cand: Candidate,
    args: argparse.Namespace,
    grid_resolution: float,
    min_overlap: int,
) -> Candidate:
    c = math.cos(cand.yaw)
    s = math.sin(cand.yaw)
    x = c * curr.xyz[:, 0] - s * curr.xyz[:, 1] + cand.dx
    y = s * curr.xyz[:, 0] + c * curr.xyz[:, 1] + cand.dy
    n = ref_img.shape[0]
    col = np.floor((x + args.grid_radius) / grid_resolution).astype(np.int64)
    row = np.floor((y + args.grid_radius) / grid_resolution).astype(np.int64)
    valid = (row >= 0) & (row < n) & (col >= 0) & (col < n)
    if not np.any(valid):
        return Candidate(cand.dx, cand.dy, cand.yaw, -1.0, 0)

    ref_vals = ref_img[row[valid], col[valid]]
    curr_vals = curr.intensity[valid]
    nonzero = ref_vals > 0.0
    if int(nonzero.sum()) < min_overlap:
        return Candidate(cand.dx, cand.dy, cand.yaw, -1.0, int(nonzero.sum()))

    a = curr_vals[nonzero]
    b = ref_vals[nonzero]
    a = a - float(a.mean())
    b = b - float(b.mean())
    denom = float(np.linalg.norm(a) * np.linalg.norm(b))
    if denom <= 1e-12:
        score = -1.0
    else:
        score = float(a.dot(b) / denom)
    return Candidate(cand.dx, cand.dy, cand.yaw, score, int(nonzero.sum()))


def values_around(center: float, window: float, step: float) -> np.ndarray:
    count = int(math.floor(window / step))
    vals = center + np.arange(-count, count + 1, dtype=np.float64) * step
    return vals


def estimate_relative(
    prev: Scan,
    curr: Scan,
    args: argparse.Namespace,
    prior: Candidate | None = None,
) -> Candidate:
    if prior is None:
        best = Candidate(0.0, 0.0, 0.0, -1.0, 0)
    else:
        best = Candidate(prior.dx, prior.dy, prior.yaw, -1.0, 0)
    anchor = Candidate(best.dx, best.dy, best.yaw, -1.0, 0)

    pyramid_scales = [2**i for i in reversed(range(max(1, args.pyramid_levels)))]
    for scale in pyramid_scales:
        grid_resolution = args.grid_resolution * scale
        ref_img = rasterize_bev(prev, args, grid_resolution)
        level_best = Candidate(best.dx, best.dy, best.yaw, -1.0, 0)
        scale_root = math.sqrt(float(scale))
        x_window = args.x_window * scale_root
        y_window = args.y_window * scale_root
        yaw_window = math.radians(args.yaw_window_deg) * scale_root
        x_step = args.x_step * scale_root
        y_step = args.y_step * scale_root
        yaw_step = math.radians(args.yaw_step_deg) * scale_root
        min_overlap = max(40, int(args.min_overlap / float(scale * scale)))

        levels = max(1, args.refine_levels)
        for _ in range(levels):
            anchor_scored = score_candidate(
                ref_img, curr, anchor, args, grid_resolution, min_overlap
            )
            if anchor_scored.score > level_best.score:
                level_best = anchor_scored
            for yaw in values_around(level_best.yaw, yaw_window, yaw_step):
                for dx in values_around(level_best.dx, x_window, x_step):
                    for dy in values_around(level_best.dy, y_window, y_step):
                        scored = score_candidate(
                            ref_img,
                            curr,
                            Candidate(dx, dy, yaw, -1.0, 0),
                            args,
                            grid_resolution,
                            min_overlap,
                        )
                        if scored.score > level_best.score:
                            level_best = scored
            x_window = max(x_step, x_window * 0.5)
            y_window = max(y_step, y_window * 0.5)
            yaw_window = max(yaw_step, yaw_window * 0.5)
            x_step *= 0.5
            y_step *= 0.5
            yaw_step *= 0.5
        if level_best.score >= 0.0:
            best = level_best
    return best


def gt_relative_trajectory(gt: np.ndarray, n: int) -> np.ndarray:
    mats = [se2_matrix(float(x), float(y), float(yaw)) for x, y, yaw in gt[:n]]
    anchor_inv = se2_inverse(mats[0])
    rel = [anchor_inv @ m for m in mats]
    return np.asarray([[m[0, 2], m[1, 2], math.atan2(m[1, 0], m[0, 0])] for m in rel])


def compute_metrics(poses: np.ndarray, gt: np.ndarray | None) -> dict[str, float] | None:
    if gt is None or gt.shape[0] < poses.shape[0]:
        return None
    gt_rel = gt_relative_trajectory(gt, poses.shape[0])
    err = poses[:, :2] - gt_rel[:, :2]
    ate = float(np.sqrt(np.mean(np.sum(err * err, axis=1))))

    if poses.shape[0] > 1:
        pred_step = np.linalg.norm(np.diff(poses[:, :2], axis=0), axis=1)
        gt_step = np.linalg.norm(np.diff(gt_rel[:, :2], axis=0), axis=1)
        rpe = float(np.sqrt(np.mean((pred_step - gt_step) ** 2)))
        path = float(np.sum(gt_step))
        rpe_pct = 100.0 * rpe / max(float(np.mean(gt_step)), 1e-9)
    else:
        rpe = 0.0
        path = 0.0
        rpe_pct = 0.0

    return {
        "ate_xy_m": ate,
        "step_length_rmse_m": rpe,
        "step_length_rmse_pct_of_mean_step": rpe_pct,
        "gt_path_length_m": path,
    }


def passes_motion_gate(cand: Candidate, args: argparse.Namespace) -> bool:
    if args.max_step_translation > 0.0:
        if math.hypot(cand.dx, cand.dy) > args.max_step_translation:
            return False
    if args.max_step_yaw_deg > 0.0:
        if abs(math.degrees(cand.yaw)) > args.max_step_yaw_deg:
            return False
    return True


def main() -> int:
    args = parse_args()
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    started_at = time.perf_counter()

    scans: list[Scan] = []
    pcd_paths: list[Path] = []
    for offset in range(args.max_frames):
        frame_index = args.start_frame + offset
        pcd_path = args.sequence_pcd_dir / f"{frame_index:08d}" / "cloud.pcd"
        if not pcd_path.is_file():
            break
        scans.append(preprocess_scan(read_binary_pcd_xyz_intensity(pcd_path), args))
        pcd_paths.append(pcd_path)
    if len(scans) < 2:
        raise RuntimeError("Need at least two scans")

    gt = load_gt_xyyaw(args.gt_csv) if args.gt_csv is not None else None

    pose = np.eye(3)
    poses = [[0.0, 0.0, 0.0]]
    pairs = []
    prior: Candidate | None = None
    for i in range(1, len(scans)):
        cand = estimate_relative(
            scans[i - 1],
            scans[i],
            args,
            prior if args.motion_prior else None,
        )
        accepted = cand.score >= 0.0 and passes_motion_gate(cand, args)
        if accepted:
            pose_cand = cand
            prior = cand
        elif prior is not None and passes_motion_gate(prior, args):
            pose_cand = prior
        else:
            pose_cand = Candidate(0.0, 0.0, 0.0, cand.score, cand.overlap)
        t_curr_to_prev = se2_matrix(pose_cand.dx, pose_cand.dy, pose_cand.yaw)
        pose = pose @ t_curr_to_prev
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
                f"[intensity-bev] {i}/{len(scans)-1} "
                f"score={cand.score:.3f} overlap={cand.overlap}"
            )

    pose_arr = np.asarray(poses, dtype=np.float64)
    metrics = compute_metrics(pose_arr, gt)
    payload = {
        "sequence_pcd_dir": str(args.sequence_pcd_dir),
        "gt_csv": str(args.gt_csv) if args.gt_csv is not None else None,
        "frames": len(scans),
        "method": "intensity_bev_direct_odometry",
        "parameters": {
            "grid_resolution": args.grid_resolution,
            "grid_radius": args.grid_radius,
            "preset": args.preset,
            "max_points": args.max_points,
            "x_window": args.x_window,
            "y_window": args.y_window,
            "yaw_window_deg": args.yaw_window_deg,
            "x_step": args.x_step,
            "y_step": args.y_step,
            "yaw_step_deg": args.yaw_step_deg,
            "refine_levels": args.refine_levels,
            "pyramid_levels": args.pyramid_levels,
            "min_overlap": args.min_overlap,
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
