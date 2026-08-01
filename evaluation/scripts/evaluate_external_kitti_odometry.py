#!/usr/bin/env python3
"""Evaluate an external KITTI- or TUM-format estimate under the Zoo protocol."""

from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path
from typing import Iterable

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--estimate", required=True, help="External pose file.")
    parser.add_argument(
        "--estimate-format",
        choices=("kitti", "tum"),
        default="kitti",
        help="Input pose format (default: kitti).",
    )
    parser.add_argument("--reference-csv", required=True, help="Zoo lidar_pose GT CSV.")
    parser.add_argument("--method", required=True, help="Method name for the JSON row.")
    parser.add_argument("--output-json", required=True)
    parser.add_argument(
        "--conjugation",
        default="",
        help=(
            "Optional 12 comma-separated floats for E when the external runner wrote "
            "T_out=E*T_sensor*inv(E). The evaluator restores sensor-frame poses."
        ),
    )
    parser.add_argument(
        "--runtime-seconds",
        type=float,
        default=None,
        help="Optional end-to-end runtime used to report FPS.",
    )
    parser.add_argument(
        "--segment-length",
        type=float,
        default=100.0,
        help="RPE segment length in metres (default: 100).",
    )
    return parser.parse_args()


def matrix_from_flat(values: Iterable[float]) -> np.ndarray:
    flat = list(values)
    if len(flat) != 12:
        raise ValueError(f"Expected 12 matrix values, got {len(flat)}")
    matrix = np.eye(4)
    matrix[:3, :] = np.asarray(flat, dtype=float).reshape(3, 4)
    return matrix


def load_kitti_poses(path: Path) -> list[np.ndarray]:
    poses: list[np.ndarray] = []
    for line_number, line in enumerate(path.read_text().splitlines(), start=1):
        if not line.strip():
            continue
        values = [float(value) for value in line.split()]
        if len(values) != 12:
            raise ValueError(f"{path}:{line_number}: expected 12 floats")
        poses.append(matrix_from_flat(values))
    if not poses:
        raise ValueError(f"No poses found in {path}")
    return poses


def quaternion_matrix(qx: float, qy: float, qz: float, qw: float) -> np.ndarray:
    quaternion = np.asarray([qx, qy, qz, qw], dtype=float)
    norm = float(quaternion @ quaternion)
    if norm <= np.finfo(float).eps:
        raise ValueError("Quaternion has zero norm")
    quaternion *= math.sqrt(2.0 / norm)
    outer = np.outer(quaternion, quaternion)
    return np.array(
        [
            [1.0 - outer[1, 1] - outer[2, 2], outer[0, 1] - outer[2, 3], outer[0, 2] + outer[1, 3]],
            [outer[0, 1] + outer[2, 3], 1.0 - outer[0, 0] - outer[2, 2], outer[1, 2] - outer[0, 3]],
            [outer[0, 2] - outer[1, 3], outer[1, 2] + outer[0, 3], 1.0 - outer[0, 0] - outer[1, 1]],
        ]
    )


