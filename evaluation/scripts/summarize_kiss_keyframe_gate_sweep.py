#!/usr/bin/env python3
"""Summarize KISS keyframe correction-gate benchmark directories."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


DEFAULT_GATES = (
    ("0.3", "kiss_keyframe_i10_c0p3"),
    ("0.5", "kiss_keyframe_i10_c0p5"),
    ("0.7", "kiss_keyframe_i10_c0p7"),
    ("1.0", "kiss_keyframe_i10"),
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


def aggregate_row(summary: dict, policy: str) -> dict:
    aggregate = summary["aggregate"]
    if isinstance(aggregate, dict):
        return aggregate[policy]
    for row in aggregate:
        if row.get("policy") == policy:
            return row
    raise KeyError(f"missing aggregate policy {policy!r}")


def aggregate_step_rmse(row: dict) -> float:
    return float(row.get("mean_step_length_rmse_m", row.get("mean_step_rmse_m")))


def main() -> int:
    args = parse_args()
    rows = []
    windows_by_gate = {}
    for gate, dirname in DEFAULT_GATES:
        path = args.benchmark_dir / dirname / "kiss_keyframe_summary.json"
        payload = load_json(path)
        aggregate = payload["aggregate"]
        ate = aggregate["kiss_keyframe_ate_xy_m"]
        step = aggregate["kiss_keyframe_step_length_rmse_m"]
        row = {
            "gate_m": float(gate),
            "summary_json": str(path),
            "mean_ate_xy_m": float(ate["mean"]),
            "median_ate_xy_m": float(ate["median"]),
            "max_ate_xy_m": float(ate["max"]),
            "mean_step_length_rmse_m": float(step["mean"]),
            "improved_vs_stability_motion_count": int(
                aggregate["improved_vs_stability_motion_count"]
            ),
            "improved_vs_kiss_pair_count": int(aggregate["improved_vs_kiss_pair_count"]),
        }
        rows.append(row)
        windows_by_gate[gate] = payload["windows"]

    baseline_summary = load_json(args.benchmark_dir / "window_benchmark_summary.json")
    stability = aggregate_row(baseline_summary, "stability_motion")
    kiss = aggregate_row(baseline_summary, "kiss")
    best_mean = min(rows, key=lambda row: row["mean_ate_xy_m"])
    best_tail = min(rows, key=lambda row: (row["max_ate_xy_m"], row["mean_ate_xy_m"]))
    best_product = min(
        rows,
        key=lambda row: (
            row["max_ate_xy_m"],
            -row["improved_vs_stability_motion_count"],
            row["mean_step_length_rmse_m"],
        ),
    )

    out = {
        "benchmark_dir": str(args.benchmark_dir),
        "baseline": {
            "kiss": kiss,
            "stability_motion": stability,
        },
        "best_mean_gate_m": best_mean["gate_m"],
        "best_tail_gate_m": best_tail["gate_m"],
        "recommended_product_gate_m": best_product["gate_m"],
        "gates": rows,
    }

    output_json = args.output_json or (
        args.benchmark_dir / "kiss_keyframe_gate_sweep_summary.json"
    )
    output_md = args.output_md or (
        args.benchmark_dir / "kiss_keyframe_gate_sweep_summary.md"
    )
    output_json.write_text(json.dumps(out, indent=2) + "\n", encoding="utf-8")

    md = [
        "# KISS Keyframe Correction Gate Sweep",
        "",
        f"Benchmark: `{args.benchmark_dir}`",
        "",
        "## Aggregate",
        "",
        "| Method / gate | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Improved vs stability |",
        "| --- | ---: | ---: | ---: | ---: | ---: |",
        (
            f"| KISS pair | {kiss['mean_ate_xy_m']:.3f} | {kiss['median_ate_xy_m']:.3f} | "
            f"{kiss['max_ate_xy_m']:.3f} | {aggregate_step_rmse(kiss):.3f} | - |"
        ),
        (
            f"| Stability motion 0.0025 | {stability['mean_ate_xy_m']:.3f} | "
            f"{stability['median_ate_xy_m']:.3f} | {stability['max_ate_xy_m']:.3f} | "
            f"{aggregate_step_rmse(stability):.3f} | - |"
        ),
    ]
    for row in sorted(rows, key=lambda row: row["gate_m"]):
        md.append(
            f"| Keyframe c<={row['gate_m']:.1f} | {row['mean_ate_xy_m']:.3f} | "
            f"{row['median_ate_xy_m']:.3f} | {row['max_ate_xy_m']:.3f} | "
            f"{row['mean_step_length_rmse_m']:.3f} | "
            f"{row['improved_vs_stability_motion_count']}/12 |"
        )
    md.extend([
        "",
        "## Selection",
        "",
        f"- Best mean ATE gate: `{best_mean['gate_m']:.1f} m`.",
        f"- Best tail ATE gate: `{best_tail['gate_m']:.1f} m`.",
        f"- Product-risk recommendation: `{best_product['gate_m']:.1f} m`, because it minimizes max ATE first.",
        "",
    ])
    output_md.write_text("\n".join(md), encoding="utf-8")
    print(
        f"[done] best_mean={best_mean['gate_m']:.1f}m "
        f"best_tail={best_tail['gate_m']:.1f}m recommended={best_product['gate_m']:.1f}m "
        f"-> {output_md}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
