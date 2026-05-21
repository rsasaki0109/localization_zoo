#!/usr/bin/env python3
"""Run KISS keyframe-anchor odometry over window benchmark directories."""

from __future__ import annotations

import argparse
import json
import statistics
import subprocess
from pathlib import Path


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--benchmark-dir",
        type=Path,
        default=Path("experiments/results/intensity_odometry/window_benchmark_60"),
    )
    p.add_argument("--binary", type=Path, default=Path("build/evaluation/kiss_keyframe_odometry"))
    p.add_argument("--output-dir", type=Path)
    p.add_argument("--max-frames", type=int, default=60)
    p.add_argument("--keyframe-interval", type=int, default=10)
    p.add_argument("--max-step-translation", type=float, default=2.0)
    p.add_argument("--max-step-yaw-deg", type=float, default=6.0)
    p.add_argument("--max-keyframe-correction", type=float, default=1.0)
    p.add_argument("--max-keyframe-yaw-deg", type=float, default=5.0)
    p.add_argument("--max-keyframe-rmse", type=float, default=1.2)
    p.add_argument("--min-keyframe-correspondences", type=int, default=6000)
    p.add_argument("--reuse", action="store_true")
    return p.parse_args()


def load_json(path: Path) -> dict:
    return json.loads(path.read_text())


def metric(payload: dict, key: str) -> float:
    value = (payload.get("metrics") or {}).get(key)
    return float(value) if value is not None else float("nan")


def aggregate(values: list[float]) -> dict[str, float]:
    return {
        "mean": statistics.mean(values),
        "median": statistics.median(values),
        "max": max(values),
    }


def main() -> int:
    args = parse_args()
    output_dir = args.output_dir or (
        args.benchmark_dir / f"kiss_keyframe_i{args.keyframe_interval}"
    )
    results_dir = output_dir / "results"
    results_dir.mkdir(parents=True, exist_ok=True)

    summary = load_json(args.benchmark_dir / "window_benchmark_summary.json")
    windows = [str(row["window"]) for row in summary["windows"]]
    baseline_by_window = {
        str(row["window"]): row
        for row in summary["windows"]
    }

    rows = []
    for window in windows:
        pcd_dir = args.benchmark_dir / "windows" / window / "pcd"
        gt_csv = args.benchmark_dir / "windows" / window / "gt.csv"
        out_json = results_dir / f"{window}_kiss_keyframe_i{args.keyframe_interval}.json"
        if not args.reuse or not out_json.is_file():
            cmd = [
                str(args.binary),
                str(pcd_dir),
                str(gt_csv),
                str(out_json),
                str(args.max_frames),
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
            ]
            subprocess.run(cmd, check=True)
        payload = load_json(out_json)
        base = baseline_by_window[window]
        pairs = payload.get("pairs", [])
        attempted = sum(1 for pair in pairs if pair.get("keyframe_attempted"))
        accepted = sum(1 for pair in pairs if pair.get("keyframe_accepted"))
        rows.append({
            "window": window,
            "path": str(out_json),
            "ate_xy_m": metric(payload, "ate_xy_m"),
            "step_length_rmse_m": metric(payload, "step_length_rmse_m"),
            "kiss_pair_ate_xy_m": float(base["kiss_ate_xy_m"]),
            "stability_motion_ate_xy_m": float(base["stability_motion_ate_xy_m"]),
            "delta_vs_kiss_pair_m": metric(payload, "ate_xy_m") - float(base["kiss_ate_xy_m"]),
            "delta_vs_stability_motion_m": metric(payload, "ate_xy_m") - float(base["stability_motion_ate_xy_m"]),
            "keyframe_attempted": attempted,
            "keyframe_accepted": accepted,
        })

    ate = [row["ate_xy_m"] for row in rows]
    step = [row["step_length_rmse_m"] for row in rows]
    kiss = [row["kiss_pair_ate_xy_m"] for row in rows]
    stability = [row["stability_motion_ate_xy_m"] for row in rows]
    out = {
        "benchmark_dir": str(args.benchmark_dir),
        "parameters": {
            "max_frames": args.max_frames,
            "keyframe_interval": args.keyframe_interval,
            "max_step_translation": args.max_step_translation,
            "max_step_yaw_deg": args.max_step_yaw_deg,
            "max_keyframe_correction": args.max_keyframe_correction,
            "max_keyframe_yaw_deg": args.max_keyframe_yaw_deg,
            "max_keyframe_rmse": args.max_keyframe_rmse,
            "min_keyframe_correspondences": args.min_keyframe_correspondences,
        },
        "aggregate": {
            "kiss_keyframe_ate_xy_m": aggregate(ate),
            "kiss_keyframe_step_length_rmse_m": aggregate(step),
            "kiss_pair_ate_xy_m": aggregate(kiss),
            "stability_motion_ate_xy_m": aggregate(stability),
            "improved_vs_kiss_pair_count": sum(row["delta_vs_kiss_pair_m"] < 0.0 for row in rows),
            "improved_vs_stability_motion_count": sum(
                row["delta_vs_stability_motion_m"] < 0.0 for row in rows
            ),
        },
        "windows": rows,
    }
    (output_dir / "kiss_keyframe_summary.json").write_text(
        json.dumps(out, indent=2) + "\n",
        encoding="utf-8",
    )

    md = [
        "# KISS Keyframe Window Benchmark",
        "",
        f"Benchmark: `{args.benchmark_dir}`",
        f"Keyframe interval: `{args.keyframe_interval}` frames",
        "",
        "## Aggregate",
        "",
        "| Method | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) |",
        "| --- | ---: | ---: | ---: | ---: |",
        (
            f"| KISS pair | {statistics.mean(kiss):.3f} | {statistics.median(kiss):.3f} | "
            f"{max(kiss):.3f} | - |"
        ),
        (
            f"| Stability motion 0.0025 | {statistics.mean(stability):.3f} | "
            f"{statistics.median(stability):.3f} | {max(stability):.3f} | - |"
        ),
        (
            f"| KISS keyframe i{args.keyframe_interval} | {statistics.mean(ate):.3f} | "
            f"{statistics.median(ate):.3f} | {max(ate):.3f} | {statistics.mean(step):.3f} |"
        ),
        "",
        "## Windows",
        "",
        "| Window | KISS pair ATE | Stability motion ATE | Keyframe ATE | Delta vs stability | Step RMSE | Keyframes |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for row in rows:
        md.append(
            f"| {row['window']} | {row['kiss_pair_ate_xy_m']:.3f} | "
            f"{row['stability_motion_ate_xy_m']:.3f} | {row['ate_xy_m']:.3f} | "
            f"{row['delta_vs_stability_motion_m']:+.3f} | {row['step_length_rmse_m']:.3f} | "
            f"{row['keyframe_accepted']}/{row['keyframe_attempted']} |"
        )
    md.append("")
    (output_dir / "kiss_keyframe_summary.md").write_text("\n".join(md), encoding="utf-8")
    print(
        f"[done] keyframe mean_ate={statistics.mean(ate):.3f}m "
        f"stability_mean={statistics.mean(stability):.3f}m -> {output_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
