#!/usr/bin/env python3

"""Build imu.csv for an existing KITTI Raw dogfooding tree from OXTS packets.

`pcd_dogfooding` expects `imu.csv` stamps in the same numeric timeline as
`frame_timestamps.csv` (``timestamp`` column). The stock exporter
(`kitti_raw_to_benchmark.py`) uses Velodyne **file indices** (0, 1, 2, …) there.
This script writes one IMU row at the midpoint between consecutive scan indices
so each LiDAR interval (i-1, i) contains a single sample — enough for DLIO /
CT-LIO style windows without regenerating GT or wall-clock timestamps.

OXTS layout follows pykitti ``OxtsPacket`` (``dataformat.txt``): after roll,
pitch, yaw come velocities, then ax, ay, az, … then wx, wy, wz (rad/s and
m/s² in the OXTS body frame — same convention as other KITTI tooling).

Example:
  python3 evaluation/scripts/kitti_oxts_imu_for_dogfooding.py \\
    --drive-dir /data/kitti/2011_09_26/2011_09_26_drive_0009_sync \\
    --pcd-dir dogfooding_results/kitti_raw_0009_full
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--drive-dir",
        required=True,
        help="KITTI Raw sync directory (contains oxts/data/*.txt).",
    )
    parser.add_argument(
        "--pcd-dir",
        required=True,
        help="Dogfooding directory with frame_timestamps.csv (from kitti_raw_to_benchmark).",
    )
    parser.add_argument(
        "--output-imu",
        default="",
        help="Output imu.csv path (default: <pcd-dir>/imu.csv).",
    )
    return parser.parse_args()


def load_oxts_packets_sorted(oxts_dir: Path) -> list[list[float]]:
    """One float list per OXTS file (numeric fields only; KITTI flags stripped)."""
    files = sorted(oxts_dir.glob("*.txt"))
    rows: list[list[float]] = []
    for path in files:
        parts = path.read_text().strip().split()
        if len(parts) < 30:
            raise RuntimeError(f"Unexpected OXTS line length in {path}")
        rows.append([float(x) for x in parts[:-5]])
    return rows


def parse_frame_global_indices(pcd_dir: Path) -> list[int]:
    ts_path = pcd_dir / "frame_timestamps.csv"
    if not ts_path.is_file():
        raise FileNotFoundError(f"Missing {ts_path}")
    indices: list[int] = []
    with ts_path.open(newline="") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None or "timestamp" not in reader.fieldnames:
            raise RuntimeError(f"{ts_path} needs a 'timestamp' column (Velodyne index).")
        for row in reader:
            indices.append(int(float(row["timestamp"])))
    return indices


def main() -> int:
    args = parse_args()
    drive_dir = Path(args.drive_dir)
    pcd_dir = Path(args.pcd_dir)
    oxts_dir = drive_dir / "oxts" / "data"
    if not oxts_dir.is_dir():
        raise FileNotFoundError(f"Not a directory: {oxts_dir}")

    oxts_rows = load_oxts_packets_sorted(oxts_dir)
    global_idx = parse_frame_global_indices(pcd_dir)

    out_path = Path(args.output_imu) if args.output_imu else pcd_dir / "imu.csv"
    out_path.parent.mkdir(parents=True, exist_ok=True)

    I_AX, I_AY, I_AZ = 11, 12, 13
    I_WX, I_WY, I_WZ = 17, 18, 19

    written = 0
    with out_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(
            [
                "stamp",
                "angular_vel.x",
                "angular_vel.y",
                "angular_vel.z",
                "linear_acc.x",
                "linear_acc.y",
                "linear_acc.z",
            ]
        )

        for i in range(1, len(global_idx)):
            g_prev = global_idx[i - 1]
            g_curr = global_idx[i]
            if not (0 <= g_prev < len(oxts_rows) and 0 <= g_curr < len(oxts_rows)):
                raise IndexError(
                    f"Frame pair ({g_prev}, {g_curr}) out of range for "
                    f"{len(oxts_rows)} OXTS packets under {oxts_dir}"
                )
            row = oxts_rows[g_curr]
            stamp = 0.5 * (float(g_prev) + float(g_curr))
            writer.writerow(
                [
                    f"{stamp:.9f}",
                    f"{row[I_WX]:.12f}",
                    f"{row[I_WY]:.12f}",
                    f"{row[I_WZ]:.12f}",
                    f"{row[I_AX]:.12f}",
                    f"{row[I_AY]:.12f}",
                    f"{row[I_AZ]:.12f}",
                ]
            )
            written += 1

    print(f"[done] wrote {written} IMU rows to {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
