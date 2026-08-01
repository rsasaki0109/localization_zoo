#!/usr/bin/env python3
"""Run and evaluate a frozen MAD-ICP baseline without exposing GT to odometry."""

from __future__ import annotations

import argparse
import hashlib
import importlib.metadata
import json
import re
import shutil
import subprocess
import time
from pathlib import Path

from evaluate_external_kitti_odometry import (
    compute_metrics,
    load_kitti_poses,
    load_reference_csv,
    matrix_from_flat,
    undo_conjugation,
)


MAD_KITTI_LIDAR_TO_BASE = [
    4.276802385584e-04,
    -9.999672484946e-01,
    -8.084491683471e-03,
    -1.198459927713e-02,
    -7.210626507497e-03,
    8.081198491645e-03,
    -9.999413164504e-01,
    -5.403984729748e-02,
    9.999738645903e-01,
    4.859485810390e-04,
    -7.206933692422e-03,
    -2.921968648686e-01,
]
MAD_MULRAN_LIDAR_TO_BASE = [
    -1.0,
    -0.0058,
    0.0,
    1.7042,
    0.0058,
    -1.0,
    0.0,
    -0.0210,
    0.0,
    0.0,
    1.0,
    1.8047,
]
ODOMETRY_TIME_PATTERN = re.compile(
    r"Time for odometry estimation \[ms\]:\s+([0-9.eE+-]+)"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-path", required=True, help="Directory of KITTI .bin scans.")
    parser.add_argument("--reference-csv", required=True, help="Zoo LiDAR-pose GT CSV.")
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--method", default="mad_icp_0.0.10_official16")
    parser.add_argument(
        "--mad-icp-executable",
        default=shutil.which("mad_icp") or "mad_icp",
    )
    parser.add_argument("--dataset-config", default="kitti")
    parser.add_argument("--mad-icp-params", default="default")
    parser.add_argument("--num-cores", type=int, default=16)
    parser.add_argument("--num-keyframes", type=int, default=16)
    parser.add_argument("--realtime", action="store_true")
    parser.add_argument(
        "--reuse-estimate",
        action="store_true",
        help="Skip odometry and evaluate output-dir/estimate.txt.",
    )
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def parse_odometry_time_ms(log_text: str) -> list[float]:
    return [
        float(match.group(1))
        for match in ODOMETRY_TIME_PATTERN.finditer(log_text)
    ]


def package_version(name: str) -> str | None:
    try:
        return importlib.metadata.version(name)
    except importlib.metadata.PackageNotFoundError:
        return None


def lidar_to_base_for_dataset(dataset_config: str) -> list[float]:
    transforms = {
        "kitti": MAD_KITTI_LIDAR_TO_BASE,
        "mulran": MAD_MULRAN_LIDAR_TO_BASE,
    }
    try:
        return transforms[dataset_config]
    except KeyError as exc:
        raise ValueError(
            f"Unsupported dataset config for pose conversion: {dataset_config}"
        ) from exc


def build_odometry_command(args: argparse.Namespace, output_dir: Path) -> list[str]:
    command = [
        args.mad_icp_executable,
        "--data-path",
        str(Path(args.data_path).resolve()),
        "--estimate-path",
        str(output_dir),
        "--dataset-config",
        args.dataset_config,
        "--mad-icp-params",
        args.mad_icp_params,
        "--num-cores",
        str(args.num_cores),
        "--num-keyframes",
        str(args.num_keyframes),
        "--noviz",
    ]
    if args.realtime:
        command.append("--realtime")
    return command


def run_odometry(command: list[str], log_path: Path) -> float:
    started = time.perf_counter()
    with log_path.open("w") as log:
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        assert process.stdout is not None
        for line in process.stdout:
            log.write(line)
            log.flush()
        return_code = process.wait()
    elapsed = time.perf_counter() - started
    if return_code != 0:
        raise subprocess.CalledProcessError(return_code, command)
    return elapsed


def main() -> int:
    args = parse_args()
    data_path = Path(args.data_path).resolve()
    reference_path = Path(args.reference_csv).resolve()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    estimate_path = output_dir / "estimate.txt"
    log_path = output_dir / "mad_icp.log"
    command = build_odometry_command(args, output_dir)

    if args.reuse_estimate:
        if not estimate_path.is_file():
            raise SystemExit(f"Estimate does not exist: {estimate_path}")
        wall_seconds = None
    else:
        # Deliberately run before opening or parsing the reference trajectory.
        # The child command contains no GT path.
        wall_seconds = run_odometry(command, log_path)
        if not estimate_path.is_file():
            raise RuntimeError(f"MAD-ICP did not write {estimate_path}")

    log_text = log_path.read_text() if log_path.is_file() else ""
    odometry_times_ms = parse_odometry_time_ms(log_text)
    estimates = undo_conjugation(
        load_kitti_poses(estimate_path),
        matrix_from_flat(lidar_to_base_for_dataset(args.dataset_config)),
    )
    references = load_reference_csv(reference_path)
    metrics = compute_metrics(estimates, references)
    algorithm_seconds = sum(odometry_times_ms) / 1000.0
    metrics["algorithm_fps"] = (
        metrics["frames"] / algorithm_seconds if algorithm_seconds > 0 else None
    )
    metrics["end_to_end_fps"] = (
        metrics["frames"] / wall_seconds
        if wall_seconds is not None and wall_seconds > 0
        else None
    )

    payload = {
        "schema_version": 1,
        "method": args.method,
        "gt_usage": "metrics_only_after_odometry_process_exit",
        "first_pose_anchor_only": True,
        "command": command,
        "configuration": {
            "dataset_config": args.dataset_config,
            "mad_icp_params": args.mad_icp_params,
            "num_cores": args.num_cores,
            "num_keyframes": args.num_keyframes,
            "realtime": args.realtime,
        },
        "versions": {
            "mad_icp": package_version("mad-icp"),
            "scikit_build_core": package_version("scikit-build-core"),
            "click": package_version("click"),
        },
        "artifacts": {
            "data_path": str(data_path),
            "estimate": str(estimate_path),
            "estimate_sha256": sha256_file(estimate_path),
            "reference_csv": str(reference_path),
            "reference_sha256": sha256_file(reference_path),
            "log": str(log_path),
        },
        "timing": {
            "wall_seconds": wall_seconds,
            "odometry_time_samples": len(odometry_times_ms),
            "algorithm_seconds": algorithm_seconds,
        },
        "metrics": metrics,
    }
    manifest_path = output_dir / "zoo_manifest.json"
    manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
