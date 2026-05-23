#!/usr/bin/env python3
"""Compare GT-free LiDAR degeneracy health summaries across methods."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_POLICY_PATH = REPO_ROOT / "evaluation" / "config" / "lidar_degeneracy_triage_policy.json"
DEFAULT_INPUTS = (
    (
        "fog_200",
        (
            ("geometry_icp", "experiments/results/lidar_degeneracy/fog_200/odometry_health/odometry_health_summary.json"),
            ("intensity_bev", "experiments/results/lidar_degeneracy/fog_200/intensity_bev_health/odometry_health_summary.json"),
            ("kiss_keyframe", "experiments/results/lidar_degeneracy/fog_200/kiss_keyframe_health/odometry_health_summary.json"),
            ("ct_icp", "experiments/results/lidar_degeneracy/fog_200/ct_icp_health/odometry_health_summary.json"),
        ),
    ),
    (
        "tunnel_geom_2700_200",
        (
            ("geometry_icp", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/odometry_health/odometry_health_summary.json"),
            ("intensity_bev", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/intensity_bev_health/odometry_health_summary.json"),
            ("kiss_keyframe", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/kiss_keyframe_health/odometry_health_summary.json"),
            ("ct_icp", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/ct_icp_health/odometry_health_summary.json"),
        ),
    ),
)


def load_policy(path: Path = DEFAULT_POLICY_PATH) -> dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    decisions = payload.get("decision_order", {})
    reasons = payload.get("reason_decisions", {})
    default = payload.get("default_decision_for_unknown_reason", "watch")
    if not decisions or not reasons:
        raise RuntimeError(f"{path}: policy requires decision_order and reason_decisions")
    if default not in decisions:
        raise RuntimeError(f"{path}: default decision {default!r} missing from decision_order")
    unknown = sorted(set(reasons.values()) - set(decisions))
    if unknown:
        raise RuntimeError(f"{path}: unknown reason decisions: {unknown}")
    return payload


POLICY = load_policy()
POLICY_VERSION = str(POLICY["policy_version"])
POLICY_DEFAULT_DECISION = str(POLICY.get("default_decision_for_unknown_reason", "watch"))
POLICY_DECISION_ORDER = {
    str(name): int(rank) for name, rank in POLICY["decision_order"].items()
}
POLICY_BY_REASON = {
    str(reason): str(decision) for reason, decision in POLICY["reason_decisions"].items()
}
DIAGNOSTIC_WATCH_FLAGS = {"ct_icp_internal_convergence_low"}
WATCH_CLEAR_ACCEPTANCE_MIN = 0.999
WATCH_CLEAR_GATE_MIN = 0.999
WATCH_CLEAR_ITERATION_MARGIN = 0.5
WATCH_CLEAR_PATH_HEALTHY_MIN = 0.5
WATCH_CLEAR_PATH_HEALTHY_MAX = 2.0
WATCH_CLEAR_PATH_ALL_MIN = 0.33
WATCH_CLEAR_PATH_ALL_MAX = 3.0
WATCH_REJECT_BLOCKERS = {"accepted_below_one", "refinement_gate_below_one"}
WATCH_ITERATION_BLOCKERS = {"iterations_unknown", "iterations_pinned"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("experiments/results/lidar_degeneracy/method_health_comparison"),
    )
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def fmt(value: Any, digits: int = 3) -> str:
    if value is None:
        return "n/a"
    try:
        number = float(value)
    except (TypeError, ValueError):
        return "n/a"
    if not math.isfinite(number):
        return "n/a"
    return f"{number:.{digits}f}"


def health_label(row: dict[str, Any]) -> str:
    flags = row.get("health_flags", {})
    active = [name for name, value in flags.items() if value]
    return ", ".join(active) if active else "ok"


def keyframe_text(row: dict[str, Any]) -> str:
    attempted = row.get("keyframe_attempted")
    accepted = row.get("keyframe_accepted")
    if attempted is None or accepted is None:
        return "n/a"
    if int(attempted) == 0:
        return "n/a"
    return f"{int(accepted)}/{int(attempted)}"


def compact_counts(counts: dict[str, int]) -> str:
    if not counts:
        return "n/a"
    return ", ".join(f"{name}:{counts[name]}" for name in sorted(counts))


def split_csv_flags(value: Any) -> list[str]:
    text = str(value or "ok")
    if text == "ok" or text == "none":
        return []
    return [part.strip() for part in text.split(",") if part.strip()]


def flag_set(value: Any) -> set[str]:
    return set(split_csv_flags(value))


def diagnostic_watch_only(value: Any) -> bool:
    return flag_set(value) == DIAGNOSTIC_WATCH_FLAGS


def normalize_angle_deg(angle_deg: float) -> float:
    return (angle_deg + 180.0) % 360.0 - 180.0


def median(values: list[float]) -> float | None:
    finite = sorted(value for value in values if math.isfinite(value))
    if not finite:
        return None
    mid = len(finite) // 2
    if len(finite) % 2:
        return finite[mid]
    return 0.5 * (finite[mid - 1] + finite[mid])


def as_finite_float(value: Any) -> float | None:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def trajectory_diagnostics(payload: dict[str, Any]) -> dict[str, Any]:
    poses = payload.get("poses_xyyaw") or []
    if len(poses) < 2:
        return {
            "pose_count": len(poses),
            "trajectory_path_length_m": None,
            "trajectory_net_displacement_m": None,
            "trajectory_yaw_change_abs_deg": None,
        }
    path_length = 0.0
    for prev, curr in zip(poses, poses[1:]):
        path_length += math.hypot(
            float(curr["x_m"]) - float(prev["x_m"]),
            float(curr["y_m"]) - float(prev["y_m"]),
        )
    first = poses[0]
    last = poses[-1]
    net_displacement = math.hypot(
        float(last["x_m"]) - float(first["x_m"]),
        float(last["y_m"]) - float(first["y_m"]),
    )
    yaw_change = abs(
        normalize_angle_deg(float(last.get("yaw_deg", 0.0)) - float(first.get("yaw_deg", 0.0)))
    )
    return {
        "pose_count": len(poses),
        "trajectory_path_length_m": path_length,
        "trajectory_net_displacement_m": net_displacement,
        "trajectory_yaw_change_abs_deg": yaw_change,
    }


def load_result_diagnostics(result_json: Any) -> dict[str, Any]:
    empty = {
        "decision_reasons": {},
        "motion_margin_rate": None,
        "low_motion_margin_rate": None,
        "best_score_rate": None,
        "pair_count": 0,
        "pose_count": 0,
        "trajectory_path_length_m": None,
        "trajectory_net_displacement_m": None,
        "trajectory_yaw_change_abs_deg": None,
    }
    if not result_json:
        return empty
    path = Path(str(result_json))
    if not path.exists():
        return empty
    payload = load_json(path)
    pairs = payload.get("pairs", [])
    counts: dict[str, int] = {}
    for pair in pairs:
        if "decision_reason" not in pair:
            continue
        reason = str(pair.get("decision_reason") or "n/a")
        counts[reason] = counts.get(reason, 0) + 1
    reason_count = sum(counts.values())
    if reason_count == 0:
        return {
            **trajectory_diagnostics(payload),
            "decision_reasons": counts,
            "motion_margin_rate": None,
            "low_motion_margin_rate": None,
            "best_score_rate": None,
            "pair_count": len(pairs),
        }
    return {
        **trajectory_diagnostics(payload),
        "decision_reasons": counts,
        "motion_margin_rate": counts.get("motion_margin", 0) / reason_count,
        "low_motion_margin_rate": counts.get("low_motion_margin", 0) / reason_count,
        "best_score_rate": counts.get("best_score", 0) / reason_count,
        "pair_count": len(pairs),
    }


def method_sort_key(method: str) -> tuple[int, str]:
    order = {"geometry_icp": 0, "intensity_bev": 1, "kiss_keyframe": 2, "ct_icp": 3}
    return (order.get(method, 99), method)


def expected_stress(sequence: str, window_name: str, obscurant_score: float) -> str:
    if window_name == "baseline":
        return "nominal"
    if "fog" in sequence and obscurant_score >= 0.5:
        return "obscurant_stress"
    if "point_count" in window_name:
        return "point_count_stress"
    if "geometry" in window_name:
        return "geometry_stress"
    if obscurant_score >= 0.2:
        return "degradation_stress"
    return "stress"


def is_stress_label(label: str) -> bool:
    return label != "nominal"


def health_state(row: dict[str, Any]) -> str:
    accepted = float(row.get("accepted_rate") or 0.0)
    converged = row.get("converged_rate")
    converged_value = None if converged is None else float(converged)
    flags = row.get("flags", "ok")
    active_flags = flag_set(flags)
    if active_flags.intersection({"nonfinite_pose", "all_pairs_failed"}) or accepted < 0.5:
        return "failed"
    if (
        active_flags.intersection(
            {
                "low_used_path",
                "motion_margin_dominant",
                "low_motion_margin_dominant",
                "overlap_tail",
            }
        )
        or accepted < 0.9
    ):
        return "suspicious"
    if diagnostic_watch_only(flags):
        return "diagnostic_watch"
    if "low_convergence" in active_flags or (
        converged_value is not None and converged_value < 0.5
    ):
        return "degraded"
    return "ok"


def failure_awareness(expected: str, state: str) -> str:
    if state == "diagnostic_watch":
        return "diagnostic_watch"
    stress = is_stress_label(expected)
    abnormal = state != "ok"
    if stress and abnormal:
        return "stress_flagged"
    if stress and not abnormal:
        return "stress_unflagged"
    if not stress and abnormal:
        return "false_alarm"
    return "nominal_ok"


def confidence_probe(row: dict[str, Any]) -> str:
    notes = []
    motion_margin_rate = row.get("motion_margin_rate")
    if motion_margin_rate is not None and float(motion_margin_rate) >= 0.5:
        notes.append("motion_margin_dominant")
    low_motion_margin_rate = row.get("low_motion_margin_rate")
    if low_motion_margin_rate is not None and float(low_motion_margin_rate) >= 0.5:
        notes.append("low_motion_margin_dominant")
    overlap_mean = row.get("overlap_mean")
    overlap_min = row.get("overlap_min")
    if overlap_mean is not None and overlap_min is not None:
        mean = float(overlap_mean)
        minimum = float(overlap_min)
        if mean > 0.0 and minimum / mean < 0.5:
            notes.append("overlap_tail")
    if row.get("failure_awareness") == "stress_unflagged":
        notes.append("needs_gt_or_cross_method_check")
    return ", ".join(notes) if notes else "none"


def ratio(value: Any, reference: Any) -> float | None:
    value_number = as_finite_float(value)
    reference_number = as_finite_float(reference)
    if value_number is None or reference_number is None or abs(reference_number) < 1e-6:
        return None
    return value_number / reference_number


def add_cross_method_consistency(windows: list[dict[str, Any]]) -> None:
    for window in windows:
        rows = window["methods"]
        all_paths = [
            value
            for row in rows.values()
            if (value := as_finite_float(row.get("trajectory_path_length_m"))) is not None
        ]
        all_path_median = median(all_paths)
        healthy_paths = [
            value
            for row in rows.values()
            if row.get("health_state") == "ok"
            if (value := as_finite_float(row.get("trajectory_path_length_m"))) is not None
        ]
        healthy_path_median = median(healthy_paths)
        for row in rows.values():
            row["all_method_path_median_m"] = all_path_median
            row["healthy_path_median_m"] = healthy_path_median
            row["path_vs_all_median"] = ratio(
                row.get("trajectory_path_length_m"), all_path_median
            )
            row["path_vs_healthy_median"] = ratio(
                row.get("trajectory_path_length_m"), healthy_path_median
            )
            row_path = as_finite_float(row.get("trajectory_path_length_m"))
            row["healthy_peer_count"] = max(
                len(healthy_paths) - (1 if row.get("health_state") == "ok" and row_path is not None else 0),
                0,
            )
            notes = []
            if row.get("failure_awareness") == "stress_unflagged":
                if int(row["healthy_peer_count"]) == 0:
                    notes.append("no_healthy_peer")
                healthy_ratio = as_finite_float(row.get("path_vs_healthy_median"))
                if healthy_ratio is not None and (healthy_ratio > 2.0 or healthy_ratio < 0.5):
                    notes.append("path_disagrees_with_healthy_median")
                all_ratio = as_finite_float(row.get("path_vs_all_median"))
                if all_ratio is not None and (all_ratio > 3.0 or all_ratio < 0.33):
                    notes.append("path_disagrees_with_all_method_median")
            row["cross_method_probe"] = ", ".join(notes) if notes else "none"


def is_cross_method_suspicious(row: dict[str, Any]) -> bool:
    probe = str(row.get("cross_method_probe") or "none")
    if "path_disagrees_with_healthy_median" in probe:
        return True
    return (
        "no_healthy_peer" in probe
        and "path_disagrees_with_all_method_median" in probe
    )


def append_flag(flags: str, flag: str) -> str:
    if flags == "ok":
        return flag
    active = [part.strip() for part in flags.split(",") if part.strip()]
    if flag not in active:
        active.append(flag)
    return ", ".join(active)


def policy_reasons(row: dict[str, Any]) -> list[str]:
    reasons: list[str] = []
    reasons.extend(split_csv_flags(row.get("flags")))
    accepted = as_finite_float(row.get("accepted_rate"))
    if accepted is not None and 0.5 <= accepted < 0.9:
        reasons.append("partial_acceptance")
    if row.get("cross_method_suspicious"):
        reasons.append("cross_method_suspicious")
    reasons.extend(
        reason
        for reason in split_csv_flags(row.get("cross_method_probe"))
        if reason.startswith("path_disagrees") or reason == "no_healthy_peer"
    )
    if not reasons:
        reasons.append("ok_no_risk")
    return sorted(dict.fromkeys(reasons))


def policy_decision(reasons: list[str]) -> str:
    decision = "pass"
    for reason in reasons:
        candidate = POLICY_BY_REASON.get(reason, POLICY_DEFAULT_DECISION)
        if POLICY_DECISION_ORDER[candidate] > POLICY_DECISION_ORDER[decision]:
            decision = candidate
    return decision


def product_local_risk(row: dict[str, Any]) -> bool:
    flags = str(row.get("flags") or "ok")
    return flags != "ok" and row.get("health_state") != "diagnostic_watch"


def ct_icp_watch_clear_status(row: dict[str, Any]) -> dict[str, Any]:
    blockers: list[str] = []
    if row.get("health_state") != "diagnostic_watch":
        return {
            "watch_clear_candidate": None,
            "watch_clear_blockers": blockers,
            "watch_action": None,
        }

    accepted = as_finite_float(row.get("accepted_rate"))
    if accepted is None or accepted < WATCH_CLEAR_ACCEPTANCE_MIN:
        blockers.append("accepted_below_one")

    refinement_gate = as_finite_float(row.get("ct_icp_refinement_gate_rate"))
    if refinement_gate is None or refinement_gate < WATCH_CLEAR_GATE_MIN:
        blockers.append("refinement_gate_below_one")

    iterations = as_finite_float(row.get("ct_icp_iterations_mean"))
    max_iterations = as_finite_float(row.get("ct_icp_max_iterations"))
    if iterations is None or max_iterations is None:
        blockers.append("iterations_unknown")
    elif iterations >= max_iterations - WATCH_CLEAR_ITERATION_MARGIN:
        blockers.append("iterations_pinned")

    path_vs_healthy = as_finite_float(row.get("path_vs_healthy_median"))
    if path_vs_healthy is None:
        blockers.append("path_vs_healthy_missing")
    elif path_vs_healthy > WATCH_CLEAR_PATH_HEALTHY_MAX:
        blockers.append("path_vs_healthy_high")
    elif path_vs_healthy < WATCH_CLEAR_PATH_HEALTHY_MIN:
        blockers.append("path_vs_healthy_low")

    path_vs_all = as_finite_float(row.get("path_vs_all_median"))
    if path_vs_all is None:
        blockers.append("path_vs_all_missing")
    elif path_vs_all > WATCH_CLEAR_PATH_ALL_MAX:
        blockers.append("path_vs_all_high")
    elif path_vs_all < WATCH_CLEAR_PATH_ALL_MIN:
        blockers.append("path_vs_all_low")

    return {
        "watch_clear_candidate": not blockers,
        "watch_clear_blockers": blockers,
        "watch_action": ct_icp_watch_action(blockers),
    }


def ct_icp_watch_action(blockers: list[str]) -> str:
    active = set(blockers)
    if not active:
        return "clear_candidate"
    if active.intersection(WATCH_REJECT_BLOCKERS):
        return "reject_or_retry"
    if any(
        blocker.startswith("path_vs_") and not blocker.endswith("_missing")
        for blocker in active
    ):
        return "fallback_required"
    if active.intersection(WATCH_ITERATION_BLOCKERS):
        return "optimizer_retry"
    if any(blocker.endswith("_missing") for blocker in active):
        return "reference_missing"
    return "investigate"


def add_cross_method_risk(windows: list[dict[str, Any]]) -> None:
    for window in windows:
        expected = window["expected_stress"]
        for row in window["methods"].values():
            cross_method_suspicious = is_cross_method_suspicious(row)
            row["cross_method_suspicious"] = cross_method_suspicious
            if row.get("health_state") == "ok" and cross_method_suspicious:
                row["risk_state"] = "cross_method_suspicious"
                row["risk_flags"] = append_flag(str(row.get("flags") or "ok"), "cross_method_suspicious")
            else:
                row["risk_state"] = row.get("health_state")
                row["risk_flags"] = row.get("flags")
            row["risk_failure_awareness"] = failure_awareness(expected, row["risk_state"])
            row["policy_reasons"] = policy_reasons(row)
            row["policy_decision"] = policy_decision(row["policy_reasons"])


def add_ct_icp_watch_clear_status(windows: list[dict[str, Any]]) -> None:
    for window in windows:
        for row in window["methods"].values():
            status = ct_icp_watch_clear_status(row)
            if status["watch_clear_candidate"] is not None:
                row.update(status)


def flatten_sequence(sequence: str, inputs: tuple[tuple[str, str], ...]) -> dict[str, Any]:
    methods: dict[str, dict[str, Any]] = {}
    windows: dict[str, dict[str, Any]] = {}
    for method, summary_path_str in inputs:
        summary_path = Path(summary_path_str)
        payload = load_json(summary_path)
        parameters = payload.get("parameters") or {}
        methods[method] = {
            "summary_json": str(summary_path),
            "method": payload.get("method", method),
            "sequence_pcd_dir": payload.get("sequence_pcd_dir"),
        }
        for row in payload["windows"]:
            window_key = f"{row['name']}:{row['window']['start']}-{row['window']['end']}"
            entry = windows.setdefault(
                window_key,
                {
                    "sequence": sequence,
                    "name": row["name"],
                    "start": int(row["window"]["start"]),
                    "end": int(row["window"]["end"]),
                    "frames": int(row["frames"]),
                    "obscurant_score": float(row["window"].get("obscurant_score", 0.0)),
                    "methods": {},
                },
            )
            expected = expected_stress(
                sequence,
                row["name"],
                float(row["window"].get("obscurant_score", 0.0)),
            )
            entry["expected_stress"] = expected
            method_row = {
                "accepted_rate": row.get("accepted_rate"),
                "converged_rate": row.get("converged_rate"),
                "score_mean": row.get("score_mean"),
                "score_min": row.get("score_min"),
                "overlap_mean": row.get("overlap_mean"),
                "overlap_min": row.get("overlap_min"),
                "ct_icp_max_iterations": parameters.get("ct_icp_max_iterations"),
                "ct_icp_refinement_gate_rate": row.get("ct_icp_refinement_gate_rate"),
                "ct_icp_iterations_mean": row.get("ct_icp_iterations_mean"),
                "used_path_length_m": row.get("used_path_length_m"),
                "used_step_max_m": row.get("used_step_max_m"),
                "keyframes": keyframe_text(row),
                "flags": health_label(row),
                "result_json": row.get("result_json"),
            }
            method_row.update(load_result_diagnostics(row.get("result_json")))
            method_row["health_state"] = health_state(method_row)
            method_row["failure_awareness"] = failure_awareness(
                expected, method_row["health_state"]
            )
            method_row["confidence_probe"] = confidence_probe(method_row)
            entry["methods"][method] = {
                **method_row,
            }
    sorted_windows = sorted(
        windows.values(),
        key=lambda row: (row["start"], row["end"], row["name"]),
    )
    add_cross_method_consistency(sorted_windows)
    add_ct_icp_watch_clear_status(sorted_windows)
    add_cross_method_risk(sorted_windows)
    return {
        "sequence": sequence,
        "methods": methods,
        "windows": sorted_windows,
    }


def summarize_sequence(sequence_payload: dict[str, Any]) -> dict[str, Any]:
    method_stats: dict[str, dict[str, Any]] = {}
    for window in sequence_payload["windows"]:
        for method, row in window["methods"].items():
            stats = method_stats.setdefault(
                method,
                {
                    "windows": 0,
                    "mean_acceptance": 0.0,
                    "mean_convergence": 0.0,
                    "_ct_icp_refinement_gate_rate_sum": 0.0,
                    "_ct_icp_refinement_gate_rate_count": 0,
                    "_ct_icp_iterations_sum": 0.0,
                    "_ct_icp_iterations_count": 0,
                    "failed_windows": 0,
                    "diagnostic_watch_windows": 0,
                    "local_risk_windows": 0,
                    "cross_method_suspicious_windows": 0,
                    "risk_windows": 0,
                    "stress_windows": 0,
                    "stress_flagged": 0,
                    "cross_method_flagged": 0,
                    "stress_unflagged": 0,
                    "residual_stress_unflagged": 0,
                    "false_alarms": 0,
                    "diagnostic_watch": 0,
                    "policy_pass": 0,
                    "policy_watch": 0,
                    "policy_investigate": 0,
                    "policy_fail": 0,
                    "min_acceptance": 1.0,
                    "max_used_path_length_m": 0.0,
                },
            )
            accepted = float(row["accepted_rate"] or 0.0)
            converged = row.get("converged_rate")
            converged_value = float(converged) if converged is not None else accepted
            stats["windows"] += 1
            stats["mean_acceptance"] += accepted
            stats["mean_convergence"] += converged_value
            ct_icp_refinement_gate_rate = as_finite_float(row.get("ct_icp_refinement_gate_rate"))
            if ct_icp_refinement_gate_rate is not None:
                stats["_ct_icp_refinement_gate_rate_sum"] += ct_icp_refinement_gate_rate
                stats["_ct_icp_refinement_gate_rate_count"] += 1
            ct_icp_iterations_mean = as_finite_float(row.get("ct_icp_iterations_mean"))
            if ct_icp_iterations_mean is not None:
                stats["_ct_icp_iterations_sum"] += ct_icp_iterations_mean
                stats["_ct_icp_iterations_count"] += 1
            stats["failed_windows"] += 1 if accepted < 0.5 else 0
            diagnostic_watch = row.get("health_state") == "diagnostic_watch"
            local_risk = product_local_risk(row)
            cross_method_suspicious = bool(row.get("cross_method_suspicious"))
            stats["diagnostic_watch_windows"] += 1 if diagnostic_watch else 0
            stats["local_risk_windows"] += 1 if local_risk else 0
            stats["cross_method_suspicious_windows"] += 1 if cross_method_suspicious else 0
            stats["risk_windows"] += 1 if local_risk or cross_method_suspicious else 0
            stats[f"policy_{row['policy_decision']}"] += 1
            expected = window["expected_stress"]
            awareness = row.get("failure_awareness")
            if is_stress_label(expected):
                stats["stress_windows"] += 1
            if awareness == "stress_flagged":
                stats["stress_flagged"] += 1
            elif awareness == "stress_unflagged":
                stats["stress_unflagged"] += 1
                if cross_method_suspicious:
                    stats["cross_method_flagged"] += 1
                else:
                    stats["residual_stress_unflagged"] += 1
            elif awareness == "false_alarm":
                stats["false_alarms"] += 1
            elif awareness == "diagnostic_watch":
                stats["diagnostic_watch"] += 1
            stats["min_acceptance"] = min(stats["min_acceptance"], accepted)
            stats["max_used_path_length_m"] = max(
                stats["max_used_path_length_m"],
                float(row.get("used_path_length_m") or 0.0),
            )
    for stats in method_stats.values():
        n = max(int(stats["windows"]), 1)
        stats["mean_acceptance"] /= n
        stats["mean_convergence"] /= n
        ct_gate_count = int(stats.pop("_ct_icp_refinement_gate_rate_count"))
        ct_gate_sum = float(stats.pop("_ct_icp_refinement_gate_rate_sum"))
        stats["ct_icp_refinement_gate_rate_mean"] = (
            ct_gate_sum / ct_gate_count if ct_gate_count else None
        )
        ct_iterations_count = int(stats.pop("_ct_icp_iterations_count"))
        ct_iterations_sum = float(stats.pop("_ct_icp_iterations_sum"))
        stats["ct_icp_iterations_mean"] = (
            ct_iterations_sum / ct_iterations_count if ct_iterations_count else None
        )
    return method_stats


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    lines = [
        "# LiDAR Degeneracy Method Health Comparison",
        "",
        f"Policy: `{payload['policy']['policy_version']}` "
        f"(`{payload['policy']['path']}`)",
        "",
        "## Method Aggregate",
        "",
        "| Sequence | Method | Windows | Mean accepted | Min accepted | Mean converged | CT gate | CT iter | Failed windows | Diagnostic watch | Local risk | Cross-method risk | Total risk | Pass | Watch | Investigate | Fail | Max used path m |",
        "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for sequence in payload["sequences"]:
        aggregates = payload["aggregates"][sequence["sequence"]]
        for method in sorted(aggregates, key=method_sort_key):
            row = aggregates[method]
            lines.append(
                f"| `{sequence['sequence']}` | `{method}` | {row['windows']} | "
                f"{fmt(row['mean_acceptance'])} | {fmt(row['min_acceptance'])} | "
                f"{fmt(row['mean_convergence'])} | "
                f"{fmt(row['ct_icp_refinement_gate_rate_mean'])} | "
                f"{fmt(row['ct_icp_iterations_mean'])} | "
                f"{row['failed_windows']} | "
                f"{row['diagnostic_watch_windows']} | "
                f"{row['local_risk_windows']} | "
                f"{row['cross_method_suspicious_windows']} | "
                f"{row['risk_windows']} | "
                f"{row['policy_pass']} | {row['policy_watch']} | "
                f"{row['policy_investigate']} | {row['policy_fail']} | "
                f"{fmt(row['max_used_path_length_m'])} |"
            )

    lines.extend(
        [
            "",
            "## Failure Awareness",
            "",
            "| Sequence | Method | Stress windows | Local flagged | Cross-method flagged | Residual unflagged | False alarms | Diagnostic watch |",
            "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |",
        ]
    )
    for sequence in payload["sequences"]:
        aggregates = payload["aggregates"][sequence["sequence"]]
        for method in sorted(aggregates, key=method_sort_key):
            row = aggregates[method]
            lines.append(
                f"| `{sequence['sequence']}` | `{method}` | "
                f"{row['stress_windows']} | {row['stress_flagged']} | "
                f"{row['cross_method_flagged']} | "
                f"{row['residual_stress_unflagged']} | {row['false_alarms']} | "
                f"{row['diagnostic_watch']} |"
            )

    lines.extend(
        [
            "",
            "## Confidence Probes",
            "",
            "| Sequence | Window | Method | Accepted | Score min | Overlap min | Motion-margin rate | Decision reasons | Probe |",
            "| --- | --- | --- | ---: | ---: | ---: | ---: | --- | --- |",
        ]
    )
    for sequence in payload["sequences"]:
        for window in sequence["windows"]:
            for method in sorted(window["methods"], key=method_sort_key):
                row = window["methods"][method]
                if row["failure_awareness"] != "stress_unflagged":
                    continue
                lines.append(
                    f"| `{sequence['sequence']}` | `{window['name']}` "
                    f"{window['start']}-{window['end']} | `{method}` | "
                    f"{fmt(row['accepted_rate'])} | {fmt(row['score_min'])} | "
                    f"{fmt(row['overlap_min'], 1)} | {fmt(row['motion_margin_rate'])} | "
                    f"{compact_counts(row['decision_reasons'])} | "
                    f"{row['confidence_probe']} |"
                )

    lines.extend(
        [
            "",
            "## Cross-Method Consistency",
            "",
            "| Sequence | Window | Method | Path m | Net m | Yaw deg | Healthy peers | Healthy median path m | Path/healthy | All median path m | Path/all | Cross risk | Probe |",
            "| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |",
        ]
    )
    for sequence in payload["sequences"]:
        for window in sequence["windows"]:
            for method in sorted(window["methods"], key=method_sort_key):
                row = window["methods"][method]
                if row["failure_awareness"] != "stress_unflagged":
                    continue
                lines.append(
                    f"| `{sequence['sequence']}` | `{window['name']}` "
                    f"{window['start']}-{window['end']} | `{method}` | "
                    f"{fmt(row['trajectory_path_length_m'])} | "
                    f"{fmt(row['trajectory_net_displacement_m'])} | "
                    f"{fmt(row['trajectory_yaw_change_abs_deg'])} | "
                    f"{row['healthy_peer_count']} | "
                    f"{fmt(row['healthy_path_median_m'])} | "
                    f"{fmt(row['path_vs_healthy_median'])} | "
                    f"{fmt(row['all_method_path_median_m'])} | "
                    f"{fmt(row['path_vs_all_median'])} | "
                    f"{'yes' if row['cross_method_suspicious'] else 'no'} | "
                    f"{row['cross_method_probe']} |"
                )

    lines.extend(
        [
            "",
            "## Diagnostic Watch Rows",
            "",
            "| Sequence | Window | Expected | Method | Accepted | Converged | CT gate | CT iter | Path/healthy | Path/all | Clear? | Action | Blockers | Policy reasons |",
            "| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- | --- |",
        ]
    )
    diagnostic_rows = 0
    for sequence in payload["sequences"]:
        for window in sequence["windows"]:
            for method in sorted(window["methods"], key=method_sort_key):
                row = window["methods"][method]
                if row["health_state"] != "diagnostic_watch":
                    continue
                diagnostic_rows += 1
                clear = row.get("watch_clear_candidate")
                clear_text = "yes" if clear is True else "no" if clear is False else "n/a"
                action = row.get("watch_action") or "n/a"
                blockers = row.get("watch_clear_blockers") or []
                blocker_text = ", ".join(str(blocker) for blocker in blockers) if blockers else "none"
                lines.append(
                    f"| `{sequence['sequence']}` | `{window['name']}` "
                    f"{window['start']}-{window['end']} | `{window['expected_stress']}` | "
                    f"`{method}` | {fmt(row['accepted_rate'])} | "
                    f"{fmt(row['converged_rate'])} | "
                    f"{fmt(row['ct_icp_refinement_gate_rate'])} | "
                    f"{fmt(row['ct_icp_iterations_mean'])} | "
                    f"{fmt(row['path_vs_healthy_median'])} | "
                    f"{fmt(row['path_vs_all_median'])} | "
                    f"{clear_text} | `{action}` | {blocker_text} | "
                    f"{', '.join(row['policy_reasons'])} |"
                )
    if diagnostic_rows == 0:
        lines.append("| n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a |")

    lines.extend(
        [
            "",
            "## Window Detail",
            "",
            "| Sequence | Window | Expected | Frames | Obscurant | Method | Accepted | Converged | CT gate | CT iter | Score | Overlap | Used path m | Max step m | State | Risk state | Policy | Failure awareness | Risk awareness | Keyframes | Flags | Risk flags | Policy reasons |",
            "| --- | --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- | --- | --- | --- | --- | --- | --- |",
        ]
    )
    for sequence in payload["sequences"]:
        for window in sequence["windows"]:
            for method in sorted(window["methods"], key=method_sort_key):
                row = window["methods"][method]
                lines.append(
                    f"| `{sequence['sequence']}` | `{window['name']}` "
                    f"{window['start']}-{window['end']} | `{window['expected_stress']}` | "
                    f"{window['frames']} | {fmt(window['obscurant_score'])} | `{method}` | "
                    f"{fmt(row['accepted_rate'])} | {fmt(row['converged_rate'])} | "
                    f"{fmt(row['ct_icp_refinement_gate_rate'])} | "
                    f"{fmt(row['ct_icp_iterations_mean'])} | "
                    f"{fmt(row['score_mean'])} | {fmt(row['overlap_mean'], 1)} | "
                    f"{fmt(row['used_path_length_m'])} | {fmt(row['used_step_max_m'])} | "
                    f"`{row['health_state']}` | `{row['risk_state']}` | "
                    f"`{row['policy_decision']}` | "
                    f"`{row['failure_awareness']}` | `{row['risk_failure_awareness']}` | "
                    f"{row['keyframes']} | {row['flags']} | {row['risk_flags']} | "
                    f"{', '.join(row['policy_reasons'])} |"
                )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- `fog_200`: intensity BEV keeps 100% acceptance on selected windows, while the stress-only low-motion regularizer downgrades ambiguous stress trajectories instead of letting weak score gains accumulate into accepted drift.",
            "- `fog_200`: geometry ICP and KISS keyframe now accept every selected short window after the fog/crop correspondence gates were relaxed; CT-ICP keeps baseline/tail accepted but drops on strongest fog.",
            "- Failure-awareness columns are heuristic because this dataset layer has no GT: `stress_unflagged` means a stress window stayed externally healthy, not necessarily that the estimate is wrong.",
            "- Intensity BEV false-confidence gates now promote dominant motion-margin decisions, dominant low-motion regularization, and sharp overlap tails to suspicious health flags on non-baseline selected windows.",
            "- Confidence probes expose stress-unflagged windows that need a GT or cross-method check, especially when score-margin decisions dominate or overlap has a sharp tail.",
            "- Cross-method consistency now contributes to total risk when a stress-unflagged trajectory disagrees with healthy-peer or all-method path medians.",
            "- Policy decisions are GT-free triage labels: `fail` for hard local failure, `investigate` for unresolved cross-method disagreement, `watch` for calibrated local confidence downgrades and medium-risk rows, and `pass` for no active risk reason.",
            "- `tunnel_geom_2700_200`: the short-window checks stay accepted, so this slice is not yet a local-odometry failure case.",
            "- CT-ICP internal convergence watch is reported as `diagnostic_watch` when it is the only active signal; accepted refinements with no product-health downgrade no longer count as false alarms.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    sequences = [flatten_sequence(name, inputs) for name, inputs in DEFAULT_INPUTS]
    aggregates = {
        sequence["sequence"]: summarize_sequence(sequence) for sequence in sequences
    }
    payload = {
        "inputs": {
            name: {method: path for method, path in inputs}
            for name, inputs in DEFAULT_INPUTS
        },
        "policy": {
            "path": str(DEFAULT_POLICY_PATH.relative_to(REPO_ROOT)),
            "schema_version": POLICY.get("schema_version"),
            "policy_version": POLICY_VERSION,
            "decision_order": POLICY_DECISION_ORDER,
            "reason_decisions": POLICY_BY_REASON,
        },
        "sequences": sequences,
        "aggregates": aggregates,
    }
    output_json = args.output_dir / "method_health_comparison.json"
    output_md = args.output_dir / "method_health_comparison.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {output_json} and {output_md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
