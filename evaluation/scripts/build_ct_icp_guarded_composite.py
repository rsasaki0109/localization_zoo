#!/usr/bin/env python3
"""Build CT-ICP guarded composite trajectories from selected fallback sources."""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
from collections import Counter
from pathlib import Path
from types import ModuleType
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
GUARDED_SCRIPT = Path(__file__).with_name("simulate_ct_icp_guarded_trajectory.py")
DEFAULT_COMPARISON_JSON = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "method_health_comparison"
    / "method_health_comparison.json"
)
DEFAULT_FALLBACK_BAKEOFF_JSON = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "ct_icp_fallback_bakeoff"
    / "ct_icp_fallback_bakeoff.json"
)
DEFAULT_OUTPUT_DIR = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "ct_icp_guarded_composite"
)

USE_REFINED = "use_refined"
HOLD_STEP = (0.0, 0.0, 0.0)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison-json", type=Path, default=DEFAULT_COMPARISON_JSON)
    parser.add_argument(
        "--fallback-bakeoff-json", type=Path, default=DEFAULT_FALLBACK_BAKEOFF_JSON
    )
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def load_module(name: str, path: Path) -> ModuleType:
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load module: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def display_path(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def result_path(value: Any) -> Path | None:
    if not value:
        return None
    path = Path(str(value))
    return path if path.is_absolute() else REPO_ROOT / path


def fmt(value: Any, digits: int = 3) -> str:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return "n/a"
    if not math.isfinite(number):
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


def step_from_pair(pair: dict[str, Any], source: str) -> tuple[float, float, float]:
    if source == "guarded":
        dx_key = "guarded_dx_curr_to_prev_m"
        dy_key = "guarded_dy_curr_to_prev_m"
        yaw_key = "guarded_yaw_curr_to_prev_deg"
    else:
        dx_key = "used_dx_curr_to_prev_m"
        dy_key = "used_dy_curr_to_prev_m"
        yaw_key = "used_yaw_curr_to_prev_deg"
    return (
        float(pair.get(dx_key, pair.get("dx_curr_to_prev_m", 0.0)) or 0.0),
        float(pair.get(dy_key, pair.get("dy_curr_to_prev_m", 0.0)) or 0.0),
        normalize_angle_deg(
            float(pair.get(yaw_key, pair.get("yaw_curr_to_prev_deg", 0.0)) or 0.0)
        ),
    )


def step_norm(step: tuple[float, float, float]) -> float:
    return math.hypot(step[0], step[1])


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
    translation = [
        math.hypot(curr[0] - prev[0], curr[1] - prev[1])
        for prev, curr in zip(steps, steps[1:])
    ]
    yaw = [
        abs(normalize_angle_deg(curr[2] - prev[2]))
        for prev, curr in zip(steps, steps[1:])
    ]
    return (max(translation), max(yaw))


def steps_by_index(payload: dict[str, Any], source: str = "used") -> dict[int, tuple[float, float, float]]:
    return {
        int(pair.get("index", i + 1)): step_from_pair(pair, source)
        for i, pair in enumerate(payload.get("pairs") or [])
    }


def trajectory_summary(
    *,
    guarded_module: ModuleType,
    steps: list[tuple[float, float, float]],
    healthy_path_median: Any,
    all_path_median: Any,
) -> dict[str, Any]:
    path_length = sum(step_norm(step) for step in steps)
    path_vs_healthy = ratio(path_length, healthy_path_median)
    path_vs_all = ratio(path_length, all_path_median)
    step_delta_max, yaw_delta_max = pair_delta_max(steps)
    return {
        "path_length_m": path_length,
        "path_vs_healthy": path_vs_healthy,
        "path_vs_all": path_vs_all,
        "path_status": guarded_module.path_status(path_vs_healthy, path_vs_all),
        "reference_error": guarded_module.reference_error(path_vs_healthy, path_vs_all),
        "step_delta_max_m": step_delta_max,
        "yaw_delta_max_deg": yaw_delta_max,
    }


def integrate_steps(steps: list[tuple[float, float, float]]) -> list[tuple[float, float, float]]:
    poses = [(0.0, 0.0, 0.0)]
    for step in steps:
        poses.append(integrate_step(poses[-1], step))
    return poses


def build_composite_payload(
    *,
    guarded_module: ModuleType,
    ct_payload: dict[str, Any],
    self_velocity_payload: dict[str, Any],
    selected_source: str | None,
    selected_payload: dict[str, Any] | None,
    guard_decision: str,
    healthy_path_median: Any,
    all_path_median: Any,
    source_result_json: Path,
    selected_result_json: Path | None = None,
) -> dict[str, Any]:
    ct_steps = steps_by_index(ct_payload, "used")
    self_steps = steps_by_index(self_velocity_payload, "guarded")
    selected_steps = steps_by_index(selected_payload, "used") if selected_payload else {}

    composite_steps: list[tuple[float, float, float]] = []
    pairs: list[dict[str, Any]] = []
    source_counts: Counter[str] = Counter()
    missing_selected_pairs = 0
    last_source: str | None = None
    source_switches = 0

    for pair in ct_payload.get("pairs") or []:
        index = int(pair.get("index", len(composite_steps) + 1))
        if guard_decision == USE_REFINED:
            source = "ct_icp_original"
            step = ct_steps.get(index, HOLD_STEP)
        elif selected_source and index in selected_steps:
            source = selected_source
            step = selected_steps[index]
        elif index in self_steps:
            source = "self_velocity"
            step = self_steps[index]
            if selected_source:
                missing_selected_pairs += 1
        else:
            source = "hold"
            step = HOLD_STEP
            if selected_source:
                missing_selected_pairs += 1

        if last_source is not None and source != last_source:
            source_switches += 1
        last_source = source
        source_counts[source] += 1
        composite_steps.append(step)
        out_pair = dict(pair)
        out_pair.update(
            {
                "composite_source": source,
                "composite_dx_curr_to_prev_m": step[0],
                "composite_dy_curr_to_prev_m": step[1],
                "composite_yaw_curr_to_prev_deg": step[2],
            }
        )
        pairs.append(out_pair)

    poses = integrate_steps(composite_steps)
    pair_count = len(composite_steps)
    fallback_pairs = pair_count - source_counts.get("ct_icp_original", 0)
    summary = trajectory_summary(
        guarded_module=guarded_module,
        steps=composite_steps,
        healthy_path_median=healthy_path_median,
        all_path_median=all_path_median,
    )
    return {
        "sequence_pcd_dir": ct_payload.get("sequence_pcd_dir"),
        "gt_csv": ct_payload.get("gt_csv"),
        "frames": len(poses),
        "method": "ct_icp_guarded_composite",
        "source_method": ct_payload.get("method"),
        "source_json": display_path(source_result_json),
        "selected_fallback_source": selected_source,
        "selected_fallback_json": (
            display_path(selected_result_json) if selected_result_json is not None else None
        ),
        "guard_decision": guard_decision,
        "parameters": {
            "composite_rule": "ct_icp_if_use_refined_else_selected_fallback_else_self_velocity",
        },
        "summary": {
            **summary,
            "pairs": pair_count,
            "source_pair_counts": dict(sorted(source_counts.items())),
            "fallback_pairs": fallback_pairs,
            "fallback_pair_rate": fallback_pairs / pair_count if pair_count else 0.0,
            "missing_selected_pairs": missing_selected_pairs,
            "source_switches": source_switches,
        },
        "poses_xyyaw": [
            {"index": i, "x_m": x, "y_m": y, "yaw_deg": yaw}
            for i, (x, y, yaw) in enumerate(poses)
        ],
        "pairs": pairs,
    }


def row_key(row: dict[str, Any]) -> tuple[str, str, int, int]:
    return (
        str(row["sequence"]),
        str(row["window"]),
        int(row["start"]),
        int(row["end"]),
    )


def comparison_windows(comparison: dict[str, Any]) -> dict[tuple[str, str, int, int], dict[str, Any]]:
    out: dict[tuple[str, str, int, int], dict[str, Any]] = {}
    for sequence in comparison["sequences"]:
        sequence_name = str(sequence["sequence"])
        for window in sequence["windows"]:
            key = (
                sequence_name,
                str(window["name"]),
                int(window["start"]),
                int(window["end"]),
            )
            out[key] = window
    return out


def output_name(row: dict[str, Any]) -> str:
    return (
        f"{row['sequence']}_{row['window']}_"
        f"{int(row['start']):04d}_{int(row['end']):04d}_composite.json"
    )


def source_payload(window: dict[str, Any], source: str | None) -> tuple[dict[str, Any] | None, Path | None]:
    if not source or source in {"self_velocity", "ct_icp_original"}:
        return (None, None)
    method_row = window["methods"].get(source)
    if not method_row:
        return (None, None)
    path = result_path(method_row.get("result_json"))
    if path is None or not path.exists():
        return (None, path)
    return (load_json(path), path)


def build_row(
    *,
    guarded_module: ModuleType,
    bakeoff_row: dict[str, Any],
    window: dict[str, Any],
    output_dir: Path,
) -> dict[str, Any]:
    ct_row = window["methods"]["ct_icp"]
    ct_result_path = result_path(ct_row.get("result_json"))
    if ct_result_path is None or not ct_result_path.exists():
        raise RuntimeError(f"Missing CT-ICP result for {row_key(bakeoff_row)}")
    ct_payload = load_json(ct_result_path)
    self_velocity_payload = guarded_module.simulate_guarded_payload(
        result_payload=ct_payload,
        guard_decision=str(bakeoff_row.get("guard_decision") or "hold_reference"),
        source_result_json=ct_result_path,
    )
    selected_source = bakeoff_row.get("selected_source")
    selected_payload, selected_path = source_payload(window, selected_source)
    composite_payload = build_composite_payload(
        guarded_module=guarded_module,
        ct_payload=ct_payload,
        self_velocity_payload=self_velocity_payload,
        selected_source=str(selected_source) if selected_source else None,
        selected_payload=selected_payload,
        guard_decision=str(bakeoff_row.get("guard_decision") or "hold_reference"),
        healthy_path_median=bakeoff_row.get("healthy_path_median_m"),
        all_path_median=bakeoff_row.get("all_method_path_median_m"),
        source_result_json=ct_result_path,
        selected_result_json=selected_path,
    )
    result_path_out = output_dir / "results" / output_name(bakeoff_row)
    result_path_out.parent.mkdir(parents=True, exist_ok=True)
    result_path_out.write_text(json.dumps(composite_payload, indent=2) + "\n", encoding="utf-8")

    original = next(
        candidate for candidate in bakeoff_row["candidates"] if candidate["source"] == "ct_icp_original"
    )
    self_velocity = next(
        candidate for candidate in bakeoff_row["candidates"] if candidate["source"] == "self_velocity"
    )
    selected = next(
        (
            candidate
            for candidate in bakeoff_row["candidates"]
            if candidate["source"] == selected_source
        ),
        None,
    )
    composite = composite_payload["summary"]
    composite_error = composite.get("reference_error")
    return {
        "sequence": bakeoff_row["sequence"],
        "window": bakeoff_row["window"],
        "start": int(bakeoff_row["start"]),
        "end": int(bakeoff_row["end"]),
        "expected_stress": bakeoff_row.get("expected_stress"),
        "guard_decision": bakeoff_row.get("guard_decision"),
        "selected_source": selected_source,
        "composite_result_json": display_path(result_path_out),
        "healthy_path_median_m": bakeoff_row.get("healthy_path_median_m"),
        "all_method_path_median_m": bakeoff_row.get("all_method_path_median_m"),
        "original_reference_error": original.get("reference_error"),
        "self_velocity_reference_error": self_velocity.get("reference_error"),
        "selected_reference_error": selected.get("reference_error") if selected else None,
        "composite_reference_error": composite_error,
        "original_path_status": original.get("path_status"),
        "self_velocity_path_status": self_velocity.get("path_status"),
        "selected_path_status": selected.get("path_status") if selected else None,
        "composite_path_status": composite.get("path_status"),
        "original_path_vs_healthy": original.get("path_vs_healthy"),
        "self_velocity_path_vs_healthy": self_velocity.get("path_vs_healthy"),
        "selected_path_vs_healthy": selected.get("path_vs_healthy") if selected else None,
        "composite_path_vs_healthy": composite.get("path_vs_healthy"),
        "composite_path_length_m": composite.get("path_length_m"),
        "fallback_pair_rate": composite.get("fallback_pair_rate"),
        "source_pair_counts": composite.get("source_pair_counts"),
        "source_switches": composite.get("source_switches"),
        "missing_selected_pairs": composite.get("missing_selected_pairs"),
        "composite_step_delta_max_m": composite.get("step_delta_max_m"),
        "composite_yaw_delta_max_deg": composite.get("yaw_delta_max_deg"),
        "composite_improves_original": (
            composite_error is not None
            and original.get("reference_error") is not None
            and composite_error < original["reference_error"]
        ),
        "composite_improves_self_velocity": (
            composite_error is not None
            and self_velocity.get("reference_error") is not None
            and composite_error < self_velocity["reference_error"]
        ),
    }


def aggregate(rows: list[dict[str, Any]]) -> dict[str, Any]:
    status_counts = Counter(str(row.get("composite_path_status")) for row in rows)
    selected_counts = Counter(str(row.get("selected_source") or "none") for row in rows)
    return {
        "rows": len(rows),
        "composite_path_status_counts": dict(sorted(status_counts.items())),
        "selected_source_counts": dict(sorted(selected_counts.items())),
        "composite_improves_original": sum(
            1 for row in rows if row.get("composite_improves_original")
        ),
        "composite_improves_self_velocity": sum(
            1 for row in rows if row.get("composite_improves_self_velocity")
        ),
        "mean_fallback_pair_rate": (
            sum(float(row.get("fallback_pair_rate") or 0.0) for row in rows) / len(rows)
            if rows
            else None
        ),
    }


def ref_value(row: dict[str, Any], prefix: str) -> str:
    value = row.get(f"{prefix}_path_vs_healthy")
    return fmt(value)


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    lines = [
        "# CT-ICP Guarded Composite",
        "",
        f"Comparison source: `{payload['comparison_json']}`",
        f"Fallback bakeoff: `{payload['fallback_bakeoff_json']}`",
        "",
        "## Aggregate",
        "",
        "| Rows | Composite statuses | Selected sources | Improves original | Improves self velocity | Mean fallback rate |",
        "| ---: | --- | --- | ---: | ---: | ---: |",
    ]
    aggregate_row = payload["aggregate"]
    statuses = ", ".join(
        f"{name}:{count}"
        for name, count in aggregate_row["composite_path_status_counts"].items()
    ) or "none"
    selected = ", ".join(
        f"{name}:{count}"
        for name, count in aggregate_row["selected_source_counts"].items()
    ) or "none"
    lines.append(
        f"| {aggregate_row['rows']} | {statuses} | {selected} | "
        f"{aggregate_row['composite_improves_original']} | "
        f"{aggregate_row['composite_improves_self_velocity']} | "
        f"{fmt(aggregate_row['mean_fallback_pair_rate'])} |"
    )

    lines.extend(
        [
            "",
            "## Rows",
            "",
            "| Sequence | Window | Guard | Selected | Original ref | Self ref | Selected ref | Composite ref | Composite status | Fallback rate | Switches | Missing pairs |",
            "| --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- | ---: | ---: | ---: |",
        ]
    )
    for row in payload["rows"]:
        lines.append(
            f"| `{row['sequence']}` | `{row['window']}` "
            f"{row['start']}-{row['end']} | `{row['guard_decision']}` | "
            f"`{row.get('selected_source') or 'none'}` | "
            f"{ref_value(row, 'original')} | {ref_value(row, 'self_velocity')} | "
            f"{ref_value(row, 'selected')} | {ref_value(row, 'composite')} | "
            f"`{row['composite_path_status']}` | {fmt(row['fallback_pair_rate'])} | "
            f"{row['source_switches']} | {row['missing_selected_pairs']} |"
        )

    lines.extend(
        [
            "",
            "## Source Counts",
            "",
            "| Sequence | Window | Composite source counts | Result |",
            "| --- | --- | --- | --- |",
        ]
    )
    for row in payload["rows"]:
        counts = ", ".join(
            f"{source}:{count}" for source, count in row["source_pair_counts"].items()
        ) or "none"
        lines.append(
            f"| `{row['sequence']}` | `{row['window']}` "
            f"{row['start']}-{row['end']} | {counts} | "
            f"`{row['composite_result_json']}` |"
        )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- The composite uses CT-ICP refined steps only for `use_refined`; otherwise it uses the selected fallback source from the fallback bakeoff, falling back to self velocity only if the selected source is missing a pair.",
            "- This is still GT-free and path-reference based. It validates the wiring contract and continuity, not final localization accuracy.",
            "- A useful next check is to replay this composite contract on GT-backed KITTI 108 windows and compare ATE/RPE against CT-ICP, geometry-only, and self-velocity fallback.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    comparison = load_json(args.comparison_json)
    bakeoff = load_json(args.fallback_bakeoff_json)
    guarded_module = load_module("simulate_ct_icp_guarded_trajectory", GUARDED_SCRIPT)
    windows = comparison_windows(comparison)
    rows = [
        build_row(
            guarded_module=guarded_module,
            bakeoff_row=row,
            window=windows[row_key(row)],
            output_dir=args.output_dir,
        )
        for row in bakeoff["rows"]
    ]
    payload = {
        "comparison_json": display_path(args.comparison_json),
        "fallback_bakeoff_json": display_path(args.fallback_bakeoff_json),
        "output_dir": display_path(args.output_dir),
        "aggregate": aggregate(rows),
        "rows": rows,
    }
    output_json = args.output_dir / "ct_icp_guarded_composite.json"
    output_md = args.output_dir / "ct_icp_guarded_composite.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
