#!/usr/bin/env python3
"""Analyze tail failures in pair-prior selector benchmark windows."""

from __future__ import annotations

import argparse
import json
import math
import statistics
from pathlib import Path

import numpy as np

from intensity_bev_odometry_demo import load_gt_xyyaw


DEFAULT_POLICIES = (
    ("kiss", "{window}_kiss_pair_gate2.json", "KISS"),
    ("small_gicp", "{window}_small_gicp_v0.75_c512.json", "SmallGICP"),
    ("stability_delta", "selected_prior_stability_delta_{window}.json", "Stability delta"),
    (
        "stability_margin_0p0010",
        "selected_prior_stability_margin_0p0010_{window}.json",
        "Stability margin 0.001",
    ),
    (
        "stability_margin_0p0025",
        "selected_prior_stability_margin_0p0025_{window}.json",
        "Stability margin 0.0025",
    ),
    (
        "stability_motion_0p0025",
        "selected_prior_stability_motion_0p0025_{window}.json",
        "Stability motion 0.0025",
    ),
    ("calibrated_full", "selected_prior_calibrated_full_{window}.json", "Calibrated full"),
)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--benchmark-dir",
        type=Path,
        default=Path("experiments/results/intensity_odometry/window_benchmark"),
    )
    p.add_argument(
        "--window",
        action="append",
        default=None,
        help="Window key to analyze. Repeatable. Defaults to worst stability-motion windows.",
    )
    p.add_argument("--tail-policy", default="stability_motion")
    p.add_argument("--tail-count", type=int, default=2)
    p.add_argument("--output-json", type=Path)
    p.add_argument("--output-md", type=Path)
    p.add_argument("--top-k", type=int, default=8)
    return p.parse_args()


def load_json(path: Path) -> dict:
    return json.loads(path.read_text())


def candidate_method_for_policy(policy: str, pair: dict) -> str | None:
    if policy in ("kiss", "small_gicp"):
        return policy
    selected = pair.get("selected")
    return str(selected) if selected else None


def policy_values(pair: dict) -> dict[str, float]:
    raw = pair.get("policy_values") or {}
    out: dict[str, float] = {}
    for key, value in raw.items():
        try:
            out[str(key)] = float(value)
        except (TypeError, ValueError):
            pass
    return out


def load_policy_payload(results_dir: Path, window: str, template: str) -> tuple[Path, dict, dict[int, dict]]:
    path = results_dir / template.format(window=window)
    payload = load_json(path)
    return path, payload, {int(pair["index"]): pair for pair in payload.get("pairs", [])}


def frame_error_summary(policy_payload: dict, gt_csv: Path) -> dict[str, float | int]:
    raw_poses = policy_payload.get("poses_xyyaw", [])
    if raw_poses and isinstance(raw_poses[0], dict):
        poses = np.asarray([
            [
                float(pose.get("x_m", 0.0)),
                float(pose.get("y_m", 0.0)),
                math.radians(float(pose.get("yaw_deg", 0.0))),
            ]
            for pose in raw_poses
        ], dtype=np.float64)
    else:
        poses = np.asarray(raw_poses, dtype=np.float64)
    if poses.size == 0:
        return {
            "mean_frame_error_m": float("nan"),
            "max_frame_error_m": float("nan"),
            "max_frame_index": -1,
        }
    gt = load_gt_xyyaw(gt_csv)[: poses.shape[0]]
    errors = np.linalg.norm(poses[:, :2] - gt[:, :2], axis=1)
    max_index = int(np.argmax(errors))
    return {
        "mean_frame_error_m": float(np.mean(errors)),
        "max_frame_error_m": float(errors[max_index]),
        "max_frame_index": max_index,
    }


