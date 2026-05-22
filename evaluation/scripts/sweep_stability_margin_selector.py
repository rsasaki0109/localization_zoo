#!/usr/bin/env python3
"""Sweep conservative stability-margin pair-prior selector thresholds."""

from __future__ import annotations

import argparse
import json
import statistics
import subprocess
import sys
from pathlib import Path


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--benchmark-dir",
        type=Path,
        default=Path("experiments/results/intensity_odometry/window_benchmark"),
    )
    p.add_argument("--preferred-name", default="small_gicp")
    p.add_argument(
        "--threshold",
        action="append",
        type=float,
        help="Stability mean-delta advantage required to override preferred candidate.",
    )
    p.add_argument("--reuse", action="store_true")
    return p.parse_args()


def run(cmd: list[str | Path], output: Path, reuse: bool) -> None:
    if reuse and output.is_file():
        print(f"[reuse] {output}")
        return
    print("[run] " + " ".join(str(part) for part in cmd))
    subprocess.run([str(part) for part in cmd], check=True)


def load_windows(benchmark_dir: Path) -> list[str]:
    summary = benchmark_dir / "window_benchmark_summary.json"
    if summary.is_file():
        payload = json.loads(summary.read_text())
        return [row["window"] for row in payload["windows"]]
    return sorted(path.name for path in (benchmark_dir / "windows").glob("kitti_seq_*"))


def metrics_from(path: Path) -> tuple[float, float]:
    payload = json.loads(path.read_text())
    metrics = payload["metrics"]
    return float(metrics["ate_xy_m"]), float(metrics["step_length_rmse_m"])


def summarize_threshold(benchmark_dir: Path, outputs: dict[str, Path]) -> dict:
    policies = {
        "gated": outputs,
        "kiss": {
            key: benchmark_dir / "results" / f"{key}_kiss_pair_gate2.json"
            for key in outputs
        },
        "small_gicp": {
            key: benchmark_dir / "results" / f"{key}_small_gicp_v0.75_c512.json"
            for key in outputs
        },
        "margin": {
            key: benchmark_dir / "results" / f"selected_prior_margin_{key}.json"
            for key in outputs
        },
        "stability_delta": {
            key: benchmark_dir / "results" / f"selected_prior_stability_delta_{key}.json"
            for key in outputs
        },
    }
    rows = []
    wins = {name: 0 for name in policies}
    for key in outputs:
        row = {"window": key}
        for name, paths in policies.items():
            ate, step = metrics_from(paths[key])
            row[f"{name}_ate_xy_m"] = ate
            row[f"{name}_step_rmse_m"] = step
        best = min(policies, key=lambda name: row[f"{name}_ate_xy_m"])
        wins[best] += 1
        rows.append(row)

    gated_ate = [row["gated_ate_xy_m"] for row in rows]
    gated_step = [row["gated_step_rmse_m"] for row in rows]
    return {
        "mean_ate_xy_m": statistics.mean(gated_ate),
        "median_ate_xy_m": statistics.median(gated_ate),
        "mean_step_rmse_m": statistics.mean(gated_step),
        "max_ate_xy_m": max(gated_ate),
        "wins": wins["gated"],
        "rows": rows,
    }


def main() -> int:
    args = parse_args()
    thresholds = args.threshold or [
        0.0,
        0.001,
        0.0015,
        0.002,
        0.0025,
        0.003,
        0.004,
        0.005,
        0.01,
        0.02,
        0.05,
        0.10,
    ]
    windows = load_windows(args.benchmark_dir)
    results_dir = args.benchmark_dir / "results"
    sweep_dir = args.benchmark_dir / "stability_margin_sweep"
    sweep_dir.mkdir(parents=True, exist_ok=True)

    summaries = []
    for threshold in thresholds:
        outputs: dict[str, Path] = {}
        threshold_tag = f"{threshold:.4f}".replace(".", "p")
        for key in windows:
            out = sweep_dir / f"selected_prior_preferred_stability_margin_{threshold_tag}_{key}.json"
            outputs[key] = out
            cmd: list[str | Path] = [
                sys.executable,
                "evaluation/scripts/select_pair_prior.py",
                "--sequence-pcd-dir",
                args.benchmark_dir / "windows" / key / "pcd",
                "--gt-csv",
                args.benchmark_dir / "windows" / key / "gt.csv",
                "--max-frames",
                "30",
                "--candidate",
                f"kiss:{results_dir / f'{key}_kiss_pair_gate2.json'}",
                "--candidate",
                f"small_gicp:{results_dir / f'{key}_small_gicp_v0.75_c512.json'}",
                "--shared-residual-json",
                results_dir / f"{key}_shared_residuals.json",
                "--policy",
                "preferred_with_stability_margin",
                "--preferred-name",
                args.preferred_name,
                "--score-margin",
                str(threshold),
                "--output-json",
                out,
                "--progress-every",
                "0",
            ]
            run(cmd, out, args.reuse)
        summary = summarize_threshold(args.benchmark_dir, outputs)
        summary["threshold"] = threshold
        summary["preferred_name"] = args.preferred_name
        summaries.append(summary)
        print(
            f"[threshold] {threshold:.4f} mean={summary['mean_ate_xy_m']:.3f} "
            f"max={summary['max_ate_xy_m']:.3f} wins={summary['wins']}"
        )

    best_mean = min(summaries, key=lambda row: row["mean_ate_xy_m"])
    best_max = min(summaries, key=lambda row: row["max_ate_xy_m"])
    payload = {
        "benchmark_dir": str(args.benchmark_dir),
        "preferred_name": args.preferred_name,
        "best_mean_threshold": best_mean["threshold"],
        "best_max_threshold": best_max["threshold"],
        "thresholds": summaries,
    }
    out_json = sweep_dir / "stability_margin_sweep_summary.json"
    out_md = sweep_dir / "stability_margin_sweep_summary.md"
    out_json.write_text(json.dumps(payload, indent=2) + "\n")

    lines = [
        "# Stability Margin Sweep",
        "",
        f"- Benchmark: `{args.benchmark_dir}`",
        f"- Preferred: `{args.preferred_name}`",
        f"- Best mean threshold: `{best_mean['threshold']}`",
        f"- Best max threshold: `{best_max['threshold']}`",
        "",
        "| Threshold | Mean ATE [m] | Median ATE [m] | Mean step RMSE [m] | Max ATE [m] | Wins |",
        "| ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for row in summaries:
        lines.append(
            f"| {row['threshold']:.4f} | {row['mean_ate_xy_m']:.3f} | "
            f"{row['median_ate_xy_m']:.3f} | {row['mean_step_rmse_m']:.3f} | "
            f"{row['max_ate_xy_m']:.3f} | {row['wins']} |"
        )
    out_md.write_text("\n".join(lines) + "\n")
    print(f"[done] summary -> {out_md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
