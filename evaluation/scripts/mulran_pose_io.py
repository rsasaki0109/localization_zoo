"""Parse MulRan global_pose.csv (timestamp + 3x4 pose matrix per row)."""

from __future__ import annotations

import bisect
import csv
import math
from dataclasses import dataclass
from pathlib import Path

import numpy as np


@dataclass(frozen=True)
class MulRanPoseRow:
    timestamp_ns: int
    x: float
    y: float
    z: float
    roll: float
    pitch: float
    yaw: float


def rotation_matrix_to_rpy(R: np.ndarray) -> tuple[float, float, float]:
    """ZYX Euler; matches kitti_poses_to_gt_csv.py / pcd_dogfooding."""
    sinp = -R[2, 0]
    if abs(sinp) >= 1.0:
        pitch = math.copysign(math.pi / 2.0, sinp)
        roll = 0.0
        yaw = math.atan2(-R[0, 1], R[0, 2])
    else:
        pitch = math.asin(sinp)
        roll = math.atan2(R[2, 1], R[2, 2])
        yaw = math.atan2(R[1, 0], R[0, 0])
    return roll, pitch, yaw


def quat_xyzw_to_R(qx: float, qy: float, qz: float, qw: float) -> np.ndarray:
    n = math.sqrt(qx * qx + qy * qy + qz * qz + qw * qw)
    if n < 1e-12:
        return np.eye(3)
    qx, qy, qz, qw = qx / n, qy / n, qz / n, qw / n
    return np.array(
        [
            [
                1 - 2 * (qy * qy + qz * qz),
                2 * (qx * qy - qw * qz),
                2 * (qx * qz + qw * qy),
            ],
            [
                2 * (qx * qy + qw * qz),
                1 - 2 * (qx * qx + qz * qz),
                2 * (qy * qz - qw * qx),
            ],
            [
                2 * (qx * qz - qw * qy),
                2 * (qy * qz + qw * qx),
                1 - 2 * (qx * qx + qy * qy),
            ],
        ],
        dtype=np.float64,
    )


def _row_to_pose(vals: list[float]) -> MulRanPoseRow:
    """MulRan file_player: 13 fields = ts + 12 floats (3x4 row-major R|t)."""
    if len(vals) >= 13:
        ts = int(vals[0])
        r00, r01, r02, tx = vals[1], vals[2], vals[3], vals[4]
        r10, r11, r12, ty = vals[5], vals[6], vals[7], vals[8]
        r20, r21, r22, tz = vals[9], vals[10], vals[11], vals[12]
        R = np.array(
            [[r00, r01, r02], [r10, r11, r12], [r20, r21, r22]], dtype=np.float64
        )
        roll, pitch, yaw = rotation_matrix_to_rpy(R)
        return MulRanPoseRow(ts, tx, ty, tz, roll, pitch, yaw)
    if len(vals) >= 8:
        ts = int(vals[0])
        x, y, z = vals[1], vals[2], vals[3]
        qx, qy, qz, qw = vals[4], vals[5], vals[6], vals[7]
        R = quat_xyzw_to_R(qx, qy, qz, qw)
        roll, pitch, yaw = rotation_matrix_to_rpy(R)
        return MulRanPoseRow(ts, x, y, z, roll, pitch, yaw)
    raise ValueError(f"Expected 8+ or 13+ fields, got {len(vals)}")


def load_global_pose_rows(path: Path) -> list[MulRanPoseRow]:
    """Load all poses; skips a single header line if the first cell is not an int timestamp."""
    with path.open() as f:
        lines = list(csv.reader(f))
    if not lines:
        return []
    start = 0
    try:
        int(lines[0][0].strip())
    except (ValueError, IndexError):
        start = 1

    rows: list[MulRanPoseRow] = []
    for parts in lines[start:]:
        if not parts:
            continue
        vals = [float(x.strip()) for x in parts]
        rows.append(_row_to_pose(vals))
    rows.sort(key=lambda r: r.timestamp_ns)
    return rows


def nearest_pose(ts_ns: int, sorted_poses: list[MulRanPoseRow]) -> MulRanPoseRow:
    """Return pose row with timestamp closest to ts_ns."""
    if not sorted_poses:
        raise RuntimeError("empty pose list")
    keys = [p.timestamp_ns for p in sorted_poses]
    i = bisect.bisect_left(keys, ts_ns)
    if i <= 0:
        return sorted_poses[0]
    if i >= len(sorted_poses):
        return sorted_poses[-1]
    a, b = sorted_poses[i - 1], sorted_poses[i]
    return a if abs(ts_ns - a.timestamp_ns) <= abs(b.timestamp_ns - ts_ns) else b
