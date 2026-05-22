#!/usr/bin/env python3
"""Calibrate LiDAR degeneracy risk flags against GT when available."""

from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path
from typing import Any

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_POLICY_PATH = REPO_ROOT / "evaluation" / "config" / "lidar_degeneracy_triage_policy.json"
DEFAULT_COMPARISON = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "method_health_comparison"
    / "method_health_comparison.json"
)
DEFAULT_OUTPUT_DIR = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "risk_gt_calibration"
)
DEFAULT_TOPIC_AUDITS = {
    "fog_200": REPO_ROOT / "data" / "lidar_degeneracy_datasets" / "fog_topics.json",
    "tunnel_geom_2700_200": REPO_ROOT / "data" / "lidar_degeneracy_datasets" / "tunnel_topics.json",
}


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


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison-json", type=Path, default=DEFAULT_COMPARISON)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument(
        "--gt-csv",
        action="append",
        default=[],
        metavar="SEQUENCE=PATH",
        help="Optional GT CSV for a sequence. Repeat for multiple sequences.",
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


def parse_gt_map(items: list[str]) -> dict[str, Path]:
    out: dict[str, Path] = {}
    for item in items:
        if "=" not in item:
            raise ValueError(f"--gt-csv must be SEQUENCE=PATH, got {item!r}")
        sequence, path = item.split("=", 1)
        gt_path = Path(path)
        if not gt_path.is_absolute():
            gt_path = REPO_ROOT / gt_path
        out[sequence] = gt_path
    return out


def quat_to_matrix(roll: float, pitch: float, yaw: float) -> np.ndarray:
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    rx = np.array([[1.0, 0.0, 0.0], [0.0, cr, -sr], [0.0, sr, cr]])
    ry = np.array([[cp, 0.0, sp], [0.0, 1.0, 0.0], [-sp, 0.0, cp]])
    rz = np.array([[cy, -sy, 0.0], [sy, cy, 0.0], [0.0, 0.0, 1.0]])
    return rz @ ry @ rx


def load_gt_xyyaw(path: Path) -> np.ndarray:
    with path.open(newline="") as handle:
        rows = list(csv.DictReader(handle))
    mats = []
    for row in rows:
        mat = np.eye(4)
        mat[:3, :3] = quat_to_matrix(
            float(row["lidar_pose.roll"]),
            float(row["lidar_pose.pitch"]),
            float(row["lidar_pose.yaw"]),
        )
        mat[:3, 3] = [
            float(row["lidar_pose.x"]),
            float(row["lidar_pose.y"]),
            float(row["lidar_pose.z"]),
        ]
        mats.append(mat)
    poses = []
    for mat in mats:
        poses.append([mat[0, 3], mat[1, 3], math.atan2(mat[1, 0], mat[0, 0])])
    return np.asarray(poses, dtype=np.float64)


def se2_matrix(x: float, y: float, yaw: float) -> np.ndarray:
    c = math.cos(yaw)
    s = math.sin(yaw)
    return np.array([[c, -s, x], [s, c, y], [0.0, 0.0, 1.0]], dtype=np.float64)


def gt_relative_trajectory(gt: np.ndarray) -> np.ndarray:
    mats = [se2_matrix(float(x), float(y), float(yaw)) for x, y, yaw in gt]
    anchor_inv = np.linalg.inv(mats[0])
    rel = [anchor_inv @ mat for mat in mats]
    return np.asarray([[mat[0, 2], mat[1, 2], math.atan2(mat[1, 0], mat[0, 0])] for mat in rel])


def load_result_poses(path: Path) -> np.ndarray:
    payload = load_json(path)
    poses = payload.get("poses_xyyaw") or []
    if not poses:
        return np.zeros((0, 3), dtype=np.float64)
    if isinstance(poses[0], dict):
        return np.asarray(
            [
                [
                    float(pose.get("x_m", 0.0)),
                    float(pose.get("y_m", 0.0)),
                    math.radians(float(pose.get("yaw_deg", 0.0))),
                ]
                for pose in poses
            ],
            dtype=np.float64,
        )
    return np.asarray(poses, dtype=np.float64)


