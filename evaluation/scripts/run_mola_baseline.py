#!/usr/bin/env python3
"""Run and evaluate MOLA-LO on KITTI without exposing GT to odometry."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import time
from pathlib import Path

from evaluate_external_kitti_odometry import (
    compute_metrics,
    load_reference_csv,
    load_tum_poses,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dataset-root", required=True, help="KITTI dataset directory.")
    parser.add_argument("--sequence", required=True)
    parser.add_argument("--reference-csv", required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--pipeline", required=True)
    parser.add_argument("--state-estimator-params", required=True)
    parser.add_argument("--method", default="mola_lo_gicp")
    parser.add_argument(
        "--mola-executable",
        default=shutil.which("mola-lidar-odometry-cli")
        or "mola-lidar-odometry-cli",
    )
    parser.add_argument("--only-first-n", type=int, default=None)
    parser.add_argument("--reuse-estimate", action="store_true")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build_odometry_command(
    args: argparse.Namespace, estimate_path: Path
) -> list[str]:
    command = [
        args.mola_executable,
        "-c",
        str(Path(args.pipeline).resolve()),
        "--state-estimator-param-file",
        str(Path(args.state_estimator_params).resolve()),
        "--input-kitti-seq",
        args.sequence,
        "--output-tum-path",
        str(estimate_path),
    ]
    if args.only_first_n is not None:
        command.extend(["--only-first-n", str(args.only_first_n)])
    return command


def assert_no_dataset_ground_truth(dataset_root: Path, sequence: str) -> None:
    """MOLA's KITTI source publishes GT when this conventional file exists."""
    gt_path = dataset_root / "poses" / f"{sequence}.txt"
    if gt_path.exists():
        raise RuntimeError(
            "Refusing to expose KITTI ground truth to MOLA child process: "
            f"{gt_path}"
        )


def limit_references_for_run(references: list, only_first_n: int | None) -> list:
    return references if only_first_n is None else references[:only_first_n]


def run_odometry(
    command: list[str], dataset_root: Path, log_path: Path
) -> float:
    child_environment = os.environ.copy()
    child_environment["KITTI_BASE_DIR"] = str(dataset_root)
    started = time.perf_counter()
    with log_path.open("w") as log:
        completed = subprocess.run(
            command,
            stdout=log,
            stderr=subprocess.STDOUT,
            env=child_environment,
            check=False,
        )
    elapsed = time.perf_counter() - started
    if completed.returncode != 0:
        raise subprocess.CalledProcessError(completed.returncode, command)
    return elapsed


def executable_version(executable: str) -> str | None:
    completed = subprocess.run(
        [executable, "--version"],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    text = completed.stdout.strip()
    return text or None


def debian_package_version(package: str) -> str | None:
    completed = subprocess.run(
        ["dpkg-query", "-W", "-f=${Version}", package],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        check=False,
    )
    return completed.stdout.strip() if completed.returncode == 0 else None


def main() -> int:
    args = parse_args()
    dataset_root = Path(args.dataset_root).resolve()
    reference_path = Path(args.reference_csv).resolve()
    pipeline_path = Path(args.pipeline).resolve()
    state_path = Path(args.state_estimator_params).resolve()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    estimate_path = output_dir / "trajectory.tum"
    log_path = output_dir / "mola.log"
    command = build_odometry_command(args, estimate_path)

    if args.reuse_estimate:
        if not estimate_path.is_file():
            raise SystemExit(f"Estimate does not exist: {estimate_path}")
        wall_seconds = None
    else:
        assert_no_dataset_ground_truth(dataset_root, args.sequence)
        # The child sees scans, timestamps, and calibration, but never the GT path.
        wall_seconds = run_odometry(command, dataset_root, log_path)
        if not estimate_path.is_file():
            raise RuntimeError(f"MOLA did not write {estimate_path}")

    # GT is deliberately opened only after the odometry process exits.
    estimates = load_tum_poses(estimate_path)
    references = limit_references_for_run(
        load_reference_csv(reference_path), args.only_first_n
    )
    metrics = compute_metrics(estimates, references)
    metrics["end_to_end_fps"] = (
        metrics["frames"] / wall_seconds
        if wall_seconds is not None and wall_seconds > 0
        else None
    )

    payload = {
        "schema_version": 1,
        "method": args.method,
        "gt_usage": "metrics_only_after_odometry_process_exit",
        "dataset_gt_absence_enforced": True,
        "first_pose_anchor_only": True,
        "command": command,
        "child_environment_overrides": {"KITTI_BASE_DIR": str(dataset_root)},
        "configuration": {
            "sequence": args.sequence,
            "only_first_n": args.only_first_n,
            "pipeline": str(pipeline_path),
            "pipeline_sha256": sha256_file(pipeline_path),
            "state_estimator_params": str(state_path),
            "state_estimator_params_sha256": sha256_file(state_path),
        },
        "versions": {
            "mola_lidar_odometry_cli": executable_version(args.mola_executable),
            "ros_humble_mola_lidar_odometry": debian_package_version(
                "ros-humble-mola-lidar-odometry"
            ),
            "ros_humble_mola_input_kitti_dataset": debian_package_version(
                "ros-humble-mola-input-kitti-dataset"
            ),
        },
        "artifacts": {
            "dataset_root": str(dataset_root),
            "estimate": str(estimate_path),
            "estimate_sha256": sha256_file(estimate_path),
            "reference_csv": str(reference_path),
            "reference_sha256": sha256_file(reference_path),
            "log": str(log_path),
        },
        "timing": {"wall_seconds": wall_seconds},
        "metrics": metrics,
    }
    manifest_path = output_dir / "zoo_manifest.json"
    manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