def analyze_policy(
    label: str,
    title: str,
    payload: dict,
    pairs_by_index: dict[int, dict],
    diagnostic_pairs: dict[int, dict],
    gt_csv: Path,
    top_k: int,
) -> dict:
    regrets = []
    miss_rows = []
    selected_counts: dict[str, int] = {}
    oracle_counts: dict[str, int] = {}
    hit_count = 0
    total = 0

    for index, diag in sorted(diagnostic_pairs.items()):
        pair = pairs_by_index.get(index, {})
        selected = candidate_method_for_policy(label, pair)
        oracle = diag.get("oracle_best")
        if selected is None or oracle is None:
            continue
        methods = diag.get("methods", {})
        if selected not in methods or oracle not in methods:
            continue
        selected_error = float(methods[selected]["translation_error_m"])
        oracle_error = float(methods[oracle]["translation_error_m"])
        regret = selected_error - oracle_error
        values = policy_values(pair)
        selected_counts[selected] = selected_counts.get(selected, 0) + 1
        oracle_counts[oracle] = oracle_counts.get(oracle, 0) + 1
        hit = selected == oracle
        hit_count += int(hit)
        total += 1
        regrets.append(regret)
        if not hit or regret > 0.02:
            row = {
                "index": index,
                "selected": selected,
                "oracle": oracle,
                "selected_error_m": selected_error,
                "oracle_error_m": oracle_error,
                "regret_m": regret,
                "policy_values": values,
                "gt_step_m": math.hypot(float(diag["gt_dx_m"]), float(diag["gt_dy_m"])),
            }
            for name, method in methods.items():
                row[f"{name}_stability_mean_delta_m"] = method.get(
                    "shared_stability_mean_delta_m"
                )
                row[f"{name}_cubemap_score"] = method.get("cubemap_score")
                row[f"{name}_translation_error_m"] = method.get("translation_error_m")
            miss_rows.append(row)

    metrics = payload.get("metrics") or {}
    frame_summary = frame_error_summary(payload, gt_csv)
    positive_regrets = [value for value in regrets if value > 0.0]
    top_misses = sorted(miss_rows, key=lambda row: row["regret_m"], reverse=True)[:top_k]
    return {
        "policy": label,
        "title": title,
        "metrics": metrics,
        **frame_summary,
        "pair_hit_rate": hit_count / total if total else None,
        "pair_hits": hit_count,
        "pair_total": total,
        "selected_counts": selected_counts,
        "oracle_counts": oracle_counts,
        "mean_pair_regret_m": statistics.mean(regrets) if regrets else None,
        "positive_pair_regret_count": len(positive_regrets),
        "positive_pair_regret_sum_m": sum(positive_regrets),
        "max_pair_regret_m": max(regrets) if regrets else None,
        "top_misses": top_misses,
    }


def analyze_window(args: argparse.Namespace, window: str) -> dict:
    results_dir = args.benchmark_dir / "results"
    window_dir = args.benchmark_dir / "windows" / window
    gt_csv = window_dir / "gt.csv"
    diagnostic_path = results_dir / f"selector_diagnostics_{window}.json"
    diagnostic = load_json(diagnostic_path)
    diagnostic_pairs = {int(pair["index"]): pair for pair in diagnostic.get("pairs", [])}

    policies = []
    for label, template, title in DEFAULT_POLICIES:
        path, payload, pairs_by_index = load_policy_payload(results_dir, window, template)
        policy = analyze_policy(
            label,
            title,
            payload,
            pairs_by_index,
            diagnostic_pairs,
            gt_csv,
            args.top_k,
        )
        policy["path"] = str(path)
        policies.append(policy)

    best_mean = min(
        policies,
        key=lambda row: float(row["metrics"].get("ate_xy_m", float("inf"))),
    )
    best_tail = min(policies, key=lambda row: float(row.get("max_frame_error_m", float("inf"))))
    return {
        "window": window,
        "gt_csv": str(gt_csv),
        "diagnostic_json": str(diagnostic_path),
        "best_ate_policy": best_mean["policy"],
        "best_max_frame_policy": best_tail["policy"],
        "policies": policies,
    }