def compute_metrics(poses: np.ndarray, gt_window: np.ndarray) -> dict[str, float] | None:
    if poses.shape[0] == 0 or gt_window.shape[0] < poses.shape[0]:
        return None
    gt_rel = gt_relative_trajectory(gt_window[: poses.shape[0]])
    errors = np.linalg.norm(poses[:, :2] - gt_rel[:, :2], axis=1)
    pred_steps = np.linalg.norm(np.diff(poses[:, :2], axis=0), axis=1)
    gt_steps = np.linalg.norm(np.diff(gt_rel[:, :2], axis=0), axis=1)
    step_rmse = float(np.sqrt(np.mean((pred_steps - gt_steps) ** 2))) if len(gt_steps) else 0.0
    return {
        "ate_xy_m": float(np.sqrt(np.mean(errors * errors))),
        "mean_frame_error_m": float(np.mean(errors)),
        "max_frame_error_m": float(np.max(errors)),
        "step_length_rmse_m": step_rmse,
        "gt_path_length_m": float(np.sum(gt_steps)) if len(gt_steps) else 0.0,
    }


def pose_like_topics(topic_audit_path: Path) -> list[str]:
    if not topic_audit_path.exists():
        return []
    payload = load_json(topic_audit_path)
    out = []
    for topic in payload.get("topics", []):
        name = str(topic.get("topic", ""))
        msgtype = str(topic.get("msgtype", ""))
        searchable = f"{name} {msgtype}".lower()
        if any(token in searchable for token in ("pose", "odom", "/tf", "ground", "mocap")):
            out.append(name)
    return out


def finite_mean(values: list[float]) -> float | None:
    finite = [value for value in values if math.isfinite(value)]
    return float(np.mean(finite)) if finite else None


def split_csv_flags(value: Any) -> list[str]:
    text = str(value or "ok")
    if text == "ok" or text == "none":
        return []
    return [part.strip() for part in text.split(",") if part.strip()]


def risk_bucket(row: dict[str, Any]) -> str:
    if row.get("risk_state") == "cross_method_suspicious":
        return "cross_method_suspicious"
    if row.get("health_state") != "ok":
        return "local_risk"
    return "ok"


