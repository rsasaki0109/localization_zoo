#!/usr/bin/env python3

"""Run GT-free odometry health checks on selected LiDAR degradation windows."""

from __future__ import annotations

import argparse
import json
import math
import statistics
import subprocess
import sys
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
GEOMETRY_ICP = REPO_ROOT / "evaluation" / "scripts" / "geometry_icp_odometry_demo.py"
INTENSITY_BEV = REPO_ROOT / "evaluation" / "scripts" / "intensity_bev_odometry_demo.py"
KISS_KEYFRAME = REPO_ROOT / "build" / "evaluation" / "kiss_keyframe_odometry"
CT_ICP_WINDOW = REPO_ROOT / "build" / "evaluation" / "ct_icp_window_odometry"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("degradation_windows_json", type=Path)
    parser.add_argument("output_dir", type=Path)
    parser.add_argument(
        "--sequence-pcd-dir",
        type=Path,
        help="Override PCD directory. Defaults to source_pcd_dir in degradation_windows_json.",
    )
    parser.add_argument(
        "--method",
        choices=["geometry_icp", "intensity_bev", "kiss_keyframe", "ct_icp"],
        default="geometry_icp",
    )
    parser.add_argument("--kiss-binary", type=Path, default=KISS_KEYFRAME)
    parser.add_argument("--ct-icp-binary", type=Path, default=CT_ICP_WINDOW)
    parser.add_argument("--max-points", type=int, default=5000)
    parser.add_argument("--voxel-size", type=float, default=0.75)
    parser.add_argument("--min-range", type=float, default=0.2)
    parser.add_argument("--max-range", type=float, default=30.0)
    parser.add_argument("--z-min", type=float, default=-5.0)
    parser.add_argument("--z-max", type=float, default=5.0)
    parser.add_argument("--max-step-translation", type=float, default=2.0)
    parser.add_argument("--max-step-yaw-deg", type=float, default=20.0)
    parser.add_argument(
        "--min-used-path-length",
        type=float,
        default=0.0,
        help="Flag windows whose accepted trajectory path is shorter than this. <=0 disables.",
    )
    parser.add_argument("--keyframe-interval", type=int, default=10)
    parser.add_argument("--max-keyframe-correction", type=float, default=1.0)
    parser.add_argument("--max-keyframe-yaw-deg", type=float, default=5.0)
    parser.add_argument("--max-keyframe-rmse", type=float, default=2.0)
    parser.add_argument("--min-keyframe-correspondences", type=int, default=1000)
    parser.add_argument("--kiss-min-correspondences", type=int, default=1000)
    parser.add_argument("--intensity-bev-preset", choices=["default", "fast", "pyramid"], default="fast")
    parser.add_argument("--intensity-bev-grid-resolution", type=float, default=0.5)
    parser.add_argument("--intensity-bev-grid-radius", type=float, default=45.0)
    parser.add_argument("--intensity-bev-min-overlap", type=int, default=200)
    parser.add_argument("--intensity-bev-max-points", type=int, default=6000)
    parser.add_argument("--intensity-bev-min-step-translation", type=float, default=0.0)
    parser.add_argument("--intensity-bev-zero-motion-score-margin", type=float, default=0.0)
    parser.add_argument(
        "--intensity-bev-motion-margin-flag-rate",
        type=float,
        default=0.5,
        help=(
            "Flag non-baseline intensity BEV windows when this fraction of pair "
            "decisions used the motion-margin fallback."
        ),
    )
    parser.add_argument(
        "--intensity-bev-overlap-tail-flag-ratio",
        type=float,
        default=0.5,
        help=(
            "Flag non-baseline intensity BEV windows when min overlap divided "
            "by mean overlap falls below this ratio."
        ),
    )
    parser.add_argument("--ct-icp-source-voxel-size", type=float, default=0.75)
    parser.add_argument("--ct-icp-max-source-points", type=int, default=500)
    parser.add_argument("--ct-icp-voxel-resolution", type=float, default=1.5)
    parser.add_argument("--ct-icp-keypoint-voxel-size", type=float, default=1.25)
    parser.add_argument("--ct-icp-max-iterations", type=int, default=8)
    parser.add_argument("--ct-icp-ceres-max-iterations", type=int, default=1)
    parser.add_argument("--ct-icp-max-frames-in-map", type=int, default=8)
    parser.add_argument("--ct-icp-planarity-threshold", type=float, default=0.08)
    parser.add_argument("--ct-icp-max-correspondence-distance", type=float)
    parser.add_argument("--ct-icp-multi-scale", action="store_true")
    parser.add_argument("--ct-icp-normal-cholesky", action="store_true")
    parser.add_argument("--ct-icp-refinement-gate", action="store_true")
    parser.add_argument("--ct-icp-native-seed", action="store_true")
    parser.add_argument("--ct-icp-require-convergence", action="store_true")
    parser.add_argument("--ct-icp-max-seed-translation-delta", type=float, default=2.0)
    parser.add_argument("--ct-icp-max-seed-rotation-delta-rad", type=float, default=0.25)
    parser.add_argument("--progress-every", type=int, default=0)
    return parser.parse_args()


