#!/usr/bin/env python3
"""Run official CT-ICP (jedeschaud/ct_icp) on KITTI Odometry without exposing ground truth.

The KITTI .bin scans are written in upstream's `KITTI` dataset layout
(`<root>/<seq>/frames/frame_XXXX.ply`, float x/y/z + double timestamp, no
poses file), `run_odometry` runs with `config/odometry/driving_config.yaml`
(viewer and progress output off), and the saved trajectory is resampled at
the KITTI frame times (0.1 i + 0.05 s, upstream's `save_mid_frame` convention)
and scored with the Zoo protocol.

Per-point time: the scans carry none, so frame i spans [0.1 i, 0.1 (i + 1)]
and the point time follows the sweep angle, 0.5 * (1 - atan2(y, x) / pi), as
in KISS-ICP's KITTI loader (`--timestamps index` uses the stored point order).

Upstream builds against its MappingResearchKEU superbuild. On a current
toolchain it needs three local patches: `alternate_stack[65536]` instead of
`[SIGSTKSZ]` in src/SlamCore/utils.cxx (glibc >= 2.34), nanoflann 1.4.3 with
`#include <cstdint>`, and, only for `--no-kitti-elevation-correction` and
`--elevation-offset-deg`, reading KITTI_GLOBAL_VERTICAL_ANGLE_OFFSET in
src/ct_icp/dataset.cpp from the CTICP_KITTI_ANGLE_OFFSET environment variable.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import time
from pathlib import Path

import numpy as np

from evaluate_external_kitti_odometry import compute_metrics, load_reference_csv


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--velodyne-dir", help="KITTI sequences/<seq>/velodyne directory.")
    parser.add_argument(
        "--convert-only",
        action="store_true",
        help="Only write the PLY frames to <output-dir>/data (to free the .bin files before the run).",
    )
    parser.add_argument("--sequence", required=True)
    parser.add_argument("--reference-csv", required=True, help="Zoo lidar_pose GT CSV (metrics only).")
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--run-odometry", required=True, help="Path to upstream's run_odometry binary.")
    parser.add_argument("--config", required=True, help="Upstream config/odometry/driving_config.yaml.")
    parser.add_argument("--method", default="official_ct_icp")
    parser.add_argument("--timestamps", choices=("azimuth", "index"), default="azimuth")
    parser.add_argument(
        "--no-kitti-elevation-correction",
        action="store_true",
        help="Run without upstream's 0.205 deg KITTI correction (needs the environment patch).",
    )
    parser.add_argument(
        "--elevation-offset-deg",
        type=float,
        default=None,
        help="Override upstream's 0.205 deg offset (needs the environment patch); for sensitivity runs.",
    )
    parser.add_argument("--keep-frames", action="store_true", help="Keep the converted PLY frames.")
    parser.add_argument(
        "--remove-scans-after-conversion",
        action="store_true",
        help="Delete each .bin scan once its PLY frame is written (for small disks; destroys the input).",
    )
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def write_frames(velodyne_dir: Path, frames_dir: Path, mode: str, remove_source: bool = False) -> int:
    frames_dir.mkdir(parents=True, exist_ok=True)
    files = sorted(velodyne_dir.glob("*.bin"))
    dtype = np.dtype([("x", "<f4"), ("y", "<f4"), ("z", "<f4"), ("t", "<f8")])
    for index, path in enumerate(files):
        points = np.fromfile(path, dtype="<f4").reshape(-1, 4)
        if mode == "azimuth":
            fraction = 0.5 * (1.0 - np.arctan2(points[:, 1], points[:, 0]) / np.pi)
        else:
            fraction = np.arange(len(points)) / max(len(points) - 1, 1)
        frame = np.empty(len(points), dtype=dtype)
        frame["x"], frame["y"], frame["z"] = points[:, 0], points[:, 1], points[:, 2]
        frame["t"] = 0.1 * index + 0.1 * np.clip(fraction, 0.0, 1.0)
        header = (
            "ply\nformat binary_little_endian 1.0\n"
            f"element vertex {len(points)}\n"
            "property float x\nproperty float y\nproperty float z\nproperty double timestamp\n"
            "end_header\n"
        ).encode()
        with (frames_dir / f"frame_{index:04d}.ply").open("wb") as handle:
            handle.write(header)
            handle.write(frame.tobytes())
        if remove_source:
            path.unlink()
    return len(files)


def write_config(template: Path, output: Path, data_root: Path, run_dir: Path, sequence: str) -> None:
    text = template.read_text()
    replacements = [
        (r"(?m)^with_viz3d: .*$", "with_viz3d: false"),
        (r"(?m)^generate_directory_prefix: .*$", "generate_directory_prefix: false"),
        (r"(?m)^progress_bar: .*$", "progress_bar: false"),
        (r"(?m)^output_dir: .*$", f"output_dir: {run_dir}"),
        (r"(?m)^(\s*- dataset: ).*$", r"\1KITTI"),
        (r"(?m)^(\s*root_path: ).*$", rf"\g<1>{data_root}"),
        (r"(?m)^(\s*- sequence_name: ).*$", rf'\g<1>"{sequence}"'),
        (r"(?m)^  debug_print: true", "  debug_print: false"),
    ]
    for pattern, replacement in replacements:
        text, count = re.subn(pattern, replacement, text, count=1)
        if count != 1:
            raise RuntimeError(f"Config template does not match {pattern!r}")
    output.write_text(text)


def read_pose_ply(path: Path) -> np.ndarray:
    data = path.read_bytes()
    end = data.index(b"end_header\n") + len(b"end_header\n")
    count = int(re.search(rb"element pose (\d+)", data[:end]).group(1))
    dtype = np.dtype(
        [(key, "<f8") for key in ("qx", "qy", "qz", "qw", "x", "y", "z", "ref_t", "dest_t")]
        + [("ref_fid", "<i4"), ("dest_fid", "<i4")]
    )
    return np.frombuffer(data[end:end + count * dtype.itemsize], dtype=dtype)


def quaternion_matrix(q: np.ndarray) -> np.ndarray:
    x, y, z, w = q / np.linalg.norm(q)
    return np.array(
        [
            [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
        ]
    )


def slerp(q0: np.ndarray, q1: np.ndarray, alpha: float) -> np.ndarray:
    dot = float(np.dot(q0, q1))
    if dot < 0.0:
        q1, dot = -q1, -dot
    if dot > 0.9995:
        q = q0 + alpha * (q1 - q0)
        return q / np.linalg.norm(q)
    theta = np.arccos(dot)
    return (np.sin((1 - alpha) * theta) * q0 + np.sin(alpha * theta) * q1) / np.sin(theta)


def resample_trajectory(poses: np.ndarray, num_frames: int) -> list[np.ndarray]:
    """Interpolate the saved poses at t = 0.1 i + 0.05, relative to the first one."""
    times = poses["dest_t"]
    order = np.argsort(times, kind="stable")
    poses, times = poses[order], times[order]
    keep = np.concatenate([[True], np.diff(times) > 1e-9])
    poses, times = poses[keep], times[keep]
    quats = np.stack([poses["qx"], poses["qy"], poses["qz"], poses["qw"]], axis=1)
    queries = np.clip(0.1 * np.arange(num_frames) + 0.05, times[0], times[-1])
    result = []
    for query in queries:
        k = min(max(int(np.searchsorted(times, query)) - 1, 0), len(times) - 2)
        alpha = (query - times[k]) / (times[k + 1] - times[k])
        pose = np.eye(4)
        pose[:3, :3] = quaternion_matrix(slerp(quats[k], quats[k + 1], alpha))
        pose[:3, 3] = [np.interp(query, times, poses[key]) for key in ("x", "y", "z")]
        result.append(pose)
    first_inverse = np.linalg.inv(result[0])
    return [first_inverse @ pose for pose in result]


def main() -> int:
    args = parse_args()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    sequence = args.sequence.zfill(2)
    data_root = output_dir / "data"
    frames_dir = data_root / sequence / "frames"
    if args.velodyne_dir:
        num_frames = write_frames(
            Path(args.velodyne_dir), frames_dir, args.timestamps, args.remove_scans_after_conversion
        )
    elif frames_dir.is_dir():
        # Frames written earlier with --convert-only.
        num_frames = len(list(frames_dir.glob("frame_*.ply")))
    else:
        raise SystemExit("--velodyne-dir is required unless --convert-only frames exist")
    if args.convert_only:
        return 0
    # Upstream's runner evaluates against <root>/<seq>/<seq>.txt when it exists;
    # leave it absent so the odometry process never sees ground truth.
    if (data_root / sequence / f"{sequence}.txt").exists():
        raise RuntimeError("Refusing to expose KITTI ground truth to CT-ICP")

    run_dir = output_dir / "run"
    config_path = output_dir / "official_config.yaml"
    write_config(Path(args.config), config_path, data_root, run_dir, sequence)
    environment = os.environ.copy()
    offset = args.elevation_offset_deg
    if args.no_kitti_elevation_correction:
        offset = 0.0
    if offset is not None:
        environment["CTICP_KITTI_ANGLE_OFFSET"] = repr(offset)
    else:
        environment.pop("CTICP_KITTI_ANGLE_OFFSET", None)
    command = [args.run_odometry, "-c", str(config_path)]
    log_path = output_dir / "official_ct_icp.log"
    start = time.perf_counter()
    with log_path.open("w") as log:
        completed = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, env=environment, check=False)
    wall_seconds = time.perf_counter() - start
    if completed.returncode != 0:
        raise subprocess.CalledProcessError(completed.returncode, command)
    if not args.keep_frames:
        shutil.rmtree(data_root)

    trajectory_path = run_dir / f"KITTI_{sequence}.PLY"
    poses = resample_trajectory(read_pose_ply(trajectory_path), num_frames)
    estimate_path = output_dir / "estimate.txt"
    with estimate_path.open("w") as handle:
        for pose in poses:
            handle.write(" ".join(f"{value:.9f}" for value in pose[:3, :].reshape(-1)) + "\n")

    # Ground truth is opened only after the odometry process exits.
    reference_path = Path(args.reference_csv).resolve()
    metrics = compute_metrics(poses, load_reference_csv(reference_path))
    metrics["end_to_end_fps"] = num_frames / wall_seconds if wall_seconds > 0 else None
    payload = {
        "schema_version": 1,
        "method": args.method,
        "gt_usage": "metrics_only_after_odometry_process_exit",
        "dataset_gt_absence_enforced": True,
        "first_pose_anchor_only": True,
        "command": command,
        "configuration": {
            "config_template": str(Path(args.config).resolve()),
            "config": str(config_path),
            "config_sha256": sha256_file(config_path),
            "dataset": "KITTI (motion-compensated odometry scans as PLY)",
            "point_timestamps": args.timestamps,
            "kitti_elevation_correction_deg": 0.205 if offset is None else offset,
            "pose_sampling": "interpolated at 0.1 i + 0.05 s (save_mid_frame)",
        },
        "artifacts": {
            "velodyne_dir": str(Path(args.velodyne_dir).resolve()) if args.velodyne_dir else None,
            "trajectory": str(trajectory_path),
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
    print(json.dumps(payload["metrics"], indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
