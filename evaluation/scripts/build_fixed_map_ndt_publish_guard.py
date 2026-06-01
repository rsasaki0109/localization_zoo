#!/usr/bin/env python3
"""Build publish-disposition guard rows from fixed-map NDT failure audit results."""

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

PUBLISH_GUARD_VERSION = "fixed_map_ndt_publish_guard_v1"
POLICY_VERSION = "lidar_degeneracy_triage_v4"

DECISION_ORDER = {
    "publish_candidate": 0,
    "profile_before_publish": 1,
    "lab_only": 2,
    "hold_for_verifier": 3,
    "return_unknown": 4,
    "block_publish": 5,
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


def publish_guard(row: dict[str, Any]) -> dict[str, Any]:
    source_class = str(row.get("source_class") or "unknown")
    seed_source = str(row.get("seed_source") or "unknown")
    policy_decision = str(row.get("policy_decision") or "missing")

    if has_reason(row, "accepted_bad_localization"):
        if has_reason(row, "unfiltered_ndt_score_trap"):
            return {
                "publish_decision": "block_publish",
                "allow_pose_publish": False,
                "safety_state": "global_candidate_score_trap",
                "component": "global_initializer",
                "required_gate": "calibrated_global_hypothesis_verifier",
                "recommended_action": (
                    "Do not publish this pose stream; NDT score alone selected bad "
                    "unfiltered global candidates."
                ),
            }
        return {
            "publish_decision": "block_publish",
            "allow_pose_publish": False,
            "safety_state": "accepted_wrong_pose",
            "component": "localization_safety_gate",
            "required_gate": "second_pose_verifier",
            "recommended_action": (
                "Treat high-acceptance/high-error localization as publish-blocking "
                "until another verifier rejects the wrong pose."
            ),
        }

    if has_reason(row, "rejected_bad_seed_detectable"):
        return {
            "publish_decision": "return_unknown",
            "allow_pose_publish": False,
            "safety_state": "bad_seed_detected",
            "component": "seed_fallback",
            "required_gate": "unknown_state_output",
            "recommended_action": (
                "Return unknown or relocalize instead of publishing dead-reckoned "
                "fallback poses."
            ),
        }

    if has_reason(row, "bad_localization"):
        return {
            "publish_decision": "hold_for_verifier",
            "allow_pose_publish": False,
            "safety_state": "map_localization_gap",
            "component": "map_localization_backend",
            "required_gate": "map_consistency_or_sequence_verifier",
            "recommended_action": (
                "Hold publish until seed quality, NDT basin limits, and fixed-map "
                "acceptance scores agree."
            ),
        }

    if source_class in {"oracle_seed", "dense_db_smoke", "basin_probe"}:
        return {
            "publish_decision": "lab_only",
            "allow_pose_publish": False,
            "safety_state": source_class,
            "component": "benchmark_scaffold",
            "required_gate": "non_oracle_runtime_initializer",
            "recommended_action": (
                "Keep this as a benchmark smoke test; it is not a deployable live "
                "initializer."
            ),
        }

    if policy_decision == "pass":
        return {
            "publish_decision": "publish_candidate",
            "allow_pose_publish": True,
            "safety_state": "candidate",
            "component": f"fixed_map_ndt:{seed_source}",
            "required_gate": "runtime_monitoring",
            "recommended_action": (
                "Allow as a candidate only with runtime, score, and drift monitoring enabled."
            ),
        }

    if has_reason(row, "runtime_below_budget"):
        return {
            "publish_decision": "profile_before_publish",
            "allow_pose_publish": False,
            "safety_state": "runtime_tail_risk",
            "component": f"fixed_map_ndt:{seed_source}",
            "required_gate": "runtime_budget",
            "recommended_action": (
                "Improve runtime or bound the runtime tail before allowing live pose publish."
            ),
        }

    return {
        "publish_decision": "hold_for_verifier",
        "allow_pose_publish": False,
        "safety_state": "needs_verifier",
        "component": f"fixed_map_ndt:{seed_source}",
        "required_gate": "additional_evidence",
        "recommended_action": (
            "Keep out of publish path until a verifier proves this row is safe."
        ),
    }


def guarded_row(row: dict[str, Any]) -> dict[str, Any]:
    guard = publish_guard(row)
    return {
        "sequence": row.get("sequence"),
        "variant": row.get("variant"),
        "seed_source": row.get("seed_source"),
        "source_class": row.get("source_class"),
        "policy_decision": row.get("policy_decision"),
        "policy_reasons": row.get("policy_reasons") or [],
        "ate_m": row.get("ate_m"),
        "rpe_trans_pct": row.get("rpe_trans_pct"),
        "fps": row.get("fps"),
        "accepted": row.get("accepted"),
        "attempted": row.get("attempted"),
        "accepted_rate": row.get("accepted_rate"),
        **guard,
    }


def aggregate(rows: list[dict[str, Any]]) -> dict[str, Any]:
    decisions = Counter(str(row["publish_decision"]) for row in rows)
    sources = Counter(str(row.get("seed_source") or "unknown") for row in rows)
    safety_states = Counter(str(row.get("safety_state") or "unknown") for row in rows)
    required_gates = Counter(str(row.get("required_gate") or "unknown") for row in rows)
    blocked_sources = Counter(
        str(row.get("seed_source") or "unknown")
        for row in rows
        if row["publish_decision"] in {"block_publish", "return_unknown"}
    )
    return {
        "rows": len(rows),
        "publishable_rows": sum(1 for row in rows if row.get("allow_pose_publish")),
        "decision_counts": dict(sorted(decisions.items())),
        "seed_source_counts": dict(sorted(sources.items())),
        "safety_state_counts": dict(sorted(safety_states.items())),
        "required_gate_counts": dict(sorted(required_gates.items())),
        "blocked_seed_source_counts": dict(sorted(blocked_sources.items())),
    }


def fmt(value: Any, digits: int = 3) -> str:
    number = as_float(value)
    if number is None:
        return "n/a"
    return f"{number:.{digits}f}"


def fraction(row: dict[str, Any]) -> str:
    accepted = row.get("accepted")
    attempted = row.get("attempted")
    if accepted is None or attempted is None:
        return "n/a"
    return f"{accepted}/{attempted}"


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    aggregate_row = payload["aggregate"]
    decisions = ", ".join(
        f"{name}:{count}" for name, count in aggregate_row["decision_counts"].items()
    )
    gates = ", ".join(
        f"{name}:{count}" for name, count in aggregate_row["required_gate_counts"].items()
    )
    if aggregate_row["publishable_rows"]:
        publish_readout = (
            f"- `{aggregate_row['publishable_rows']}` rows are publish candidates; "
            "they still require runtime, score, and drift monitoring before live use."
        )
    else:
        publish_readout = (
            "- The current fixed-map NDT slice has zero publishable non-oracle rows, "
            "so the next implementation target is a verifier for global hypotheses, "
            "not a new leaderboard table."
        )
    lines = [
        "# Fixed-Map NDT Publish Guard",
        "",
        f"Audit source: `{payload['audit_json']}`",
        "",
        "## Aggregate",
        "",
        "| Rows | Publishable | Decisions | Blocked seed sources |",
        "| ---: | ---: | --- | --- |",
        f"| {aggregate_row['rows']} | {aggregate_row['publishable_rows']} | "
        f"{decisions or 'none'} | "
        f"{', '.join(f'{name}:{count}' for name, count in aggregate_row['blocked_seed_source_counts'].items()) or 'none'} |",
        "",
        f"Required gates: {gates or 'none'}",
        "",
        "## Guard Rows",
        "",
        "| Sequence | Variant | Seed | Audit | Publish decision | Safety state | ATE [m] | RPE [%] | FPS | Accepted | Required gate |",
        "| --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |",
    ]
    rows = sorted(
        payload["rows"],
        key=lambda row: (
            DECISION_ORDER.get(str(row["publish_decision"]), 99),
            str(row.get("sequence")),
            str(row.get("variant")),
        ),
    )
    for row in rows:
        lines.append(
            f"| `{row['sequence']}` | `{row['variant']}` | `{row['seed_source']}` | "
            f"`{row['policy_decision']}` | `{row['publish_decision']}` | "
            f"`{row['safety_state']}` | {fmt(row['ate_m'])} | "
            f"{fmt(row['rpe_trans_pct'])} | {fmt(row['fps'])} | "
            f"{fraction(row)} | `{row['required_gate']}` |"
        )
    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- This guard is generated from GT-backed audit results. It is a product-contract candidate, not a runtime estimator by itself.",
            "- `allow_pose_publish=false` means the row should publish `unknown`, retry relocalization, or stay in a lab-only benchmark path.",
            publish_readout,
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def build_payload(audit_json: Path) -> dict[str, Any]:
    audit = load_json(audit_json)
    rows = [guarded_row(row) for row in audit.get("rows", [])]
    return {
        "policy": {
            "policy_version": POLICY_VERSION,
            "publish_guard_version": PUBLISH_GUARD_VERSION,
        },
        "audit_json": display_path(audit_json),
        "thresholds": audit.get("thresholds", {}),
        "aggregate": aggregate(rows),
        "rows": rows,
    }


def main() -> int:
    args = parse_args()
    payload = build_payload(args.audit_json)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    output_json = args.output_dir / "fixed_map_ndt_publish_guard.json"
    output_md = args.output_dir / "fixed_map_ndt_publish_guard.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
