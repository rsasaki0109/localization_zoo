#!/usr/bin/env python3
"""Generate a repair action plan from a LiDAR degeneracy policy gate report."""

from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_GATE_REPORT = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "policy_gate"
    / "policy_gate_report.json"
)
DEFAULT_OUTPUT_DIR = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "policy_gate"
)

DECISION_PRIORITY = {
    "fail": 0,
    "investigate": 1,
    "watch": 2,
    "pass": 3,
}

CATEGORY_PRIORITY = {
    "hard_numerical_failure": 0,
    "local_matcher_failure": 1,
    "false_confidence_risk": 2,
    "cross_method_disagreement": 3,
    "unclassified_policy_reason": 4,
}

ACTION_TEMPLATES = {
    "hard_numerical_failure": {
        "title": "Stop hard numerical failures",
        "component": "pose output contract",
        "action": (
            "Add finite-pose guards, record the source frame, and reject non-finite "
            "updates before they can enter trajectory output."
        ),
    },
    "local_matcher_failure": {
        "title": "Repair local matcher acceptance and convergence",
        "component": "local registration",
        "action": (
            "Inspect per-pair registration decisions, overlap, and acceptance gates; "
            "add fallback or retry behavior before relaxing the strict gate."
        ),
    },
    "false_confidence_risk": {
        "title": "Calibrate false-confidence health signals",
        "component": "failure awareness",
        "action": (
            "Tighten motion-margin and overlap-tail checks so confident-looking "
            "trajectories are downgraded when degeneracy evidence is strong."
        ),
    },
    "cross_method_disagreement": {
        "title": "Investigate cross-method trajectory disagreement",
        "component": "cross-method validation",
        "action": (
            "Compare the suspect method against healthy peers, inspect path-length "
            "ratios, and require a second signal before accepting the pose stream."
        ),
    },
    "unclassified_policy_reason": {
        "title": "Classify unknown policy reasons",
        "component": "triage policy",
        "action": (
            "Add an explicit reason mapping and decide whether the policy should pass, "
            "watch, investigate, or fail for this signal."
        ),
    },
}

REASON_CATEGORIES = {
    "all_pairs_failed": "local_matcher_failure",
    "ct_icp_internal_convergence_low": "local_matcher_failure",
    "low_acceptance": "local_matcher_failure",
    "low_convergence": "local_matcher_failure",
    "low_motion_margin_dominant": "false_confidence_risk",
    "low_used_path": "local_matcher_failure",
    "partial_acceptance": "local_matcher_failure",
    "motion_margin_dominant": "false_confidence_risk",
    "overlap_tail": "false_confidence_risk",
    "cross_method_suspicious": "cross_method_disagreement",
    "no_healthy_peer": "cross_method_disagreement",
    "path_disagrees_with_all_method_median": "cross_method_disagreement",
    "path_disagrees_with_healthy_median": "cross_method_disagreement",
    "nonfinite_pose": "hard_numerical_failure",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--gate-report",
        type=Path,
        default=DEFAULT_GATE_REPORT,
        help="Input policy_gate_report.json.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help="Directory for policy_gate_action_plan.json and .md.",
    )
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def classify_reason(reason: str) -> str:
    return REASON_CATEGORIES.get(reason, "unclassified_policy_reason")


def offender_categories(offender: dict[str, Any]) -> list[str]:
    categories = {
        classify_reason(str(reason))
        for reason in offender.get("policy_reasons", [])
    }
    if not categories:
        categories.add("unclassified_policy_reason")
    return sorted(categories, key=lambda category: CATEGORY_PRIORITY.get(category, 99))


def frame_range(record: dict[str, Any]) -> str:
    if record.get("start") is None or record.get("end") is None:
        return "n/a"
    return f"[{record['start']},{record['end']})"


