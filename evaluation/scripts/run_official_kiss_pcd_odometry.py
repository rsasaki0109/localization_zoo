#!/usr/bin/env python3
"""Run official KISS-ICP on a Zoo PCD sequence without any ground truth."""

from __future__ import annotations

import argparse
import hashlib
import importlib.metadata
import json
import time
from pathlib import Path

import numpy as np

from analyze_lidar_degeneracy_sequence import (
    collect_pcds,
    count_header_lines,
    pcd_dtype,
    read_pcd_header,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pcd-dir", required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--max-frames", type=int, default=-1)
    parser.add_argument(
        "--max-threads",
        type=int,
        default=0,
        help="Official registration thread cap; 0 keeps the official default.",
    )
    parser.add_argument(
        "--thread-policy",
        choices=("fixed", "timestamp_adaptive"),
        default="fixed",
        help=(
            "Thread-cap policy. timestamp_adaptive uses only the first PCD "
            "schema: timestamped or <=50k-point scans use 8 threads; larger "
            "untimestamped scans use 4."
        ),
    )
    parser.add_argument("--timestamped-max-threads", type=int, default=8)
    parser.add_argument("--untimestamped-max-threads", type=int, default=4)
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def normalize_timestamps(values: np.ndarray) -> np.ndarray:
    values = np.asarray(values, dtype=np.float64)
    if values.size == 0:
        return values
    span = float(np.max(values) - np.min(values))
    if not np.isfinite(span) or span <= np.finfo(np.float64).eps:
        return np.array([], dtype=np.float64)
    return (values - np.min(values)) / span


def load_pcd_xyz_timestamps(path: Path) -> tuple[np.ndarray, np.ndarray]:
    header, data_offset = read_pcd_header(path)
    data_kind = header.get("DATA", [""])[0].lower()
    point_count = int(header.get("POINTS", header.get("WIDTH", ["0"]))[0])
    dtype = pcd_dtype(header)
    if not {"x", "y", "z"}.issubset(dtype.names or ()):
        raise RuntimeError(f"PCD is missing x/y/z fields: {path}")
    if data_kind == "binary":
        with path.open("rb") as handle:
            handle.seek(data_offset)
            data = np.frombuffer(
                handle.read(point_count * dtype.itemsize),
                dtype=dtype,
                count=point_count,
            )
    elif data_kind == "ascii":
        data = np.genfromtxt(
            path,
            dtype=dtype,
            skip_header=count_header_lines(path),
            max_rows=point_count,
        )
        data = np.atleast_1d(data)
    else:
        raise RuntimeError(f"Unsupported PCD DATA mode `{data_kind}` in {path}")

    xyz = np.column_stack([data["x"], data["y"], data["z"]]).astype(np.float64)
    time_field = next(
        (name for name in ("time", "t", "timestamp") if name in (dtype.names or ())),
        None,
    )
    timestamps = (
        np.asarray(data[time_field], dtype=np.float64)
        if time_field is not None
        else np.array([], dtype=np.float64)
    )
    finite = np.isfinite(xyz).all(axis=1)
    if timestamps.size:
        finite &= np.isfinite(timestamps)
        timestamps = normalize_timestamps(timestamps[finite])
    return xyz[finite], timestamps


def write_kitti_poses(path: Path, poses: np.ndarray) -> None:
    np.savetxt(path, poses[:, :3, :].reshape(-1, 12), fmt="%.12g")


def pcd_schema(path: Path) -> tuple[bool, int]:
    header, _ = read_pcd_header(path)
    names = pcd_dtype(header).names or ()
    has_timestamps = any(name in names for name in ("time", "t", "timestamp"))
    point_count = int(header.get("POINTS", header.get("WIDTH", ["0"]))[0])
    return has_timestamps, point_count


def select_thread_cap(
    *,
    policy: str,
    fixed_cap: int,
    first_pcd_has_timestamps: bool,
    first_pcd_point_count: int,
    timestamped_cap: int,
    untimestamped_cap: int,
    small_scan_threshold: int = 50_000,
) -> int:
    if policy == "fixed":
        return fixed_cap
    if policy != "timestamp_adaptive":
        raise ValueError(f"Unknown thread policy: {policy}")
    use_timestamped_cap = (
        first_pcd_has_timestamps or first_pcd_point_count <= small_scan_threshold
    )
    selected = timestamped_cap if use_timestamped_cap else untimestamped_cap
    if selected <= 0:
        raise ValueError("Adaptive thread caps must be positive")
    return selected


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
    first_pcd_has_timestamps, first_pcd_point_count = pcd_schema(pcds[0])
    selected_thread_cap = select_thread_cap(
        policy=args.thread_policy,
        fixed_cap=args.max_threads,
        first_pcd_has_timestamps=first_pcd_has_timestamps,
        first_pcd_point_count=first_pcd_point_count,
        timestamped_cap=args.timestamped_max_threads,
        untimestamped_cap=args.untimestamped_max_threads,
    )
    config = load_config(None)
    if selected_thread_cap > 0:
        config.registration.max_num_threads = selected_thread_cap
    odometry = KissICP(config=config)
    poses: list[np.ndarray] = []
    algorithm_ns: list[int] = []
    source_frame_indices: list[int] = []
    skipped_empty_frames: list[int] = []
    timestamped_frames = 0
    started = time.perf_counter()
    for index, pcd in enumerate(pcds):
        points, timestamps = load_pcd_xyz_timestamps(pcd)
        if points.shape[0] == 0:
            skipped_empty_frames.append(index)
            continue
        timestamped_frames += int(
            timestamps.size > 0 and timestamps.size == points.shape[0]
        )
        frame_started = time.perf_counter_ns()
        odometry.register_frame(points, timestamps)
        algorithm_ns.append(time.perf_counter_ns() - frame_started)
        poses.append(odometry.last_pose.copy())
        source_frame_indices.append(index)
        if index % 50 == 0:
            print(f"[official-kiss-pcd] {index}/{len(pcds)}", flush=True)
    wall_seconds = time.perf_counter() - started

    estimate_path = output_dir / "estimate.txt"
    config_path = output_dir / "official_config.json"
    write_kitti_poses(estimate_path, np.asarray(poses))
    config_path.write_text(json.dumps(config.model_dump(mode="json"), indent=2) + "\n")
    algorithm_seconds = float(sum(algorithm_ns)) * 1e-9
    payload = {
        "schema_version": 1,
        "method": "official_kiss_icp_pcd_default",
        "ground_truth_used": False,
        "configuration": {
            "official_default": True,
            "thread_policy": args.thread_policy,
            "selected_max_threads": selected_thread_cap,
            "first_pcd_has_timestamps": first_pcd_has_timestamps,
            "first_pcd_point_count": first_pcd_point_count,
            "config": str(config_path),
            "config_sha256": sha256_file(config_path),
        },
        "versions": {"kiss_icp": importlib.metadata.version("kiss-icp")},
        "artifacts": {
            "pcd_dir": str(pcd_dir),
            "estimate": str(estimate_path),
            "estimate_sha256": sha256_file(estimate_path),
            "source_frame_indices": source_frame_indices,
            "skipped_empty_frames": skipped_empty_frames,
        },
        "timing": {
            "input_frames": len(pcds),
            "frames": len(poses),
            "timestamped_frames": timestamped_frames,
            "wall_seconds": wall_seconds,
            "algorithm_seconds": algorithm_seconds,
            "algorithm_fps": len(poses) / algorithm_seconds,
            "end_to_end_fps": len(poses) / wall_seconds,
        },
    }
    manifest_path = output_dir / "odometry_manifest.json"
    manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
