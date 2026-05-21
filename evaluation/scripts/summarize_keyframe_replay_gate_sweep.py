#!/usr/bin/env python3
"""Summarize keyframe-corrected prior replay gate sweeps."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


DEFAULT_RUNS = (
    ("c<=0.5, corr>=80", "stability_motion_keyframe_c0p5"),
    ("c<=0.5, corr>=5000", "stability_motion_keyframe_c0p5_corr5000"),
    ("c<=0.5, corr>=6000", "stability_motion_keyframe_c0p5_corr6000"),
    ("c<=0.5, corr>=7000", "stability_motion_keyframe_c0p5_corr7000"),
    ("c<=0.7, corr>=80", "stability_motion_keyframe_c0p7"),
)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--benchmark-dir",
        type=Path,
        default=Path("experiments/results/intensity_odometry/window_benchmark_60"),
    )
    p.add_argument("--output-json", type=Path)
    p.add_argument("--output-md", type=Path)
    return p.parse_args()


def load_json(path: Path) -> dict:
    return json.loads(path.read_text())


def main() -> int:
    args = parse_args()
    rows = []
    stability = None
    for label, dirname in DEFAULT_RUNS:
        path = args.benchmark_dir / dirname / "keyframe_corrected_prior_summary.json"
        payload = load_json(path)
        aggregate = payload["aggregate"]
        if stability is None:
            stability = aggregate["stability_motion_ate_xy_m"]
        ate = aggregate["keyframe_corrected_prior_ate_xy_m"]
        step = aggregate["keyframe_corrected_prior_step_length_rmse_m"]
        regressions = sum(
            1 for row in payload["windows"] if row["delta_vs_stability_motion_m"] > 1e-9
        )
        rows.append({
            "label": label,
            "summary_json": str(path),
            "mean_ate_xy_m": float(ate["mean"]),
            "median_ate_xy_m": float(ate["median"]),
            "max_ate_xy_m": float(ate["max"]),
            "mean_step_length_rmse_m": float(step["mean"]),
            "improved_vs_stability_motion_count": int(
                aggregate["improved_vs_stability_motion_count"]
            ),
            "regression_count": regressions,
        })

    if stability is None:
        raise RuntimeError("no runs found")
    best_mean = min(rows, key=lambda row: row["mean_ate_xy_m"])
    best_tail = min(rows, key=lambda row: (row["max_ate_xy_m"], row["mean_ate_xy_m"]))
    best_product = min(
        rows,
        key=lambda row: (
            row["regression_count"],
            row["max_ate_xy_m"],
            row["mean_ate_xy_m"],
        ),
    )
    out = {
        "benchmark_dir": str(args.benchmark_dir),
        "stability_motion": stability,
        "best_mean": best_mean,
        "best_tail": best_tail,
        "recommended_product": best_product,
        "runs": rows,
    }
    output_json = args.output_json or (
        args.benchmark_dir / "keyframe_replay_gate_sweep_summary.json"
    )
    output_md = args.output_md or (
        args.benchmark_dir / "keyframe_replay_gate_sweep_summary.md"
    )
    output_json.write_text(json.dumps(out, indent=2) + "\n", encoding="utf-8")

    md = [
        "# Keyframe Replay Gate Sweep",
        "",
        f"Benchmark: `{args.benchmark_dir}`",
        "",
        "| Run | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Improved | Regressions |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: |",
        (
            f"| Stability motion 0.0025 | {stability['mean']:.3f} | "
            f"{stability['median']:.3f} | {stability['max']:.3f} | - | - | - |"
        ),
    ]
    for row in rows:
        md.append(
            f"| {row['label']} | {row['mean_ate_xy_m']:.3f} | "
            f"{row['median_ate_xy_m']:.3f} | {row['max_ate_xy_m']:.3f} | "
            f"{row['mean_step_length_rmse_m']:.3f} | "
            f"{row['improved_vs_stability_motion_count']}/12 | "
            f"{row['regression_count']}/12 |"
        )
    md.extend([
        "",
        "## Selection",
        "",
        f"- Best mean ATE: `{best_mean['label']}`.",
        f"- Best max ATE: `{best_tail['label']}`.",
        f"- Product-risk recommendation: `{best_product['label']}`.",
        "",
    ])
    output_md.write_text("\n".join(md), encoding="utf-8")
    print(
        f"[done] best_mean={best_mean['label']} best_tail={best_tail['label']} "
        f"recommended={best_product['label']} -> {output_md}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
