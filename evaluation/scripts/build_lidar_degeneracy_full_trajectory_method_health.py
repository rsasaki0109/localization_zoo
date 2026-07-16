#!/usr/bin/env python3

"""Build calibrator-compatible method health rows from NTNU LiDAR degeneracy
full-trajectory result JSONs (tunnel_full, fog_full).

This mirrors build_hard_pcl_method_health_comparison.py exactly (same
base_method_row()/annotate_cross_method() logic, same flag thresholds:
accepted_rate == 0 -> all_pairs_failed, accepted_rate < 0.5 -> low_acceptance,
converged_rate < 0.5 -> low_convergence, health_state ok/diagnostic_watch/fail
at the 0.9/0.5 accepted_rate breakpoints, cross-method path ratio outside
[0.5, 2.0] -> cross_method_suspicious / path_disagrees_with_all_method_median)
so that calibrate_lidar_degeneracy_risk.py's policy application (reusing the
unmodified lidar_degeneracy_triage_v4 policy) can consume this comparison
JSON exactly the same way it consumes the Hard PCL one. No thresholds are
retuned here -- only the input file layout differs (NTNU full-trajectory
runs are one continuous window per method, not a windowed comparison).

Both tunnel_full and fog_full are chronically-degenerate corridors per the
full_trajectory_comparison.md write-ups for each sequence (weak longitudinal
tunnel constraint; fog obscurant corridor), so both are recorded with
expected_stress="chronic_degeneracy" (i.e. every row is a "stress" row for
calibrate_lidar_degeneracy_risk.py's is_stress-gated tables), matching how
the existing NTNU short-window calibration treats tunnel/fog windows as
stress rather than nominal.
"""

from __future__ import annotations

import argparse
import json
import math
import statistics
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_RESULTS = REPO_ROOT / "experiments" / "results" / "lidar_degeneracy"
DEFAULT_OUTPUT = DEFAULT_RESULTS / "risk_gt_calibration" / "ntnu_full_trajectory_method_health.json"
SEQUENCES = (
    ("tunnel_full", "chronic_degeneracy"),
    ("fog_full", "chronic_degeneracy"),
)
# Filename stem -> method label. Mirrors the *_full_trajectory.json naming
# convention used by SETUP_LIDAR_DEGENERACY_BENCHMARK.md's tunnel_full /
# fog_full commands.
METHOD_ALIASES = {
    "kiss_keyframe_full_trajectory": "kiss_keyframe",
    "litamin2_full_trajectory": "litamin2",
    "ct_icp_full_trajectory": "ct_icp",
    "xicp_full_trajectory": "xicp",
    "degen_sense_full_trajectory": "degen_sense_imu",
    "degen_sense_noimu_full_trajectory": "degen_sense_no_imu",
    "relead_imu_full_trajectory": "relead_imu",
    "relead_no_imu_full_trajectory": "relead_no_imu",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results-dir", type=Path, default=DEFAULT_RESULTS)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    return parser.parse_args()


def relpath(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO_ROOT))
    except ValueError:
        return str(path.resolve())


def path_length(poses: list[dict[str, Any]]) -> float:
    return sum(
        math.hypot(
            float(current["x_m"]) - float(previous["x_m"]),
            float(current["y_m"]) - float(previous["y_m"]),
        )
        for previous, current in zip(poses, poses[1:])
    )


def net_displacement(poses: list[dict[str, Any]]) -> float | None:
    if len(poses) < 2:
        return None
    first, last = poses[0], poses[-1]
    return math.hypot(
        float(last["x_m"]) - float(first["x_m"]),
        float(last["y_m"]) - float(first["y_m"]),
    )


def method_name(path: Path, payload: dict[str, Any]) -> str:
    return METHOD_ALIASES.get(path.stem, str(payload.get("method") or path.stem))


def base_method_row(path: Path) -> tuple[str, dict[str, Any]]:
    payload = json.loads(path.read_text())
    pairs = list(payload.get("pairs") or [])
    pair_count = len(pairs)
    accepted = sum(bool(pair.get("accepted")) for pair in pairs)
    converged = sum(bool(pair.get("converged")) for pair in pairs)
    accepted_rate = accepted / pair_count if pair_count else None
    converged_rate = converged / pair_count if pair_count else None

    # Same three flag thresholds as build_hard_pcl_method_health_comparison.py
    # (and run_lidar_degradation_health.py's health_flags): no re-tuning.
    flags: list[str] = []
    if accepted_rate is not None:
        if accepted_rate == 0.0:
            flags.append("all_pairs_failed")
        elif accepted_rate < 0.5:
            flags.append("low_acceptance")
    if converged_rate is not None and converged_rate < 0.5:
        flags.append("low_convergence")
    health_state = (
        "ok"
        if accepted_rate is not None and accepted_rate >= 0.9
        else "diagnostic_watch"
        if accepted_rate is not None and accepted_rate >= 0.5
        else "fail"
    )
    poses = list(payload.get("poses_xyyaw") or [])
    row = {
        "accepted_rate": accepted_rate,
        "converged_rate": converged_rate,
        "health_state": health_state,
        "risk_state": health_state,
        "flags": ",".join(flags),
        "risk_flags": ",".join(flags),
        "trajectory_path_length_m": path_length(poses),
        "trajectory_net_displacement_m": net_displacement(poses),
        "result_json": relpath(path),
        "gt_metrics": payload.get("metrics"),
    }
    return method_name(path, payload), row


def annotate_cross_method(rows: dict[str, dict[str, Any]]) -> None:
    paths = [float(row["trajectory_path_length_m"]) for row in rows.values()]
    all_median = statistics.median(paths)
    healthy_paths = [
        float(row["trajectory_path_length_m"])
        for row in rows.values()
        if row["health_state"] == "ok"
    ]
    healthy_median = statistics.median(healthy_paths) if healthy_paths else None
    for row in rows.values():
        path = float(row["trajectory_path_length_m"])
        row["path_vs_all_median"] = path / all_median if all_median > 1e-9 else None
        row["path_vs_healthy_median"] = (
            path / healthy_median
            if healthy_median is not None and healthy_median > 1e-9
            else None
        )
        suspicious = (
            row["path_vs_all_median"] is not None
            and (row["path_vs_all_median"] < 0.5 or row["path_vs_all_median"] > 2.0)
        )
        row["cross_method_suspicious"] = suspicious
        row["cross_method_probe"] = (
            "path_disagrees_with_all_method_median" if suspicious else ""
        )


def main() -> int:
    args = parse_args()
    sequences = []
    for sequence_name, expected_stress in SEQUENCES:
        result_dir = args.results_dir / sequence_name
        paths = sorted(
            p for p in result_dir.glob("*_full_trajectory.json")
        )
        if not paths:
            raise FileNotFoundError(f"No *_full_trajectory.json under {result_dir}")
        methods = dict(base_method_row(path) for path in paths)
        annotate_cross_method(methods)
        frame_count = int(json.loads(paths[0].read_text())["frames"])
        sequences.append(
            {
                "sequence": sequence_name,
                "windows": [
                    {
                        "name": "full",
                        "start": 0,
                        "end": frame_count - 1,
                        "expected_stress": expected_stress,
                        "methods": methods,
                    }
                ],
            }
        )

    payload = {
        "schema_version": 1,
        "inputs": {"results_dir": relpath(args.results_dir)},
        "sequences": sequences,
        "aggregates": {},
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"[done] wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
