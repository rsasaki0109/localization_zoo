#!/usr/bin/env python3
"""Generate a deterministic IMU-only demo and optionally replay it.

The generated sequence contains a static startup, linear motion, an impact,
free-fall, and a final static segment.  It deliberately has no LiDAR or other
aid, so it is suitable for a quick CLI smoke test on any machine with Python.
"""

import argparse
import csv
import pathlib
import subprocess
import sys


GRAVITY = 9.80665


def rows():
    # Keep all values deterministic and human-readable.  The first 1.20 s is
    # static, which satisfies the CLI's default one-second startup window.
    for index in range(120):
        yield index * 0.01, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY
    # Constant body-x specific force gives a clearly visible short-term
    # relative velocity/position segment after startup.
    for index in range(120, 180):
        yield index * 0.01, 0.0, 0.0, 0.0, 1.40, 0.0, GRAVITY
    # One high-g sample, then 100 ms of free-fall.
    yield 1.80, 0.0, 0.0, 0.0, 0.0, 0.0, 30.0
    for index in range(181, 191):
        yield index * 0.01, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    # Leave enough quiet time for the rolling vibration window and event latch
    # to clear, so the final summary demonstrates recovery to stationary.
    for index in range(191, 251):
        yield index * 0.01, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY


def write_csv(path: pathlib.Path) -> int:
    path.parent.mkdir(parents=True, exist_ok=True)
    count = 0
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(["timestamp", "gx", "gy", "gz", "ax", "ay", "az"])
        for sample in rows():
            writer.writerow([f"{value:.9f}" for value in sample])
            count += 1
    return count


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        required=True,
        type=pathlib.Path,
        help="CSV path to generate (overwritten)",
    )
    parser.add_argument(
        "--cli",
        type=pathlib.Path,
        help="optional imu_motion_health_cli executable to run after generation",
    )
    parser.add_argument(
        "--jsonl-output",
        type=pathlib.Path,
        help="JSONL trace path when --cli is supplied",
    )
    parser.add_argument(
        "--summary-output",
        type=pathlib.Path,
        help="summary JSON path when --cli is supplied",
    )
    args = parser.parse_args()

    count = write_csv(args.output)
    print(f"wrote {count} samples to {args.output}")
    if args.cli is None:
        return 0
    if not args.cli.is_file():
        print(f"CLI executable not found: {args.cli}", file=sys.stderr)
        return 2

    jsonl_output = args.jsonl_output or args.output.with_suffix(".jsonl")
    summary_output = args.summary_output or args.output.with_suffix(".summary.json")
    command = [
        str(args.cli),
        "--input",
        str(args.output),
        "--jsonl-output",
        str(jsonl_output),
        "--summary-output",
        str(summary_output),
    ]
    print("running:", " ".join(command))
    result = subprocess.run(command, check=False)
    print(f"JSONL: {jsonl_output}")
    print(f"summary: {summary_output}")
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
