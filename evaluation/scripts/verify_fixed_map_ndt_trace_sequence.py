#!/usr/bin/env python3
"""Verify fixed-map NDT frame traces for sequence-level publish safety."""

from __future__ import annotations

import argparse
import glob
import json
import math
from collections import Counter
from pathlib import Path
from typing import Any, Callable


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_TRACE_GLOB = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "fixed_map_ndt"
    / "traces"
    / "*_trace.json"
)
DEFAULT_OUTPUT_DIR = (
    REPO_ROOT / "experiments" / "results" / "fixed_map_ndt" / "sequence_verifier"
)

POLICY_VERSION = "lidar_degeneracy_triage_v4"
TRACE_SEQUENCE_VERIFIER_VERSION = "fixed_map_ndt_trace_sequence_verifier_v1"

PUBLISH_ERROR_M = 1.0
SEVERE_ERROR_M = 5.0
JUMP_STEP_RATIO = 3.0
RECOVERY_ERROR_DROP_M = 1.0
CORRECTION_WORSEN_M = 0.5
MIN_STABLE_PUBLISH_FRAMES = 5
MIN_FALLBACK_STREAK_FRAMES = 3

DECISION_ORDER = {
    "publish_candidate": 0,
    "hold_for_stability": 1,
    "hold_recovery_frame": 2,
    "return_unknown": 3,
    "block_publish": 4,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace-glob", type=Path, default=DEFAULT_TRACE_GLOB)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def as_float(value: Any) -> float | None:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def ratio(numerator: Any, denominator: Any) -> float | None:
    num = as_float(numerator)
    den = as_float(denominator)
    if num is None or den is None or den <= 1e-9:
        return None
    return num / den


def trace_id_from_path(path: Path) -> str:
    stem = path.stem
    return stem[:-6] if stem.endswith("_trace") else stem


def frame_index(frame: dict[str, Any]) -> int:
    return int(frame.get("frame_index") or 0)


def frame_diagnostics(
    frame: dict[str, Any],
    previous_frame: dict[str, Any] | None,
) -> dict[str, Any]:
    final_error = as_float(frame.get("final_translation_error_m"))
    seed_error = as_float(frame.get("seed_translation_error_m"))
    refined_error = as_float(frame.get("refined_translation_error_m"))
    step_ratio = ratio(frame.get("final_step_m"), frame.get("gt_step_m"))
    correction_delta = (
        seed_error - final_error
        if seed_error is not None and final_error is not None
        else None
    )
    previous_error = (
        as_float(previous_frame.get("final_translation_error_m"))
        if previous_frame is not None
        else None
    )
    transition_delta = (
        previous_error - final_error
        if previous_error is not None and final_error is not None
        else None
    )
    is_jump = step_ratio is not None and step_ratio > JUMP_STEP_RATIO
    is_recovery_jump = (
        is_jump
        and transition_delta is not None
        and transition_delta >= RECOVERY_ERROR_DROP_M
        and final_error is not None
        and final_error <= PUBLISH_ERROR_M
    )
    is_unsafe_jump = (
        is_jump
        and not is_recovery_jump
        and (
            final_error is None
            or final_error > PUBLISH_ERROR_M
            or (transition_delta is not None and transition_delta < 0.0)
        )
    )
    correction_worsened = (
        bool(frame.get("accepted"))
        and correction_delta is not None
        and correction_delta < -CORRECTION_WORSEN_M
    )
    correction_improved = (
        bool(frame.get("accepted"))
        and correction_delta is not None
        and correction_delta > CORRECTION_WORSEN_M
    )
    return {
        "final_error_m": final_error,
        "seed_error_m": seed_error,
        "refined_error_m": refined_error,
        "step_ratio": step_ratio,
        "correction_error_delta_m": correction_delta,
        "transition_error_delta_m": transition_delta,
        "is_wrong_pose": final_error is None or final_error > PUBLISH_ERROR_M,
        "is_severe_wrong_pose": final_error is None or final_error > SEVERE_ERROR_M,
        "is_jump": is_jump,
        "is_recovery_jump": is_recovery_jump,
        "is_unsafe_jump": is_unsafe_jump,
        "correction_worsened": correction_worsened,
        "correction_improved": correction_improved,
    }


def classify_frame(
    frame: dict[str, Any],
    diagnostics: dict[str, Any],
    stable_publishable: bool,
) -> dict[str, Any]:
    decision = str(frame.get("decision") or "unknown")
    if decision == "anchor":
        return {
            "publish_decision": "hold_for_stability",
            "allow_pose_publish": False,
            "safety_state": "anchor_frame",
            "reason": "initial anchor is not a publish proof",
        }
    if diagnostics["correction_worsened"]:
        return {
            "publish_decision": "block_publish",
            "allow_pose_publish": False,
            "safety_state": "accepted_correction_worsened",
            "reason": "accepted NDT correction increased GT error",
        }
    if diagnostics["is_unsafe_jump"]:
        return {
            "publish_decision": "block_publish",
            "allow_pose_publish": False,
            "safety_state": "unsafe_jump",
            "reason": "pose step is inconsistent with GT motion and not a recovery",
        }
    if diagnostics["is_wrong_pose"]:
        if frame.get("accepted"):
            state = "accepted_wrong_pose"
            reason = "NDT accepted a pose outside the publish error envelope"
        elif frame.get("seed_fallback"):
            state = "fallback_tracking_gap"
            reason = "seed fallback left the stream outside the publish error envelope"
        else:
            state = "tracking_gap"
            reason = "final pose is outside the publish error envelope"
        return {
            "publish_decision": "return_unknown",
            "allow_pose_publish": False,
            "safety_state": state,
            "reason": reason,
        }
    if diagnostics["is_recovery_jump"]:
        return {
            "publish_decision": "hold_recovery_frame",
            "allow_pose_publish": False,
            "safety_state": "recovery_jump",
            "reason": "large step reduced error; wait for post-recovery stability",
        }
    if stable_publishable:
        return {
            "publish_decision": "publish_candidate",
            "allow_pose_publish": True,
            "safety_state": "stable_inlier_sequence",
            "reason": "frame is inside the publish envelope after a stable run",
        }
    return {
        "publish_decision": "hold_for_stability",
        "allow_pose_publish": False,
        "safety_state": "short_inlier_run",
        "reason": "pose is accurate in GT trace but not stable long enough to publish",
    }


def intervals_for(
    frames: list[dict[str, Any]],
    predicate: Callable[[dict[str, Any]], bool],
    *,
    min_length: int = 1,
) -> list[dict[str, int]]:
    intervals: list[dict[str, int]] = []
    start: int | None = None
    last_index: int | None = None

    def finish_interval() -> None:
        nonlocal start, last_index
        if start is not None and last_index is not None:
            length = last_index - start + 1
            if length >= min_length:
                intervals.append({"start": start, "end": last_index, "length": length})
        start = None
        last_index = None

    for frame in frames:
        index = frame_index(frame)
        if predicate(frame):
            if start is None or (
                last_index is not None and index != last_index + 1
            ):
                finish_interval()
                start = index
            last_index = index
            continue
        finish_interval()
    finish_interval()
    return intervals


def stable_inlier_intervals(rows: list[dict[str, Any]]) -> list[dict[str, int]]:
    return intervals_for(
        rows,
        lambda row: (
            row["final_error_m"] is not None
            and row["final_error_m"] <= PUBLISH_ERROR_M
            and not row["is_recovery_jump"]
            and str(row.get("raw_decision")) != "anchor"
        ),
    )


def annotate_trace(trace: dict[str, Any], path: Path) -> dict[str, Any]:
    raw_frames = list(trace.get("frames") or [])
    raw_frames_by_index = {frame_index(frame): frame for frame in raw_frames}
    diagnostics_rows: list[dict[str, Any]] = []
    for i, frame in enumerate(raw_frames):
        previous = raw_frames[i - 1] if i > 0 else None
        diag = frame_diagnostics(frame, previous)
        diagnostics_rows.append(
            {
                "frame_index": frame_index(frame),
                "raw_decision": frame.get("decision"),
                "accepted": bool(frame.get("accepted")),
                "seed_fallback": bool(frame.get("seed_fallback")),
                "scan_context_hit": bool(frame.get("scan_context_hit")),
                "scan_context_candidates_evaluated": int(
                    frame.get("scan_context_candidates_evaluated") or 0
                ),
                "ndt_score": as_float(frame.get("ndt_score")),
                **diag,
            }
        )

    stable_intervals = stable_inlier_intervals(diagnostics_rows)
    stable_publishable_frames: set[int] = set()
    for interval in stable_intervals:
        if interval["length"] >= MIN_STABLE_PUBLISH_FRAMES:
            stable_publishable_frames.update(range(interval["start"], interval["end"] + 1))

    frame_rows = []
    for row in diagnostics_rows:
        classified = classify_frame(
            raw_frames_by_index[row["frame_index"]],
            row,
            row["frame_index"] in stable_publishable_frames,
        )
        frame_rows.append({**row, **classified})

    unknown_intervals = intervals_for(
        frame_rows, lambda row: row["publish_decision"] == "return_unknown"
    )
    fallback_streak_intervals = intervals_for(
        frame_rows,
        lambda row: bool(row.get("seed_fallback")),
        min_length=MIN_FALLBACK_STREAK_FRAMES,
    )
    accepted_wrong_pose_frames = [
        row
        for row in frame_rows
        if row.get("accepted") and row["final_error_m"] is not None
        and row["final_error_m"] > PUBLISH_ERROR_M
    ]
    recovery_jumps = [row for row in frame_rows if row["is_recovery_jump"]]
    unsafe_jumps = [row for row in frame_rows if row["is_unsafe_jump"]]
    correction_worsened = [row for row in frame_rows if row["correction_worsened"]]
    decision_counts = Counter(str(row["publish_decision"]) for row in frame_rows)
    safety_counts = Counter(str(row["safety_state"]) for row in frame_rows)
    publishable_rows = [row for row in frame_rows if row["allow_pose_publish"]]
    final_errors = [
        row["final_error_m"] for row in frame_rows if row["final_error_m"] is not None
    ]
    step_ratios = [
        row["step_ratio"] for row in frame_rows if row["step_ratio"] is not None
    ]
    max_stable_run = max(
        (interval["length"] for interval in stable_intervals),
        default=0,
    )

    if correction_worsened or unsafe_jumps:
        sequence_decision = "block_publish"
        sequence_state = "unsafe_transition"
    elif accepted_wrong_pose_frames or unknown_intervals:
        sequence_decision = "return_unknown"
        sequence_state = "tracking_not_publishable"
    elif not publishable_rows:
        sequence_decision = "hold_for_stability"
        sequence_state = "insufficient_stable_run"
    else:
        sequence_decision = "publish_candidate"
        sequence_state = "stable_trace_candidate"

    return {
        "trace_id": trace_id_from_path(path),
        "trace_json": display_path(path),
        "seed_source": trace.get("seed_source"),
        "map_stride": trace.get("map_stride"),
        "scan_context_top_k": trace.get("scan_context_top_k"),
        "frames": len(frame_rows),
        "sequence_decision": sequence_decision,
        "sequence_state": sequence_state,
        "allow_pose_publish": sequence_decision == "publish_candidate",
        "frame_counts": {
            "accepted": sum(1 for row in frame_rows if row.get("accepted")),
            "seed_fallback": sum(1 for row in frame_rows if row.get("seed_fallback")),
            "scan_context_hit": sum(1 for row in frame_rows if row.get("scan_context_hit")),
            "scan_context_candidate_evals": sum(
                int(row.get("scan_context_candidates_evaluated") or 0)
                for row in frame_rows
            ),
            "publishable": len(publishable_rows),
            "return_unknown": decision_counts.get("return_unknown", 0),
            "accepted_wrong_pose": len(accepted_wrong_pose_frames),
            "recovery_jump": len(recovery_jumps),
            "unsafe_jump": len(unsafe_jumps),
            "correction_worsened": len(correction_worsened),
        },
        "metrics": {
            "max_final_error_m": max(final_errors) if final_errors else None,
            "max_step_ratio": max(step_ratios) if step_ratios else None,
            "max_stable_inlier_run": max_stable_run,
            "min_stable_publish_frames": MIN_STABLE_PUBLISH_FRAMES,
        },
        "decision_counts": dict(sorted(decision_counts.items())),
        "safety_state_counts": dict(sorted(safety_counts.items())),
        "unknown_intervals": unknown_intervals,
        "fallback_streak_intervals": fallback_streak_intervals,
        "stable_inlier_intervals": stable_intervals,
        "recovery_jumps": [
            {
                "frame_index": row["frame_index"],
                "step_ratio": row["step_ratio"],
                "previous_to_final_error_drop_m": row["transition_error_delta_m"],
                "final_error_m": row["final_error_m"],
            }
            for row in recovery_jumps
        ],
        "unsafe_jumps": [
            {
                "frame_index": row["frame_index"],
                "step_ratio": row["step_ratio"],
                "transition_error_delta_m": row["transition_error_delta_m"],
                "final_error_m": row["final_error_m"],
            }
            for row in unsafe_jumps
        ],
        "frame_rows": frame_rows,
    }


def aggregate(trace_reports: list[dict[str, Any]]) -> dict[str, Any]:
    decisions = Counter(str(report["sequence_decision"]) for report in trace_reports)
    total_frames = sum(int(report["frames"]) for report in trace_reports)
    max_final_error = max(
        (
            as_float(report["metrics"].get("max_final_error_m"))
            for report in trace_reports
            if as_float(report["metrics"].get("max_final_error_m")) is not None
        ),
        default=None,
    )
    max_step_ratio = max(
        (
            as_float(report["metrics"].get("max_step_ratio"))
            for report in trace_reports
            if as_float(report["metrics"].get("max_step_ratio")) is not None
        ),
        default=None,
    )
    return {
        "traces": len(trace_reports),
        "frames": total_frames,
        "publish_candidate_traces": decisions.get("publish_candidate", 0),
        "decision_counts": dict(sorted(decisions.items())),
        "allow_pose_publish_traces": sum(
            1 for report in trace_reports if report.get("allow_pose_publish")
        ),
        "return_unknown_frames": sum(
            report["frame_counts"]["return_unknown"] for report in trace_reports
        ),
        "accepted_wrong_pose_frames": sum(
            report["frame_counts"]["accepted_wrong_pose"] for report in trace_reports
        ),
        "recovery_jump_frames": sum(
            report["frame_counts"]["recovery_jump"] for report in trace_reports
        ),
        "unsafe_jump_frames": sum(
            report["frame_counts"]["unsafe_jump"] for report in trace_reports
        ),
        "max_final_error_m": max_final_error,
        "max_step_ratio": max_step_ratio,
    }


def discover_trace_paths(trace_glob: Path) -> list[Path]:
    paths = [Path(item) for item in glob.glob(str(trace_glob))]
    return sorted(path for path in paths if path.is_file())


def build_payload(trace_glob: Path) -> dict[str, Any]:
    trace_paths = discover_trace_paths(trace_glob)
    reports = [annotate_trace(load_json(path), path) for path in trace_paths]
    return {
        "policy": {
            "policy_version": POLICY_VERSION,
            "verifier_version": TRACE_SEQUENCE_VERIFIER_VERSION,
        },
        "trace_glob": display_path(trace_glob),
        "thresholds": {
            "publish_error_m": PUBLISH_ERROR_M,
            "severe_error_m": SEVERE_ERROR_M,
            "jump_step_ratio": JUMP_STEP_RATIO,
            "recovery_error_drop_m": RECOVERY_ERROR_DROP_M,
            "correction_worsen_m": CORRECTION_WORSEN_M,
            "min_stable_publish_frames": MIN_STABLE_PUBLISH_FRAMES,
            "min_fallback_streak_frames": MIN_FALLBACK_STREAK_FRAMES,
        },
        "aggregate": aggregate(reports),
        "traces": reports,
    }


def fmt(value: Any, digits: int = 3) -> str:
    number = as_float(value)
    if number is None:
        return "n/a"
    return f"{number:.{digits}f}"


def interval_text(intervals: list[dict[str, int]], limit: int = 6) -> str:
    if not intervals:
        return "none"
    rendered = [
        f"{item['start']}-{item['end']} ({item['length']})"
        for item in intervals[:limit]
    ]
    if len(intervals) > limit:
        rendered.append(f"+{len(intervals) - limit} more")
    return ", ".join(rendered)


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    aggregate_row = payload["aggregate"]
    decisions = ", ".join(
        f"{name}:{count}" for name, count in aggregate_row["decision_counts"].items()
    )
    lines = [
        "# Fixed-Map NDT Trace Sequence Verifier",
        "",
        f"Trace glob: `{payload['trace_glob']}`",
        "",
        "## Aggregate",
        "",
        "| Traces | Frames | Publishable traces | Return-unknown frames | Accepted wrong-pose frames | Recovery jumps | Unsafe jumps | Max final error [m] | Max step ratio |",
        "| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        f"| {aggregate_row['traces']} | {aggregate_row['frames']} | "
        f"{aggregate_row['allow_pose_publish_traces']} | "
        f"{aggregate_row['return_unknown_frames']} | "
        f"{aggregate_row['accepted_wrong_pose_frames']} | "
        f"{aggregate_row['recovery_jump_frames']} | "
        f"{aggregate_row['unsafe_jump_frames']} | "
        f"{fmt(aggregate_row['max_final_error_m'])} | "
        f"{fmt(aggregate_row['max_step_ratio'])} |",
        "",
        f"Trace decisions: {decisions or 'none'}",
        "",
        "## Trace Reports",
        "",
        "| Trace | Decision | State | Publishable frames | Unknown frames | Accepted wrong | Fallback streaks | Stable inlier intervals | Max stable run | Max error [m] | Max step ratio |",
        "| --- | --- | --- | ---: | ---: | ---: | --- | --- | ---: | ---: | ---: |",
    ]
    for report in payload["traces"]:
        lines.append(
            f"| `{report['trace_id']}` | `{report['sequence_decision']}` | "
            f"`{report['sequence_state']}` | "
            f"{report['frame_counts']['publishable']} | "
            f"{report['frame_counts']['return_unknown']} | "
            f"{report['frame_counts']['accepted_wrong_pose']} | "
            f"{interval_text(report['fallback_streak_intervals'], 3)} | "
            f"{interval_text(report['stable_inlier_intervals'], 3)} | "
            f"{report['metrics']['max_stable_inlier_run']} | "
            f"{fmt(report['metrics']['max_final_error_m'])} | "
            f"{fmt(report['metrics']['max_step_ratio'])} |"
        )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- This verifier consumes per-frame trace transitions, not only run-level ATE.",
            "- Large pose steps are split into recovery jumps and unsafe jumps by checking whether GT error drops after the transition.",
            "- Accurate single recovery frames are held until a stable inlier run reaches the configured publish length.",
            "- `return_unknown` frames mark intervals where a live component should suppress pose publish and relocalize.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    payload = build_payload(args.trace_glob)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    output_json = args.output_dir / "fixed_map_ndt_trace_sequence_verifier.json"
    output_md = args.output_dir / "fixed_map_ndt_trace_sequence_verifier.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
