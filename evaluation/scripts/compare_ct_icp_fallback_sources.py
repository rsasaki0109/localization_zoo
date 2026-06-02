#!/usr/bin/env python3
"""Compare CT-ICP fallback sources against GT-free path references."""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
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
DEFAULT_OUTPUT_DIR = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "ct_icp_fallback_bakeoff"
)

SOURCE_PRIORITY = {
    "geometry_icp": 0,
    "kiss_keyframe": 1,
    "self_velocity": 2,
    "intensity_bev": 3,
    "ct_icp_original": 99,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison-json", type=Path, default=DEFAULT_COMPARISON_JSON)
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


def normalize_angle_deg(angle_deg: float) -> float:
    return (angle_deg + 180.0) % 360.0 - 180.0


def step_from_pair(pair: dict[str, Any]) -> tuple[float, float, float]:
    return (
        float(pair.get("used_dx_curr_to_prev_m", pair.get("dx_curr_to_prev_m", 0.0)) or 0.0),
        float(pair.get("used_dy_curr_to_prev_m", pair.get("dy_curr_to_prev_m", 0.0)) or 0.0),
        normalize_angle_deg(
            float(
                pair.get(
                    "used_yaw_curr_to_prev_deg",
                    pair.get("yaw_curr_to_prev_deg", 0.0),
                )
                or 0.0
            )
        ),
    )


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


def trajectory_stats(payload: dict[str, Any]) -> dict[str, Any]:
    pairs = payload.get("pairs") or []
    steps = [step_from_pair(pair) for pair in pairs]
    path = sum(math.hypot(step[0], step[1]) for step in steps)
    step_delta_max, yaw_delta_max = pair_delta_max(steps)
    accepted = [bool(pair.get("accepted")) for pair in pairs]
    return {
        "pairs": len(pairs),
        "path_length_m": path,
        "step_delta_max_m": step_delta_max,
        "yaw_delta_max_deg": yaw_delta_max,
        "accepted_rate": sum(accepted) / len(accepted) if accepted else None,
    }


def ratio(value: Any, reference: Any) -> float | None:
    value_number = as_float(value)
    reference_number = as_float(reference)
    if value_number is None or reference_number is None or abs(reference_number) < 1e-9:
        return None
    return value_number / reference_number


def candidate_path_status(
    guarded_module: ModuleType,
    path_vs_healthy: float | None,
    path_vs_all: float | None,
) -> str:
    return str(guarded_module.path_status(path_vs_healthy, path_vs_all))


def candidate_reference_error(
    guarded_module: ModuleType,
    path_vs_healthy: float | None,
    path_vs_all: float | None,
) -> float | None:
    return guarded_module.reference_error(path_vs_healthy, path_vs_all)


def eligibility_blockers(candidate: dict[str, Any]) -> list[str]:
    blockers: list[str] = []
    source = str(candidate["source"])
    if source == "ct_icp_original":
        blockers.append("guard_rejects_refined")
    elif source != "self_velocity":
        if candidate.get("health_state") != "ok":
            blockers.append("source_health_not_ok")
        if candidate.get("risk_state") != "ok":
            blockers.append("source_risk_not_ok")
        accepted = as_float(candidate.get("accepted_rate"))
        if accepted is None or accepted < 0.9:
            blockers.append("source_acceptance_low")

    if candidate.get("path_status") != "reference_consistent":
        blockers.append("path_not_reference_consistent")
    if candidate.get("reference_error") is None:
        blockers.append("reference_error_missing")
    return blockers


def mark_candidate_eligibility(candidate: dict[str, Any]) -> dict[str, Any]:
    blockers = eligibility_blockers(candidate)
    return {
        **candidate,
        "eligible": not blockers,
        "eligibility_blockers": blockers,
    }


def select_best_candidate(candidates: list[dict[str, Any]]) -> dict[str, Any] | None:
    eligible = [candidate for candidate in candidates if candidate.get("eligible")]
    if not eligible:
        return None
    return min(
        eligible,
        key=lambda candidate: (
            float(candidate["reference_error"]),
            SOURCE_PRIORITY.get(str(candidate["source"]), 50),
            str(candidate["source"]),
        ),
    )


def result_path(value: Any) -> Path | None:
    if not value:
        return None
    path = Path(str(value))
    return path if path.is_absolute() else REPO_ROOT / path


def build_original_candidate(
    *,
    guarded_module: ModuleType,
    row: dict[str, Any],
    healthy_path_median: Any,
    all_path_median: Any,
) -> dict[str, Any]:
    path_length = row.get("trajectory_path_length_m")
    path_vs_healthy = ratio(path_length, healthy_path_median)
    path_vs_all = ratio(path_length, all_path_median)
    return mark_candidate_eligibility(
        {
            "source": "ct_icp_original",
            "source_kind": "ct_icp_refined",
            "health_state": row.get("health_state"),
            "risk_state": row.get("risk_state"),
            "accepted_rate": row.get("accepted_rate"),
            "path_length_m": path_length,
            "path_vs_healthy": path_vs_healthy,
            "path_vs_all": path_vs_all,
            "path_status": candidate_path_status(
                guarded_module, path_vs_healthy, path_vs_all
            ),
            "reference_error": candidate_reference_error(
                guarded_module, path_vs_healthy, path_vs_all
            ),
            "step_delta_max_m": None,
            "yaw_delta_max_deg": None,
        }
    )


def build_peer_candidate(
    *,
    guarded_module: ModuleType,
    method: str,
    row: dict[str, Any],
    healthy_path_median: Any,
    all_path_median: Any,
) -> dict[str, Any] | None:
    path = result_path(row.get("result_json"))
    if path is None or not path.exists():
        return None
    stats = trajectory_stats(load_json(path))
    path_vs_healthy = ratio(stats["path_length_m"], healthy_path_median)
    path_vs_all = ratio(stats["path_length_m"], all_path_median)
    return mark_candidate_eligibility(
        {
            "source": method,
            "source_kind": "external_peer",
            "health_state": row.get("health_state"),
            "risk_state": row.get("risk_state"),
            "accepted_rate": row.get("accepted_rate", stats.get("accepted_rate")),
            "path_length_m": stats["path_length_m"],
            "path_vs_healthy": path_vs_healthy,
            "path_vs_all": path_vs_all,
            "path_status": candidate_path_status(
                guarded_module, path_vs_healthy, path_vs_all
            ),
            "reference_error": candidate_reference_error(
                guarded_module, path_vs_healthy, path_vs_all
            ),
            "step_delta_max_m": stats["step_delta_max_m"],
            "yaw_delta_max_deg": stats["yaw_delta_max_deg"],
        }
    )


def build_self_velocity_candidate(
    *,
    guarded_module: ModuleType,
    row: dict[str, Any],
    healthy_path_median: Any,
    all_path_median: Any,
) -> dict[str, Any] | None:
    path = result_path(row.get("result_json"))
    if path is None or not path.exists():
        return None
    guarded_payload = guarded_module.simulate_guarded_payload(
        result_payload=load_json(path),
        guard_decision=str(row.get("ct_icp_guard_decision") or "hold_reference"),
        source_result_json=path,
    )
    summary = guarded_payload["summary"]
    path_vs_healthy = ratio(summary["guarded_path_length_m"], healthy_path_median)
    path_vs_all = ratio(summary["guarded_path_length_m"], all_path_median)
    return mark_candidate_eligibility(
        {
            "source": "self_velocity",
            "source_kind": "internal_prior",
            "health_state": row.get("health_state"),
            "risk_state": row.get("risk_state"),
            "accepted_rate": row.get("accepted_rate"),
            "path_length_m": summary["guarded_path_length_m"],
            "path_vs_healthy": path_vs_healthy,
            "path_vs_all": path_vs_all,
            "path_status": candidate_path_status(
                guarded_module, path_vs_healthy, path_vs_all
            ),
            "reference_error": candidate_reference_error(
                guarded_module, path_vs_healthy, path_vs_all
            ),
            "step_delta_max_m": summary["guarded_step_delta_max_m"],
            "yaw_delta_max_deg": summary["guarded_yaw_delta_max_deg"],
            "velocity_prior_pair_rate": summary["velocity_prior_pair_rate"],
            "hold_pair_rate": summary["hold_pair_rate"],
        }
    )


def build_window_bakeoff(
    *,
    guarded_module: ModuleType,
    sequence_name: str,
    window: dict[str, Any],
) -> dict[str, Any] | None:
    methods = window["methods"]
    ct_row = methods.get("ct_icp")
    if not ct_row:
        return None
    healthy_path_median = ct_row.get("healthy_path_median_m")
    all_path_median = ct_row.get("all_method_path_median_m")
    candidates: list[dict[str, Any]] = []
    candidates.append(
        build_original_candidate(
            guarded_module=guarded_module,
            row=ct_row,
            healthy_path_median=healthy_path_median,
            all_path_median=all_path_median,
        )
    )
    self_velocity = build_self_velocity_candidate(
        guarded_module=guarded_module,
        row=ct_row,
        healthy_path_median=healthy_path_median,
        all_path_median=all_path_median,
    )
    if self_velocity is not None:
        candidates.append(self_velocity)
    for method in ("geometry_icp", "kiss_keyframe", "intensity_bev"):
        row = methods.get(method)
        if not row:
            continue
        candidate = build_peer_candidate(
            guarded_module=guarded_module,
            method=method,
            row=row,
            healthy_path_median=healthy_path_median,
            all_path_median=all_path_median,
        )
        if candidate is not None:
            candidates.append(candidate)

    selected = select_best_candidate(candidates)
    original = next(candidate for candidate in candidates if candidate["source"] == "ct_icp_original")
    self_candidate = next(
        (candidate for candidate in candidates if candidate["source"] == "self_velocity"),
        None,
    )
    selected_error = selected.get("reference_error") if selected else None
    original_error = original.get("reference_error")
    self_error = self_candidate.get("reference_error") if self_candidate else None
    return {
        "sequence": sequence_name,
        "window": window["name"],
        "start": int(window["start"]),
        "end": int(window["end"]),
        "expected_stress": window.get("expected_stress"),
        "guard_decision": ct_row.get("ct_icp_guard_decision"),
        "guard_reasons": ct_row.get("ct_icp_guard_reasons") or [],
        "healthy_path_median_m": healthy_path_median,
        "all_method_path_median_m": all_path_median,
        "selected_source": selected.get("source") if selected else None,
        "selected_reference_error": selected_error,
        "selected_improves_original": (
            selected_error is not None
            and original_error is not None
            and selected_error < original_error
        ),
        "selected_improves_self_velocity": (
            selected_error is not None
            and self_error is not None
            and selected_error < self_error
        ),
        "candidates": candidates,
    }


def build_payload(comparison: dict[str, Any], guarded_module: ModuleType, output_dir: Path) -> dict[str, Any]:
    rows: list[dict[str, Any]] = []
    for sequence in comparison["sequences"]:
        sequence_name = str(sequence["sequence"])
        for window in sequence["windows"]:
            row = build_window_bakeoff(
                guarded_module=guarded_module,
                sequence_name=sequence_name,
                window=window,
            )
            if row is not None:
                rows.append(row)
    return {
        "comparison_json": display_path(DEFAULT_COMPARISON_JSON),
        "output_dir": display_path(output_dir),
        "rows": rows,
        "aggregate": aggregate(rows),
    }


def aggregate(rows: list[dict[str, Any]]) -> dict[str, Any]:
    sources = sorted(
        {str(candidate["source"]) for row in rows for candidate in row["candidates"]}
    )
    selected_sources = sorted(
        {str(row["selected_source"]) for row in rows if row.get("selected_source")}
    )
    return {
        "rows": len(rows),
        "source_counts": {
            source: {
                "rows": sum(
                    1
                    for row in rows
                    for candidate in row["candidates"]
                    if candidate["source"] == source
                ),
                "eligible": sum(
                    1
                    for row in rows
                    for candidate in row["candidates"]
                    if candidate["source"] == source and candidate["eligible"]
                ),
                "reference_consistent": sum(
                    1
                    for row in rows
                    for candidate in row["candidates"]
                    if candidate["source"] == source
                    and candidate["path_status"] == "reference_consistent"
                ),
            }
            for source in sources
        },
        "selected_source_counts": {
            source: sum(1 for row in rows if row.get("selected_source") == source)
            for source in selected_sources
        },
        "selected_improves_original": sum(
            1 for row in rows if row.get("selected_improves_original")
        ),
        "selected_improves_self_velocity": sum(
            1 for row in rows if row.get("selected_improves_self_velocity")
        ),
    }


def candidate_by_source(row: dict[str, Any], source: str) -> dict[str, Any] | None:
    return next(
        (candidate for candidate in row["candidates"] if candidate["source"] == source),
        None,
    )


def source_ref(candidate: dict[str, Any] | None) -> str:
    if candidate is None:
        return "n/a"
    value = candidate.get("path_vs_healthy")
    if value is None:
        value = candidate.get("path_vs_all")
    return fmt(value)


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    lines = [
        "# CT-ICP Fallback Source Bakeoff",
        "",
        f"Comparison source: `{payload['comparison_json']}`",
        "",
        "## Source Summary",
        "",
        "| Source | Rows | Eligible | Reference consistent | Selected |",
        "| --- | ---: | ---: | ---: | ---: |",
    ]
    selected_counts = payload["aggregate"]["selected_source_counts"]
    for source, row in payload["aggregate"]["source_counts"].items():
        lines.append(
            f"| `{source}` | {row['rows']} | {row['eligible']} | "
            f"{row['reference_consistent']} | {selected_counts.get(source, 0)} |"
        )

    lines.extend(
        [
            "",
            "## Window Selection",
            "",
            "| Sequence | Window | Guard | CT original | Self velocity | Selected source | Selected ref | Improves original? | Improves self? |",
            "| --- | --- | --- | ---: | ---: | --- | ---: | --- | --- |",
        ]
    )
    for row in payload["rows"]:
        original = candidate_by_source(row, "ct_icp_original")
        self_velocity = candidate_by_source(row, "self_velocity")
        selected = candidate_by_source(row, str(row.get("selected_source")))
        lines.append(
            f"| `{row['sequence']}` | `{row['window']}` "
            f"{row['start']}-{row['end']} | `{row.get('guard_decision')}` | "
            f"{source_ref(original)} | {source_ref(self_velocity)} | "
            f"`{row.get('selected_source') or 'none'}` | {source_ref(selected)} | "
            f"{'yes' if row.get('selected_improves_original') else 'no'} | "
            f"{'yes' if row.get('selected_improves_self_velocity') else 'no'} |"
        )

    lines.extend(
        [
            "",
            "## Candidate Detail",
            "",
            "| Sequence | Window | Source | Kind | Health | Risk | Ref | Status | Eligible | Blockers |",
            "| --- | --- | --- | --- | --- | --- | ---: | --- | --- | --- |",
        ]
    )
    for row in payload["rows"]:
        for candidate in sorted(
            row["candidates"],
            key=lambda candidate: SOURCE_PRIORITY.get(str(candidate["source"]), 50),
        ):
            blockers = ", ".join(candidate.get("eligibility_blockers") or []) or "none"
            lines.append(
                f"| `{row['sequence']}` | `{row['window']}` "
                f"{row['start']}-{row['end']} | `{candidate['source']}` | "
                f"`{candidate['source_kind']}` | `{candidate.get('health_state')}` | "
                f"`{candidate.get('risk_state')}` | {source_ref(candidate)} | "
                f"`{candidate['path_status']}` | "
                f"{'yes' if candidate.get('eligible') else 'no'} | {blockers} |"
            )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- This bakeoff does not rerun localization. It compares already-computed candidate trajectories under the same GT-free path-reference contract.",
            "- A source is eligible only when it is healthy, risk-free, sufficiently accepted, and path-reference consistent.",
            "- `ct_icp_original` is shown as the rejected refined trajectory baseline; it is never eligible when the guard rejects refined CT-ICP.",
            "- External healthy peers are fallback-source candidates, not final production truth. GT-backed evaluation is still required before wiring a real component.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    comparison = load_json(args.comparison_json)
    guarded_module = load_module("simulate_ct_icp_guarded_trajectory", GUARDED_SCRIPT)
    payload = build_payload(comparison, guarded_module, args.output_dir)
    payload["comparison_json"] = display_path(args.comparison_json)
    output_json = args.output_dir / "ct_icp_fallback_bakeoff.json"
    output_md = args.output_dir / "ct_icp_fallback_bakeoff.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
