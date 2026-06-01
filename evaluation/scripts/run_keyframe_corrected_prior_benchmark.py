#!/usr/bin/env python3
"""Apply KISS keyframe corrections to stability-motion prior benchmark windows."""

from __future__ import annotations

import argparse
import json
import statistics
from pathlib import Path

from keyframe_correction_component import (
    KeyframeCorrectionConfig,
    apply_keyframe_corrections,
)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--benchmark-dir",
        type=Path,
        default=Path("experiments/results/intensity_odometry/window_benchmark_60"),
    )
    p.add_argument("--output-dir", type=Path)
    p.add_argument("--anchor-dir-name", default="kiss_keyframe_i10_c0p5_relog")
    p.add_argument("--prior-template", default="selected_prior_stability_motion_0p0025_{window}.json")
    p.add_argument("--anchor-template", default="{window}_kiss_keyframe_i10.json")
    p.add_argument("--recipe-json", type=Path)
    p.add_argument("--max-keyframe-correction", type=float, default=0.5)
    p.add_argument("--max-keyframe-yaw-deg", type=float, default=5.0)
    p.add_argument("--max-keyframe-rmse", type=float, default=1.2)
    p.add_argument("--min-keyframe-correspondences", type=int, default=6000)
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


def load_config(args: argparse.Namespace) -> tuple[KeyframeCorrectionConfig, dict | None]:
    if args.recipe_json is None:
        return (
            KeyframeCorrectionConfig(
                max_keyframe_correction_m=args.max_keyframe_correction,
                max_keyframe_yaw_deg=args.max_keyframe_yaw_deg,
                max_keyframe_rmse_m=args.max_keyframe_rmse,
                min_keyframe_correspondences=args.min_keyframe_correspondences,
            ),
            None,
        )
    recipe = load_json(args.recipe_json)
    return KeyframeCorrectionConfig.from_recipe(recipe), recipe


def main() -> int:
    args = parse_args()
    config, recipe = load_config(args)
    output_dir = args.output_dir or (
        args.benchmark_dir / f"stability_motion_keyframe_c{config.max_keyframe_correction_m:g}".replace(".", "p")
    )
    results_dir = output_dir / "results"
    results_dir.mkdir(parents=True, exist_ok=True)

    summary = load_json(args.benchmark_dir / "window_benchmark_summary.json")
    windows = [str(row["window"]) for row in summary["windows"]]
    baseline_by_window = {str(row["window"]): row for row in summary["windows"]}
    rows = []
    for window in windows:
        prior_json = args.benchmark_dir / "results" / args.prior_template.format(window=window)
        keyframe_json = (
            args.benchmark_dir
            / args.anchor_dir_name
            / "results"
            / args.anchor_template.format(window=window)
        )
        out_json = results_dir / f"{window}_stability_motion_keyframe.json"
        prior_payload = load_json(prior_json)
        keyframe_payload = load_json(keyframe_json)
        payload = apply_keyframe_corrections(
            prior_payload,
            keyframe_payload,
            config,
            source_prior_json=str(prior_json),
            source_keyframe_json=str(keyframe_json),
        )
        out_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
        base = baseline_by_window[window]
        rows.append({
            "window": window,
            "path": str(out_json),
            "ate_xy_m": metric(payload, "ate_xy_m"),
            "step_length_rmse_m": metric(payload, "step_length_rmse_m"),
            "stability_motion_ate_xy_m": float(base["stability_motion_ate_xy_m"]),
            "delta_vs_stability_motion_m": metric(payload, "ate_xy_m") - float(base["stability_motion_ate_xy_m"]),
            "correction_counts": payload.get("correction_counts", {}),
        })

    ate = [row["ate_xy_m"] for row in rows]
    step = [row["step_length_rmse_m"] for row in rows]
    stability = [row["stability_motion_ate_xy_m"] for row in rows]
    out = {
        "benchmark_dir": str(args.benchmark_dir),
        "anchor_dir_name": args.anchor_dir_name,
        "recipe_json": str(args.recipe_json) if args.recipe_json else None,
        "recipe": recipe,
        "parameters": {
            "max_keyframe_correction": config.max_keyframe_correction_m,
            "max_keyframe_yaw_deg": config.max_keyframe_yaw_deg,
            "max_keyframe_rmse": config.max_keyframe_rmse_m,
            "min_keyframe_correspondences": config.min_keyframe_correspondences,
        },
        "aggregate": {
            "keyframe_corrected_prior_ate_xy_m": aggregate(ate),
            "keyframe_corrected_prior_step_length_rmse_m": aggregate(step),
            "stability_motion_ate_xy_m": aggregate(stability),
            "improved_vs_stability_motion_count": sum(row["delta_vs_stability_motion_m"] < 0.0 for row in rows),
        },
        "windows": rows,
    }
    (output_dir / "keyframe_corrected_prior_summary.json").write_text(
        json.dumps(out, indent=2) + "\n",
        encoding="utf-8",
    )
    md = [
        "# Keyframe-Corrected Stability Prior Benchmark",
        "",
        f"Benchmark: `{args.benchmark_dir}`",
        f"Anchor dir: `{args.anchor_dir_name}`",
        "",
        "## Aggregate",
        "",
        "| Method | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Improved windows |",
        "| --- | ---: | ---: | ---: | ---: | ---: |",
        (
            f"| Stability motion 0.0025 | {statistics.mean(stability):.3f} | "
            f"{statistics.median(stability):.3f} | {max(stability):.3f} | - | - |"
        ),
        (
            f"| Stability + keyframe corrections | {statistics.mean(ate):.3f} | "
            f"{statistics.median(ate):.3f} | {max(ate):.3f} | "
            f"{statistics.mean(step):.3f} | "
            f"{sum(row['delta_vs_stability_motion_m'] < 0.0 for row in rows)}/12 |"
        ),
        "",
        "## Windows",
        "",
        "| Window | Stability ATE | Corrected ATE | Delta | Step RMSE | Corrections |",
        "| --- | ---: | ---: | ---: | ---: | --- |",
    ]
    for row in rows:
        counts = row["correction_counts"]
        accepted = counts.get("accepted", 0)
        md.append(
            f"| {row['window']} | {row['stability_motion_ate_xy_m']:.3f} | "
            f"{row['ate_xy_m']:.3f} | {row['delta_vs_stability_motion_m']:+.3f} | "
            f"{row['step_length_rmse_m']:.3f} | {accepted} accepted |"
        )
    md.append("")
    (output_dir / "keyframe_corrected_prior_summary.md").write_text("\n".join(md), encoding="utf-8")
    print(
        f"[done] mean_ate={statistics.mean(ate):.3f}m "
        f"stability_mean={statistics.mean(stability):.3f}m -> {output_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