def risk_reasons(record: dict[str, Any]) -> list[str]:
    reasons: list[str] = []
    reasons.extend(split_csv_flags(record.get("flags")))
    accepted = record.get("accepted_rate")
    if accepted is not None and 0.5 <= float(accepted) < 0.9:
        reasons.append("partial_acceptance")
    if record.get("cross_method_suspicious"):
        reasons.append("cross_method_suspicious")
    reasons.extend(
        reason
        for reason in split_csv_flags(record.get("cross_method_probe"))
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


def policy_reason_summary(reasons: list[str]) -> str:
    parts = []
    for reason in reasons:
        parts.append(f"{reason}:{POLICY_BY_REASON.get(reason, POLICY_DEFAULT_DECISION)}")
    return ", ".join(parts)


def append_proxy_stats(records: list[dict[str, Any]], *, stress_only: bool) -> dict[str, dict[str, Any]]:
    if stress_only:
        records = [record for record in records if record.get("is_stress")]
    buckets: dict[str, dict[str, Any]] = {}
    for record in records:
        bucket = buckets.setdefault(
            str(record["risk_bucket"]),
            {
                "windows": 0,
                "mean_path_vs_healthy": None,
                "mean_path_vs_all": None,
                "mean_path_m": None,
                "max_path_vs_healthy": None,
                "max_path_vs_all": None,
            },
        )
        bucket["windows"] += 1
    for name, bucket in buckets.items():
        rows = [record for record in records if record["risk_bucket"] == name]
        healthy_ratios = [
            float(record["path_vs_healthy_median"])
            for record in rows
            if record.get("path_vs_healthy_median") is not None
        ]
        all_ratios = [
            float(record["path_vs_all_median"])
            for record in rows
            if record.get("path_vs_all_median") is not None
        ]
        paths = [
            float(record["trajectory_path_length_m"])
            for record in rows
            if record.get("trajectory_path_length_m") is not None
        ]
        bucket["mean_path_vs_healthy"] = finite_mean(healthy_ratios)
        bucket["mean_path_vs_all"] = finite_mean(all_ratios)
        bucket["mean_path_m"] = finite_mean(paths)
        bucket["max_path_vs_healthy"] = max(healthy_ratios) if healthy_ratios else None
        bucket["max_path_vs_all"] = max(all_ratios) if all_ratios else None
    return buckets


def policy_stats(records: list[dict[str, Any]], *, stress_only: bool) -> dict[str, dict[str, Any]]:
    if stress_only:
        records = [record for record in records if record.get("is_stress")]
    out: dict[str, dict[str, Any]] = {}
    for decision in POLICY_DECISION_ORDER:
        out[decision] = {
            "rows": 0,
            "methods": [],
            "mean_path_vs_healthy": None,
            "mean_path_vs_all": None,
            "mean_acceptance": None,
            "mean_convergence": None,
        }
    for decision in POLICY_DECISION_ORDER:
        rows = [record for record in records if record.get("policy_decision") == decision]
        out[decision]["rows"] = len(rows)
        out[decision]["methods"] = sorted({str(row["method"]) for row in rows})
        out[decision]["mean_path_vs_healthy"] = finite_mean(
            [
                float(row["path_vs_healthy_median"])
                for row in rows
                if row.get("path_vs_healthy_median") is not None
            ]
        )
        out[decision]["mean_path_vs_all"] = finite_mean(
            [
                float(row["path_vs_all_median"])
                for row in rows
                if row.get("path_vs_all_median") is not None
            ]
        )
        out[decision]["mean_acceptance"] = finite_mean(
            [
                float(row["accepted_rate"])
                for row in rows
                if row.get("accepted_rate") is not None
            ]
        )
        out[decision]["mean_convergence"] = finite_mean(
            [
                float(row["converged_rate"])
                for row in rows
                if row.get("converged_rate") is not None
            ]
        )
    return out


def reason_drilldown(records: list[dict[str, Any]], *, stress_only: bool) -> dict[str, dict[str, Any]]:
    if stress_only:
        records = [record for record in records if record.get("is_stress")]
    by_reason: dict[str, list[dict[str, Any]]] = {}
    for record in records:
        for reason in record["risk_reasons"]:
            by_reason.setdefault(reason, []).append(record)

    out: dict[str, dict[str, Any]] = {}
    for reason, rows in by_reason.items():
        accepted = [
            float(row["accepted_rate"])
            for row in rows
            if row.get("accepted_rate") is not None
        ]
        converged = [
            float(row["converged_rate"])
            for row in rows
            if row.get("converged_rate") is not None
        ]
        paths = [
            float(row["trajectory_path_length_m"])
            for row in rows
            if row.get("trajectory_path_length_m") is not None
        ]
        healthy_ratios = [
            float(row["path_vs_healthy_median"])
            for row in rows
            if row.get("path_vs_healthy_median") is not None
        ]
        all_ratios = [
            float(row["path_vs_all_median"])
            for row in rows
            if row.get("path_vs_all_median") is not None
        ]
        out[reason] = {
            "rows": len(rows),
            "policy_decision": policy_decision([reason]),
            "methods": sorted({str(row["method"]) for row in rows}),
            "risk_buckets": sorted({str(row["risk_bucket"]) for row in rows}),
            "mean_acceptance": finite_mean(accepted),
            "mean_convergence": finite_mean(converged),
            "mean_path_m": finite_mean(paths),
            "mean_path_vs_healthy": finite_mean(healthy_ratios),
            "max_path_vs_healthy": max(healthy_ratios) if healthy_ratios else None,
            "mean_path_vs_all": finite_mean(all_ratios),
            "max_path_vs_all": max(all_ratios) if all_ratios else None,
        }
    return out


def build_calibration(comparison: dict[str, Any], gt_map: dict[str, Path]) -> dict[str, Any]:
    gt_cache: dict[str, np.ndarray] = {}
    records: list[dict[str, Any]] = []
    gt_rows: list[dict[str, Any]] = []
    sequence_gt_status: dict[str, dict[str, Any]] = {}

    for sequence in comparison["sequences"]:
        sequence_name = str(sequence["sequence"])
        gt_path = gt_map.get(sequence_name)
        topic_audit_path = DEFAULT_TOPIC_AUDITS.get(sequence_name)
        sequence_gt_status[sequence_name] = {
            "gt_csv": str(gt_path) if gt_path else None,
            "gt_csv_exists": bool(gt_path and gt_path.exists()),
            "topic_audit": str(topic_audit_path) if topic_audit_path else None,
            "pose_like_topics": pose_like_topics(topic_audit_path) if topic_audit_path else [],
        }
        if gt_path and gt_path.exists():
            gt_cache[sequence_name] = load_gt_xyyaw(gt_path)
        for window in sequence["windows"]:
            for method, row in window["methods"].items():
                record = {
                    "sequence": sequence_name,
                    "window": window["name"],
                    "start": int(window["start"]),
                    "end": int(window["end"]),
                    "expected_stress": window.get("expected_stress"),
                    "is_stress": window.get("expected_stress") != "nominal",
                    "method": method,
                    "accepted_rate": row.get("accepted_rate"),
                    "converged_rate": row.get("converged_rate"),
                    "health_state": row.get("health_state"),
                    "risk_state": row.get("risk_state"),
                    "risk_bucket": risk_bucket(row),
                    "flags": row.get("flags"),
                    "risk_flags": row.get("risk_flags"),
                    "cross_method_probe": row.get("cross_method_probe"),
                    "confidence_probe": row.get("confidence_probe"),
                    "failure_awareness": row.get("failure_awareness"),
                    "risk_failure_awareness": row.get("risk_failure_awareness"),
                    "cross_method_suspicious": bool(row.get("cross_method_suspicious")),
                    "trajectory_path_length_m": row.get("trajectory_path_length_m"),
                    "path_vs_healthy_median": row.get("path_vs_healthy_median"),
                    "path_vs_all_median": row.get("path_vs_all_median"),
                    "result_json": row.get("result_json"),
                }
                record["risk_reasons"] = risk_reasons(record)
                record["policy_decision"] = policy_decision(record["risk_reasons"])
                record["policy_reasons"] = policy_reason_summary(record["risk_reasons"])
                records.append(record)
                if sequence_name not in gt_cache:
                    continue
                result_json = row.get("result_json")
                if not result_json:
                    continue
                result_path = Path(str(result_json))
                if not result_path.is_absolute():
                    result_path = REPO_ROOT / result_path
                poses = load_result_poses(result_path)
                start = int(window["start"])
                gt_window = gt_cache[sequence_name][start : start + poses.shape[0]]
                metrics = compute_metrics(poses, gt_window)
                if metrics is None:
                    continue
                gt_rows.append({**record, **metrics})

    return {
        "comparison_json": str(DEFAULT_COMPARISON),
        "sequence_gt_status": sequence_gt_status,
        "gt_metric_rows": gt_rows,
        "proxy_records": records,
        "proxy_bucket_stats_stress": append_proxy_stats(records, stress_only=True),
        "proxy_bucket_stats_all": append_proxy_stats(records, stress_only=False),
        "policy": {
            "path": str(DEFAULT_POLICY_PATH.relative_to(REPO_ROOT)),
            "schema_version": POLICY.get("schema_version"),
            "policy_version": POLICY_VERSION,
            "decision_order": POLICY_DECISION_ORDER,
            "reason_decisions": POLICY_BY_REASON,
        },
        "policy_stats_stress": policy_stats(records, stress_only=True),
        "policy_stats_all": policy_stats(records, stress_only=False),
        "reason_drilldown_stress": reason_drilldown(records, stress_only=True),
        "reason_drilldown_all": reason_drilldown(records, stress_only=False),
    }


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    lines = [
        "# LiDAR Degeneracy Risk GT Calibration",
        "",
        f"Policy: `{payload['policy']['policy_version']}` "
        f"(`{payload['policy']['path']}`)",
        "",
        "## GT Availability",
        "",
        "| Sequence | GT CSV | Exists | Pose-like bag topics |",
        "| --- | --- | --- | --- |",
    ]
    for sequence, row in payload["sequence_gt_status"].items():
        pose_topics = row["pose_like_topics"]
        lines.append(
            f"| `{sequence}` | {row['gt_csv'] or 'n/a'} | "
            f"{'yes' if row['gt_csv_exists'] else 'no'} | "
            f"{', '.join(f'`{topic}`' for topic in pose_topics) if pose_topics else 'none'} |"
        )

    gt_rows = payload["gt_metric_rows"]
    lines.extend(["", "## GT Metrics", ""])
    if not gt_rows:
        lines.extend(
            [
                "No GT-backed rows were produced. The downloaded NTNU LiDAR degeneracy bags expose LiDAR packets, IMU, trigger topics, and radar PointCloud2, but no pose/odom/tf topic in the current topic audits.",
                "",
                "Pass `--gt-csv sequence=path/to/gt.csv` to compute window ATE and step RMSE when an external reference becomes available.",
            ]
        )
    else:
        lines.extend(
            [
                "| Sequence | Window | Method | Risk bucket | ATE m | Mean err m | Max err m | Step RMSE m | GT path m |",
                "| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: |",
            ]
        )
        for row in gt_rows:
            lines.append(
                f"| `{row['sequence']}` | `{row['window']}` {row['start']}-{row['end']} | "
                f"`{row['method']}` | `{row['risk_bucket']}` | "
                f"{fmt(row['ate_xy_m'])} | {fmt(row['mean_frame_error_m'])} | "
                f"{fmt(row['max_frame_error_m'])} | {fmt(row['step_length_rmse_m'])} | "
                f"{fmt(row['gt_path_length_m'])} |"
            )

    lines.extend(
        [
            "",
            "## Proxy Calibration",
            "",
            "Stress-window rows only.",
            "",
            "| Risk bucket | Windows | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |",
            "| --- | ---: | ---: | ---: | ---: | ---: | ---: |",
        ]
    )
    for bucket, row in sorted(payload["proxy_bucket_stats_stress"].items()):
        lines.append(
            f"| `{bucket}` | {row['windows']} | {fmt(row['mean_path_m'])} | "
            f"{fmt(row['mean_path_vs_healthy'])} | {fmt(row['max_path_vs_healthy'])} | "
            f"{fmt(row['mean_path_vs_all'])} | {fmt(row['max_path_vs_all'])} |"
        )

    lines.extend(
        [
            "",
            "## Policy Decisions",
            "",
            "Stress-window rows only.",
            "",
            "| Decision | Rows | Methods | Mean accepted | Mean converged | Mean path/healthy | Mean path/all |",
            "| --- | ---: | --- | ---: | ---: | ---: | ---: |",
        ]
    )
    for decision in POLICY_DECISION_ORDER:
        row = payload["policy_stats_stress"][decision]
        lines.append(
            f"| `{decision}` | {row['rows']} | "
            f"{', '.join(f'`{method}`' for method in row['methods']) if row['methods'] else 'n/a'} | "
            f"{fmt(row['mean_acceptance'])} | {fmt(row['mean_convergence'])} | "
            f"{fmt(row['mean_path_vs_healthy'])} | {fmt(row['mean_path_vs_all'])} |"
        )

    lines.extend(
        [
            "",
            "## Reason Drilldown",
            "",
            "Stress-window rows only. Rows with multiple active signals appear under each reason.",
            "",
            "| Reason | Policy | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |",
            "| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        ]
    )
    for reason, row in sorted(
        payload["reason_drilldown_stress"].items(),
        key=lambda item: (
            -float(item[1]["mean_path_vs_healthy"] or 0.0),
            -float(item[1]["mean_path_vs_all"] or 0.0),
            item[0],
        ),
    ):
        lines.append(
            f"| `{reason}` | `{row['policy_decision']}` | {row['rows']} | "
            f"{', '.join(f'`{method}`' for method in row['methods'])} | "
            f"{', '.join(f'`{bucket}`' for bucket in row['risk_buckets'])} | "
            f"{fmt(row['mean_acceptance'])} | {fmt(row['mean_convergence'])} | "
            f"{fmt(row['mean_path_m'])} | {fmt(row['mean_path_vs_healthy'])} | "
            f"{fmt(row['max_path_vs_healthy'])} | {fmt(row['mean_path_vs_all'])} | "
            f"{fmt(row['max_path_vs_all'])} |"
        )

    lines.extend(
        [
            "",
            "## Policy Detail",
            "",
            "Stress-window rows only.",
            "",
            "| Sequence | Window | Method | Decision | Risk bucket | Accepted | Converged | Path/healthy | Path/all | Reasons |",
            "| --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |",
        ]
    )
    stress_records = [
        record for record in payload["proxy_records"] if record.get("is_stress")
    ]
    for record in sorted(
        stress_records,
        key=lambda row: (
            -POLICY_DECISION_ORDER[str(row["policy_decision"])],
            str(row["sequence"]),
            int(row["start"]),
            str(row["method"]),
        ),
    ):
        lines.append(
            f"| `{record['sequence']}` | `{record['window']}` {record['start']}-{record['end']} | "
            f"`{record['method']}` | `{record['policy_decision']}` | "
            f"`{record['risk_bucket']}` | {fmt(record['accepted_rate'])} | "
            f"{fmt(record['converged_rate'])} | {fmt(record['path_vs_healthy_median'])} | "
            f"{fmt(record['path_vs_all_median'])} | {', '.join(f'`{reason}`' for reason in record['risk_reasons'])} |"
        )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- GT calibration is currently blocked by missing public pose/odom/tf for the local NTNU LiDAR degeneracy extraction.",
            "- The comparison remains GT-free for now: local risk and cross-method risk should be treated as triage signals, not error labels.",
            "- Policy decisions are pre-GT triage labels: `fail` for hard local failure, `investigate` for strong false-confidence signals, `watch` for medium risk, and `pass` for rows with no active risk reason.",
            "- Reason drilldown separates local failure signals from cross-method disagreement so the strongest triage signals can be checked first when GT arrives.",
            "- The script is ready to rerun with external GT via `--gt-csv fog_200=... --gt-csv tunnel_geom_2700_200=...`.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    comparison = load_json(args.comparison_json)
    payload = build_calibration(comparison, parse_gt_map(args.gt_csv))
    payload["comparison_json"] = str(args.comparison_json)

    output_json = args.output_dir / "risk_gt_calibration.json"
    output_md = args.output_dir / "risk_gt_calibration.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {output_json} and {output_md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
