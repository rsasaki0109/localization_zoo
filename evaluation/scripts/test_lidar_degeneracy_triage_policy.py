#!/usr/bin/env python3
"""Lightweight regression checks for the LiDAR degeneracy triage policy."""

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
from types import ModuleType


REPO_ROOT = Path(__file__).resolve().parents[2]
POLICY_PATH = REPO_ROOT / "evaluation" / "config" / "lidar_degeneracy_triage_policy.json"
SCRIPT_DIR = REPO_ROOT / "evaluation" / "scripts"


EXPECTED_REASON_DECISIONS = {
    "ok_no_risk": "pass",
    "low_convergence": "watch",
    "low_motion_margin_dominant": "watch",
    "partial_acceptance": "watch",
    "motion_margin_dominant": "watch",
    "overlap_tail": "watch",
    "path_disagrees_with_healthy_median": "investigate",
    "low_acceptance": "fail",
}


def load_module(name: str, path: Path) -> ModuleType:
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load module spec for {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def assert_equal(actual: object, expected: object, label: str) -> None:
    if actual != expected:
        raise AssertionError(f"{label}: expected {expected!r}, got {actual!r}")


def main() -> int:
    policy = json.loads(POLICY_PATH.read_text(encoding="utf-8"))
    for key in (
        "schema_version",
        "policy_version",
        "default_decision_for_unknown_reason",
        "decision_order",
        "reason_decisions",
        "enforcement",
    ):
        if key not in policy:
            raise AssertionError(f"policy missing required key: {key}")

    decisions = policy["decision_order"]
    reason_decisions = policy["reason_decisions"]
    strict_gate = policy["enforcement"].get("strict_gate", {})
    assert_equal(policy["policy_version"], "lidar_degeneracy_triage_v3", "policy_version")
    assert_equal(policy["default_decision_for_unknown_reason"], "watch", "default decision")
    assert_equal(decisions, {"pass": 0, "watch": 1, "investigate": 2, "fail": 3}, "decision order")
    assert_equal(policy["enforcement"].get("schema_version"), 1, "enforcement schema")
    assert_equal(strict_gate.get("max_fail_rows"), 0, "strict gate max_fail_rows")
    assert_equal(
        strict_gate.get("max_investigate_rows"),
        0,
        "strict gate max_investigate_rows",
    )
    assert_equal(
        strict_gate.get("require_policy_version_match"),
        True,
        "strict gate policy version match",
    )

    for reason, expected in EXPECTED_REASON_DECISIONS.items():
        assert_equal(reason_decisions.get(reason), expected, f"reason {reason}")

    summarize = load_module(
        "summarize_lidar_degeneracy_health",
        SCRIPT_DIR / "summarize_lidar_degeneracy_health.py",
    )
    calibrate = load_module(
        "calibrate_lidar_degeneracy_risk",
        SCRIPT_DIR / "calibrate_lidar_degeneracy_risk.py",
    )

    reason_sets = [
        ["ok_no_risk"],
        ["low_convergence"],
        ["motion_margin_dominant"],
        ["path_disagrees_with_healthy_median"],
        ["low_acceptance"],
        ["ok_no_risk", "motion_margin_dominant"],
        ["low_convergence", "low_acceptance"],
        ["unknown_future_reason"],
    ]
    for reasons in reason_sets:
        summarize_decision = summarize.policy_decision(reasons)
        calibrate_decision = calibrate.policy_decision(reasons)
        assert_equal(summarize_decision, calibrate_decision, f"script agreement {reasons}")
        if reasons == ["unknown_future_reason"]:
            assert_equal(summarize_decision, "watch", "unknown reason default")

    print(
        f"[ok] {POLICY_PATH.relative_to(REPO_ROOT)} "
        f"version={policy['policy_version']} reasons={len(reason_decisions)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