def build_action_plan(gate_report: dict[str, Any], *, gate_report_path: Path | None = None) -> dict[str, Any]:
    groups: dict[tuple[str, str, str], dict[str, Any]] = {}
    for offender in gate_report.get("offenders", []):
        decision = str(offender.get("policy_decision", "missing"))
        method = str(offender.get("method", "unknown_method"))
        for category in offender_categories(offender):
            key = (decision, category, method)
            group = groups.setdefault(
                key,
                {
                    "policy_decision": decision,
                    "category": category,
                    "method": method,
                    "offender_rows": 0,
                    "reports": Counter(),
                    "reasons": Counter(),
                    "windows": {},
                    "evidence": [],
                },
            )
            group["offender_rows"] += 1
            group["reports"][str(offender.get("report", "unknown_report"))] += 1
            for reason in offender.get("policy_reasons", []):
                if classify_reason(str(reason)) == category:
                    group["reasons"][str(reason)] += 1
            window_key = (
                str(offender.get("sequence", "unknown_sequence")),
                str(offender.get("window", "unknown_window")),
                offender.get("start"),
                offender.get("end"),
            )
            window = group["windows"].setdefault(
                window_key,
                {
                    "sequence": window_key[0],
                    "window": window_key[1],
                    "start": window_key[2],
                    "end": window_key[3],
                    "rows": 0,
                },
            )
            window["rows"] += 1
            if len(group["evidence"]) < 8:
                group["evidence"].append(offender)

    action_items: list[dict[str, Any]] = []
    for group in groups.values():
        template = ACTION_TEMPLATES[group["category"]]
        windows = sorted(
            group["windows"].values(),
            key=lambda row: (
                str(row["sequence"]),
                int(row["start"]) if row.get("start") is not None else -1,
                str(row["window"]),
            ),
        )
        action_items.append(
            {
                "priority": 0,
                "policy_decision": group["policy_decision"],
                "category": group["category"],
                "method": group["method"],
                "title": template["title"],
                "component": template["component"],
                "recommended_action": template["action"],
                "offender_rows": group["offender_rows"],
                "reports": dict(sorted(group["reports"].items())),
                "reasons": dict(group["reasons"].most_common()),
                "windows": windows,
                "evidence": group["evidence"],
            }
        )

    action_items.sort(
        key=lambda item: (
            DECISION_PRIORITY.get(str(item["policy_decision"]), 99),
            CATEGORY_PRIORITY.get(str(item["category"]), 99),
            -int(item["offender_rows"]),
            str(item["method"]),
        )
    )
    for index, item in enumerate(action_items, start=1):
        item["priority"] = index

    category_counts: dict[str, int] = defaultdict(int)
    method_counts: dict[str, int] = defaultdict(int)
    for item in action_items:
        category_counts[str(item["category"])] += int(item["offender_rows"])
        method_counts[str(item["method"])] += int(item["offender_rows"])

    return {
        "source_gate_report": display_path(gate_report_path) if gate_report_path else None,
        "policy": gate_report.get("policy", {}),
        "summary": {
            "action_items": len(action_items),
            "offender_rows": int(gate_report.get("offender_count", 0)),
            "category_rows": dict(sorted(category_counts.items())),
            "method_rows": dict(sorted(method_counts.items())),
        },
        "action_items": action_items,
    }


def write_markdown(path: Path, plan: dict[str, Any]) -> None:
    lines = [
        "# LiDAR Degeneracy Policy Gate Action Plan",
        "",
        f"Source: `{plan.get('source_gate_report')}`",
        f"Policy: `{plan.get('policy', {}).get('policy_version')}`",
        "",
        "## Summary",
        "",
        f"- Action items: {plan['summary']['action_items']}",
        f"- Offender rows: {plan['summary']['offender_rows']}",
        "",
        "## Recommended Order",
        "",
        "| Priority | Decision | Category | Method | Rows | Top reasons | Action |",
        "| ---: | --- | --- | --- | ---: | --- | --- |",
    ]
    for item in plan["action_items"]:
        reasons = ", ".join(item["reasons"].keys()) if item["reasons"] else "n/a"
        lines.append(
            f"| {item['priority']} | `{item['policy_decision']}` | "
            f"`{item['category']}` | `{item['method']}` | {item['offender_rows']} | "
            f"{reasons} | {item['recommended_action']} |"
        )

    lines.extend(["", "## Action Detail", ""])
    for item in plan["action_items"]:
        lines.extend(
            [
                f"### P{item['priority']} {item['title']} - `{item['method']}`",
                "",
                f"- Decision: `{item['policy_decision']}`",
                f"- Category: `{item['category']}`",
                f"- Component: {item['component']}",
                f"- Rows: {item['offender_rows']}",
                f"- Reports: {', '.join(f'{name}={count}' for name, count in item['reports'].items())}",
                f"- Reasons: {', '.join(f'{name}={count}' for name, count in item['reasons'].items()) or 'n/a'}",
                f"- Recommended action: {item['recommended_action']}",
                "",
                "| Sequence | Window | Frames | Rows |",
                "| --- | --- | --- | ---: |",
            ]
        )
        for window in item["windows"]:
            lines.append(
                f"| `{window['sequence']}` | `{window['window']}` | "
                f"`{frame_range(window)}` | {window['rows']} |"
            )
        lines.append("")

    path.write_text("\n".join(lines).rstrip() + "\n", encoding="utf-8")


def write_outputs(plan: dict[str, Any], output_dir: Path) -> tuple[Path, Path]:
    output_dir.mkdir(parents=True, exist_ok=True)
    output_json = output_dir / "policy_gate_action_plan.json"
    output_md = output_dir / "policy_gate_action_plan.md"
    output_json.write_text(json.dumps(plan, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, plan)
    return output_json, output_md


def main() -> int:
    args = parse_args()
    gate_report = load_json(args.gate_report)
    plan = build_action_plan(gate_report, gate_report_path=args.gate_report)
    output_json, output_md = write_outputs(plan, args.output_dir)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
