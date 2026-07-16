#!/usr/bin/env python3

"""Add planar GT-backed ATE/RPE metrics to a window-odometry result JSON."""

from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--result", required=True, help="Runner result JSON.")
    parser.add_argument(
        "--gt-csv",
        default="",
        help="Override the gt_csv recorded in the result JSON.",
    )
    parser.add_argument(
        "--start-frame",
        type=int,
        default=None,
        help="Override parameters.start_frame (default: 0 when absent).",
    )
    parser.add_argument(
        "--output",
        default="",
        help="Output JSON. Default overwrites --result atomically.",
    )
    return parser.parse_args()


def wrap_angle(angle: float) -> float:
    return math.atan2(math.sin(angle), math.cos(angle))


def load_gt(path: Path) -> list[np.ndarray]:
    poses: list[np.ndarray] = []
    with path.open() as handle:
        reader = csv.DictReader(handle)
        required = {
            "lidar_pose.x", "lidar_pose.y", "lidar_pose.z",
            "lidar_pose.roll", "lidar_pose.pitch", "lidar_pose.yaw",
        }
        missing = required.difference(reader.fieldnames or [])
        if missing:
            raise ValueError(f"{path}: missing columns: {', '.join(sorted(missing))}")
        for line_number, row in enumerate(reader, start=2):
            values = tuple(float(row[name]) for name in (
                "lidar_pose.x", "lidar_pose.y", "lidar_pose.z",
                "lidar_pose.roll", "lidar_pose.pitch", "lidar_pose.yaw",
            ))
            if not all(math.isfinite(value) for value in values):
                raise ValueError(f"{path}:{line_number}: non-finite pose")
            x, y, z, roll, pitch, yaw = values
            cx, sx = math.cos(roll), math.sin(roll)
            cy, sy = math.cos(pitch), math.sin(pitch)
            cz, sz = math.cos(yaw), math.sin(yaw)
            rotation = np.array([
                [cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx],
                [sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx],
                [-sy, cy * sx, cy * cx],
            ])
            transform = np.eye(4)
            transform[:3, :3] = rotation
            transform[:3, 3] = (x, y, z)
            poses.append(transform)
    if not poses:
        raise ValueError(f"{path}: no GT poses")
    return poses


def invert_transform(transform: np.ndarray) -> np.ndarray:
    inverse = np.eye(4)
    inverse[:3, :3] = transform[:3, :3].T
    inverse[:3, 3] = -inverse[:3, :3] @ transform[:3, 3]
    return inverse


def estimated_transform(pose: tuple[float, float, float]) -> np.ndarray:
    x, y, yaw = pose
    c, s = math.cos(yaw), math.sin(yaw)
    transform = np.eye(4)
    transform[:2, :2] = ((c, -s), (s, c))
    transform[:2, 3] = (x, y)
    return transform


def yaw_from_rotation(rotation: np.ndarray) -> float:
    return math.atan2(float(rotation[1, 0]), float(rotation[0, 0]))


def rmse(values: list[float]) -> float | None:
    if not values:
        return None
    return math.sqrt(sum(value * value for value in values) / len(values))


def evaluate(
    estimated: list[tuple[float, float, float]],
    gt_absolute: list[np.ndarray],
) -> dict[str, float | int | None]:
    if len(estimated) != len(gt_absolute):
        raise ValueError(
            f"pose count mismatch: result={len(estimated)} gt={len(gt_absolute)}"
        )
    gt0_inverse = invert_transform(gt_absolute[0])
    gt = [gt0_inverse @ pose for pose in gt_absolute]
    estimated_matrices = [estimated_transform(pose) for pose in estimated]
    ate_xy_errors = []
    for est, ref in zip(estimated_matrices, gt):
        error = est[:3, 3] - ref[:3, 3]
        ate_xy_errors.append(float(np.linalg.norm(error[:2])))
    step_errors: list[float] = []
    rpe_translation_xy_errors: list[float] = []
    rpe_yaw_errors: list[float] = []
    for index in range(1, len(estimated)):
        est_delta = invert_transform(estimated_matrices[index - 1]) @ estimated_matrices[index]
        gt_delta = invert_transform(gt[index - 1]) @ gt[index]
        step_errors.append(
            float(np.linalg.norm(est_delta[:2, 3]))
            - float(np.linalg.norm(gt_delta[:2, 3]))
        )
        relative_error = invert_transform(gt_delta) @ est_delta
        rpe_translation_xy_errors.append(float(np.linalg.norm(relative_error[:2, 3])))
        rpe_yaw_errors.append(math.degrees(wrap_angle(
            yaw_from_rotation(est_delta[:3, :3])
            - yaw_from_rotation(gt_delta[:3, :3])
        )))
    return {
        "gt_associated_frames": len(estimated),
        "ate_xy_m": rmse(ate_xy_errors),
        "step_length_rmse_m": rmse(step_errors),
        "rpe_translation_xy_m": rmse(rpe_translation_xy_errors),
        "rpe_yaw_deg": rmse(rpe_yaw_errors),
    }


def main() -> int:
    args = parse_args()
    result_path = Path(args.result)
    payload = json.loads(result_path.read_text())
    gt_path = Path(args.gt_csv or payload.get("gt_csv", ""))
    if not str(gt_path) or str(gt_path) == "-":
        raise ValueError("No GT CSV supplied or recorded in the result")
    start_frame = args.start_frame
    if start_frame is None:
        start_frame = int(payload.get("parameters", {}).get("start_frame", 0))
    estimated = [
        (float(pose["x_m"]), float(pose["y_m"]), math.radians(float(pose["yaw_deg"])))
        for pose in payload.get("poses_xyyaw", [])
    ]
    gt_all = load_gt(gt_path)
    gt = gt_all[start_frame : start_frame + len(estimated)]
    metrics = payload.setdefault("metrics", {})
    for obsolete in (
        "ate_translation_3d_m", "rpe_translation_3d_m", "rpe_rotation_deg"
    ):
        metrics.pop(obsolete, None)
    metrics.update(evaluate(estimated, gt))
    payload["gt_evaluation"] = {
        "mode": "initial_pose_aligned_se3_gt_vs_planar_estimate",
        "start_frame": start_frame,
        "gt_csv": str(gt_path),
        "rpe_delta_frames": 1,
    }

    output_path = Path(args.output) if args.output else result_path
    temporary_path = output_path.with_suffix(output_path.suffix + ".tmp")
    temporary_path.write_text(json.dumps(payload, indent=2) + "\n")
    temporary_path.replace(output_path)
    print(
        f"[done] {output_path}: frames={len(estimated)} "
        f"ATE_XY={payload['metrics']['ate_xy_m']:.6f} m "
        f"RPE_XY={payload['metrics']['rpe_translation_xy_m']:.6f} m"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