def write_markdown(payload: dict, output_md: Path) -> None:
    windows_text = ", ".join(f"`{window}`" for window in payload["windows_analyzed"])
    lines = [
        "# Pair Prior Tail Failure Analysis",
        "",
        f"- Benchmark: `{payload['benchmark_dir']}`",
        f"- Windows: {windows_text}" if windows_text else "- Windows: none",
        "",
    ]
    for window in payload["windows"]:
        lines.extend([
            f"## {window['window']}",
            "",
            f"- Best ATE policy: `{window['best_ate_policy']}`",
            f"- Best max-frame policy: `{window['best_max_frame_policy']}`",
            "",
            "| Policy | ATE [m] | Step RMSE [m] | Max frame err [m] | Max frame | Pair hits | Positive regret sum [m] | Selected counts |",
            "| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |",
        ])
        for policy in window["policies"]:
            metrics = policy["metrics"]
            pair_hits = (
                f"{policy['pair_hits']}/{policy['pair_total']}"
                if policy["pair_total"]
                else "-"
            )
            lines.append(
                f"| {policy['title']} | {metrics.get('ate_xy_m', float('nan')):.3f} | "
                f"{metrics.get('step_length_rmse_m', float('nan')):.3f} | "
                f"{policy['max_frame_error_m']:.3f} | {policy['max_frame_index']} | "
                f"{pair_hits} | {policy['positive_pair_regret_sum_m']:.3f} | "
                f"`{policy['selected_counts']}` |"
            )

        lines.extend([
            "",
            "### Top Regret Pairs",
            "",
            "| Policy | Pair | Selected | Oracle | Regret [m] | Selected err [m] | Oracle err [m] | GT step [m] |",
            "| --- | ---: | --- | --- | ---: | ---: | ---: | ---: |",
        ])
        for policy in window["policies"]:
            for miss in policy["top_misses"][:5]:
                lines.append(
                    f"| {policy['title']} | {miss['index']} | {miss['selected']} | "
                    f"{miss['oracle']} | {miss['regret_m']:.3f} | "
                    f"{miss['selected_error_m']:.3f} | {miss['oracle_error_m']:.3f} | "
                    f"{miss['gt_step_m']:.3f} |"
                )
        lines.append("")

    output_md.write_text("\n".join(lines) + "\n")


def default_tail_windows(benchmark_dir: Path, policy: str, count: int) -> list[str]:
    summary = benchmark_dir / "window_benchmark_summary.json"
    if not summary.is_file():
        return [
            "kitti_seq_08_start0030_30",
            "kitti_seq_08_start0090_30",
        ]
    payload = load_json(summary)
    key = f"{policy}_ate_xy_m"
    rows = [
        row for row in payload.get("windows", [])
        if row.get(key) is not None
    ]
    if not rows:
        rows = [
            row for row in payload.get("windows", [])
            if row.get("window")
        ]
        key = "small_gicp_ate_xy_m"
    rows.sort(key=lambda row: float(row.get(key, -float("inf"))), reverse=True)
    return [str(row["window"]) for row in rows[:count]]


def main() -> int:
    args = parse_args()
    if args.window is None:
        args.window = default_tail_windows(args.benchmark_dir, args.tail_policy, args.tail_count)
    out_dir = args.benchmark_dir / "tail_failure_analysis"
    output_json = args.output_json or out_dir / "tail_failure_analysis.json"
    output_md = args.output_md or out_dir / "tail_failure_analysis.md"
    output_json.parent.mkdir(parents=True, exist_ok=True)
    output_md.parent.mkdir(parents=True, exist_ok=True)

    windows = [analyze_window(args, window) for window in args.window]
    payload = {
        "benchmark_dir": str(args.benchmark_dir),
        "windows_analyzed": args.window,
        "windows": windows,
    }
    output_json.write_text(json.dumps(payload, indent=2) + "\n")
    write_markdown(payload, output_md)
    print(f"[done] wrote {output_md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
