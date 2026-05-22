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
    return parser.parse_args()


def run(cmd: list[str]) -> None:
    print("[run] " + " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def load_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        raise FileNotFoundError(f"required policy gate input is missing: {path}")
    return json.loads(path.read_text(encoding="utf-8"))


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


def method_health_policy_records(payload: dict[str, Any]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for sequence in payload.get("sequences", []):
        sequence_name = str(sequence.get("sequence", "unknown_sequence"))
        for window in sequence.get("windows", []):
            window_name = str(window.get("name", "unknown_window"))
            for method, row in window.get("methods", {}).items():
                records.append(
                    {
                        "sequence": sequence_name,
                        "window": window_name,
                        "method": str(method),
                        "policy_decision": str(row.get("policy_decision") or "missing"),
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
                "method": str(row.get("method", "unknown_method")),
                "policy_decision": str(row.get("policy_decision") or "missing"),
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


def enforce_policy_gate(
    *,
    policy_path: Path,
    method_health_json: Path,
    risk_calibration_json: Path,
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
    for name, path, record_loader in reports:
        payload = load_json(path)
        records = record_loader(payload)
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

    if violations:
        print("[fail] lidar degeneracy policy gate failed")
        for violation in violations:
            print(f"  - {violation}")
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
            )
        except (FileNotFoundError, json.JSONDecodeError, RuntimeError, ValueError) as exc:
            print(f"[fail] {exc}", file=sys.stderr)
            return 1
        if gate_status != 0:
            return gate_status
    print("[ok] lidar degeneracy checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