def load_payload(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def unique_selected_windows(selected: dict[str, dict[str, Any]]) -> list[tuple[str, dict[str, Any]]]:
    seen: set[tuple[int, int]] = set()
    out: list[tuple[str, dict[str, Any]]] = []
    for name, row in selected.items():
        key = (int(row["start"]), int(row["end"]))
        if key in seen:
            continue
        seen.add(key)
        out.append((name, row))
    return out


def run_geometry_icp(args: argparse.Namespace, sequence_pcd_dir: Path, name: str, row: dict[str, Any], output_dir: Path) -> Path:
    result_path = output_dir / "results" / f"{name}_{int(row['start']):04d}_{int(row['end']):04d}_geometry_icp.json"
    result_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        sys.executable,
        str(GEOMETRY_ICP),
        "--sequence-pcd-dir",
        str(sequence_pcd_dir),
        "--output-json",
        str(result_path),
        "--start-frame",
        str(int(row["start"])),
        "--max-frames",
        str(int(row["frames"])),
        "--min-range",
        str(args.min_range),
        "--max-range",
        str(args.max_range),
        "--z-min",
        str(args.z_min),
        "--z-max",
        str(args.z_max),
        "--max-points",
        str(args.max_points),
        "--voxel-size",
        str(args.voxel_size),
        "--max-step-translation",
        str(args.max_step_translation),
        "--max-step-yaw-deg",
        str(args.max_step_yaw_deg),
        "--progress-every",
        str(args.progress_every),
    ]
    print("[run] " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    return result_path


def run_kiss_keyframe(
    args: argparse.Namespace,
    sequence_pcd_dir: Path,
    name: str,
    row: dict[str, Any],
    output_dir: Path,
) -> Path:
    result_path = (
        output_dir
        / "results"
        / f"{name}_{int(row['start']):04d}_{int(row['end']):04d}_kiss_keyframe.json"
    )
    result_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(args.kiss_binary),
        str(sequence_pcd_dir),
        "-",
        str(result_path),
        str(int(row["frames"])),
        "--start-frame",
        str(int(row["start"])),
        "--keyframe-interval",
        str(args.keyframe_interval),
        "--max-step-translation",
        str(args.max_step_translation),
        "--max-step-yaw-deg",
        str(args.max_step_yaw_deg),
        "--max-keyframe-correction",
        str(args.max_keyframe_correction),
        "--max-keyframe-yaw-deg",
        str(args.max_keyframe_yaw_deg),
        "--max-keyframe-rmse",
        str(args.max_keyframe_rmse),
        "--min-keyframe-correspondences",
        str(args.min_keyframe_correspondences),
        "--min-correspondences",
        str(args.kiss_min_correspondences),
    ]
    print("[run] " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    return result_path


def run_intensity_bev(
    args: argparse.Namespace,
    sequence_pcd_dir: Path,
    name: str,
    row: dict[str, Any],
    output_dir: Path,
) -> Path:
    result_path = (
        output_dir
        / "results"
        / f"{name}_{int(row['start']):04d}_{int(row['end']):04d}_intensity_bev.json"
    )
    result_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        sys.executable,
        str(INTENSITY_BEV),
        "--sequence-pcd-dir",
        str(sequence_pcd_dir),
        "--output-json",
        str(result_path),
        "--start-frame",
        str(int(row["start"])),
        "--max-frames",
        str(int(row["frames"])),
        "--preset",
        args.intensity_bev_preset,
        "--min-range",
        str(args.min_range),
        "--max-range",
        str(args.max_range),
        "--z-min",
        str(args.z_min),
        "--z-max",
        str(args.z_max),
        "--max-points",
        str(args.intensity_bev_max_points),
        "--grid-resolution",
        str(args.intensity_bev_grid_resolution),
        "--grid-radius",
        str(args.intensity_bev_grid_radius),
        "--min-overlap",
        str(args.intensity_bev_min_overlap),
        "--max-step-translation",
        str(args.max_step_translation),
        "--max-step-yaw-deg",
        str(args.max_step_yaw_deg),
        "--min-step-translation",
        str(args.intensity_bev_min_step_translation),
        "--zero-motion-score-margin",
        str(args.intensity_bev_zero_motion_score_margin),
        "--progress-every",
        str(args.progress_every),
    ]
    print("[run] " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    return result_path


def run_ct_icp(
    args: argparse.Namespace,
    sequence_pcd_dir: Path,
    name: str,
    row: dict[str, Any],
    output_dir: Path,
) -> Path:
    result_path = (
        output_dir
        / "results"
        / f"{name}_{int(row['start']):04d}_{int(row['end']):04d}_ct_icp.json"
    )
    result_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(args.ct_icp_binary),
        str(sequence_pcd_dir),
        "-",
        str(result_path),
        str(int(row["frames"])),
        "--start-frame",
        str(int(row["start"])),
        "--source-voxel-size",
        str(args.ct_icp_source_voxel_size),
        "--max-source-points",
        str(args.ct_icp_max_source_points),
        "--voxel-resolution",
        str(args.ct_icp_voxel_resolution),
        "--keypoint-voxel-size",
        str(args.ct_icp_keypoint_voxel_size),
        "--max-iterations",
        str(args.ct_icp_max_iterations),
        "--ceres-max-iterations",
        str(args.ct_icp_ceres_max_iterations),
        "--max-frames-in-map",
        str(args.ct_icp_max_frames_in_map),
        "--planarity-threshold",
        str(args.ct_icp_planarity_threshold),
        "--max-step-translation",
        str(args.max_step_translation),
        "--max-step-yaw-deg",
        str(args.max_step_yaw_deg),
        "--max-seed-translation-delta",
        str(args.ct_icp_max_seed_translation_delta),
        "--max-seed-rotation-delta-rad",
        str(args.ct_icp_max_seed_rotation_delta_rad),
    ]
    if args.ct_icp_max_correspondence_distance is not None:
        cmd.extend(
            [
                "--max-correspondence-distance",
                str(args.ct_icp_max_correspondence_distance),
            ]
        )
    if args.ct_icp_multi_scale:
        cmd.append("--multi-scale")
    if args.ct_icp_normal_cholesky:
        cmd.append("--normal-cholesky")
    if args.ct_icp_refinement_gate:
        cmd.append("--refinement-gate")
    if args.ct_icp_native_seed:
        cmd.append("--native-ct-icp-seed")
    if args.ct_icp_require_convergence:
        cmd.append("--require-convergence")
    print("[run] " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    return result_path


def finite(values: list[float]) -> list[float]:
    return [value for value in values if math.isfinite(value)]


def safe_mean(values: list[float]) -> float | None:
    values = finite(values)
    return statistics.fmean(values) if values else None


def safe_min(values: list[float]) -> float | None:
    values = finite(values)
    return min(values) if values else None


def safe_max(values: list[float]) -> float | None:
    values = finite(values)
    return max(values) if values else None


def optional_float(value: Any) -> float | None:
    if value is None:
        return None
    try:
        out = float(value)
    except (TypeError, ValueError):
        return None
    return out if math.isfinite(out) else None


def reason_counts(pairs: list[dict[str, Any]]) -> dict[str, int]:
    counts: dict[str, int] = {}
    for pair in pairs:
        if "decision_reason" not in pair:
            continue
        reason = str(pair.get("decision_reason") or "n/a")
        counts[reason] = counts.get(reason, 0) + 1
    return counts


def ratio_or_none(numerator: float | None, denominator: float | None) -> float | None:
    if numerator is None or denominator is None or denominator <= 0.0:
        return None
    return numerator / denominator


def is_stress_window(name: str, window: dict[str, Any]) -> bool:
    if name != "baseline":
        return True
    obscurant_score = optional_float(window.get("obscurant_score"))
    return obscurant_score is not None and obscurant_score >= 0.2


def summarize_result(
    name: str,
    window: dict[str, Any],
    result_path: Path,
    method: str,
    min_used_path_length: float = 0.0,
    intensity_bev_motion_margin_flag_rate: float = 0.5,
    intensity_bev_overlap_tail_flag_ratio: float = 0.5,
) -> dict[str, Any]:
    payload = load_payload(result_path)
    pairs = payload.get("pairs", [])
    poses = payload.get("poses_xyyaw", [])
    pair_count = len(pairs)
    accepted = [bool(pair.get("accepted")) for pair in pairs]
    converged = [
        bool(pair["converged"]) if "converged" in pair else bool(pair.get("accepted"))
        for pair in pairs
    ]
    scores = [optional_float(pair.get("score")) for pair in pairs]
    valid_scores = [score for score in scores if score is not None and score >= 0.0]
    overlaps = [
        overlap
        for overlap in (optional_float(pair.get("overlap")) for pair in pairs)
        if overlap is not None
    ]
    decision_reasons = reason_counts(pairs)
    decision_count = sum(decision_reasons.values())
    motion_margin_rate = (
        decision_reasons.get("motion_margin", 0) / decision_count
        if decision_count
        else None
    )
    best_score_rate = (
        decision_reasons.get("best_score", 0) / decision_count
        if decision_count
        else None
    )
    used_steps = [
        math.hypot(float(pair.get("used_dx_curr_to_prev_m", 0.0)), float(pair.get("used_dy_curr_to_prev_m", 0.0)))
        for pair in pairs
    ]
    raw_steps = [
        math.hypot(float(pair.get("dx_curr_to_prev_m", 0.0)), float(pair.get("dy_curr_to_prev_m", 0.0)))
        for pair in pairs
        if optional_float(pair.get("score")) is None or optional_float(pair.get("score")) >= 0.0
    ]
    yaws = [abs(float(pair.get("used_yaw_curr_to_prev_deg", 0.0))) for pair in pairs]
    nonfinite_pose_count = sum(
        not all(math.isfinite(float(pose[key])) for key in ("x_m", "y_m", "yaw_deg"))
        for pose in poses
    )
    used_path_length = float(sum(used_steps))
    accepted_rate = float(sum(accepted) / pair_count) if pair_count else 0.0
    converged_rate = float(sum(converged) / pair_count) if pair_count else 0.0
    overlap_mean = safe_mean(overlaps)
    overlap_min = safe_min(overlaps)
    overlap_min_to_mean = ratio_or_none(overlap_min, overlap_mean)
    apply_intensity_confidence_flags = method == "intensity_bev" and is_stress_window(name, window)
    motion_margin_dominant = (
        apply_intensity_confidence_flags
        and motion_margin_rate is not None
        and motion_margin_rate >= intensity_bev_motion_margin_flag_rate
    )
    overlap_tail = (
        apply_intensity_confidence_flags
        and overlap_min_to_mean is not None
        and overlap_min_to_mean < intensity_bev_overlap_tail_flag_ratio
    )
    return {
        "name": name,
        "window": window,
        "result_json": str(result_path),
        "frames": int(payload["frames"]),
        "pairs": pair_count,
        "runtime_s": float(payload["runtime_s"]) if payload.get("runtime_s") is not None else None,
        "accepted_pairs": int(sum(accepted)),
        "accepted_rate": accepted_rate,
        "converged_pairs": int(sum(converged)),
        "converged_rate": converged_rate,
        "failed_pairs": int(pair_count - sum(accepted)),
        "score_mean": safe_mean(valid_scores),
        "score_min": safe_min(valid_scores),
        "overlap_mean": overlap_mean,
        "overlap_min": overlap_min,
        "overlap_min_to_mean": overlap_min_to_mean,
        "decision_reasons": decision_reasons,
        "motion_margin_rate": motion_margin_rate,
        "best_score_rate": best_score_rate,
        "raw_step_mean_m": safe_mean(raw_steps),
        "used_path_length_m": used_path_length,
        "used_step_max_m": safe_max(used_steps),
        "used_abs_yaw_max_deg": safe_max(yaws),
        "nonfinite_pose_count": int(nonfinite_pose_count),
        "keyframe_attempted": int(
            sum(1 for pair in pairs if pair.get("keyframe_attempted"))
        ),
        "keyframe_accepted": int(
            sum(1 for pair in pairs if pair.get("keyframe_accepted"))
        ),
        "health_flags": {
            "all_pairs_failed": sum(accepted) == 0 and pair_count > 0,
            "low_acceptance": accepted_rate < 0.5 if pair_count else True,
            "low_convergence": converged_rate < 0.5 if pair_count else True,
            "low_used_path": (
                min_used_path_length > 0.0
                and accepted_rate >= 0.5
                and used_path_length < min_used_path_length
            ),
            "motion_margin_dominant": motion_margin_dominant,
            "overlap_tail": overlap_tail,
            "nonfinite_pose": nonfinite_pose_count > 0,
        },
    }


def write_markdown(path: Path, rows: list[dict[str, Any]]) -> None:
    lines = [
        "# LiDAR Degradation Odometry Health",
        "",
        "| Window | Frames | Obscurant | Accepted | Converged | Score mean | Overlap mean | Used path m | Keyframes | Flags |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|---|",
    ]
    for row in rows:
        flags = [name for name, value in row["health_flags"].items() if value]
        score_mean = row["score_mean"]
        overlap_mean = row["overlap_mean"]
        score_text = f"{score_mean:.3f}" if score_mean is not None else "n/a"
        overlap_text = f"{overlap_mean:.1f}" if overlap_mean is not None else "n/a"
        lines.append(
            f"| `{row['name']}` {row['window']['start']}-{row['window']['end']} | "
            f"{row['frames']} | {float(row['window']['obscurant_score']):.3f} | "
            f"{row['accepted_rate']:.3f} | {row['converged_rate']:.3f} | "
            f"{score_text} | {overlap_text} | "
            f"{row['used_path_length_m']:.3f} | "
            f"{row['keyframe_accepted']}/{row['keyframe_attempted']} | "
            f"{', '.join(flags) if flags else 'ok'} |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    windows_payload = load_payload(args.degradation_windows_json)
    sequence_pcd_dir = args.sequence_pcd_dir or Path(windows_payload["source_pcd_dir"])
    args.output_dir.mkdir(parents=True, exist_ok=True)

    rows: list[dict[str, Any]] = []
    for name, row in unique_selected_windows(windows_payload["selected"]):
        if args.method == "geometry_icp":
            result_path = run_geometry_icp(args, sequence_pcd_dir, name, row, args.output_dir)
        elif args.method == "intensity_bev":
            result_path = run_intensity_bev(args, sequence_pcd_dir, name, row, args.output_dir)
        elif args.method == "kiss_keyframe":
            result_path = run_kiss_keyframe(args, sequence_pcd_dir, name, row, args.output_dir)
        elif args.method == "ct_icp":
            result_path = run_ct_icp(args, sequence_pcd_dir, name, row, args.output_dir)
        else:
            raise ValueError(f"Unsupported method: {args.method}")
        rows.append(
            summarize_result(
                name,
                row,
                result_path,
                method=args.method,
                min_used_path_length=args.min_used_path_length,
                intensity_bev_motion_margin_flag_rate=args.intensity_bev_motion_margin_flag_rate,
                intensity_bev_overlap_tail_flag_ratio=args.intensity_bev_overlap_tail_flag_ratio,
            )
        )

    summary = {
        "degradation_windows_json": str(args.degradation_windows_json),
        "sequence_pcd_dir": str(sequence_pcd_dir),
        "method": args.method,
        "parameters": {
            "max_points": args.max_points,
            "voxel_size": args.voxel_size,
            "min_range": args.min_range,
            "max_range": args.max_range,
            "z_min": args.z_min,
            "z_max": args.z_max,
            "max_step_translation": args.max_step_translation,
            "max_step_yaw_deg": args.max_step_yaw_deg,
            "min_used_path_length": args.min_used_path_length,
            "keyframe_interval": args.keyframe_interval,
            "max_keyframe_correction": args.max_keyframe_correction,
            "max_keyframe_yaw_deg": args.max_keyframe_yaw_deg,
            "max_keyframe_rmse": args.max_keyframe_rmse,
            "min_keyframe_correspondences": args.min_keyframe_correspondences,
            "kiss_min_correspondences": args.kiss_min_correspondences,
            "intensity_bev_preset": args.intensity_bev_preset,
            "intensity_bev_grid_resolution": args.intensity_bev_grid_resolution,
            "intensity_bev_grid_radius": args.intensity_bev_grid_radius,
            "intensity_bev_min_overlap": args.intensity_bev_min_overlap,
            "intensity_bev_max_points": args.intensity_bev_max_points,
            "intensity_bev_min_step_translation": args.intensity_bev_min_step_translation,
            "intensity_bev_zero_motion_score_margin": args.intensity_bev_zero_motion_score_margin,
            "intensity_bev_motion_margin_flag_rate": args.intensity_bev_motion_margin_flag_rate,
            "intensity_bev_overlap_tail_flag_ratio": args.intensity_bev_overlap_tail_flag_ratio,
            "ct_icp_source_voxel_size": args.ct_icp_source_voxel_size,
            "ct_icp_max_source_points": args.ct_icp_max_source_points,
            "ct_icp_voxel_resolution": args.ct_icp_voxel_resolution,
            "ct_icp_keypoint_voxel_size": args.ct_icp_keypoint_voxel_size,
            "ct_icp_max_iterations": args.ct_icp_max_iterations,
            "ct_icp_ceres_max_iterations": args.ct_icp_ceres_max_iterations,
            "ct_icp_max_frames_in_map": args.ct_icp_max_frames_in_map,
            "ct_icp_planarity_threshold": args.ct_icp_planarity_threshold,
            "ct_icp_max_correspondence_distance": args.ct_icp_max_correspondence_distance,
            "ct_icp_multi_scale": args.ct_icp_multi_scale,
            "ct_icp_normal_cholesky": args.ct_icp_normal_cholesky,
            "ct_icp_refinement_gate": args.ct_icp_refinement_gate,
            "ct_icp_native_seed": args.ct_icp_native_seed,
            "ct_icp_require_convergence": args.ct_icp_require_convergence,
            "ct_icp_max_seed_translation_delta": args.ct_icp_max_seed_translation_delta,
            "ct_icp_max_seed_rotation_delta_rad": args.ct_icp_max_seed_rotation_delta_rad,
        },
        "windows": rows,
    }
    (args.output_dir / "odometry_health_summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    write_markdown(args.output_dir / "odometry_health_summary.md", rows)
    print(f"[done] windows={len(rows)} output={args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
