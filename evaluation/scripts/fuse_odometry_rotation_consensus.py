#!/usr/bin/env python3
"""Causally fuse a primary trajectory with a slow rotation-only consensus.

The reference trajectory is another GT-free odometry estimate.  Its absolute
translation is never copied.  Instead, a slow left-multiplicative rotation
tracks the orientation disagreement and is applied to the primary pose.  The
operation is online: output i depends only on inputs 0..i.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import time
from pathlib import Path

import numpy as np

from evaluate_external_kitti_odometry import load_kitti_poses


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--primary-poses", required=True)
    parser.add_argument("--reference-poses", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--gain", type=float, default=0.0002)
    parser.add_argument("--deadband-deg", type=float, default=0.0)
    parser.add_argument(
        "--failover-reference-motion-m",
        type=float,
        default=0.0,
        help="Switch to an aligned reference trajectory when primary holds while the reference moves by at least this distance; 0 disables.",
    )
    parser.add_argument("--hold-tolerance", type=float, default=1e-12)
    parser.add_argument("--manifest")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def skew(vector: np.ndarray) -> np.ndarray:
    x, y, z = vector
    return np.array([[0.0, -z, y], [z, 0.0, -x], [-y, x, 0.0]])


def so3_log(rotation: np.ndarray) -> np.ndarray:
    cosine = float(np.clip((np.trace(rotation) - 1.0) * 0.5, -1.0, 1.0))
    angle = math.acos(cosine)
    vee = np.array(
        [
            rotation[2, 1] - rotation[1, 2],
            rotation[0, 2] - rotation[2, 0],
            rotation[1, 0] - rotation[0, 1],
        ]
    )
    if angle < 1e-8:
        return 0.5 * vee
    if math.pi - angle < 1e-6:
        eigenvalues, eigenvectors = np.linalg.eig(rotation)
        axis = np.real(eigenvectors[:, np.argmin(np.abs(eigenvalues - 1.0))])
        axis /= np.linalg.norm(axis)
        return angle * axis
    return (angle / (2.0 * math.sin(angle))) * vee


def so3_exp(rotation_vector: np.ndarray) -> np.ndarray:
    angle = float(np.linalg.norm(rotation_vector))
    omega = skew(rotation_vector)
    if angle < 1e-8:
        return np.eye(3) + omega + 0.5 * omega @ omega
    axis_skew = omega / angle
    return (
        np.eye(3)
        + math.sin(angle) * axis_skew
        + (1.0 - math.cos(angle)) * axis_skew @ axis_skew
    )


def fuse_rotation_consensus(
    primary: list[np.ndarray],
    reference: list[np.ndarray],
    gain: float,
    deadband_deg: float = 0.0,
    failover_reference_motion_m: float = 0.0,
    hold_tolerance: float = 1e-12,
    diagnostics: dict[str, object] | None = None,
) -> list[np.ndarray]:
    if len(primary) != len(reference):
        raise ValueError(
            f"trajectory length mismatch: primary={len(primary)}, "
            f"reference={len(reference)}"
        )
    if not primary:
        raise ValueError("trajectories must not be empty")
    if not 0.0 <= gain <= 1.0:
        raise ValueError("gain must be in [0, 1]")
    if deadband_deg < 0.0:
        raise ValueError("deadband_deg must be non-negative")
    if failover_reference_motion_m < 0.0 or hold_tolerance < 0.0:
        raise ValueError("failover thresholds must be non-negative")

    primary_anchor = primary[0]
    reference_alignment = primary_anchor @ np.linalg.inv(reference[0])
    correction_rotation = np.eye(3)
    failover_alignment: np.ndarray | None = None
    failover_index: int | None = None
    fused: list[np.ndarray] = []
    for index, (primary_pose, reference_pose_raw) in enumerate(zip(primary, reference)):
        reference_pose = reference_alignment @ reference_pose_raw
        if failover_alignment is not None:
            fused.append(failover_alignment @ reference_pose)
            continue
        desired_rotation = reference_pose[:3, :3] @ primary_pose[:3, :3].T
        residual = correction_rotation.T @ desired_rotation
        desired_angle = float(np.linalg.norm(so3_log(desired_rotation)))
        effective_gain = gain if desired_angle >= math.radians(deadband_deg) else 0.0
        correction_rotation = correction_rotation @ so3_exp(
            effective_gain * so3_log(residual)
        )

        output_pose = primary_pose.copy()
        output_pose[:3, :3] = correction_rotation @ primary_pose[:3, :3]
        output_pose[:3, 3] = correction_rotation @ primary_pose[:3, 3]
        if index > 0 and failover_reference_motion_m > 0.0:
            primary_holds = (
                float(np.max(np.abs(primary_pose - primary[index - 1])))
                <= hold_tolerance
            )
            reference_motion = float(
                np.linalg.norm(
                    reference_pose[:3, 3]
                    - (reference_alignment @ reference[index - 1])[:3, 3]
                )
            )
            if primary_holds and reference_motion >= failover_reference_motion_m:
                failover_alignment = output_pose @ np.linalg.inv(reference_pose)
                failover_index = index
        fused.append(output_pose)
    if diagnostics is not None:
        diagnostics["failover_index"] = failover_index
        diagnostics["failover_triggered"] = failover_index is not None
    return fused


def write_kitti_poses(path: Path, poses: list[np.ndarray]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    np.savetxt(path, np.asarray(poses)[:, :3, :].reshape(-1, 12), fmt="%.12g")


def main() -> int:
    args = parse_args()
    primary_path = Path(args.primary_poses).resolve()
    reference_path = Path(args.reference_poses).resolve()
    output_path = Path(args.output).resolve()
    started = time.perf_counter()
    diagnostics: dict[str, object] = {}
    fused = fuse_rotation_consensus(
        load_kitti_poses(primary_path),
        load_kitti_poses(reference_path),
        args.gain,
        args.deadband_deg,
        args.failover_reference_motion_m,
        args.hold_tolerance,
        diagnostics,
    )
    fusion_seconds = time.perf_counter() - started
    write_kitti_poses(output_path, fused)

    manifest_path = (
        Path(args.manifest).resolve()
        if args.manifest
        else output_path.with_suffix(".manifest.json")
    )
    payload = {
        "schema_version": 1,
        "method": "causal_rotation_consensus",
        "ground_truth_used": False,
        "gain": args.gain,
        "deadband_deg": args.deadband_deg,
        "failover_reference_motion_m": args.failover_reference_motion_m,
        "hold_tolerance": args.hold_tolerance,
        **diagnostics,
        "frames": len(fused),
        "fusion_seconds_including_pose_io": fusion_seconds,
        "fusion_fps_including_pose_io": len(fused) / fusion_seconds,
        "primary_poses": str(primary_path),
        "primary_sha256": sha256_file(primary_path),
        "reference_poses": str(reference_path),
        "reference_sha256": sha256_file(reference_path),
        "output": str(output_path),
        "output_sha256": sha256_file(output_path),
        "translation_policy": "rotate primary translation; never copy reference translation",
        "causality": "output frame i uses only primary/reference frames 0..i",
    }
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
