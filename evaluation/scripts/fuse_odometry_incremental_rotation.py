#!/usr/bin/env python3
"""Causally re-integrate primary translation with blended odometry rotation.

Only relative odometry increments are consumed. Reference translation is never
used; each output step keeps the primary local-frame translation and blends
the primary/reference relative rotations before composing it online.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import time
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--primary-poses", required=True)
    parser.add_argument("--reference-poses", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--rotation-blend", type=float, required=True)
    parser.add_argument("--max-increment-disagreement-deg", type=float, default=5.0)
    parser.add_argument("--manifest")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_kitti_poses(path: Path) -> list[np.ndarray]:
    poses: list[np.ndarray] = []
    for line_number, line in enumerate(path.read_text().splitlines(), start=1):
        if not line.strip():
            continue
        values = [float(token) for token in line.split()]
        if len(values) != 12:
            raise RuntimeError(f"{path}:{line_number}: expected 12 values")
        pose = np.eye(4)
        pose[:3, :] = np.asarray(values).reshape(3, 4)
        if not np.isfinite(pose).all():
            raise RuntimeError(f"{path}:{line_number}: non-finite pose")
        poses.append(pose)
    if not poses:
        raise RuntimeError(f"No poses in {path}")
    return poses


def write_kitti_poses(path: Path, poses: list[np.ndarray]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    np.savetxt(path, np.asarray(poses)[:, :3, :].reshape(-1, 12), fmt="%.12g")


def log_so3(rotation: np.ndarray) -> np.ndarray:
    cosine = float(np.clip((np.trace(rotation) - 1.0) * 0.5, -1.0, 1.0))
    angle = math.acos(cosine)
    if angle < 1e-10:
        return 0.5 * np.array(
            [rotation[2, 1] - rotation[1, 2], rotation[0, 2] - rotation[2, 0], rotation[1, 0] - rotation[0, 1]]
        )
    return angle / (2.0 * math.sin(angle)) * np.array(
        [rotation[2, 1] - rotation[1, 2], rotation[0, 2] - rotation[2, 0], rotation[1, 0] - rotation[0, 1]]
    )


def exp_so3(vector: np.ndarray) -> np.ndarray:
    angle = float(np.linalg.norm(vector))
    if angle < 1e-10:
        skew = np.array(
            [[0.0, -vector[2], vector[1]], [vector[2], 0.0, -vector[0]], [-vector[1], vector[0], 0.0]]
        )
        return np.eye(3) + skew
    axis = vector / angle
    skew = np.array(
        [[0.0, -axis[2], axis[1]], [axis[2], 0.0, -axis[0]], [-axis[1], axis[0], 0.0]]
    )
    return np.eye(3) + math.sin(angle) * skew + (1.0 - math.cos(angle)) * (skew @ skew)


def fuse_incremental_rotations(
    primary: list[np.ndarray],
    reference: list[np.ndarray],
    rotation_blend: float,
    max_disagreement_rad: float,
) -> tuple[list[np.ndarray], int]:
    if len(primary) != len(reference):
        raise RuntimeError("Primary and reference pose counts differ")
    if not 0.0 <= rotation_blend <= 1.0:
        raise RuntimeError("rotation_blend must be in [0, 1]")

    output = [primary[0].copy()]
    gated_increments = 0
    for index in range(1, len(primary)):
        primary_delta = np.linalg.inv(primary[index - 1]) @ primary[index]
        reference_delta = np.linalg.inv(reference[index - 1]) @ reference[index]
        disagreement = log_so3(primary_delta[:3, :3].T @ reference_delta[:3, :3])
        blend = rotation_blend
        if np.linalg.norm(disagreement) > max_disagreement_rad:
            blend = 0.0
            gated_increments += 1
        fused_delta = np.eye(4)
        fused_delta[:3, :3] = primary_delta[:3, :3] @ exp_so3(blend * disagreement)
        fused_delta[:3, 3] = primary_delta[:3, 3]
        output.append(output[-1] @ fused_delta)
    return output, gated_increments


def main() -> int:
    args = parse_args()
    primary_path = Path(args.primary_poses).resolve()
    reference_path = Path(args.reference_poses).resolve()
    output_path = Path(args.output).resolve()
    started = time.perf_counter()
    primary = load_kitti_poses(primary_path)
    reference = load_kitti_poses(reference_path)
    output, gated = fuse_incremental_rotations(
        primary,
        reference,
        args.rotation_blend,
        math.radians(args.max_increment_disagreement_deg),
    )
    write_kitti_poses(output_path, output)
    elapsed = time.perf_counter() - started
    payload = {
        "schema_version": 1,
        "method": "causal_incremental_rotation_blend",
        "ground_truth_used": False,
        "frames": len(output),
        "rotation_blend": args.rotation_blend,
        "max_increment_disagreement_deg": args.max_increment_disagreement_deg,
        "gated_increments": gated,
        "translation_policy": "copy primary relative translation only; never use reference translation",
        "causality": "output i uses primary/reference increments through i",
        "seconds_including_pose_io": elapsed,
        "fps_including_pose_io": len(output) / elapsed if elapsed > 0 else None,
        "primary_sha256": sha256_file(primary_path),
        "reference_sha256": sha256_file(reference_path),
        "output_sha256": sha256_file(output_path),
    }
    if args.manifest:
        manifest_path = Path(args.manifest).resolve()
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
