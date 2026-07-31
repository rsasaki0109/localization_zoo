#!/usr/bin/env python3
"""Profile official KISS-ICP stages on PCD scans without loading ground truth."""

from __future__ import annotations

import argparse
import hashlib
import json
import platform
import time
from pathlib import Path

import numpy as np

from run_official_kiss_pcd_odometry import (
    collect_pcds,
    load_pcd_xyz_timestamps,
    pcd_schema,
    write_kitti_poses,
)


STAGES = (
    "input_load",
    "density_filter",
    "preprocess",
    "voxelize",
    "threshold_and_guess",
    "registration",
    "threshold_update",
    "map_update",
    "pose_update",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pcd-dir", required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--max-frames", type=int, default=-1)
    parser.add_argument("--max-threads", type=int, default=8)
    parser.add_argument(
        "--max-input-points",
        type=int,
        default=0,
        help="Causal deterministic point cap; 0 keeps every input point.",
    )
    parser.add_argument("--expected-estimate")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def summarize_durations(values_ns: list[int], frames: int) -> dict[str, float]:
    values_ms = np.asarray(values_ns, dtype=np.float64) * 1e-6
    seconds = float(values_ms.sum() * 1e-3)
    return {
        "seconds": seconds,
        "mean_ms": float(values_ms.mean()),
        "p50_ms": float(np.percentile(values_ms, 50)),
        "p95_ms": float(np.percentile(values_ms, 95)),
        "stage_fps": float(frames / seconds) if seconds > 0 else float("inf"),
    }


def summarize_counts(values: list[int]) -> dict[str, float | int]:
    counts = np.asarray(values, dtype=np.int64)
    return {
        "min": int(counts.min()),
        "mean": float(counts.mean()),
        "p50": float(np.percentile(counts, 50)),
        "p95": float(np.percentile(counts, 95)),
        "max": int(counts.max()),
    }


def compare_estimates(actual_path: Path, expected_path: Path) -> dict[str, float | bool]:
    actual = np.loadtxt(actual_path, dtype=np.float64)
    expected = np.loadtxt(expected_path, dtype=np.float64)
    if actual.shape != expected.shape:
        return {
            "same_shape": False,
            "max_abs_matrix_delta": float("inf"),
            "numeric_equivalent_at_1e_10": False,
        }
    max_delta = float(np.max(np.abs(actual - expected)))
    return {
        "same_shape": True,
        "max_abs_matrix_delta": max_delta,
        "numeric_equivalent_at_1e_10": bool(
            np.allclose(actual, expected, rtol=0.0, atol=1e-10)
        ),
    }


def density_cap(
    points: np.ndarray, timestamps: np.ndarray, max_points: int
) -> tuple[np.ndarray, np.ndarray]:
    if max_points <= 0 or points.shape[0] <= max_points:
        return points, timestamps
    indices = np.linspace(0, points.shape[0] - 1, num=max_points, dtype=np.int64)
    selected_timestamps = timestamps[indices] if timestamps.size else timestamps
    return points[indices], selected_timestamps


def profile_frame(odometry, points: np.ndarray, timestamps: np.ndarray):
    durations: dict[str, int] = {}

    started = time.perf_counter_ns()
    frame = odometry.preprocessor.preprocess(points, timestamps, odometry.last_delta)
    durations["preprocess"] = time.perf_counter_ns() - started

    started = time.perf_counter_ns()
    source, frame_downsample = odometry.voxelize(frame)
    durations["voxelize"] = time.perf_counter_ns() - started

    started = time.perf_counter_ns()
    sigma = odometry.adaptive_threshold.get_threshold()
    initial_guess = odometry.last_pose @ odometry.last_delta
    durations["threshold_and_guess"] = time.perf_counter_ns() - started

    started = time.perf_counter_ns()
    new_pose = odometry.registration.align_points_to_map(
        points=source,
        voxel_map=odometry.local_map,
        initial_guess=initial_guess,
        max_correspondance_distance=3 * sigma,
        kernel=sigma,
    )
    durations["registration"] = time.perf_counter_ns() - started

    started = time.perf_counter_ns()
    model_deviation = np.linalg.inv(initial_guess) @ new_pose
    odometry.adaptive_threshold.update_model_deviation(model_deviation)
    durations["threshold_update"] = time.perf_counter_ns() - started

    started = time.perf_counter_ns()
    odometry.local_map.update(frame_downsample, new_pose)
    durations["map_update"] = time.perf_counter_ns() - started

    started = time.perf_counter_ns()
    odometry.last_delta = np.linalg.inv(odometry.last_pose) @ new_pose
    odometry.last_pose = new_pose
    durations["pose_update"] = time.perf_counter_ns() - started
    return durations, frame.shape[0], frame_downsample.shape[0], source.shape[0]


