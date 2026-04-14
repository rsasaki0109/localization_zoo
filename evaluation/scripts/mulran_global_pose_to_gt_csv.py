#!/usr/bin/env python3

"""Convert MulRan global_pose.csv into pcd_dogfooding GT CSV (sequential rows)."""

from __future__ import annotations

import argparse
import csv
import importlib.util
from pathlib import Path


def load_pose_io():
    import sys

    script_dir = Path(__file__).resolve().parent
    spec = importlib.util.spec_from_file_location("mulran_pose_io", script_dir / "mulran_pose_io.py")
    if spec is None or spec.loader is None:
        raise RuntimeError("Failed to load mulran_pose_io.py")
    mod = importlib.util.module_from_spec(spec)
    sys.modules["mulran_pose_io"] = mod
    spec.loader.exec_module(mod)
    return mod


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Convert MulRan global_pose.csv (timestamp + 3x4 matrix or xyz+quat) "
            "into timestamp,lidar_pose.* CSV by sequential slicing."
        )
    )
    parser.add_argument("--global-pose", required=True, help="Path to global_pose.csv.")
    parser.add_argument("--output", required=True, help="Output GT CSV path.")
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First pose row index after load/sort (default: 0).",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=-1,
        help="Maximum rows to export (-1 for all).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    pose_path = Path(args.global_pose)
    output_path = Path(args.output)
    pose_io = load_pose_io()

    rows = pose_io.load_global_pose_rows(pose_path)
    if not rows:
        raise RuntimeError(f"No poses in {pose_path}")

    end = len(rows)
    if args.max_frames >= 0:
        end = min(args.start_frame + args.max_frames, len(rows))
    selected = rows[args.start_frame:end]
    if not selected:
        raise RuntimeError(f"No poses in range [{args.start_frame}, {end})")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "timestamp",
            "lidar_pose.x",
            "lidar_pose.y",
            "lidar_pose.z",
            "lidar_pose.roll",
            "lidar_pose.pitch",
            "lidar_pose.yaw",
        ])
        for p in selected:
            ts_sec = p.timestamp_ns * 1e-9
            writer.writerow([
                f"{ts_sec:.10f}",
                f"{p.x:.10f}",
                f"{p.y:.10f}",
                f"{p.z:.10f}",
                f"{p.roll:.10f}",
                f"{p.pitch:.10f}",
                f"{p.yaw:.10f}",
            ])

    print(f"[done] wrote {output_path}")
    print(f"[done] poses={len(selected)} range=[{args.start_frame}, {args.start_frame + len(selected)})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
