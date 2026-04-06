#!/usr/bin/env python3

"""Slice a trajectory CSV to the timestamp span covered by extracted frames."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Keep trajectory rows that overlap a frame_timestamps.csv window."
    )
    parser.add_argument("--input", required=True, help="Input trajectory CSV.")
    parser.add_argument(
        "--frame-timestamps",
        required=True,
        help="frame_timestamps.csv exported beside a PCD sequence.",
    )
    parser.add_argument("--output", required=True, help="Output trajectory CSV.")
    parser.add_argument(
        "--pad-sec",
        type=float,
        default=0.0,
        help="Optional symmetric padding around the frame timestamp span.",
    )
    return parser.parse_args()


def load_timestamp_span(path: Path) -> tuple[float, float]:
    with path.open() as handle:
        reader = csv.DictReader(handle)
        timestamps = [float(row["timestamp"]) for row in reader]
    if not timestamps:
        raise RuntimeError(f"No timestamps found in {path}")
    return min(timestamps), max(timestamps)


def main() -> int:
    args = parse_args()
    input_path = Path(args.input)
    frame_path = Path(args.frame_timestamps)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    min_ts, max_ts = load_timestamp_span(frame_path)
    pad = float(args.pad_sec)
    kept = 0

    with input_path.open() as handle:
        reader = csv.DictReader(handle)
        fieldnames = reader.fieldnames
        if not fieldnames:
            raise RuntimeError(f"No header found in {input_path}")
        with output_path.open("w", newline="") as out_handle:
            writer = csv.DictWriter(out_handle, fieldnames=fieldnames)
            writer.writeheader()
            for row in reader:
                timestamp = float(row["timestamp"])
                if min_ts - pad <= timestamp <= max_ts + pad:
                    writer.writerow(row)
                    kept += 1

    if kept == 0:
        raise RuntimeError(
            "No rows overlapped the requested timestamp span. "
            f"input={input_path} frame_timestamps={frame_path}"
        )

    print(f"[done] wrote {output_path}")
    print(
        "[done] kept_rows="
        f"{kept} timestamp_span=[{min_ts - pad:.9f}, {max_ts + pad:.9f}]"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
