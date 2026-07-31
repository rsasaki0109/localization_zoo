#!/usr/bin/env python3
"""Re-score and audit the promoted LiDAR odometry v10 artifacts on external storage."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
CANDIDATE_PATH = (
    REPO_ROOT
    / "evaluation/data/lidar_odometry_candidate_direction_consistent_rotation_v10.json"
)
EVALUATOR_PATH = REPO_ROOT / "evaluation/scripts/evaluate_external_kitti_odometry.py"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--data-root",
        default=os.environ.get("LOCALIZATION_ZOO_DATA_ROOT", ""),
        help="External data root; defaults to LOCALIZATION_ZOO_DATA_ROOT.",
    )
    parser.add_argument(
        "--output-dir",
        default="",
        help="Audit output directory (default: <data-root>/results/v10_reproduction_audit).",
    )
    parser.add_argument("--skip-tests", action="store_true")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_evaluator() -> Any:
    spec = importlib.util.spec_from_file_location("lidar_v10_evaluator", EVALUATOR_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load evaluator: {EVALUATOR_PATH}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def check_hash(path: Path, expected: str) -> dict[str, Any]:
    actual = sha256_file(path) if path.is_file() else None
    return {
        "path": str(path),
        "exists": path.is_file(),
        "expected_sha256": expected,
        "actual_sha256": actual,
        "passed": actual == expected,
    }


def compare_metrics(
    actual: dict[str, Any], expected: dict[str, Any], tolerance: float = 1e-8
) -> dict[str, Any]:
    comparisons: dict[str, Any] = {}
    for key in ("ate_m", "rpe_trans_pct", "rpe_rot_deg_per_m"):
        expected_value = expected.get(key)
        actual_value = actual.get(key)
        if expected_value is None:
            continue
        delta = abs(float(actual_value) - float(expected_value))
        comparisons[key] = {
            "expected": expected_value,
            "actual": actual_value,
            "absolute_delta": delta,
            "passed": delta <= tolerance,
        }
    return comparisons


def score(
    evaluator: Any,
    estimate: Path,
    reference: Path,
    expected: dict[str, Any],
) -> dict[str, Any]:
    actual = evaluator.compute_metrics(
        evaluator.load_kitti_poses(estimate),
        evaluator.load_reference_csv(reference),
        100.0,
    )
    comparisons = compare_metrics(actual, expected)
    return {
        "estimate": str(estimate),
        "reference": str(reference),
        "metrics": actual,
        "comparisons": comparisons,
        "passed": all(item["passed"] for item in comparisons.values()),
    }


def relevant_test_command() -> list[str]:
    test_files = [
        "tests/test_direction_consistent_rotation_fusion.py",
        "tests/test_lidar_v10_manifest.py",
        "tests/test_lidar_sota_tools.py",
        "tests/test_official_kiss_pcd_odometry.py",
        "tests/test_external_odometry_evaluator.py",
        "tests/test_reproduce_lidar_v10.py",
        "tests/test_boreas_window_tools.py",
    ]
    return [sys.executable, "-m", "unittest", "-q", *test_files]


def main() -> int:
    args = parse_args()
    if not args.data_root:
        raise SystemExit("Set LOCALIZATION_ZOO_DATA_ROOT or pass --data-root")
    data_root = Path(args.data_root).resolve()
    output_dir = (
        Path(args.output_dir).resolve()
        if args.output_dir
        else data_root / "results/v10_reproduction_audit"
    )
    output_dir.mkdir(parents=True, exist_ok=True)
    candidate = json.loads(CANDIDATE_PATH.read_text(encoding="utf-8"))

    implementation = [
        check_hash(REPO_ROOT / relative, expected)
        for relative, expected in candidate["implementation_sha256"].items()
    ]
    fresh = candidate["fresh_held_out_result"]
    same_input = candidate["same_input_public_baseline_comparison"]
    fresh_root = data_root / "results/v10_fresh_heldout/kitti_raw_0023"
    fresh_gt = data_root / "reference_data/kitti_raw_2011_09_26_drive_0023_sync_gt.csv"
    same_root = data_root / "results/v10_same_input_baselines/kitti04"
    same_gt = data_root / "reference_data/kitti_seq_04_full_gt.csv"

    artifact_specs = [
        (
            fresh_root / "primary/dogfooding_results/KISS_ICP.txt",
            fresh["trajectory_sha256_before_gt_extraction"]["v6_primary"],
        ),
        (
            fresh_root / "official_kiss_adaptive/estimate.txt",
            fresh["trajectory_sha256_before_gt_extraction"]["load_adaptive_official_kiss"],
        ),
        (
            fresh_root / "fused/estimate.txt",
            fresh["trajectory_sha256_before_gt_extraction"]["frozen_v10"],
        ),
        (fresh_gt, fresh["reference_sha256_after_trajectory_freeze"]),
        (
            same_root / "v10_estimate.txt",
            same_input["rows"][0]["trajectory_sha256"],
        ),
    ]
    artifacts = [check_hash(path, expected) for path, expected in artifact_specs]
    evaluator = load_evaluator()
    scores = {
        "fresh_v6": score(
            evaluator,
            artifact_specs[0][0],
            fresh_gt,
            fresh["v6_primary"],
        ),
        "fresh_v10": score(
            evaluator,
            artifact_specs[2][0],
            fresh_gt,
            fresh["frozen_v10"],
        ),
        "same_input_v10": score(
            evaluator,
            same_root / "v10_estimate.txt",
            same_gt,
            same_input["rows"][0],
        ),
    }

    boreas_report: dict[str, Any] = {"available": False, "passed": True}
    boreas_manifest_path = REPO_ROOT / "evaluation/data/lidar_odometry_boreas_external_validation.json"
    boreas_manifest = json.loads(boreas_manifest_path.read_text(encoding="utf-8"))
    boreas_result = boreas_manifest.get("result", {})
    boreas_root = data_root / "results/v10_external_boreas"
    boreas_gt = data_root / "reference_data/boreas_2021_06_03_16_00_first600_gt.csv"
    if boreas_gt.is_file():
        boreas_artifact_specs = [
            (
                boreas_root / "primary/dogfooding_results/KISS_ICP.txt",
                boreas_result["trajectory_sha256_before_gt_download"]["v6_primary"],
            ),
            (
                boreas_root / "official_kiss/estimate.txt",
                boreas_result["trajectory_sha256_before_gt_download"]["official_kiss_1.3.0"],
            ),
            (
                boreas_root / "fused/estimate.txt",
                boreas_result["trajectory_sha256_before_gt_download"]["frozen_v10"],
            ),
            (boreas_gt, boreas_result["aligned_reference_sha256"]),
        ]
        boreas_artifacts = [check_hash(path, expected) for path, expected in boreas_artifact_specs]
        boreas_scores = {
            "v6_primary": score(
                evaluator, boreas_artifact_specs[0][0], boreas_gt, boreas_result["v6_primary"]
            ),
            "official_kiss_1.3.0": score(
                evaluator,
                boreas_artifact_specs[1][0],
                boreas_gt,
                boreas_result["official_kiss_1.3.0"],
            ),
            "frozen_v10": score(
                evaluator, boreas_artifact_specs[2][0], boreas_gt, boreas_result["frozen_v10"]
            ),
        }
        boreas_report = {
            "available": True,
            "artifacts": boreas_artifacts,
            "scores": boreas_scores,
            "passed": all(item["passed"] for item in boreas_artifacts)
            and all(item["passed"] for item in boreas_scores.values()),
        }

    tests: dict[str, Any] = {"skipped": args.skip_tests, "passed": True}
    if not args.skip_tests:
        completed = subprocess.run(
            relevant_test_command(), cwd=REPO_ROOT, text=True, capture_output=True
        )
        tests = {
            "skipped": False,
            "passed": completed.returncode == 0,
            "returncode": completed.returncode,
            "stdout": completed.stdout,
            "stderr": completed.stderr,
        }

    passed = (
        all(item["passed"] for item in implementation)
        and all(item["passed"] for item in artifacts)
        and all(item["passed"] for item in scores.values())
        and boreas_report["passed"]
        and tests["passed"]
    )
    payload = {
        "schema_version": 1,
        "candidate_id": candidate["candidate_id"],
        "data_root": str(data_root),
        "implementation_hashes": implementation,
        "artifact_hashes": artifacts,
        "scores": scores,
        "boreas_external": boreas_report,
        "tests": tests,
        "passed": passed,
    }
    report_path = output_dir / "audit.json"
    report_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": passed, "report": str(report_path)}, indent=2))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