def main() -> int:
    from kiss_icp.config import load_config
    from kiss_icp.kiss_icp import KissICP

    args = parse_args()
    pcd_dir = Path(args.pcd_dir).resolve()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    pcds = collect_pcds(pcd_dir, args.max_frames)
    if not pcds:
        raise RuntimeError(f"No PCD frames found in {pcd_dir}")

    has_timestamps, first_points = pcd_schema(pcds[0])
    config = load_config(None)
    config.registration.max_num_threads = args.max_threads
    odometry = KissICP(config=config)
    timings: dict[str, list[int]] = {stage: [] for stage in STAGES}
    raw_counts: list[int] = []
    selected_counts: list[int] = []
    processed_counts: list[int] = []
    map_counts: list[int] = []
    source_counts: list[int] = []
    poses: list[np.ndarray] = []

    wall_started = time.perf_counter_ns()
    for index, pcd in enumerate(pcds):
        started = time.perf_counter_ns()
        points, timestamps = load_pcd_xyz_timestamps(pcd)
        timings["input_load"].append(time.perf_counter_ns() - started)
        if points.shape[0] == 0:
            raise RuntimeError(f"Empty frame is unsupported for profiling: {pcd}")
        raw_counts.append(points.shape[0])
        started = time.perf_counter_ns()
        points, timestamps = density_cap(points, timestamps, args.max_input_points)
        timings["density_filter"].append(time.perf_counter_ns() - started)
        selected_counts.append(points.shape[0])
        durations, processed, map_points, source = profile_frame(
            odometry, points, timestamps
        )
        for stage, elapsed in durations.items():
            timings[stage].append(elapsed)
        processed_counts.append(processed)
        map_counts.append(map_points)
        source_counts.append(source)
        poses.append(odometry.last_pose.copy())
        if index % 50 == 0:
            print(f"[kiss-profile] {index}/{len(pcds)}", flush=True)
    wall_seconds = (time.perf_counter_ns() - wall_started) * 1e-9

    estimate_path = output_dir / "estimate.txt"
    write_kitti_poses(estimate_path, np.asarray(poses))
    estimate_hash = sha256_file(estimate_path)
    expected_path = Path(args.expected_estimate).resolve() if args.expected_estimate else None
    expected_hash = sha256_file(expected_path) if expected_path else None
    comparison = (
        compare_estimates(estimate_path, expected_path) if expected_path else None
    )
    stage_summary = {
        stage: summarize_durations(values, len(poses))
        for stage, values in timings.items()
    }
    measured_seconds = sum(row["seconds"] for row in stage_summary.values())
    for row in stage_summary.values():
        row["measured_share_pct"] = 100.0 * row["seconds"] / measured_seconds

    payload = {
        "schema_version": 1,
        "ground_truth_used": False,
        "input": {
            "pcd_dir": str(pcd_dir),
            "frames": len(poses),
            "first_pcd_has_timestamps": has_timestamps,
            "first_pcd_point_count": first_points,
        },
        "configuration": {
            "official_default": True,
            "max_threads": args.max_threads,
            "max_input_points": args.max_input_points,
            "platform": platform.platform(),
            "processor": platform.processor(),
        },
        "point_counts": {
            "raw": summarize_counts(raw_counts),
            "after_density_filter": summarize_counts(selected_counts),
            "after_preprocess": summarize_counts(processed_counts),
            "map_update": summarize_counts(map_counts),
            "registration_source": summarize_counts(source_counts),
        },
        "timing": {
            "wall_seconds": wall_seconds,
            "end_to_end_fps": len(poses) / wall_seconds,
            "measured_seconds": measured_seconds,
            "stages": stage_summary,
        },
        "artifacts": {
            "estimate": str(estimate_path),
            "estimate_sha256": estimate_hash,
            "expected_estimate": str(expected_path) if expected_path else None,
            "expected_estimate_sha256": expected_hash,
            "trajectory_hash_matches": expected_hash == estimate_hash if expected_hash else None,
            "trajectory_comparison": comparison,
        },
    }
    output = output_dir / "profile.json"
    output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2))
    equivalent = comparison is None or comparison["numeric_equivalent_at_1e_10"]
    return 0 if equivalent else 2


if __name__ == "__main__":
    raise SystemExit(main())
