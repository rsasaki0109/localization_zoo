#!/usr/bin/env python3
"""Run official KISS-ICP on KITTI without exposing ground truth to odometry."""

from __future__ import annotations

import argparse
import glob
import hashlib
import importlib.metadata
import json
import os
import subprocess
import time
from pathlib import Path

import numpy as np

from evaluate_external_kitti_odometry import compute_metrics, load_kitti_poses, load_reference_csv


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dataset-root", required=True)
    parser.add_argument("--sequence", required=True)
    parser.add_argument("--reference-csv")
    parser.add_argument("--output-dir", required=True)
    parser.add_argument(
        "--official-python",
        default="/root/.venvs/kiss-icp-official/bin/python",
    )
    parser.add_argument("--method", default="official_kiss_icp_1.3.0")
    parser.add_argument(
        "--max-threads",
        type=int,
        default=0,
        help="Official registration thread cap; 0 keeps the official default.",
    )
    parser.add_argument("--_odometry-only", action="store_true", help=argparse.SUPPRESS)
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def assert_no_dataset_ground_truth(dataset_root: Path, sequence: str) -> None:
    path = dataset_root / "poses" / f"{sequence.zfill(2)}.txt"
    if path.exists():
        raise RuntimeError(f"Refusing to expose KITTI ground truth to KISS-ICP: {path}")


def write_kitti_poses(path: Path, poses: np.ndarray) -> None:
    np.savetxt(path, poses[:, :3, :].reshape(-1, 12), fmt="%.12g")


def build_odometry_command(args: argparse.Namespace, output_dir: Path) -> list[str]:
    """Build the GT-isolated child command.

    Keep this separate from ``main`` so a regression test can prove that the
    reference path never crosses the odometry-process boundary.
    """
    command = [
        args.official_python,
        str(Path(__file__).resolve()),
        "--dataset-root",
        str(Path(args.dataset_root).resolve()),
        "--sequence",
        args.sequence,
        "--output-dir",
        str(output_dir.resolve()),
    ]
    if getattr(args, "max_threads", 0) > 0:
        command.extend(["--max-threads", str(args.max_threads)])
    command.append("--_odometry-only")
    return command


def run_odometry_only(args: argparse.Namespace) -> int:
    from kiss_icp.config import load_config
    from kiss_icp.kiss_icp import KissICP
    from kiss_icp.pybind import kiss_icp_pybind

    dataset_root = Path(args.dataset_root).resolve()
    sequence = args.sequence.zfill(2)
    assert_no_dataset_ground_truth(dataset_root, sequence)
    scan_files = sorted(
        glob.glob(str(dataset_root / "sequences" / sequence / "velodyne" / "*.bin"))
    )
    if not scan_files:
        raise RuntimeError(f"No KITTI scans found for sequence {sequence}")

    config = load_config(None)
    if args.max_threads > 0:
        config.registration.max_num_threads = args.max_threads
    odometry = KissICP(config=config)
    poses = np.zeros((len(scan_files), 4, 4), dtype=np.float64)
    algorithm_ns = np.zeros(len(scan_files), dtype=np.int64)
    started = time.perf_counter()
    for index, scan_file in enumerate(scan_files):
        points = (
            np.fromfile(scan_file, dtype=np.float32)
            .reshape((-1, 4))[:, :3]
            .astype(np.float64)
        )
        points = np.asarray(
            kiss_icp_pybind._correct_kitti_scan(
                kiss_icp_pybind._Vector3dVector(points)
            )
        )
        frame_started = time.perf_counter_ns()
        odometry.register_frame(points, np.array([]))
        algorithm_ns[index] = time.perf_counter_ns() - frame_started
        poses[index] = odometry.last_pose
        if index % 50 == 0:
            print(f"[official-kiss] {index}/{len(scan_files)}", flush=True)
    wall_seconds = time.perf_counter() - started

    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    write_kitti_poses(output_dir / "estimate.txt", poses)
    (output_dir / "odometry_timing.json").write_text(
        json.dumps(
            {
                "frames": len(scan_files),
                "wall_seconds": wall_seconds,
                "algorithm_seconds": float(algorithm_ns.sum()) * 1e-9,
                "algorithm_time_samples": len(algorithm_ns),
                "kiss_icp_version": importlib.metadata.version("kiss-icp"),
            },
            indent=2,
        )
        + "\n"
    )
    (output_dir / "official_config.json").write_text(
        json.dumps(config.model_dump(mode="json"), indent=2) + "\n"
    )
    return 0


def main() -> int:
    args = parse_args()
    if args._odometry_only:
        return run_odometry_only(args)
    if not args.reference_csv:
        raise SystemExit("--reference-csv is required for evaluation")

    dataset_root = Path(args.dataset_root).resolve()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    assert_no_dataset_ground_truth(dataset_root, args.sequence)
    command = build_odometry_command(args, output_dir)
    child_environment = os.environ.copy()
    for key in list(child_environment):
        if key.lower().startswith("kiss_icp_"):
            child_environment.pop(key)
    log_path = output_dir / "official_kiss.log"
    with log_path.open("w") as log:
        completed = subprocess.run(
            command,
            stdout=log,
            stderr=subprocess.STDOUT,
            env=child_environment,
            check=False,
        )
    if completed.returncode != 0:
        raise subprocess.CalledProcessError(completed.returncode, command)

    # Ground truth is opened only after the isolated odometry child exits.
    estimate_path = output_dir / "estimate.txt"
    reference_path = Path(args.reference_csv).resolve()
    timing = json.loads((output_dir / "odometry_timing.json").read_text())
    metrics = compute_metrics(
        load_kitti_poses(estimate_path), load_reference_csv(reference_path)
    )
    metrics["algorithm_fps"] = (
        metrics["frames"] / timing["algorithm_seconds"]
        if timing["algorithm_seconds"] > 0
        else None
    )
    metrics["end_to_end_fps"] = (
        metrics["frames"] / timing["wall_seconds"]
        if timing["wall_seconds"] > 0
        else None
    )
    config_path = output_dir / "official_config.json"
    payload = {
        "schema_version": 1,
        "method": args.method,
        "gt_usage": "metrics_only_after_odometry_process_exit",
        "dataset_gt_absence_enforced": True,
        "first_pose_anchor_only": True,
        "command": command,
        "configuration": {
            "official_default": True,
            "config": str(config_path),
            "config_sha256": sha256_file(config_path),
            "kitti_elevation_correction": "official _correct_kitti_scan",
        },
        "versions": {
            "kiss_icp": timing["kiss_icp_version"],
            "official_python": args.official_python,
        },
        "artifacts": {
            "dataset_root": str(dataset_root),
            "estimate": str(estimate_path),
            "estimate_sha256": sha256_file(estimate_path),
            "reference_csv": str(reference_path),
            "reference_sha256": sha256_file(reference_path),
            "log": str(log_path),
        },
        "timing": timing,
        "metrics": metrics,
    }
    manifest_path = output_dir / "zoo_manifest.json"
    manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
