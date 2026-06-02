#!/usr/bin/env python3
"""Calibrate a Scan Context global-hypothesis verifier from fixed-map NDT audit rows."""

from __future__ import annotations

import argparse
import json
import math
from collections import Counter
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_AUDIT_JSON = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "fixed_map_ndt"
    / "fixed_map_ndt_failure_audit.json"
)
DEFAULT_OUTPUT_DIR = REPO_ROOT / "experiments" / "results" / "fixed_map_ndt"

POLICY_VERSION = "lidar_degeneracy_triage_v4"
VERIFIER_VERSION = "fixed_map_global_hypothesis_verifier_v1"

SCAN_CONTEXT_MAX_MEAN_DISTANCE = 0.12
SCAN_CONTEXT_MIN_HIT_RATE = 0.30
SCAN_CONTEXT_MAX_CANDIDATE_EVALS_PER_FRAME = 1.0
LOCALIZER_MIN_ACCEPTED_RATE = 0.50

LAB_ONLY_SOURCE_CLASSES = {"oracle_seed", "dense_db_smoke", "basin_probe"}
REFINEMENT_ACCEPT_DECISION = "accept_for_refinement"

DECISION_ORDER = {
    "accept_for_refinement": 0,
    "lab_only": 1,
    "out_of_scope": 2,
    "hold_for_sequence_prior": 3,
    "hold_low_localizer_acceptance": 4,
    "reject_missing_retrieval_metrics": 5,
    "reject_distance_outlier": 6,
    "reject_score_trap": 7,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--audit-json", type=Path, default=DEFAULT_AUDIT_JSON)
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


def has_reason(row: dict[str, Any], reason: str) -> bool:
    return reason in {str(item) for item in row.get("policy_reasons") or []}


def ratio(numerator: Any, denominator: Any) -> float | None:
    num = as_float(numerator)
    den = as_float(denominator)
    if num is None or den is None or den <= 0.0:
        return None
    return num / den


def candidate_evals_per_frame(row: dict[str, Any]) -> float | None:
    evals = row.get("ndt_candidate_evals")
    if evals is None:
        return None
    return ratio(evals, row.get("attempted"))


def label_row(row: dict[str, Any]) -> str:
    source_class = str(row.get("source_class") or "unknown")
    seed_source = str(row.get("seed_source") or "unknown")
    if source_class in LAB_ONLY_SOURCE_CLASSES:
        return source_class
    if seed_source != "scan_context":
        return "out_of_scope"
    if has_reason(row, "unfiltered_ndt_score_trap"):
        return "unsafe_score_trap"
    if has_reason(row, "accepted_bad_localization"):
        return "unsafe_wrong_pose"
    if has_reason(row, "bad_localization"):
        return "unsafe_localization_gap"
    if has_reason(row, "global_initializer_near_basin"):
        return "near_basin"
    if has_reason(row, "near_candidate_but_threshold_miss"):
        return "threshold_miss"
    if has_reason(row, "dense_db_place_recognition_smoke_pass"):
        return "dense_db_smoke"
    return "unlabeled"


def verify_row(row: dict[str, Any]) -> dict[str, Any]:
    source_class = str(row.get("source_class") or "unknown")
    seed_source = str(row.get("seed_source") or "unknown")
    mean_distance = as_float(row.get("scan_context_mean_distance"))
    hit_rate = as_float(row.get("scan_context_hit_rate"))
    evals_per_frame = candidate_evals_per_frame(row)
    accepted_rate = as_float(row.get("accepted_rate"))

    if source_class in LAB_ONLY_SOURCE_CLASSES:
        return {
            "verifier_decision": "lab_only",
            "allow_global_hypothesis_refinement": False,
            "allow_pose_publish": False,
            "verifier_state": source_class,
            "required_next_gate": "non_oracle_runtime_initializer",
            "verifier_reason": "benchmark scaffold rows cannot become live global hypotheses",
        }

    if seed_source != "scan_context":
        return {
            "verifier_decision": "out_of_scope",
            "allow_global_hypothesis_refinement": False,
            "allow_pose_publish": False,
            "verifier_state": "not_scan_context",
            "required_next_gate": "seed_specific_verifier",
            "verifier_reason": "this verifier only calibrates Scan Context global hypotheses",
        }

    if row.get("scan_context_unfiltered") or (
        evals_per_frame is not None
        and evals_per_frame > SCAN_CONTEXT_MAX_CANDIDATE_EVALS_PER_FRAME
    ):
        return {
            "verifier_decision": "reject_score_trap",
            "allow_global_hypothesis_refinement": False,
            "allow_pose_publish": False,
            "verifier_state": "unfiltered_ndt_score_selection",
            "required_next_gate": "filtered_retrieval_or_geometric_verifier",
            "verifier_reason": "NDT score alone is not calibrated for unfiltered top-K global candidates",
        }

    if mean_distance is None or hit_rate is None:
        return {
            "verifier_decision": "reject_missing_retrieval_metrics",
            "allow_global_hypothesis_refinement": False,
            "allow_pose_publish": False,
            "verifier_state": "missing_scan_context_metrics",
            "required_next_gate": "retrieval_metric_contract",
            "verifier_reason": "runtime retrieval metrics are required before local refinement",
        }

    if mean_distance > SCAN_CONTEXT_MAX_MEAN_DISTANCE:
        return {
            "verifier_decision": "reject_distance_outlier",
            "allow_global_hypothesis_refinement": False,
            "allow_pose_publish": False,
            "verifier_state": "scan_context_distance_outlier",
            "required_next_gate": "stronger_global_place_recognition",
            "verifier_reason": "Scan Context distance is outside the calibrated refinement envelope",
        }

    if hit_rate < SCAN_CONTEXT_MIN_HIT_RATE:
        return {
            "verifier_decision": "hold_for_sequence_prior",
            "allow_global_hypothesis_refinement": False,
            "allow_pose_publish": False,
            "verifier_state": "low_retrieval_support",
            "required_next_gate": "sequential_place_prior",
            "verifier_reason": "single-frame retrieval support is too sparse for fixed-map NDT",
        }

    if accepted_rate is not None and accepted_rate < LOCALIZER_MIN_ACCEPTED_RATE:
        return {
            "verifier_decision": "hold_low_localizer_acceptance",
            "allow_global_hypothesis_refinement": False,
            "allow_pose_publish": False,
            "verifier_state": "low_localizer_acceptance",
            "required_next_gate": "map_consistency_or_sequence_verifier",
            "verifier_reason": "NDT accepts too few refinements to trust this hypothesis stream",
        }

    return {
        "verifier_decision": REFINEMENT_ACCEPT_DECISION,
        "allow_global_hypothesis_refinement": True,
        "allow_pose_publish": False,
        "verifier_state": "filtered_supported_retrieval",
        "required_next_gate": "map_consistency_or_sequence_verifier",
        "verifier_reason": "filtered Scan Context retrieval is inside the calibrated NDT refinement envelope",
    }


def outcome(row: dict[str, Any]) -> str:
    label = str(row["gt_label"])
    accepted = bool(row["allow_global_hypothesis_refinement"])
    if label.startswith("unsafe_"):
        return "unsafe_accepted" if accepted else "unsafe_blocked"
    if label == "near_basin":
        return "near_basin_retained" if accepted else "near_basin_lost"
    if label == "threshold_miss":
        return "threshold_miss_retained" if accepted else "threshold_miss_held"
    if label in LAB_ONLY_SOURCE_CLASSES:
        return "lab_only"
    if label == "out_of_scope":
        return "out_of_scope"
    return "unscored"


def verifier_row(row: dict[str, Any]) -> dict[str, Any]:
    verified = verify_row(row)
    out = {
        "sequence": row.get("sequence"),
        "variant": row.get("variant"),
        "seed_source": row.get("seed_source"),
        "source_class": row.get("source_class"),
        "policy_decision": row.get("policy_decision"),
        "policy_reasons": row.get("policy_reasons") or [],
        "gt_label": label_row(row),
        "ate_m": row.get("ate_m"),
        "rpe_trans_pct": row.get("rpe_trans_pct"),
        "fps": row.get("fps"),
        "accepted_rate": row.get("accepted_rate"),
        "scan_context_hit_rate": row.get("scan_context_hit_rate"),
        "scan_context_mean_distance": row.get("scan_context_mean_distance"),
        "scan_context_unfiltered": row.get("scan_context_unfiltered"),
        "ndt_candidate_evals": row.get("ndt_candidate_evals"),
        "candidate_evals_per_frame": candidate_evals_per_frame(row),
        **verified,
    }
    out["verifier_outcome"] = outcome(out)
    return out


def aggregate(rows: list[dict[str, Any]]) -> dict[str, Any]:
    decisions = Counter(str(row["verifier_decision"]) for row in rows)
    labels = Counter(str(row["gt_label"]) for row in rows)
    outcomes = Counter(str(row["verifier_outcome"]) for row in rows)
    scan_context_rows = [row for row in rows if row.get("seed_source") == "scan_context"]
    unsafe_rows = [row for row in rows if str(row["gt_label"]).startswith("unsafe_")]
    score_traps = [row for row in rows if row["gt_label"] == "unsafe_score_trap"]
    near_basin_rows = [row for row in rows if row["gt_label"] == "near_basin"]
    accepted_for_refinement = [
        row for row in rows if row.get("allow_global_hypothesis_refinement")
    ]
    unsafe_accepted = [row for row in unsafe_rows if row.get("allow_global_hypothesis_refinement")]
    score_traps_blocked = [
        row for row in score_traps if not row.get("allow_global_hypothesis_refinement")
    ]
    near_basin_retained = [
        row for row in near_basin_rows if row.get("allow_global_hypothesis_refinement")
    ]
    return {
        "rows": len(rows),
        "scan_context_rows": len(scan_context_rows),
        "accepted_for_refinement": len(accepted_for_refinement),
        "pose_publishable_rows": sum(1 for row in rows if row.get("allow_pose_publish")),
        "unsafe_rows": len(unsafe_rows),
        "unsafe_accepted": len(unsafe_accepted),
        "score_trap_rows": len(score_traps),
        "score_traps_blocked": len(score_traps_blocked),
        "near_basin_rows": len(near_basin_rows),
        "near_basin_retained": len(near_basin_retained),
        "decision_counts": dict(sorted(decisions.items())),
        "gt_label_counts": dict(sorted(labels.items())),
        "outcome_counts": dict(sorted(outcomes.items())),
    }


def fmt(value: Any, digits: int = 3) -> str:
    number = as_float(value)
    if number is None:
        return "n/a"
    return f"{number:.{digits}f}"


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    aggregate_row = payload["aggregate"]
    thresholds = payload["thresholds"]
    decisions = ", ".join(
        f"{name}:{count}" for name, count in aggregate_row["decision_counts"].items()
    )
    outcomes = ", ".join(
        f"{name}:{count}" for name, count in aggregate_row["outcome_counts"].items()
    )
    rows = sorted(
        payload["rows"],
        key=lambda row: (
            DECISION_ORDER.get(str(row["verifier_decision"]), 99),
            str(row.get("sequence")),
            str(row.get("variant")),
        ),
    )
    lines = [
        "# Fixed-Map Global Hypothesis Verifier",
        "",
        f"Audit source: `{payload['audit_json']}`",
        "",
        "## Thresholds",
        "",
        "| Feature | Gate |",
        "| --- | ---: |",
        f"| Scan Context mean distance | <= {thresholds['scan_context_max_mean_distance']:.3f} |",
        f"| Scan Context hit rate | >= {thresholds['scan_context_min_hit_rate']:.3f} |",
        f"| NDT candidate evals per frame | <= {thresholds['scan_context_max_candidate_evals_per_frame']:.3f} |",
        f"| NDT accepted rate | >= {thresholds['localizer_min_accepted_rate']:.3f} |",
        "",
        "## Aggregate",
        "",
        "| Rows | ScanContext | Accept for refinement | Pose publishable | Unsafe accepted | Score traps blocked | Near-basin retained |",
        "| ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        f"| {aggregate_row['rows']} | {aggregate_row['scan_context_rows']} | "
        f"{aggregate_row['accepted_for_refinement']} | "
        f"{aggregate_row['pose_publishable_rows']} | "
        f"{aggregate_row['unsafe_accepted']} | "
        f"{aggregate_row['score_traps_blocked']}/{aggregate_row['score_trap_rows']} | "
        f"{aggregate_row['near_basin_retained']}/{aggregate_row['near_basin_rows']} |",
        "",
        f"Decisions: {decisions or 'none'}",
        "",
        f"Outcomes: {outcomes or 'none'}",
        "",
        "## Verifier Rows",
        "",
        "| Sequence | Variant | GT label | Decision | Mean distance | Hit rate | Evals/frame | Accepted rate | ATE [m] | RPE [%] | Next gate |",
        "| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |",
    ]
    for row in rows:
        lines.append(
            f"| `{row['sequence']}` | `{row['variant']}` | `{row['gt_label']}` | "
            f"`{row['verifier_decision']}` | {fmt(row['scan_context_mean_distance'])} | "
            f"{fmt(row['scan_context_hit_rate'])} | "
            f"{fmt(row['candidate_evals_per_frame'])} | "
            f"{fmt(row['accepted_rate'])} | {fmt(row['ate_m'])} | "
            f"{fmt(row['rpe_trans_pct'])} | `{row['required_next_gate']}` |"
        )
    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- This verifier only decides whether a Scan Context global hypothesis stream may enter local NDT refinement.",
            "- It never allows direct pose publish; publish still requires map consistency and runtime health gates.",
            "- Unfiltered top-K NDT score selection is blocked before refinement because the audit shows it can select high-acceptance wrong poses.",
            "- Filtered stride-5 retrieval is retained for refinement, but it is still not a product localization output by itself.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def build_payload(audit_json: Path) -> dict[str, Any]:
    audit = load_json(audit_json)
    rows = [verifier_row(row) for row in audit.get("rows", [])]
    return {
        "policy": {
            "policy_version": POLICY_VERSION,
            "verifier_version": VERIFIER_VERSION,
        },
        "audit_json": display_path(audit_json),
        "thresholds": {
            "scan_context_max_mean_distance": SCAN_CONTEXT_MAX_MEAN_DISTANCE,
            "scan_context_min_hit_rate": SCAN_CONTEXT_MIN_HIT_RATE,
            "scan_context_max_candidate_evals_per_frame": (
                SCAN_CONTEXT_MAX_CANDIDATE_EVALS_PER_FRAME
            ),
            "localizer_min_accepted_rate": LOCALIZER_MIN_ACCEPTED_RATE,
        },
        "aggregate": aggregate(rows),
        "rows": rows,
    }


def main() -> int:
    args = parse_args()
    payload = build_payload(args.audit_json)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    output_json = args.output_dir / "fixed_map_global_hypothesis_verifier.json"
    output_md = args.output_dir / "fixed_map_global_hypothesis_verifier.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
