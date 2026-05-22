#!/usr/bin/env python3
"""Run lightweight LiDAR degeneracy report checks."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from collections import Counter
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = REPO_ROOT / "evaluation" / "scripts"
DEFAULT_POLICY_PATH = REPO_ROOT / "evaluation" / "config" / "lidar_degeneracy_triage_policy.json"
DEFAULT_METHOD_HEALTH_JSON = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "method_health_comparison"
    / "method_health_comparison.json"
)
DEFAULT_RISK_CALIBRATION_JSON = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "risk_gt_calibration"
    / "risk_gt_calibration.json"
)
COMPILE_TARGETS = [
    SCRIPTS / "run_lidar_degeneracy_checks.py",
    SCRIPTS / "summarize_lidar_degeneracy_health.py",
    SCRIPTS / "calibrate_lidar_degeneracy_risk.py",
    SCRIPTS / "test_lidar_degeneracy_triage_policy.py",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--regenerate-reports",
        action="store_true",
        help="Also rerun the method-health and risk-calibration report generators.",
    )
    parser.add_argument(
        "--enforce-policy",
        action="store_true",
        help="Fail if the generated or supplied report JSON exceeds the strict policy gate.",
    )
    parser.add_argument(
        "--policy-json",
        type=Path,
        default=DEFAULT_POLICY_PATH,
        help="Policy JSON with enforcement.strict_gate thresholds.",
    )
    parser.add_argument(
        "--method-health-json",
        type=Path,
        default=DEFAULT_METHOD_HEALTH_JSON,
        help="method_health_comparison.json to enforce when --enforce-policy is set.",
    )
    parser.add_argument(
        "--risk-calibration-json",
        type=Path,
        default=DEFAULT_RISK_CALIBRATION_JSON,
        help="risk_gt_calibration.json to enforce when --enforce-policy is set.",
    )
    parser.add_argument(
        "--policy-gate-output-dir",
        type=Path,
        default=None,
        help="Optional directory for policy_gate_report.json and .md from --enforce-policy.",
    )
    return parser.parse_args()


def run(cmd: list[str]) -> None:
    print("[run] " + " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def load_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        raise FileNotFoundError(f"required policy gate input is missing: {path}")
    return json.loads(path.read_text(encoding="utf-8"))


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def strict_gate_config(policy: dict[str, Any]) -> dict[str, Any]:
    enforcement = policy.get("enforcement")
    if not isinstance(enforcement, dict):
        raise RuntimeError("policy missing enforcement config")
    strict_gate = enforcement.get("strict_gate")
    if not isinstance(strict_gate, dict):
        raise RuntimeError("policy missing enforcement.strict_gate config")

    config = {
        "max_fail_rows": strict_gate.get("max_fail_rows", 0),
        "max_investigate_rows": strict_gate.get("max_investigate_rows", 0),
        "require_policy_version_match": bool(strict_gate.get("require_policy_version_match", True)),
    }
    for key in ("max_fail_rows", "max_investigate_rows"):
        value = config[key]
        if not isinstance(value, int) or value < 0:
            raise RuntimeError(f"policy enforcement.strict_gate.{key} must be a non-negative integer")
    return config


def ordered_policy_decisions(policy: dict[str, Any]) -> list[str]:
    decision_order = policy.get("decision_order")
    if not isinstance(decision_order, dict):
        raise RuntimeError("policy missing decision_order")
    return [
        str(name)
        for name, _rank in sorted(
            decision_order.items(),
            key=lambda item: int(item[1]),
        )
    ]


def normalize_policy_reasons(value: Any) -> list[str]:
    if isinstance(value, list):
        return [str(item) for item in value if str(item)]
    if isinstance(value, str):
        parts = [part.strip() for part in value.split(",") if part.strip()]
        return [part.split(":", 1)[0].strip() for part in parts]
    return []


def method_health_policy_records(payload: dict[str, Any]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for sequence in payload.get("sequences", []):
        sequence_name = str(sequence.get("sequence", "unknown_sequence"))
        for window in sequence.get("windows", []):
            window_name = str(window.get("name", "unknown_window"))
            start = window.get("start")
            end = window.get("end")
            for method, row in window.get("methods", {}).items():
                records.append(
                    {
                        "sequence": sequence_name,
                        "window": window_name,
                        "start": start,
                        "end": end,
                        "method": str(method),
                        "policy_decision": str(row.get("policy_decision") or "missing"),
                        "policy_reasons": normalize_policy_reasons(row.get("policy_reasons")),
                    }
                )
    return records


def risk_calibration_policy_records(payload: dict[str, Any]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for row in payload.get("proxy_records", []):
        records.append(
            {
                "sequence": str(row.get("sequence", "unknown_sequence")),
                "window": str(row.get("window", "unknown_window")),
                "start": row.get("start"),
                "end": row.get("end"),
                "method": str(row.get("method", "unknown_method")),
                "policy_decision": str(row.get("policy_decision") or "missing"),
                "policy_reasons": normalize_policy_reasons(
                    row.get("risk_reasons") or row.get("policy_reasons")
                ),
            }
        )
    return records


def report_policy_version(payload: dict[str, Any]) -> str | None:
    policy = payload.get("policy")
    if not isinstance(policy, dict):
        return None
    version = policy.get("policy_version")
    return str(version) if version is not None else None


def format_counts(counts: Counter[str], decisions: list[str]) -> str:
    parts = [f"{decision}={counts.get(decision, 0)}" for decision in decisions]
    known = set(decisions)
    for decision in sorted(set(counts) - known):
        parts.append(f"{decision}={counts[decision]}")
    return " ".join(parts)


def serializable_counts(counts: Counter[str], decisions: list[str]) -> dict[str, int]:
    out = {decision: int(counts.get(decision, 0)) for decision in decisions}
    for decision in sorted(set(counts) - set(decisions)):
        out[decision] = int(counts[decision])
    return out


def report_summary(
    *,
    name: str,
    path: Path,
    payload: dict[str, Any],
    records: list[dict[str, Any]],
    decisions: list[str],
) -> dict[str, Any]:
    counts = Counter(str(record.get("policy_decision") or "missing") for record in records)
    return {
        "path": display_path(path),
        "policy_version": report_policy_version(payload),
        "total_rows": sum(counts.values()),
        "counts": serializable_counts(counts, decisions),
    }


def evaluate_report(
    *,
    name: str,
    path: Path,
    payload: dict[str, Any],
    records: list[dict[str, Any]],
    policy: dict[str, Any],
    decisions: list[str],
    gate: dict[str, Any],
) -> list[str]:
    counts = Counter(str(record.get("policy_decision") or "missing") for record in records)
    total = sum(counts.values())
    print(f"[gate] {name}: total={total} {format_counts(counts, decisions)}")

    violations: list[str] = []
    if total == 0:
        violations.append(f"{name}: no policy rows found in {path}")

    unknown_decisions = sorted(set(counts) - set(decisions))
    if unknown_decisions:
        violations.append(f"{name}: unknown policy decisions {unknown_decisions}")

    expected_version = str(policy.get("policy_version"))
    actual_version = report_policy_version(payload)
    if gate["require_policy_version_match"] and actual_version != expected_version:
        violations.append(
            f"{name}: report policy version {actual_version!r} != {expected_version!r}"
        )

    if counts.get("fail", 0) > gate["max_fail_rows"]:
        violations.append(
            f"{name}: fail rows {counts['fail']} > {gate['max_fail_rows']}"
        )
    if counts.get("investigate", 0) > gate["max_investigate_rows"]:
        violations.append(
            f"{name}: investigate rows {counts['investigate']} > {gate['max_investigate_rows']}"
        )
    return violations


def policy_gate_offenders(
    records_by_report: dict[str, list[dict[str, Any]]],
    decisions: list[str],
) -> list[dict[str, Any]]:
    known = set(decisions)
    offenders: list[dict[str, Any]] = []
    for report, records in records_by_report.items():
        for record in records:
            decision = str(record.get("policy_decision") or "missing")
            if decision in {"fail", "investigate"} or decision not in known:
                offenders.append({"report": report, **record})

    rank = {decision: idx for idx, decision in enumerate(decisions)}
    unknown_rank = len(rank)
    offenders.sort(
        key=lambda row: (
            -rank.get(str(row.get("policy_decision")), unknown_rank),
            str(row.get("report")),
            str(row.get("sequence")),
            str(row.get("window")),
            str(row.get("method")),
        )
    )
    return offenders


def offender_line(record: dict[str, Any]) -> str:
    reasons = record.get("policy_reasons") or []
    reason_text = ",".join(str(reason) for reason in reasons) if reasons else "n/a"
    frame_range = ""
    if record.get("start") is not None and record.get("end") is not None:
        frame_range = f"[{record['start']},{record['end']}) "
    return (
        f"{record['report']} {record['sequence']} {frame_range}{record['window']} "
        f"{record['method']} decision={record['policy_decision']} reasons={reason_text}"
    )


def print_top_offenders(offenders: list[dict[str, Any]], *, limit: int = 10) -> None:
    if not offenders:
        return
    print(f"[gate] top offenders (showing {min(limit, len(offenders))}/{len(offenders)}):")
    for record in offenders[:limit]:
        print(f"  - {offender_line(record)}")


def write_policy_gate_report(
    *,
    output_dir: Path,
    policy_path: Path,
    policy: dict[str, Any],
    gate: dict[str, Any],
    reports: dict[str, dict[str, Any]],
    violations: list[str],
    offenders: list[dict[str, Any]],
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    payload = {
        "policy": {
            "path": display_path(policy_path),
            "policy_version": policy.get("policy_version"),
            "strict_gate": gate,
        },
        "reports": reports,
        "violations": violations,
        "offender_count": len(offenders),
        "offenders": offenders,
    }
    output_json = output_dir / "policy_gate_report.json"
    output_md = output_dir / "policy_gate_report.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")

    lines = [
        "# LiDAR Degeneracy Policy Gate",
        "",
        f"Policy: `{policy.get('policy_version')}`",
        "",
        "## Reports",
        "",
        "| Report | Total | Pass | Watch | Investigate | Fail |",
        "| --- | ---: | ---: | ---: | ---: | ---: |",
    ]
    for name, summary in reports.items():
        counts = summary["counts"]
        lines.append(
            f"| `{name}` | {summary['total_rows']} | {counts.get('pass', 0)} | "
            f"{counts.get('watch', 0)} | {counts.get('investigate', 0)} | "
            f"{counts.get('fail', 0)} |"
        )
    lines.extend(["", "## Violations", ""])
    if violations:
        lines.extend(f"- {violation}" for violation in violations)
    else:
        lines.append("- none")
    lines.extend(
        [
            "",
            "## Top Offenders",
            "",
            "| Report | Sequence | Window | Method | Decision | Reasons |",
            "| --- | --- | --- | --- | --- | --- |",
        ]
    )
    for record in offenders[:50]:
        reasons = record.get("policy_reasons") or []
        reason_text = ", ".join(str(reason) for reason in reasons) if reasons else "n/a"
        lines.append(
            f"| `{record['report']}` | `{record['sequence']}` | `{record['window']}` | "
            f"`{record['method']}` | `{record['policy_decision']}` | {reason_text} |"
        )
    if not offenders:
        lines.append("| n/a | n/a | n/a | n/a | n/a | n/a |")
    output_md.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"[gate] wrote {output_json} and {output_md}")


def enforce_policy_gate(
    *,
    policy_path: Path,
    method_health_json: Path,
    risk_calibration_json: Path,
    output_dir: Path | None,
) -> int:
    policy = load_json(policy_path)
    gate = strict_gate_config(policy)
    decisions = ordered_policy_decisions(policy)
    reports = [
        (
            "method_health_comparison",
            method_health_json,
            method_health_policy_records,
        ),
        (
            "risk_gt_calibration",
            risk_calibration_json,
            risk_calibration_policy_records,
        ),
    ]

    violations: list[str] = []
    records_by_report: dict[str, list[dict[str, Any]]] = {}
    report_summaries: dict[str, dict[str, Any]] = {}
    for name, path, record_loader in reports:
        payload = load_json(path)
        records = record_loader(payload)
        records_by_report[name] = records
        report_summaries[name] = report_summary(
            name=name,
            path=path,
            payload=payload,
            records=records,
            decisions=decisions,
        )
        violations.extend(
            evaluate_report(
                name=name,
                path=path,
                payload=payload,
                records=records,
                policy=policy,
                decisions=decisions,
                gate=gate,
            )
        )

    offenders = policy_gate_offenders(records_by_report, decisions)
    if output_dir is not None:
        write_policy_gate_report(
            output_dir=output_dir,
            policy_path=policy_path,
            policy=policy,
            gate=gate,
            reports=report_summaries,
            violations=violations,
            offenders=offenders,
        )

    if violations:
        print("[fail] lidar degeneracy policy gate failed")
        for violation in violations:
            print(f"  - {violation}")
        print_top_offenders(offenders)
        return 1
    print("[ok] lidar degeneracy policy gate passed")
    return 0


def main() -> int:
    args = parse_args()
    run([sys.executable, "-m", "py_compile", *[str(path) for path in COMPILE_TARGETS]])
    run([sys.executable, str(SCRIPTS / "test_lidar_degeneracy_triage_policy.py")])
    if args.regenerate_reports:
        run([sys.executable, str(SCRIPTS / "summarize_lidar_degeneracy_health.py")])
        run([sys.executable, str(SCRIPTS / "calibrate_lidar_degeneracy_risk.py")])
    if args.enforce_policy:
        try:
            gate_status = enforce_policy_gate(
                policy_path=args.policy_json,
                method_health_json=args.method_health_json,
                risk_calibration_json=args.risk_calibration_json,
                output_dir=args.policy_gate_output_dir,
            )
        except (FileNotFoundError, json.JSONDecodeError, OSError, RuntimeError, ValueError) as exc:
            print(f"[fail] {exc}", file=sys.stderr)
            return 1
        if gate_status != 0:
            return gate_status
    print("[ok] lidar degeneracy checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
