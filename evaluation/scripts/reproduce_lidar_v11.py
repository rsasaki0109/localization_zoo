#!/usr/bin/env python3
"""Audit the frozen, non-promoted LiDAR odometry v11 experiment on the SSD."""

from __future__ import annotations

import argparse
import importlib.util
import json
import os
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_DIR = Path(__file__).resolve().parent
CANDIDATE_PATH = REPO_ROOT / "evaluation/data/lidar_odometry_candidate_density_adaptive_reference_v11.json"
EXTERNAL_PATH = REPO_ROOT / "evaluation/data/lidar_odometry_v11_external_validation.json"


def load_module(name: str, path: Path):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--data-root", default=os.environ.get("LOCALIZATION_ZOO_DATA_ROOT", "")
    )
    parser.add_argument("--output-dir", default="")
    parser.add_argument("--skip-tests", action="store_true")
    return parser.parse_args()


def promotion_policy(candidate: dict, external: dict) -> dict[str, bool]:
    result = external["result"]
    accuracy = result["accuracy"]
    runtime = result["runtime_before_gt_download"]
    return {
        "tracking_gate_passed": accuracy["tracking_gate"].startswith("pass"),
        "accuracy_gate_passed": accuracy["accuracy_gate"] == "pass",
        "runtime_gate_passed": runtime["v11_runtime_gate"].startswith("pass"),
        "not_promoted": not candidate["promoted"],
        "rejection_recorded": result["promotion"].startswith("rejected_runtime"),
        "post_result_tuning_absent": not result["post_result_tuning"],
    }


def main() -> int:
    args = parse_args()
    if not args.data_root:
        raise SystemExit("Set LOCALIZATION_ZOO_DATA_ROOT or pass --data-root")
    data_root = Path(args.data_root).resolve()
    output_dir = (
        Path(args.output_dir).resolve()
        if args.output_dir
        else data_root / "results/v11_reproduction_audit"
    )
    output_dir.mkdir(parents=True, exist_ok=True)
    candidate = json.loads(CANDIDATE_PATH.read_text(encoding="utf-8"))
    external = json.loads(EXTERNAL_PATH.read_text(encoding="utf-8"))
    result = external["result"]
    v10_tools = load_module("v11_reproduce_helpers", SCRIPT_DIR / "reproduce_lidar_v10.py")
    evaluator = v10_tools.load_evaluator()

    implementation = [
        v10_tools.check_hash(REPO_ROOT / relative, expected)
        for relative, expected in candidate["implementation_sha256"].items()
    ]
    result_root = data_root / "results/v11_external_boreas"
    gt = data_root / "reference_data/boreas_2021_09_02_11_42_first600_gt.csv"
    trajectory_paths = {
        "v6_primary": result_root / "primary/dogfooding_results/KISS_ICP.txt",
        "v10_uncapped_reference": result_root / "v10_reference/estimate.txt",
        "frozen_v10": result_root / "v10_fused.txt",
        "v11_80k_reference": result_root / "v11_reference/estimate.txt",
        "frozen_v11": result_root / "v11_fused.txt",
    }
    artifact_hashes = [
        v10_tools.check_hash(
            path, result["trajectory_sha256_before_gt_download"][name]
        )
        for name, path in trajectory_paths.items()
    ]
    artifact_hashes.append(
        v10_tools.check_hash(
            gt, result["ground_truth_after_trajectory_freeze"]["aligned_reference_sha256"]
        )
    )
    scores = {
        name: v10_tools.score(evaluator, trajectory_paths[name], gt, expected)
        for name, expected in result["accuracy"].items()
        if name in trajectory_paths
    }
    policy = promotion_policy(candidate, external)
    policy_passed = (
        policy["tracking_gate_passed"]
        and policy["accuracy_gate_passed"]
        and not policy["runtime_gate_passed"]
        and policy["not_promoted"]
        and policy["rejection_recorded"]
        and policy["post_result_tuning_absent"]
    )

    tests = {"skipped": args.skip_tests, "passed": True}
    if not args.skip_tests:
        command = [
            sys.executable,
            "-m",
            "unittest",
            "-q",
            "tests/test_profile_official_kiss_pcd.py",
            "tests/test_lidar_v11_manifest.py",
            "tests/test_reproduce_lidar_v11.py",
        ]
        completed = subprocess.run(command, cwd=REPO_ROOT, text=True, capture_output=True)
        tests = {
            "skipped": False,
            "passed": completed.returncode == 0,
            "returncode": completed.returncode,
            "stdout": completed.stdout,
            "stderr": completed.stderr,
        }

    passed = (
        all(row["passed"] for row in implementation)
        and all(row["passed"] for row in artifact_hashes)
        and all(row["passed"] for row in scores.values())
        and policy_passed
        and tests["passed"]
    )
    payload = {
        "schema_version": 1,
        "candidate_id": candidate["candidate_id"],
        "data_root": str(data_root),
        "implementation_hashes": implementation,
        "artifact_hashes": artifact_hashes,
        "scores": scores,
        "promotion_policy": policy,
        "promotion_policy_passed": policy_passed,
        "tests": tests,
        "passed": passed,
    }
    report = output_dir / "audit.json"
    report.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": passed, "report": str(report)}, indent=2))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
