#!/usr/bin/env python3

"""Evaluate an upstream BIEVR-LIO TUM trajectory against Hard PCL GT."""

from __future__ import annotations

import argparse
import bisect
import csv
import json
import math
import re
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trajectory", required=True, type=Path)
    parser.add_argument("--gt-csv", required=True, type=Path)
    parser.add_argument("--sensor-yaml", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--max-dt", type=float, default=0.02)
    parser.add_argument("--upstream-commit", default="")
    parser.add_argument("--runtime-s", type=float, default=None)
    return parser.parse_args()


def quaternion_matrix(qx: float, qy: float, qz: float, qw: float) -> np.ndarray:
    norm = math.sqrt(qx * qx + qy * qy + qz * qz + qw * qw)
    qx, qy, qz, qw = qx / norm, qy / norm, qz / norm, qw / norm
    return np.array([
        [1 - 2 * (qy * qy + qz * qz), 2 * (qx * qy - qz * qw), 2 * (qx * qz + qy * qw)],
        [2 * (qx * qy + qz * qw), 1 - 2 * (qx * qx + qz * qz), 2 * (qy * qz - qx * qw)],
        [2 * (qx * qz - qy * qw), 2 * (qy * qz + qx * qw), 1 - 2 * (qx * qx + qy * qy)],
    ])


def rpy_matrix(roll: float, pitch: float, yaw: float) -> np.ndarray:
    cx, sx = math.cos(roll), math.sin(roll)
    cy, sy = math.cos(pitch), math.sin(pitch)
    cz, sz = math.cos(yaw), math.sin(yaw)
    return np.array([
        [cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx],
        [sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx],
        [-sy, cy * sx, cy * cx],
    ])


def transform(rotation: np.ndarray, translation: tuple[float, float, float]) -> np.ndarray:
    result = np.eye(4)
    result[:3, :3] = rotation
    result[:3, 3] = translation
    return result


def invert(value: np.ndarray) -> np.ndarray:
    result = np.eye(4)
    result[:3, :3] = value[:3, :3].T
    result[:3, 3] = -result[:3, :3] @ value[:3, 3]
    return result


def load_tum(path: Path) -> list[tuple[float, np.ndarray]]:
    rows = []
    with path.open() as handle:
        for line_number, raw in enumerate(handle, start=1):
            tokens = raw.split()
            if not tokens:
                continue
            if len(tokens) != 8:
                raise ValueError(f"{path}:{line_number}: expected 8 TUM columns")
            values = [float(token) for token in tokens]
            if not all(math.isfinite(value) for value in values):
                raise ValueError(f"{path}:{line_number}: non-finite value")
            stamp, x, y, z, qx, qy, qz, qw = values
            rows.append((stamp, transform(quaternion_matrix(qx, qy, qz, qw), (x, y, z))))
    if not rows:
        raise ValueError(f"{path}: no trajectory poses")
    return rows


def load_gt(path: Path) -> list[tuple[float, np.ndarray]]:
    rows = []
    with path.open() as handle:
        for row in csv.DictReader(handle):
            stamp = float(row["timestamp"])
            xyz = tuple(float(row[f"lidar_pose.{axis}"]) for axis in ("x", "y", "z"))
            roll, pitch, yaw = (
                float(row[f"lidar_pose.{axis}"]) for axis in ("roll", "pitch", "yaw")
            )
            rows.append((stamp, transform(rpy_matrix(roll, pitch, yaw), xyz)))
    if not rows:
        raise ValueError(f"{path}: no GT poses")
    return rows


def load_extrinsic(path: Path) -> np.ndarray:
    text = path.read_text()
    translation_match = re.search(r"translation:\s*\[([^\]]+)\]", text, re.DOTALL)
    rotation_match = re.search(r"rotation:\s*\[([^\]]+)\]", text, re.DOTALL)
    if not translation_match or not rotation_match:
        raise ValueError(f"{path}: calibration translation/rotation not found")
    translation = [
        float(token) for token in translation_match.group(1).replace("\n", " ").split(",")
    ]
    rotation = [
        float(token) for token in rotation_match.group(1).replace("\n", " ").split(",")
    ]
    if len(translation) != 3 or len(rotation) != 9:
        raise ValueError(f"{path}: invalid calibration dimensions")
    return transform(np.array(rotation).reshape(3, 3), tuple(translation))


def associate(
    estimated: list[tuple[float, np.ndarray]],
    gt: list[tuple[float, np.ndarray]],
    max_dt: float,
) -> list[tuple[float, np.ndarray, np.ndarray, float]]:
    gt_stamps = [row[0] for row in gt]
    matches = []
    for stamp, pose in estimated:
        right = bisect.bisect_left(gt_stamps, stamp)
        candidates = [index for index in (right - 1, right) if 0 <= index < len(gt)]
        index = min(candidates, key=lambda candidate: abs(gt_stamps[candidate] - stamp))
        dt = abs(gt_stamps[index] - stamp)
        if dt <= max_dt:
            matches.append((stamp, pose, gt[index][1], dt))
    if len(matches) < 2:
        raise ValueError("Fewer than two timestamp-associated trajectory poses")
    return matches


def yaw(rotation: np.ndarray) -> float:
    return math.atan2(float(rotation[1, 0]), float(rotation[0, 0]))


def rmse(values: list[float]) -> float:
    return math.sqrt(sum(value * value for value in values) / len(values))


def path_length_xy(poses: list[np.ndarray]) -> float:
    return sum(
        float(np.linalg.norm(current[:2, 3] - previous[:2, 3]))
        for previous, current in zip(poses, poses[1:])
    )


def main() -> int:
    args = parse_args()
    t_i_l = load_extrinsic(args.sensor_yaml)
    estimated = [
        (stamp, t_w_i @ t_i_l)
        for stamp, t_w_i in load_tum(args.trajectory)
    ]
    gt = load_gt(args.gt_csv)
    matches = associate(estimated, gt, args.max_dt)
    est0_inv = invert(matches[0][1])
    gt0_inv = invert(matches[0][2])
    est_relative = [est0_inv @ row[1] for row in matches]
    gt_relative = [gt0_inv @ row[2] for row in matches]

    ate = [
        float(np.linalg.norm(est[:2, 3] - ref[:2, 3]))
        for est, ref in zip(est_relative, gt_relative)
    ]
    rpe_translation = []
    rpe_yaw = []
    for index in range(1, len(matches)):
        est_delta = invert(est_relative[index - 1]) @ est_relative[index]
        gt_delta = invert(gt_relative[index - 1]) @ gt_relative[index]
        error = invert(gt_delta) @ est_delta
        rpe_translation.append(float(np.linalg.norm(error[:2, 3])))
        angle = yaw(est_delta[:3, :3]) - yaw(gt_delta[:3, :3])
        rpe_yaw.append(math.degrees(math.atan2(math.sin(angle), math.cos(angle))))

    threshold_crossings = {}
    for threshold in (1.0, 10.0, 100.0):
        crossing = next(
            (
                {"associated_frame": index, "timestamp": matches[index][0], "error_xy_m": error}
                for index, error in enumerate(ate)
                if error > threshold
            ),
            None,
        )
        threshold_crossings[f"first_over_{int(threshold)}m"] = crossing

    payload = {
        "schema_version": 1,
        "method": "upstream_bievr_lio",
        "upstream_commit": args.upstream_commit,
        "trajectory_tum": str(args.trajectory),
        "gt_csv": str(args.gt_csv),
        "sensor_yaml": str(args.sensor_yaml),
        "parameters": {"association_max_dt_s": args.max_dt},
        "metrics": {
            "trajectory_poses": len(estimated),
            "gt_associated_frames": len(matches),
            "max_association_dt_s": max(row[3] for row in matches),
            "ate_xy_m": rmse(ate),
            "max_error_xy_m": max(ate),
            "final_error_xy_m": ate[-1],
            "rpe_translation_xy_m": rmse(rpe_translation),
            "rpe_yaw_deg": rmse(rpe_yaw),
            "estimated_path_length_xy_m": path_length_xy(est_relative),
            "gt_path_length_xy_m": path_length_xy(gt_relative),
            "runtime_s": args.runtime_s,
        },
        "failure_thresholds": threshold_crossings,
        "gt_evaluation": {
            "mode": "T_W_I_times_T_I_L_to_lidar_then_timestamp_associated_initial_pose_aligned_se3_planar_metrics",
            "rpe_delta_frames": 1,
        },
        "poses_xyyaw": [
            {
                "frame": index,
                "timestamp": row[0],
                "x_m": float(pose[0, 3]),
                "y_m": float(pose[1, 3]),
                "yaw_deg": math.degrees(yaw(pose[:3, :3])),
            }
            for index, (row, pose) in enumerate(zip(matches, est_relative))
        ],
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    print(
        f"[done] {args.output}: associated={len(matches)}/{len(estimated)} "
        f"ATE_XY={payload['metrics']['ate_xy_m']:.6f} m "
        f"RPE_XY={payload['metrics']['rpe_translation_xy_m']:.6f} m"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
