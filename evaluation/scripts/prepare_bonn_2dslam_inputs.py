#!/usr/bin/env python3
"""Export Bonn 2D-SLAM JSON (Intel / fr079 / MIT) to scan_dogfooding layout.

Source: https://www.ipb.uni-bonn.de/html/projects/kuang2023ral/2dslam.zip
(classical indoor 2D datasets converted from Radish CARMEN logs).

Example:
  wget -O /tmp/2dslam.zip \\
    https://www.ipb.uni-bonn.de/html/projects/kuang2023ral/2dslam.zip
  unzip /tmp/2dslam.zip intel/val.json -d /tmp/2dslam
  python3 evaluation/scripts/prepare_bonn_2dslam_inputs.py \\
    --json /tmp/2dslam/intel/val.json \\
    --output-dir evaluation/fixtures/intel_val_73
"""

from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert Bonn 2D-SLAM JSON to scan_dogfooding fixture tree.",
    )
    parser.add_argument(
        "--json",
        required=True,
        help="Path to intel/fr079/mit {train,val,test}.json from 2dslam.zip.",
    )
    parser.add_argument(
        "--output-dir",
        required=True,
        help="Directory for scan_meta.json, NNNNNNNN/scan.csv, gt.csv.",
    )
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First scan index to export.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=-1,
        help="Maximum frames to export (-1 = all).",
    )
    parser.add_argument(
        "--scan-rate-hz",
        type=float,
        default=10.0,
        help="scan_rate_hz written to scan_meta.json.",
    )
    return parser.parse_args()


def yaw_from_matrix(m: list[list[float]]) -> float:
    return math.atan2(m[1][0], m[0][0])


def main() -> int:
    args = parse_args()
    src = Path(args.json)
    out = Path(args.output_dir)
    data = json.loads(src.read_text())

    scans = data["scans"]
    start = max(0, args.start_frame)
    end = len(scans) if args.max_frames < 0 else min(len(scans), start + args.max_frames)
    scans = scans[start:end]
    if not scans:
        raise SystemExit("No scans to export")

    angle_min = float(data["angle_min"])
    angle_max = float(data["angle_max"])
    angle_res = float(data["angle_res"])
    max_range = float(data.get("max_range", 30.0))

    meta = {
        "angle_min": angle_min,
        "angle_max": angle_max,
        "angle_increment": angle_res,
        "range_min": 0.1,
        "range_max": max_range,
        "scan_rate_hz": args.scan_rate_hz,
        "provenance": "Bonn 2D-SLAM JSON (Intel/fr079/MIT from 2dslam.zip)",
        "dataset": src.parent.name,
        "split": src.stem,
    }
    out.mkdir(parents=True, exist_ok=True)
    (out / "scan_meta.json").write_text(json.dumps(meta, indent=2) + "\n")

    gt_rows = [["timestamp", "x", "y", "yaw"]]
    for i, scan in enumerate(scans):
        frame_dir = out / f"{i:08d}"
        frame_dir.mkdir(exist_ok=True)
        ranges = scan["range_readings"]
        (frame_dir / "scan.csv").write_text(
            ",".join(f"{float(r):.6f}" for r in ranges) + "\n"
        )

        odom = scan["odom"]
        x, y = float(odom[0]), float(odom[1])
        yaw = float(odom[2]) if len(odom) >= 3 else yaw_from_matrix(scan["transform_matrix"])
        t = i / args.scan_rate_hz
        gt_rows.append([f"{t:.6f}", f"{x:.6f}", f"{y:.6f}", f"{yaw:.6f}"])

    with (out / "gt.csv").open("w", newline="") as f:
        csv.writer(f).writerows(gt_rows)

    print(f"Exported {len(scans)} scans to {out}")
    print(f"  dataset={meta['dataset']} split={meta['split']} beams={len(scans[0]['range_readings'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
