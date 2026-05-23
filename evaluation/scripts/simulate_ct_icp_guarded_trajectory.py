#!/usr/bin/env python3
"""Simulate CT-ICP guarded trajectories from GT-free guard decisions."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COMPARISON_JSON = (
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
    / "ct_icp_guarded_trajectory"
)

USE_REFINED = "use_refined"
VELOCITY_PRIOR_DECISIONS = {"fallback_to_prior", "retry_optimizer", "reject_or_retry"}
PATH_HEALTHY_MIN = 0.5
PATH_HEALTHY_MAX = 2.0
PATH_ALL_MIN = 0.33
PATH_ALL_MAX = 3.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison-json", type=Path, default=DEFAULT_COMPARISON_JSON)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def display_path(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def fmt(value: Any, digits: int = 3) -> str:
    number = as_float(value)
    if number is None:
        return "n/a"
    return f"{number:.{digits}f}"


def as_float(value: Any) -> float | None:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def ratio(value: Any, reference: Any) -> float | None:
    value_number = as_float(value)
    reference_number = as_float(reference)
    if value_number is None or reference_number is None or abs(reference_number) < 1e-9:
        return None
    return value_number / reference_number


def normalize_angle_deg(angle_deg: float) -> float:
    return (angle_deg + 180.0) % 360.0 - 180.0


def step_from_pair(pair: dict[str, Any], prefix: str) -> tuple[float, float, float]:
    if prefix == "used":
        dx_key = "used_dx_curr_to_prev_m"
        dy_key = "used_dy_curr_to_prev_m"
        yaw_key = "used_yaw_curr_to_prev_deg"
    elif prefix == "raw":
        dx_key = "dx_curr_to_prev_m"
        dy_key = "dy_curr_to_prev_m"
        yaw_key = "yaw_curr_to_prev_deg"
    else:
        raise ValueError(f"Unsupported pair prefix: {prefix}")
    return (
        float(pair.get(dx_key, 0.0) or 0.0),
        float(pair.get(dy_key, 0.0) or 0.0),
        normalize_angle_deg(float(pair.get(yaw_key, 0.0) or 0.0)),
    )


def step_norm(step: tuple[float, float, float]) -> float:
    return math.hypot(step[0], step[1])


def is_finite_step(step: tuple[float, float, float]) -> bool:
    return all(math.isfinite(value) for value in step)


def bounded_step(
    step: tuple[float, float, float],
    max_translation_m: float,
    max_yaw_deg: float,
) -> tuple[float, float, float]:
    dx, dy, yaw = step
    norm = math.hypot(dx, dy)
    if max_translation_m > 0.0 and norm > max_translation_m:
        scale = max_translation_m / max(norm, 1e-12)
        dx *= scale
        dy *= scale
    if max_yaw_deg > 0.0:
        yaw = max(-max_yaw_deg, min(max_yaw_deg, yaw))
    return (dx, dy, normalize_angle_deg(yaw))


def integrate_step(
    pose: tuple[float, float, float],
    step: tuple[float, float, float],
) -> tuple[float, float, float]:
    x, y, yaw = pose
    dx, dy, step_yaw = step
    yaw_rad = math.radians(yaw)
    next_x = x + math.cos(yaw_rad) * dx - math.sin(yaw_rad) * dy
    next_y = y + math.sin(yaw_rad) * dx + math.cos(yaw_rad) * dy
    return (next_x, next_y, normalize_angle_deg(yaw + step_yaw))


def pair_delta_max(steps: list[tuple[float, float, float]]) -> tuple[float | None, float | None]:
    if len(steps) < 2:
        return (None, None)
    translation_deltas = [
        math.hypot(curr[0] - prev[0], curr[1] - prev[1])
        for prev, curr in zip(steps, steps[1:])
    ]
    yaw_deltas = [
        abs(normalize_angle_deg(curr[2] - prev[2]))
        for prev, curr in zip(steps, steps[1:])
    ]
    return (max(translation_deltas), max(yaw_deltas))


def path_status(path_vs_healthy: float | None, path_vs_all: float | None) -> str:
    if path_vs_healthy is None and path_vs_all is None:
        return "reference_missing"
    if path_vs_healthy is not None:
        if path_vs_healthy < PATH_HEALTHY_MIN:
            return "path_collapse"
        if path_vs_healthy > PATH_HEALTHY_MAX:
            return "path_high"
    if path_vs_all is not None:
        if path_vs_all < PATH_ALL_MIN:
            return "path_collapse"
        if path_vs_all > PATH_ALL_MAX:
            return "path_high"
    return "reference_consistent"


def reference_error(path_vs_healthy: float | None, path_vs_all: float | None) -> float | None:
    reference = path_vs_healthy if path_vs_healthy is not None else path_vs_all
    if reference is None or reference <= 0.0:
        return None
    return abs(math.log(reference))


def window_rows(comparison: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for sequence in comparison["sequences"]:
        sequence_name = str(sequence["sequence"])
        for window in sequence["windows"]:
            row = window["methods"].get("ct_icp")
            if not row:
                continue
            rows.append(
                {
                    "sequence": sequence_name,
                    "window": window["name"],
                    "start": int(window["start"]),
                    "end": int(window["end"]),
                    "expected_stress": window.get("expected_stress"),
                    "health_state": row.get("health_state"),
                    "risk_state": row.get("risk_state"),
                    "watch_action": row.get("watch_action"),
                    "guard_decision": row.get("ct_icp_guard_decision"),
                    "guard_reasons": row.get("ct_icp_guard_reasons") or [],
                    "accepted_rate": row.get("accepted_rate"),
                    "result_json": row.get("result_json"),
                    "healthy_path_median_m": row.get("healthy_path_median_m"),
                    "all_method_path_median_m": row.get("all_method_path_median_m"),
                    "original_path_vs_healthy": row.get("path_vs_healthy_median"),
                    "original_path_vs_all": row.get("path_vs_all_median"),
                }
            )
    return rows


def simulate_guarded_payload(
    *,
    result_payload: dict[str, Any],
    guard_decision: str,
    source_result_json: Path,
) -> dict[str, Any]:
    parameters = result_payload.get("parameters") or {}
    max_translation = float(parameters.get("max_step_translation") or 0.0)
    max_yaw = float(parameters.get("max_step_yaw_deg") or 0.0)

    poses: list[tuple[float, float, float]] = [(0.0, 0.0, 0.0)]
    guarded_pairs: list[dict[str, Any]] = []
    guarded_steps: list[tuple[float, float, float]] = []
    original_steps: list[tuple[float, float, float]] = []
    last_prior_step: tuple[float, float, float] | None = None
    refined_pairs = 0
    velocity_prior_pairs = 0
    prior_bootstrap_pairs = 0
    hold_pairs = 0

    for pair in result_payload.get("pairs", []):
        original_step = step_from_pair(pair, "used")
        raw_step = step_from_pair(pair, "raw")
        original_steps.append(original_step)
        accepted = bool(pair.get("accepted"))

        if guard_decision == USE_REFINED:
            guarded_step = original_step
            source = "refined"
            refined_pairs += 1
            if accepted and is_finite_step(guarded_step) and step_norm(guarded_step) > 1e-9:
                last_prior_step = bounded_step(guarded_step, max_translation, max_yaw)
        elif guard_decision in VELOCITY_PRIOR_DECISIONS:
            if last_prior_step is None:
                bootstrap = raw_step if step_norm(raw_step) > 1e-9 else original_step
                if is_finite_step(bootstrap) and step_norm(bootstrap) > 1e-9:
                    guarded_step = bounded_step(bootstrap, max_translation, max_yaw)
                    last_prior_step = guarded_step
                    source = "velocity_bootstrap"
                    prior_bootstrap_pairs += 1
                    velocity_prior_pairs += 1
                else:
                    guarded_step = (0.0, 0.0, 0.0)
                    source = "hold"
                    hold_pairs += 1
            else:
                guarded_step = last_prior_step
                source = "velocity_prior"
                velocity_prior_pairs += 1
        else:
            guarded_step = (0.0, 0.0, 0.0)
            source = "hold"
            hold_pairs += 1

        guarded_steps.append(guarded_step)
        poses.append(integrate_step(poses[-1], guarded_step))
        out_pair = dict(pair)
        out_pair.update(
            {
                "guard_decision": guard_decision,
                "guard_source": source,
                "original_used_dx_curr_to_prev_m": original_step[0],
                "original_used_dy_curr_to_prev_m": original_step[1],
                "original_used_yaw_curr_to_prev_deg": original_step[2],
                "guarded_dx_curr_to_prev_m": guarded_step[0],
                "guarded_dy_curr_to_prev_m": guarded_step[1],
                "guarded_yaw_curr_to_prev_deg": guarded_step[2],
            }
        )
        guarded_pairs.append(out_pair)

    original_path = sum(step_norm(step) for step in original_steps)
    guarded_path = sum(step_norm(step) for step in guarded_steps)
    original_step_delta_max, original_yaw_delta_max = pair_delta_max(original_steps)
    guarded_step_delta_max, guarded_yaw_delta_max = pair_delta_max(guarded_steps)
    pair_count = len(guarded_pairs)
    return {
        "sequence_pcd_dir": result_payload.get("sequence_pcd_dir"),
        "gt_csv": result_payload.get("gt_csv"),
        "frames": len(poses),
        "method": "ct_icp_guarded_velocity_prior",
        "source_method": result_payload.get("method"),
        "source_json": display_path(source_result_json),
        "guard_decision": guard_decision,
        "parameters": {
            "fallback_model": "bounded_constant_velocity_prior",
            "max_step_translation": max_translation,
            "max_step_yaw_deg": max_yaw,
        },
        "summary": {
            "pairs": pair_count,
            "refined_pairs": refined_pairs,
            "velocity_prior_pairs": velocity_prior_pairs,
            "prior_bootstrap_pairs": prior_bootstrap_pairs,
            "hold_pairs": hold_pairs,
            "refined_pair_rate": refined_pairs / pair_count if pair_count else 0.0,
            "velocity_prior_pair_rate": velocity_prior_pairs / pair_count if pair_count else 0.0,
            "hold_pair_rate": hold_pairs / pair_count if pair_count else 0.0,
            "original_path_length_m": original_path,
            "guarded_path_length_m": guarded_path,
            "original_step_delta_max_m": original_step_delta_max,
            "guarded_step_delta_max_m": guarded_step_delta_max,
            "original_yaw_delta_max_deg": original_yaw_delta_max,
            "guarded_yaw_delta_max_deg": guarded_yaw_delta_max,
        },
        "poses_xyyaw": [
            {"index": i, "x_m": x, "y_m": y, "yaw_deg": yaw}
            for i, (x, y, yaw) in enumerate(poses)
        ],
        "pairs": guarded_pairs,
    }


def output_name(row: dict[str, Any]) -> str:
    return (
        f"{row['sequence']}_{row['window']}_"
        f"{int(row['start']):04d}_{int(row['end']):04d}_guarded.json"
    )


def simulate_row(row: dict[str, Any], output_dir: Path) -> dict[str, Any]:
    result_json = Path(str(row["result_json"]))
    if not result_json.is_absolute():
        result_json = REPO_ROOT / result_json
    payload = load_json(result_json)
    guard_decision = str(row.get("guard_decision") or "hold_reference")
    guarded_payload = simulate_guarded_payload(
        result_payload=payload,
        guard_decision=guard_decision,
        source_result_json=result_json,
    )
    result_path = output_dir / "results" / output_name(row)
    result_path.parent.mkdir(parents=True, exist_ok=True)
    result_path.write_text(json.dumps(guarded_payload, indent=2) + "\n", encoding="utf-8")

    summary = guarded_payload["summary"]
    guarded_vs_healthy = ratio(summary["guarded_path_length_m"], row["healthy_path_median_m"])
    guarded_vs_all = ratio(summary["guarded_path_length_m"], row["all_method_path_median_m"])
    original_error = reference_error(
        as_float(row.get("original_path_vs_healthy")),
        as_float(row.get("original_path_vs_all")),
    )
    guarded_error = reference_error(guarded_vs_healthy, guarded_vs_all)
    improved = (
        guarded_error is not None
        and original_error is not None
        and guarded_error < original_error
    )
    return {
        **row,
        "guarded_result_json": display_path(result_path),
        "pairs": summary["pairs"],
        "refined_pairs": summary["refined_pairs"],
        "velocity_prior_pairs": summary["velocity_prior_pairs"],
        "prior_bootstrap_pairs": summary["prior_bootstrap_pairs"],
        "hold_pairs": summary["hold_pairs"],
        "refined_pair_rate": summary["refined_pair_rate"],
        "velocity_prior_pair_rate": summary["velocity_prior_pair_rate"],
        "hold_pair_rate": summary["hold_pair_rate"],
        "original_path_length_m": summary["original_path_length_m"],
        "guarded_path_length_m": summary["guarded_path_length_m"],
        "guarded_path_vs_healthy": guarded_vs_healthy,
        "guarded_path_vs_all": guarded_vs_all,
        "original_path_status": path_status(
            as_float(row.get("original_path_vs_healthy")),
            as_float(row.get("original_path_vs_all")),
        ),
        "guarded_path_status": path_status(guarded_vs_healthy, guarded_vs_all),
        "reference_error_improved": improved,
        "original_reference_error": original_error,
        "guarded_reference_error": guarded_error,
        "original_step_delta_max_m": summary["original_step_delta_max_m"],
        "guarded_step_delta_max_m": summary["guarded_step_delta_max_m"],
        "original_yaw_delta_max_deg": summary["original_yaw_delta_max_deg"],
        "guarded_yaw_delta_max_deg": summary["guarded_yaw_delta_max_deg"],
    }


def aggregate(rows: list[dict[str, Any]]) -> dict[str, Any]:
    decisions = sorted({str(row.get("guard_decision")) for row in rows})
    return {
        "rows": len(rows),
        "decisions": {
            decision: {
                "rows": sum(1 for row in rows if row.get("guard_decision") == decision),
                "path_status_counts": {
                    status: sum(
                        1
                        for row in rows
                        if row.get("guard_decision") == decision
                        and row.get("guarded_path_status") == status
                    )
                    for status in sorted({str(row.get("guarded_path_status")) for row in rows})
                },
                "reference_error_improved": sum(
                    1
                    for row in rows
                    if row.get("guard_decision") == decision
                    and row.get("reference_error_improved")
                ),
            }
            for decision in decisions
        },
    }


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    lines = [
        "# CT-ICP Guarded Trajectory Simulation",
        "",
        f"Comparison source: `{payload['comparison_json']}`",
        "",
        "## Aggregate",
        "",
        "| Guard decision | Rows | Improved | Guarded path status |",
        "| --- | ---: | ---: | --- |",
    ]
    for decision, row in payload["aggregate"]["decisions"].items():
        statuses = ", ".join(
            f"{status}:{count}"
            for status, count in row["path_status_counts"].items()
            if count
        ) or "none"
        lines.append(
            f"| `{decision}` | {row['rows']} | "
            f"{row['reference_error_improved']} | {statuses} |"
        )

    lines.extend(
        [
            "",
            "## Rows",
            "",
            "| Sequence | Window | Guard | Original path | Guarded path | Original ref | Guarded ref | Status | Prior rate | Hold rate | Step delta max | Yaw delta max |",
            "| --- | --- | --- | ---: | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: |",
        ]
    )
    for row in payload["rows"]:
        original_ref = (
            row.get("original_path_vs_healthy")
            if row.get("original_path_vs_healthy") is not None
            else row.get("original_path_vs_all")
        )
        guarded_ref = (
            row.get("guarded_path_vs_healthy")
            if row.get("guarded_path_vs_healthy") is not None
            else row.get("guarded_path_vs_all")
        )
        lines.append(
            f"| `{row['sequence']}` | `{row['window']}` "
            f"{row['start']}-{row['end']} | `{row['guard_decision']}` | "
            f"{fmt(row['original_path_length_m'])} | "
            f"{fmt(row['guarded_path_length_m'])} | {fmt(original_ref)} | "
            f"{fmt(guarded_ref)} | `{row['guarded_path_status']}` | "
            f"{fmt(row['velocity_prior_pair_rate'])} | {fmt(row['hold_pair_rate'])} | "
            f"{fmt(row['guarded_step_delta_max_m'])} | "
            f"{fmt(row['guarded_yaw_delta_max_deg'])} |"
        )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- This is an offline simulation of the CT-ICP production guard; it does not rerun scan matching.",
            "- `fallback_to_prior`, `retry_optimizer`, and `reject_or_retry` use a bounded constant-velocity prior instead of trusted refined CT-ICP steps.",
            "- `velocity_bootstrap` pairs are counted in the per-window JSON so a path that only works by bootstrapping from suspect refined motion remains visible.",
            "- `path_collapse` means the guard avoided the original path excursion but produced too little motion relative to healthy/all-method references.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    comparison = load_json(args.comparison_json)
    rows = [
        simulate_row(row, args.output_dir)
        for row in window_rows(comparison)
        if row.get("result_json")
    ]
    payload = {
        "comparison_json": display_path(args.comparison_json),
        "output_dir": display_path(args.output_dir),
        "aggregate": aggregate(rows),
        "rows": rows,
    }
    output_json = args.output_dir / "ct_icp_guarded_trajectory.json"
    output_md = args.output_dir / "ct_icp_guarded_trajectory.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
