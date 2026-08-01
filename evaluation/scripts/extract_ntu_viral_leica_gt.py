#!/usr/bin/env python3
"""Extract position-only Leica ground truth from an NTU VIRAL ROS bag."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bag", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--topic", default="/leica/pose/relative")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    import rosbag  # Available in the pinned official ROS container.

    args.output.parent.mkdir(parents=True, exist_ok=True)
    rows = 0
    with args.output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(("timestamp", "prism_x", "prism_y", "prism_z"))
        with rosbag.Bag(str(args.bag), "r") as bag:
            for _, message, _ in bag.read_messages(topics=[args.topic]):
                position = message.pose.position
                writer.writerow(
                    (
                        f"{message.header.stamp.to_sec():.9f}",
                        f"{position.x:.15g}",
                        f"{position.y:.15g}",
                        f"{position.z:.15g}",
                    )
                )
                rows += 1
    if rows == 0:
        raise RuntimeError(f"no messages found on {args.topic}")
    print(f"[done] {rows} Leica positions -> {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
