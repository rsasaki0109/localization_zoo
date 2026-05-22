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


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("degradation_windows_json", type=Path)
    parser.add_argument("output_dir", type=Path)
    parser.add_argument(
        "--sequence-pcd-dir",
        type=Path,
        help="Override PCD directory. Defaults to source_pcd_dir in degradation_windows_json.",
    )
    parser.add_argument("--method", choices=["geometry_icp"], default="geometry_icp")
    parser.add_argument("--max-points", type=int, default=5000)
    parser.add_argument("--voxel-size", type=float, default=0.75)
    parser.add_argument("--min-range", type=float, default=0.2)
    parser.add_argument("--max-range", type=float, default=30.0)
    parser.add_argument("--z-min", type=float, default=-5.0)
    parser.add_argument("--z-max", type=float, default=5.0)
    parser.add_argument("--max-step-translation", type=float, default=2.0)
    parser.add_argument("--max-step-yaw-deg", type=float, default=20.0)
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


def summarize_result(name: str, window: dict[str, Any], result_path: Path) -> dict[str, Any]:
    payload = load_payload(result_path)
    pairs = payload.get("pairs", [])
    poses = payload.get("poses_xyyaw", [])
    pair_count = len(pairs)
    accepted = [bool(pair.get("accepted")) for pair in pairs]
    scores = [float(pair.get("score", float("nan"))) for pair in pairs]
    valid_scores = [score for score in scores if score >= 0.0 and math.isfinite(score)]
    overlaps = [float(pair.get("overlap", 0.0)) for pair in pairs]
    used_steps = [
        math.hypot(float(pair.get("used_dx_curr_to_prev_m", 0.0)), float(pair.get("used_dy_curr_to_prev_m", 0.0)))
        for pair in pairs
    ]
    raw_steps = [
        math.hypot(float(pair.get("dx_curr_to_prev_m", 0.0)), float(pair.get("dy_curr_to_prev_m", 0.0)))
        for pair in pairs
        if float(pair.get("score", -1.0)) >= 0.0
    ]
    yaws = [abs(float(pair.get("used_yaw_curr_to_prev_deg", 0.0))) for pair in pairs]
    nonfinite_pose_count = sum(
        not all(math.isfinite(float(pose[key])) for key in ("x_m", "y_m", "yaw_deg"))
        for pose in poses
    )
    return {
        "name": name,
        "window": window,
        "result_json": str(result_path),
        "frames": int(payload["frames"]),
        "pairs": pair_count,
        "runtime_s": float(payload["runtime_s"]),
        "accepted_pairs": int(sum(accepted)),
        "accepted_rate": float(sum(accepted) / pair_count) if pair_count else 0.0,
        "failed_pairs": int(pair_count - sum(accepted)),
        "score_mean": safe_mean(valid_scores),
        "score_min": safe_min(valid_scores),
        "overlap_mean": safe_mean(overlaps),
        "overlap_min": safe_min(overlaps),
        "raw_step_mean_m": safe_mean(raw_steps),
        "used_path_length_m": float(sum(used_steps)),
        "used_step_max_m": safe_max(used_steps),
        "used_abs_yaw_max_deg": safe_max(yaws),
        "nonfinite_pose_count": int(nonfinite_pose_count),
        "health_flags": {
            "all_pairs_failed": sum(accepted) == 0 and pair_count > 0,
            "low_acceptance": (sum(accepted) / pair_count) < 0.5 if pair_count else True,
            "nonfinite_pose": nonfinite_pose_count > 0,
        },
    }


def write_markdown(path: Path, rows: list[dict[str, Any]]) -> None:
    lines = [
        "# LiDAR Degradation Odometry Health",
        "",
        "| Window | Frames | Obscurant | Accepted | Score mean | Overlap mean | Used path m | Flags |",
        "|---|---:|---:|---:|---:|---:|---:|---|",
    ]
    for row in rows:
        flags = [name for name, value in row["health_flags"].items() if value]
        score_mean = row["score_mean"]
        overlap_mean = row["overlap_mean"]
        lines.append(
            f"| `{row['name']}` {row['window']['start']}-{row['window']['end']} | "
            f"{row['frames']} | {float(row['window']['obscurant_score']):.3f} | "
            f"{row['accepted_rate']:.3f} | "
            f"{score_mean:.3f} | " if score_mean is not None else
            f"| `{row['name']}` {row['window']['start']}-{row['window']['end']} | "
            f"{row['frames']} | {float(row['window']['obscurant_score']):.3f} | "
            f"{row['accepted_rate']:.3f} | n/a | "
        )
        lines[-1] += (
            f"{overlap_mean:.1f} | " if overlap_mean is not None else "n/a | "
        )
        lines[-1] += f"{row['used_path_length_m']:.3f} | {', '.join(flags) if flags else 'ok'} |"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    windows_payload = load_payload(args.degradation_windows_json)
    sequence_pcd_dir = args.sequence_pcd_dir or Path(windows_payload["source_pcd_dir"])
    args.output_dir.mkdir(parents=True, exist_ok=True)

    rows: list[dict[str, Any]] = []
    for name, row in unique_selected_windows(windows_payload["selected"]):
        result_path = run_geometry_icp(args, sequence_pcd_dir, name, row, args.output_dir)
        rows.append(summarize_result(name, row, result_path))

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
        },
        "windows": rows,
    }
    (args.output_dir / "odometry_health_summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    write_markdown(args.output_dir / "odometry_health_summary.md", rows)
    print(f"[done] windows={len(rows)} output={args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
