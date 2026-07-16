#!/usr/bin/env python3

"""Convert a TUM trajectory into pcd_dogfooding ground-truth CSV."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, help="TUM trajectory path.")
    parser.add_argument("--output", required=True, help="Output GT CSV path.")
    return parser.parse_args()


def quat_to_rpy(
    qx: float, qy: float, qz: float, qw: float
) -> tuple[float, float, float]:
    norm = math.sqrt(qx * qx + qy * qy + qz * qz + qw * qw)
    if not math.isfinite(norm) or norm <= 1e-12:
        raise ValueError("quaternion has zero or non-finite norm")
    qx, qy, qz, qw = qx / norm, qy / norm, qz / norm, qw / norm

    sinr_cosp = 2.0 * (qw * qx + qy * qz)
    cosr_cosp = 1.0 - 2.0 * (qx * qx + qy * qy)
    roll = math.atan2(sinr_cosp, cosr_cosp)

    sinp = max(-1.0, min(1.0, 2.0 * (qw * qy - qz * qx)))
    pitch = math.asin(sinp)

    siny_cosp = 2.0 * (qw * qz + qx * qy)
    cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz)
    yaw = math.atan2(siny_cosp, cosy_cosp)
    return roll, pitch, yaw


def load_tum(path: Path) -> list[tuple[float, ...]]:
    rows: list[tuple[float, ...]] = []
    previous_timestamp = -math.inf
    with path.open() as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue
            tokens = line.split()
            if len(tokens) != 8:
                raise ValueError(
                    f"{path}:{line_number}: expected 8 TUM columns, got {len(tokens)}"
                )
            try:
                values = tuple(float(token) for token in tokens)
            except ValueError as exc:
                raise ValueError(f"{path}:{line_number}: invalid numeric value") from exc
            if not all(math.isfinite(value) for value in values):
                raise ValueError(f"{path}:{line_number}: non-finite numeric value")
            if values[0] <= previous_timestamp:
                raise ValueError(
                    f"{path}:{line_number}: timestamps must be strictly increasing"
                )
            previous_timestamp = values[0]
            rows.append(values)
    if not rows:
        raise ValueError(f"No poses found in {path}")
    return rows


def main() -> int:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)
    rows = load_tum(input_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            [
                "timestamp",
                "lidar_pose.x",
                "lidar_pose.y",
                "lidar_pose.z",
                "lidar_pose.roll",
                "lidar_pose.pitch",
                "lidar_pose.yaw",
            ]
        )
        for timestamp, x, y, z, qx, qy, qz, qw in rows:
            try:
                roll, pitch, yaw = quat_to_rpy(qx, qy, qz, qw)
            except ValueError as exc:
                raise ValueError(f"{input_path}: timestamp {timestamp:.9f}: {exc}") from exc
            writer.writerow(
                [
                    f"{timestamp:.9f}",
                    f"{x:.10f}",
                    f"{y:.10f}",
                    f"{z:.10f}",
                    f"{roll:.10f}",
                    f"{pitch:.10f}",
                    f"{yaw:.10f}",
                ]
            )

    print(f"[done] wrote {output_path} ({len(rows)} poses)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
