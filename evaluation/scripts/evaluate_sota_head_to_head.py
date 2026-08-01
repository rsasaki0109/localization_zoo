#!/usr/bin/env python3
"""Apply the frozen v12 CUBE-LIO and FAST-LIVO2 head-to-head gates."""

from __future__ import annotations

import argparse
import json
import math
import statistics
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PROTOCOL = ROOT / "evaluation/data/lidar_odometry_sota_head_to_head_v12_protocol.json"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("results", type=Path)
    parser.add_argument("--protocol", type=Path, default=DEFAULT_PROTOCOL)
    parser.add_argument("--output", type=Path)
    return parser.parse_args()


def finite_positive(row: dict, key: str) -> bool:
    value = row.get(key)
    return isinstance(value, (int, float)) and math.isfinite(value) and value > 0.0


def validate_metrics(row: dict) -> None:
    for side in ("candidate", "comparator"):
        metrics = row.get(side, {})
        for key in ("ate_m", "rpe_trans_pct", "tracking_success_rate", "fps", "peak_rss_mb"):
            if not finite_positive(metrics, key):
                raise ValueError(f"missing or invalid {side}.{key}")
        if metrics["tracking_success_rate"] > 1.0:
            raise ValueError(f"invalid {side}.tracking_success_rate")


def evaluate_cube(protocol: dict, rows: dict) -> dict:
    gate = protocol["cube_lio_track"]["win_gate"]
    required = gate["required_rows"]
    missing = [name for name in required if name not in rows]
    decisions = {}
    for name in required:
        if name not in rows:
            continue
        validate_metrics(rows[name])
        candidate = rows[name]["candidate"]
        comparator = rows[name]["comparator"]
        checks = {
            "ate": candidate["ate_m"] < comparator["ate_m"],
            "rpe_trans": candidate["rpe_trans_pct"] < comparator["rpe_trans_pct"],
            "tracking": candidate["tracking_success_rate"] >= gate["candidate_tracking_success_rate_each_row"],
            "runtime": candidate["fps"] >= gate["candidate_fps_at_least_each_row"],
        }
        decisions[name] = {"checks": checks, "passed": all(checks.values())}
    return {
        "missing_rows": missing,
        "rows": decisions,
        "passed": not missing and all(row["passed"] for row in decisions.values()),
    }


def evaluate_fast_livo2(protocol: dict, rows: dict) -> dict:
    gate = protocol["fast_livo2_track"]["win_gate"]
    decisions = {}
    ate_ratios = []
    rpe_ratios = []
    candidate_fps = []
    for name, row in sorted(rows.items()):
        validate_metrics(row)
        candidate = row["candidate"]
        comparator = row["comparator"]
        ate_ratio = candidate["ate_m"] / comparator["ate_m"]
        rpe_ratio = candidate["rpe_trans_pct"] / comparator["rpe_trans_pct"]
        checks = {
            "ate_regression_cap": ate_ratio <= gate["maximum_per_row_ate_ratio"],
            "rpe_regression_cap": rpe_ratio <= gate["maximum_per_row_rpe_trans_ratio"],
            "tracking": candidate["tracking_success_rate"] >= comparator["tracking_success_rate"],
        }
        decisions[name] = {
            "ate_ratio": ate_ratio,
            "rpe_trans_ratio": rpe_ratio,
            "checks": checks,
            "passed": all(checks.values()),
        }
        ate_ratios.append(ate_ratio)
        rpe_ratios.append(rpe_ratio)
        candidate_fps.append(candidate["fps"])
    enough_rows = len(rows) >= gate["minimum_rows"]
    aggregate = {
        "rows": len(rows),
        "median_ate_ratio": statistics.median(ate_ratios) if ate_ratios else None,
        "median_rpe_trans_ratio": statistics.median(rpe_ratios) if rpe_ratios else None,
        "harmonic_mean_candidate_fps": statistics.harmonic_mean(candidate_fps) if candidate_fps else None,
    }
    aggregate_checks = {
        "minimum_rows": enough_rows,
        "median_ate": enough_rows and aggregate["median_ate_ratio"] < gate["candidate_median_ate_ratio_below"],
        "median_rpe_trans": enough_rows and aggregate["median_rpe_trans_ratio"] < gate["candidate_median_rpe_trans_ratio_below"],
        "runtime": enough_rows and aggregate["harmonic_mean_candidate_fps"] >= gate["candidate_aggregate_fps_at_least"],
    }
    return {
        "rows": decisions,
        "aggregate": aggregate,
        "aggregate_checks": aggregate_checks,
        "passed": all(aggregate_checks.values()) and all(row["passed"] for row in decisions.values()),
    }


def evaluate(protocol: dict, results: dict) -> dict:
    cube = evaluate_cube(protocol, results.get("cube_lio_track", {}).get("rows", {}))
    fast = evaluate_fast_livo2(protocol, results.get("fast_livo2_track", {}).get("rows", {}))
    return {
        "schema_version": 1,
        "protocol_id": protocol["protocol_id"],
        "cube_lio_track": cube,
        "fast_livo2_track": fast,
        "promotion_passed": cube["passed"] and fast["passed"],
    }


def main() -> int:
    args = parse_args()
    protocol = json.loads(args.protocol.read_text(encoding="utf-8"))
    results = json.loads(args.results.read_text(encoding="utf-8"))
    report = evaluate(protocol, results)
    rendered = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0 if report["promotion_passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