def load_tum_poses(path: Path) -> list[np.ndarray]:
    """Load ``timestamp tx ty tz qx qy qz qw`` poses in file order."""
    poses: list[np.ndarray] = []
    for line_number, line in enumerate(path.read_text().splitlines(), start=1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        values = [float(value) for value in line.split()]
        if len(values) != 8:
            raise ValueError(f"{path}:{line_number}: expected 8 floats")
        pose = np.eye(4)
        pose[:3, 3] = values[1:4]
        pose[:3, :3] = quaternion_matrix(*values[4:8])
        poses.append(pose)
    if not poses:
        raise ValueError(f"No poses found in {path}")
    return poses


def rpy_matrix(roll: float, pitch: float, yaw: float) -> np.ndarray:
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    rx = np.array([[1, 0, 0], [0, cr, -sr], [0, sr, cr]])
    ry = np.array([[cp, 0, sp], [0, 1, 0], [-sp, 0, cp]])
    rz = np.array([[cy, -sy, 0], [sy, cy, 0], [0, 0, 1]])
    return rz @ ry @ rx


def load_reference_csv(path: Path) -> list[np.ndarray]:
    poses: list[np.ndarray] = []
    with path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            pose = np.eye(4)
            pose[:3, 3] = [
                float(row["lidar_pose.x"]),
                float(row["lidar_pose.y"]),
                float(row["lidar_pose.z"]),
            ]
            pose[:3, :3] = rpy_matrix(
                float(row["lidar_pose.roll"]),
                float(row["lidar_pose.pitch"]),
                float(row["lidar_pose.yaw"]),
            )
            poses.append(pose)
    if not poses:
        raise ValueError(f"No reference poses found in {path}")
    return poses


def normalize_first_pose(poses: list[np.ndarray]) -> list[np.ndarray]:
    first_inverse = np.linalg.inv(poses[0])
    return [first_inverse @ pose for pose in poses]


def undo_conjugation(
    poses: list[np.ndarray], conjugation: np.ndarray
) -> list[np.ndarray]:
    inverse = np.linalg.inv(conjugation)
    return [inverse @ pose @ conjugation for pose in poses]


def rotation_angle(rotation: np.ndarray) -> float:
    cosine = float(np.clip((np.trace(rotation) - 1.0) / 2.0, -1.0, 1.0))
    return math.acos(cosine)


def compute_metrics(
    estimates: list[np.ndarray],
    references: list[np.ndarray],
    segment_length: float = 100.0,
) -> dict[str, float | int | None]:
    reference_count = len(references)
    frame_count = min(len(estimates), len(references))
    estimates = normalize_first_pose(estimates[:frame_count])
    references = normalize_first_pose(references[:frame_count])

    position_errors = [
        float(np.linalg.norm(estimates[index][:3, 3] - references[index][:3, 3]))
        for index in range(frame_count)
    ]
    ate = math.sqrt(float(np.mean(np.square(position_errors))))

    cumulative_distance = [0.0]
    for index in range(1, frame_count):
        step = np.linalg.norm(
            references[index][:3, 3] - references[index - 1][:3, 3]
        )
        cumulative_distance.append(cumulative_distance[-1] + float(step))

    translation_errors: list[float] = []
    rotation_errors: list[float] = []
    end = 1
    for start in range(frame_count):
        end = max(end, start + 1)
        while (
            end < frame_count
            and cumulative_distance[end] - cumulative_distance[start] < segment_length
        ):
            end += 1
        if end >= frame_count:
            break
        distance = cumulative_distance[end] - cumulative_distance[start]
        estimate_delta = np.linalg.inv(estimates[start]) @ estimates[end]
        reference_delta = np.linalg.inv(references[start]) @ references[end]
        error = np.linalg.inv(reference_delta) @ estimate_delta
        translation_errors.append(float(np.linalg.norm(error[:3, 3]) / distance * 100))
        rotation_errors.append(math.degrees(rotation_angle(error[:3, :3])) / distance)

    return {
        "frames": frame_count,
        "reference_frames": reference_count,
        "tracking_success_rate": frame_count / reference_count,
        "ate_m": ate,
        "rpe_trans_pct": (
            float(np.mean(translation_errors)) if translation_errors else None
        ),
        "rpe_rot_deg_per_m": (
            float(np.mean(rotation_errors)) if rotation_errors else None
        ),
        "rpe_segments": len(translation_errors),
        "trajectory_length_m": cumulative_distance[-1],
    }


def main() -> int:
    args = parse_args()
    estimate_path = Path(args.estimate)
    reference_path = Path(args.reference_csv)
    estimates = (
        load_kitti_poses(estimate_path)
        if args.estimate_format == "kitti"
        else load_tum_poses(estimate_path)
    )
    references = load_reference_csv(reference_path)
    if args.conjugation:
        values = [float(value) for value in args.conjugation.split(",")]
        estimates = undo_conjugation(estimates, matrix_from_flat(values))

    metrics = compute_metrics(estimates, references, args.segment_length)
    metrics["fps"] = (
        metrics["frames"] / args.runtime_seconds
        if args.runtime_seconds is not None and args.runtime_seconds > 0
        else None
    )
    payload = {
        "schema_version": 1,
        "method": args.method,
        "estimate_format": args.estimate_format,
        "estimate": str(estimate_path),
        "reference_csv": str(reference_path),
        "first_pose_anchor_only": True,
        "conjugation_undone": bool(args.conjugation),
        "metrics": metrics,
    }
    output_path = Path(args.output_json)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
