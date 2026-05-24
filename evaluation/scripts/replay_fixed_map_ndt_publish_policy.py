#!/usr/bin/env python3
"""Replay a failure-aware publish policy on fixed-map NDT frame traces."""

from __future__ import annotations

import argparse
import glob
import importlib.util
import json
import math
from collections import Counter
from pathlib import Path
from types import ModuleType
from typing import Any


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
    REPO_ROOT
    / "experiments"
    / "results"
    / "fixed_map_ndt"
    / "publish_policy_replay"
)
SEQUENCE_VERIFIER_PATH = Path(__file__).with_name(
    "verify_fixed_map_ndt_trace_sequence.py"
)

POLICY_VERSION = "lidar_degeneracy_triage_v4"
PUBLISH_POLICY_REPLAY_VERSION = "fixed_map_ndt_publish_policy_replay_v1"
MAX_HOLD_FRAMES = 3


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace-glob", type=Path, default=DEFAULT_TRACE_GLOB)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument(
        "--max-hold-frames",
        type=int,
        default=MAX_HOLD_FRAMES,
        help="Maximum consecutive frames allowed to hold the last safe pose.",
    )
    return parser.parse_args()


def load_sequence_verifier() -> ModuleType:
    spec = importlib.util.spec_from_file_location(
        "fixed_map_ndt_trace_sequence_verifier_runtime",
        SEQUENCE_VERIFIER_PATH,
    )
    if spec is None or spec.loader is None:
        raise RuntimeError(f"failed to load {SEQUENCE_VERIFIER_PATH}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


SEQUENCE_VERIFIER = load_sequence_verifier()
PUBLISH_ERROR_M = SEQUENCE_VERIFIER.PUBLISH_ERROR_M


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def discover_trace_paths(trace_glob: Path) -> list[Path]:
    paths = [Path(item) for item in glob.glob(str(trace_glob))]
    return sorted(path for path in paths if path.is_file())


def as_float(value: Any) -> float | None:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def finite_max(values: list[float | None]) -> float | None:
    finite = [value for value in values if value is not None and math.isfinite(value)]
    return max(finite) if finite else None


def pose_translation(pose: Any) -> tuple[float, float, float] | None:
    if not isinstance(pose, list) or len(pose) < 12:
        return None
    tx = as_float(pose[3])
    ty = as_float(pose[7])
    tz = as_float(pose[11])
    if tx is None or ty is None or tz is None:
        return None
    return tx, ty, tz


def translation_error_m(pose: Any, gt_pose: Any) -> float | None:
    pose_xyz = pose_translation(pose)
    gt_xyz = pose_translation(gt_pose)
    if pose_xyz is None or gt_xyz is None:
        return None
    dx = pose_xyz[0] - gt_xyz[0]
    dy = pose_xyz[1] - gt_xyz[1]
    dz = pose_xyz[2] - gt_xyz[2]
    return math.sqrt(dx * dx + dy * dy + dz * dz)


def has_wrong_pose(error_m: float | None) -> bool:
    return error_m is None or error_m > PUBLISH_ERROR_M


def max_streak(actions: list[str], accepted: set[str]) -> int:
    best = 0
    current = 0
    for action in actions:
        if action in accepted:
            current += 1
            best = max(best, current)
        else:
            current = 0
    return best


def embedded_policy_summary(raw_frames: list[dict[str, Any]]) -> dict[str, Any]:
    action_counts = Counter(
        str(frame.get("publish_action") or "missing") for frame in raw_frames
    )
    pose_output_frames = [
        frame for frame in raw_frames if bool(frame.get("has_pose_output"))
    ]
    wrong_output_frames = [
        frame for frame in pose_output_frames if bool(frame.get("gt_wrong_pose"))
    ]
    unsafe_output_frames = [
        frame for frame in pose_output_frames if bool(frame.get("gt_unsafe_transition"))
    ]
    suppressed_wrong = [
        frame
        for frame in raw_frames
        if bool(frame.get("gt_wrong_pose")) and not bool(frame.get("has_pose_output"))
    ]
    suppressed_unsafe = [
        frame
        for frame in raw_frames
        if bool(frame.get("gt_unsafe_transition"))
        and not bool(frame.get("has_pose_output"))
    ]
    published_errors = [
        as_float(frame.get("published_translation_error_m"))
        for frame in pose_output_frames
    ]
    relock_streaks = [
        int(frame.get("relock_streak") or 0)
        for frame in raw_frames
        if "relock_streak" in frame
    ]
    first_publish_frame = next(
        (
            int(frame.get("frame_index"))
            for frame in raw_frames
            if str(frame.get("publish_action")) == "publish_pose"
        ),
        None,
    )
    return {
        "available": any("publish_action" in frame for frame in raw_frames),
        "action_counts": dict(sorted(action_counts.items())),
        "pose_output_frames": len(pose_output_frames),
        "publish_pose_frames": action_counts.get("publish_pose", 0),
        "first_publish_frame": first_publish_frame,
        "wrong_pose_output_frames": len(wrong_output_frames),
        "unsafe_transition_output_frames": len(unsafe_output_frames),
        "unknown_frames": action_counts.get("return_unknown", 0),
        "hold_frames": action_counts.get("hold_last_pose", 0),
        "relock_candidate_frames": sum(
            1 for frame in raw_frames if bool(frame.get("relock_candidate"))
        ),
        "max_relock_streak": max(relock_streaks, default=0),
        "suppressed_wrong_pose_frames": len(suppressed_wrong),
        "suppressed_unsafe_transition_frames": len(suppressed_unsafe),
        "max_published_error_m": finite_max(published_errors),
    }


def replay_policy_rows(
    *,
    trace_id: str,
    frame_rows: list[dict[str, Any]],
    raw_frames: list[dict[str, Any]],
    max_hold_frames: int = MAX_HOLD_FRAMES,
) -> dict[str, Any]:
    raw_by_index = {
        int(frame.get("frame_index") or 0): frame
        for frame in raw_frames
        if isinstance(frame, dict)
    }
    last_published_pose: Any = None
    last_published_frame: int | None = None
    hold_streak = 0
    timeline: list[dict[str, Any]] = []

    for row in frame_rows:
        index = int(row["frame_index"])
        raw_frame = raw_by_index.get(index, {})
        verifier_decision = str(row.get("publish_decision") or "return_unknown")
        raw_error = as_float(row.get("final_error_m"))
        raw_wrong = has_wrong_pose(raw_error)

        replay_action = "return_unknown"
        has_pose_output = False
        published_error: float | None = None
        stale_frames: int | None = None

        if row.get("allow_pose_publish"):
            replay_action = "publish_pose"
            has_pose_output = True
            published_error = raw_error
            last_published_pose = raw_frame.get("final_pose")
            last_published_frame = index
            hold_streak = 0
        elif verifier_decision in {"hold_for_stability", "hold_recovery_frame"}:
            can_hold = last_published_pose is not None and hold_streak < max_hold_frames
            if can_hold:
                replay_action = "hold_last_pose"
                has_pose_output = True
                hold_streak += 1
                stale_frames = (
                    index - last_published_frame
                    if last_published_frame is not None
                    else None
                )
                published_error = translation_error_m(
                    last_published_pose,
                    raw_frame.get("gt_pose"),
                )
            else:
                replay_action = "return_unknown"
                hold_streak = 0
        else:
            replay_action = verifier_decision
            hold_streak = 0

        output_wrong = has_pose_output and has_wrong_pose(published_error)
        suppressed_wrong = raw_wrong and not has_pose_output
        suppressed_safe = not raw_wrong and not has_pose_output
        unsafe_jump = bool(row.get("is_unsafe_jump"))
        timeline.append(
            {
                "frame_index": index,
                "raw_decision": row.get("raw_decision"),
                "verifier_decision": verifier_decision,
                "safety_state": row.get("safety_state"),
                "replay_action": replay_action,
                "has_pose_output": has_pose_output,
                "raw_final_error_m": raw_error,
                "published_error_m": published_error,
                "stale_frames": stale_frames,
                "raw_wrong_pose": raw_wrong,
                "output_wrong_pose": output_wrong,
                "raw_unsafe_jump": unsafe_jump,
                "suppressed_wrong_pose": suppressed_wrong,
                "suppressed_safe_pose": suppressed_safe,
                "suppressed_unsafe_jump": unsafe_jump and not has_pose_output,
            }
        )

    action_counts = Counter(str(row["replay_action"]) for row in timeline)
    output_rows = [row for row in timeline if row["has_pose_output"]]
    raw_wrong_rows = [row for row in timeline if row["raw_wrong_pose"]]
    wrong_output_rows = [row for row in timeline if row["output_wrong_pose"]]
    unsafe_output_rows = [
        row
        for row in timeline
        if row["has_pose_output"] and row["raw_unsafe_jump"]
    ]
    suppressed_wrong_rows = [row for row in timeline if row["suppressed_wrong_pose"]]
    suppressed_unsafe_rows = [row for row in timeline if row["suppressed_unsafe_jump"]]
    published_errors = [as_float(row["published_error_m"]) for row in output_rows]
    held_errors = [
        as_float(row["published_error_m"])
        for row in timeline
        if row["replay_action"] == "hold_last_pose"
    ]

    if wrong_output_rows or unsafe_output_rows:
        sequence_decision = "unsafe_publish"
    elif not output_rows and raw_wrong_rows:
        sequence_decision = "fail_closed_all_unknown"
    elif suppressed_wrong_rows:
        sequence_decision = "fail_closed_suppressed"
    elif output_rows:
        sequence_decision = "publish_safe_candidate"
    else:
        sequence_decision = "no_pose_output"

    first_publish_frame = next(
        (
            int(row["frame_index"])
            for row in timeline
            if row["replay_action"] == "publish_pose"
        ),
        None,
    )
    return {
        "trace_id": trace_id,
        "frames": len(timeline),
        "sequence_decision": sequence_decision,
        "raw_pose_frames": len(timeline),
        "raw_wrong_pose_frames": len(raw_wrong_rows),
        "raw_unsafe_jump_frames": sum(1 for row in timeline if row["raw_unsafe_jump"]),
        "gated_pose_frames": len(output_rows),
        "gated_publish_frames": action_counts.get("publish_pose", 0),
        "gated_hold_frames": action_counts.get("hold_last_pose", 0),
        "gated_unknown_frames": action_counts.get("return_unknown", 0),
        "gated_block_frames": action_counts.get("block_publish", 0),
        "gated_wrong_pose_frames": len(wrong_output_rows),
        "gated_unsafe_jump_frames": len(unsafe_output_rows),
        "suppressed_wrong_pose_frames": len(suppressed_wrong_rows),
        "suppressed_safe_pose_frames": sum(
            1 for row in timeline if row["suppressed_safe_pose"]
        ),
        "suppressed_unsafe_jump_frames": len(suppressed_unsafe_rows),
        "first_publish_frame": first_publish_frame,
        "max_published_error_m": finite_max(published_errors),
        "max_held_error_m": finite_max(held_errors),
        "max_unknown_streak": max_streak(
            [str(row["replay_action"]) for row in timeline],
            {"return_unknown", "block_publish"},
        ),
        "max_hold_streak": max_streak(
            [str(row["replay_action"]) for row in timeline],
            {"hold_last_pose"},
        ),
        "action_counts": dict(sorted(action_counts.items())),
        "timeline": timeline,
    }


def replay_trace(
    trace: dict[str, Any],
    path: Path,
    *,
    max_hold_frames: int = MAX_HOLD_FRAMES,
) -> dict[str, Any]:
    raw_frames = list(trace.get("frames") or [])
    verifier_report = SEQUENCE_VERIFIER.annotate_trace(trace, path)
    replay = replay_policy_rows(
        trace_id=verifier_report["trace_id"],
        frame_rows=list(verifier_report["frame_rows"]),
        raw_frames=raw_frames,
        max_hold_frames=max_hold_frames,
    )
    return {
        "trace_id": verifier_report["trace_id"],
        "trace_json": display_path(path),
        "seed_source": trace.get("seed_source"),
        "map_stride": trace.get("map_stride"),
        "scan_context_top_k": trace.get("scan_context_top_k"),
        "embedded_publish_policy": trace.get("publish_policy") or {},
        "embedded_policy_summary": embedded_policy_summary(raw_frames),
        "sequence_verifier_decision": verifier_report["sequence_decision"],
        "sequence_verifier_state": verifier_report["sequence_state"],
        **replay,
    }


def aggregate(trace_reports: list[dict[str, Any]]) -> dict[str, Any]:
    decisions = Counter(str(report["sequence_decision"]) for report in trace_reports)
    return {
        "traces": len(trace_reports),
        "frames": sum(int(report["frames"]) for report in trace_reports),
        "decision_counts": dict(sorted(decisions.items())),
        "raw_wrong_pose_frames": sum(
            int(report["raw_wrong_pose_frames"]) for report in trace_reports
        ),
        "raw_unsafe_jump_frames": sum(
            int(report["raw_unsafe_jump_frames"]) for report in trace_reports
        ),
        "gated_pose_frames": sum(
            int(report["gated_pose_frames"]) for report in trace_reports
        ),
        "gated_wrong_pose_frames": sum(
            int(report["gated_wrong_pose_frames"]) for report in trace_reports
        ),
        "gated_unsafe_jump_frames": sum(
            int(report["gated_unsafe_jump_frames"]) for report in trace_reports
        ),
        "gated_unknown_frames": sum(
            int(report["gated_unknown_frames"]) for report in trace_reports
        ),
        "gated_block_frames": sum(
            int(report["gated_block_frames"]) for report in trace_reports
        ),
        "suppressed_wrong_pose_frames": sum(
            int(report["suppressed_wrong_pose_frames"]) for report in trace_reports
        ),
        "suppressed_unsafe_jump_frames": sum(
            int(report["suppressed_unsafe_jump_frames"]) for report in trace_reports
        ),
        "embedded_pose_output_frames": sum(
            int(report["embedded_policy_summary"]["pose_output_frames"])
            for report in trace_reports
        ),
        "embedded_wrong_pose_output_frames": sum(
            int(report["embedded_policy_summary"]["wrong_pose_output_frames"])
            for report in trace_reports
        ),
        "embedded_unknown_frames": sum(
            int(report["embedded_policy_summary"]["unknown_frames"])
            for report in trace_reports
        ),
        "embedded_relock_candidate_frames": sum(
            int(report["embedded_policy_summary"]["relock_candidate_frames"])
            for report in trace_reports
        ),
        "embedded_max_relock_streak": max(
            (
                int(report["embedded_policy_summary"]["max_relock_streak"])
                for report in trace_reports
            ),
            default=0,
        ),
        "embedded_suppressed_wrong_pose_frames": sum(
            int(report["embedded_policy_summary"]["suppressed_wrong_pose_frames"])
            for report in trace_reports
        ),
        "embedded_suppressed_unsafe_transition_frames": sum(
            int(report["embedded_policy_summary"]["suppressed_unsafe_transition_frames"])
            for report in trace_reports
        ),
        "max_published_error_m": finite_max(
            [as_float(report["max_published_error_m"]) for report in trace_reports]
        ),
        "max_held_error_m": finite_max(
            [as_float(report["max_held_error_m"]) for report in trace_reports]
        ),
    }


def build_payload(trace_glob: Path, *, max_hold_frames: int) -> dict[str, Any]:
    trace_paths = discover_trace_paths(trace_glob)
    reports = [
        replay_trace(load_json(path), path, max_hold_frames=max_hold_frames)
        for path in trace_paths
    ]
    return {
        "policy": {
            "policy_version": POLICY_VERSION,
            "replay_version": PUBLISH_POLICY_REPLAY_VERSION,
            "sequence_verifier_version": SEQUENCE_VERIFIER.TRACE_SEQUENCE_VERIFIER_VERSION,
        },
        "trace_glob": display_path(trace_glob),
        "thresholds": {
            "publish_error_m": PUBLISH_ERROR_M,
            "max_hold_frames": max_hold_frames,
        },
        "aggregate": aggregate(reports),
        "traces": reports,
    }


def fmt(value: Any, digits: int = 3) -> str:
    number = as_float(value)
    if number is None:
        return "n/a"
    return f"{number:.{digits}f}"


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    aggregate_row = payload["aggregate"]
    decisions = ", ".join(
        f"{name}:{count}" for name, count in aggregate_row["decision_counts"].items()
    )
    lines = [
        "# Fixed-Map NDT Publish Policy Replay",
        "",
        f"Trace glob: `{payload['trace_glob']}`",
        "",
        "## Aggregate",
        "",
        "| Traces | Frames | Raw wrong | Raw unsafe jumps | Gated pose outputs | Gated wrong outputs | Unknown frames | Block frames | Suppressed wrong | Suppressed unsafe jumps | Max published error [m] |",
        "| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        f"| {aggregate_row['traces']} | {aggregate_row['frames']} | "
        f"{aggregate_row['raw_wrong_pose_frames']} | "
        f"{aggregate_row['raw_unsafe_jump_frames']} | "
        f"{aggregate_row['gated_pose_frames']} | "
        f"{aggregate_row['gated_wrong_pose_frames']} | "
        f"{aggregate_row['gated_unknown_frames']} | "
        f"{aggregate_row['gated_block_frames']} | "
        f"{aggregate_row['suppressed_wrong_pose_frames']} | "
        f"{aggregate_row['suppressed_unsafe_jump_frames']} | "
        f"{fmt(aggregate_row['max_published_error_m'])} |",
        "",
        "| Embedded runtime outputs | Embedded wrong outputs | Embedded unknown | Embedded relock candidates | Embedded max relock streak | Embedded suppressed wrong | Embedded suppressed unsafe |",
        "| ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        f"| {aggregate_row['embedded_pose_output_frames']} | "
        f"{aggregate_row['embedded_wrong_pose_output_frames']} | "
        f"{aggregate_row['embedded_unknown_frames']} | "
        f"{aggregate_row['embedded_relock_candidate_frames']} | "
        f"{aggregate_row['embedded_max_relock_streak']} | "
        f"{aggregate_row['embedded_suppressed_wrong_pose_frames']} | "
        f"{aggregate_row['embedded_suppressed_unsafe_transition_frames']} |",
        "",
        f"Replay decisions: {decisions or 'none'}",
        "",
        "## Trace Reports",
        "",
        "| Trace | Replay decision | Verifier decision | Raw wrong | Raw unsafe | Gated outputs | Gated wrong | Unknown | Block | Suppressed wrong | Embedded outputs | Embedded unknown | Relock candidates | Max relock streak | Embedded first publish | Replay first publish | Max published error [m] | Max unknown streak |",
        "| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for report in payload["traces"]:
        first_publish = (
            str(report["first_publish_frame"])
            if report["first_publish_frame"] is not None
            else "none"
        )
        embedded_first_publish = (
            str(report["embedded_policy_summary"]["first_publish_frame"])
            if report["embedded_policy_summary"]["first_publish_frame"] is not None
            else "none"
        )
        lines.append(
            f"| `{report['trace_id']}` | `{report['sequence_decision']}` | "
            f"`{report['sequence_verifier_decision']}` | "
            f"{report['raw_wrong_pose_frames']} | "
            f"{report['raw_unsafe_jump_frames']} | "
            f"{report['gated_pose_frames']} | "
            f"{report['gated_wrong_pose_frames']} | "
            f"{report['gated_unknown_frames']} | "
            f"{report['gated_block_frames']} | "
            f"{report['suppressed_wrong_pose_frames']} | "
            f"{report['embedded_policy_summary']['pose_output_frames']} | "
            f"{report['embedded_policy_summary']['unknown_frames']} | "
            f"{report['embedded_policy_summary']['relock_candidate_frames']} | "
            f"{report['embedded_policy_summary']['max_relock_streak']} | "
            f"{embedded_first_publish} | "
            f"{first_publish} | "
            f"{fmt(report['max_published_error_m'])} | "
            f"{report['max_unknown_streak']} |"
        )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- Raw fixed-map NDT is treated as publishing every frame's `final_pose`.",
            "- Gated replay publishes only sequence-verifier publish candidates, can briefly hold the last safe pose, and otherwise returns unknown or blocks publish.",
            "- Embedded runtime outputs are the GT-free publish actions written by `pcd_dogfooding` itself.",
            "- `suppressed_wrong_pose_frames` measures false-pose suppression; `gated_wrong_pose_frames` must stay zero before this policy can be considered publish-safe.",
            "- A `fail_closed_all_unknown` post-hoc replay is still fully closed; compare it with embedded relock outputs to measure recovery availability.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    if args.max_hold_frames < 0:
        raise RuntimeError("--max-hold-frames must be non-negative")
    payload = build_payload(args.trace_glob, max_hold_frames=args.max_hold_frames)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    output_json = args.output_dir / "fixed_map_ndt_publish_policy_replay.json"
    output_md = args.output_dir / "fixed_map_ndt_publish_policy_replay.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
