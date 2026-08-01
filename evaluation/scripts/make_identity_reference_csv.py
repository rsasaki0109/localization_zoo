#!/usr/bin/env python3
"""Create an explicitly synthetic identity reference for GT-free health runs."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--frame-timestamps")
    source.add_argument(
        "--frames",
        type=int,
        help="Generate frame-index timestamps 0..N-1 without reading sensor metadata.",
    )
    parser.add_argument("--output", required=True)
    parser.add_argument(
        "--frame-timestamps-output",
        help="Optionally write the corresponding frame,timestamp CSV for strict association.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output_path = Path(args.output)

    if args.frames is not None:
        if args.frames <= 0:
            raise RuntimeError("--frames must be positive")
        timestamps = [str(index) for index in range(args.frames)]
    else:
        timestamps_path = Path(args.frame_timestamps)
        with timestamps_path.open(newline="") as handle:
            rows = list(csv.DictReader(handle))
        if not rows:
            raise RuntimeError(f"No frame timestamps in {timestamps_path}")
        timestamps = [
            row.get("timestamp", row.get("stamp", str(index)))
            for index, row in enumerate(rows)
        ]

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle, lineterminator="\n")
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
        for timestamp in timestamps:
            writer.writerow([timestamp, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])

    if args.frame_timestamps_output:
        frame_timestamps_path = Path(args.frame_timestamps_output)
        frame_timestamps_path.parent.mkdir(parents=True, exist_ok=True)
        with frame_timestamps_path.open("w", newline="") as handle:
            writer = csv.writer(handle, lineterminator="\n")
            writer.writerow(["frame", "timestamp"])
            for index, timestamp in enumerate(timestamps):
                writer.writerow([index, timestamp])

    print(
        f"Wrote {len(timestamps)} identity poses to {output_path}; "
        "metrics against this file are invalid by construction."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
