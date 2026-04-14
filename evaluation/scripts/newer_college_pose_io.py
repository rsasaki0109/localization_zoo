"""Parse Newer College ground-truth CSV (sec, nsec, position, quaternion xyzw)."""

from __future__ import annotations

import bisect
import csv
import math
from dataclasses import dataclass
from pathlib import Path

import numpy as np


@dataclass(frozen=True)
class NewerCollegePoseRow:
    timestamp_ns: int
    x: float
    y: float
    z: float
    roll: float
    pitch: float
    yaw: float


def rotation_matrix_to_rpy(R: np.ndarray) -> tuple[float, float, float]:
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


def _time_ns_from_sec_nsec(sec: float, nsec: float) -> int:
    return int(round((float(sec) + float(nsec) * 1e-9) * 1e9))


def load_ground_truth_csv(path: Path) -> list[NewerCollegePoseRow]:
    """Load official GT: sec, nsec, x, y, z, qx, qy, qz, qw (optional header)."""
    rows: list[NewerCollegePoseRow] = []
    with path.open() as f:
        lines = list(csv.reader(f))
    if not lines:
        return rows

    start = 0
    try:
        float(lines[0][0])
    except (ValueError, IndexError):
        start = 1

    for parts in lines[start:]:
        if len(parts) < 9:
            continue
        sec = float(parts[0].strip())
        nsec = float(parts[1].strip())
        x, y, z = float(parts[2]), float(parts[3]), float(parts[4])
        qx, qy, qz, qw = float(parts[5]), float(parts[6]), float(parts[7]), float(parts[8])
        ts_ns = _time_ns_from_sec_nsec(sec, nsec)
        R = quat_xyzw_to_R(qx, qy, qz, qw)
        roll, pitch, yaw = rotation_matrix_to_rpy(R)
        rows.append(NewerCollegePoseRow(ts_ns, x, y, z, roll, pitch, yaw))
    rows.sort(key=lambda r: r.timestamp_ns)
    return rows


def nearest_pose(ts_ns: int, sorted_poses: list[NewerCollegePoseRow]) -> NewerCollegePoseRow:
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


def parse_pcd_stem_to_timestamp_ns(stem: str) -> int:
    """Infer epoch time in nanoseconds from a Newer College/EuRoC-style PCD filename stem."""
    stem = stem.replace(".pcd", "")
    if "_" in stem:
        left, right = stem.split("_", 1)
        return int(left) * 10**9 + int(right)
    if "." in stem and stem.count(".") == 1:
        whole, frac = stem.split(".", 1)
        frac9 = (frac + "000000000")[:9]
        return int(whole) * 10**9 + int(frac9)
    return int(stem)
