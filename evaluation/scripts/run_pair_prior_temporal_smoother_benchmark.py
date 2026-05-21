#!/usr/bin/env python3
"""Run temporal smoothing over pair-prior window benchmark outputs."""

from __future__ import annotations

import argparse
import json
import statistics
from pathlib import Path

from smooth_pair_prior_trajectory import SmootherParams, smooth_payload


def parse_float_list(raw: str) -> list[float]:
    return [float(item) for item in raw.split(",") if item.strip()]


def tag_float(value: float) -> str:
    return f"{value:.4f}".rstrip("0").rstrip(".").replace(".", "p")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--benchmark-dir",
        type=Path,
        default=Path("experiments/results/intensity_odometry/window_benchmark_60"),
    )
    p.add_argument("--output-dir", type=Path)
    p.add_argument(
        "--input-template",
        default="selected_prior_stability_motion_0p0025_{window}.json",
    )
    p.add_argument("--lambda-translation-grid", default="0.25,0.5,1.0,2.0,4.0")
    p.add_argument("--lambda-yaw-grid", default="0.0,0.25,0.5,1.0")
    p.add_argument("--robust-iterations", type=int, default=3)
    p.add_argument("--huber-translation-m", type=float, default=0.08)
    p.add_argument("--huber-yaw-deg", type=float, default=1.0)
    p.add_argument("--max-correction-translation-m", type=float, default=0.12)
    p.add_argument("--max-correction-yaw-deg", type=float, default=1.5)
    return p.parse_args()


def metric(payload: dict, key: str) -> float:
    metrics = payload.get("metrics") or {}
    value = metrics.get(key)
    return float(value) if value is not None else float("nan")


def aggregate(rows: list[dict]) -> dict:
    ates = [float(row["ate_xy_m"]) for row in rows]
    steps = [float(row["step_length_rmse_m"]) for row in rows]
    return {
        "mean_ate_xy_m": statistics.mean(ates),
        "median_ate_xy_m": statistics.median(ates),
        "max_ate_xy_m": max(ates),
        "mean_step_length_rmse_m": statistics.mean(steps),
        "best_window_count": 0,
    }


def main() -> int:
    args = parse_args()
    output_dir = args.output_dir or (args.benchmark_dir / "temporal_smoother")
    results_dir = args.benchmark_dir / "results"
    output_results = output_dir / "results"
    output_results.mkdir(parents=True, exist_ok=True)

    summary_path = args.benchmark_dir / "window_benchmark_summary.json"
    summary = json.loads(summary_path.read_text())
    windows = [str(row["window"]) for row in summary["windows"]]
    baseline_by_window = {
        str(row["window"]): float(row["stability_motion_ate_xy_m"])
        for row in summary["windows"]
    }

    lambda_translation_grid = parse_float_list(args.lambda_translation_grid)
    lambda_yaw_grid = parse_float_list(args.lambda_yaw_grid)
    config_rows = []
    by_config: dict[str, list[dict]] = {}

    for lambda_translation in lambda_translation_grid:
        for lambda_yaw in lambda_yaw_grid:
            config_key = (
                f"lamT{tag_float(lambda_translation)}_"
                f"lamY{tag_float(lambda_yaw)}"
            )
            params = SmootherParams(
                lambda_translation=lambda_translation,
                lambda_yaw=lambda_yaw,
                robust_iterations=args.robust_iterations,
                huber_translation_m=args.huber_translation_m,
                huber_yaw_deg=args.huber_yaw_deg,
                max_correction_translation_m=args.max_correction_translation_m,
                max_correction_yaw_deg=args.max_correction_yaw_deg,
            )
            rows = []
            for window in windows:
                input_path = results_dir / args.input_template.format(window=window)
                payload = json.loads(input_path.read_text())
                out = smooth_payload(payload, params, input_json=input_path)
                output_path = output_results / f"temporal_smooth_{config_key}_{window}.json"
                output_path.write_text(json.dumps(out, indent=2) + "\n", encoding="utf-8")
                rows.append({
                    "window": window,
                    "path": str(output_path),
                    "ate_xy_m": metric(out, "ate_xy_m"),
                    "step_length_rmse_m": metric(out, "step_length_rmse_m"),
                    "baseline_stability_motion_ate_xy_m": baseline_by_window[window],
                    "ate_delta_vs_baseline_m": metric(out, "ate_xy_m") - baseline_by_window[window],
                })
            agg = aggregate(rows)
            agg["config"] = config_key
            agg["lambda_translation"] = lambda_translation
            agg["lambda_yaw"] = lambda_yaw
            config_rows.append(agg)
            by_config[config_key] = rows

    for window in windows:
        best = min(config_rows, key=lambda row: next(
            item["ate_xy_m"] for item in by_config[row["config"]] if item["window"] == window
        ))
        best["best_window_count"] += 1

    baseline_ates = list(baseline_by_window.values())
    baseline = {
        "mean_ate_xy_m": statistics.mean(baseline_ates),
        "median_ate_xy_m": statistics.median(baseline_ates),
        "max_ate_xy_m": max(baseline_ates),
    }
    best_config = min(config_rows, key=lambda row: (row["mean_ate_xy_m"], row["max_ate_xy_m"]))

    payload = {
        "benchmark_dir": str(args.benchmark_dir),
        "input_template": args.input_template,
        "baseline": baseline,
        "best_config": best_config,
        "configs": sorted(config_rows, key=lambda row: row["mean_ate_xy_m"]),
        "windows": by_config[best_config["config"]],
    }
    (output_dir / "temporal_smoother_summary.json").write_text(
        json.dumps(payload, indent=2) + "\n",
        encoding="utf-8",
    )

    md = [
        "# Temporal Smoother Benchmark",
        "",
        f"Benchmark: `{args.benchmark_dir}`",
        f"Input: `{args.input_template}`",
        "",
        "## Aggregate",
        "",
        "| Config | lambda_t | lambda_yaw | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Best windows |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for row in sorted(config_rows, key=lambda row: row["mean_ate_xy_m"]):
        md.append(
            f"| {row['config']} | {row['lambda_translation']:.3f} | {row['lambda_yaw']:.3f} | "
            f"{row['mean_ate_xy_m']:.3f} | {row['median_ate_xy_m']:.3f} | "
            f"{row['max_ate_xy_m']:.3f} | {row['mean_step_length_rmse_m']:.3f} | "
            f"{row['best_window_count']} |"
        )
    md.extend([
        "",
        "## Best Config Windows",
        "",
        f"Best config by mean ATE: `{best_config['config']}`.",
        "",
        "| Window | Baseline ATE (m) | Smoothed ATE (m) | Delta (m) | Step RMSE (m) |",
        "| --- | ---: | ---: | ---: | ---: |",
    ])
    for row in by_config[best_config["config"]]:
        md.append(
            f"| {row['window']} | {row['baseline_stability_motion_ate_xy_m']:.3f} | "
            f"{row['ate_xy_m']:.3f} | {row['ate_delta_vs_baseline_m']:+.3f} | "
            f"{row['step_length_rmse_m']:.3f} |"
        )
    md.append("")
    (output_dir / "temporal_smoother_summary.md").write_text("\n".join(md), encoding="utf-8")

    print(
        f"[done] best={best_config['config']} mean_ate={best_config['mean_ate_xy_m']:.3f}m "
        f"baseline_mean={baseline['mean_ate_xy_m']:.3f}m -> {output_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
