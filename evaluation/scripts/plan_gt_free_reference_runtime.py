#!/usr/bin/env python3
"""Plan whether the independent reference can fit the real-time budget without GT."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from run_official_kiss_pcd_odometry import collect_scans, scan_schema

DEFAULT_MAX_REFERENCE_FIRST_SCAN_POINTS = 150_000


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--scan-dir", type=Path)
    source.add_argument(
        "--first-scan",
        type=Path,
        help="Frozen first scan; avoids enumerating the full input tree.",
    )
    parser.add_argument(
        "--frames",
        type=int,
        help="Frozen input frame count; required with --first-scan.",
    )
    parser.add_argument("--output-json", type=Path, required=True)
    parser.add_argument(
        "--max-reference-first-scan-points",
        type=int,
        default=DEFAULT_MAX_REFERENCE_FIRST_SCAN_POINTS,
        help=(
            "Maximum first-scan point count for running the independent "
            "reference (default: 150000)."
        ),
    )
    return parser.parse_args()


def reference_is_runtime_feasible(
    first_scan_points: int, maximum_first_scan_points: int
) -> bool:
    if first_scan_points < 0:
        raise ValueError("first_scan_points must be non-negative")
    if maximum_first_scan_points <= 0:
        raise ValueError("maximum_first_scan_points must be positive")
    return first_scan_points <= maximum_first_scan_points


def main() -> int:
    args = parse_args()
    if args.first_scan is not None:
        if args.frames is None or args.frames <= 0:
            raise ValueError("--frames must be positive with --first-scan")
        first_scan = args.first_scan.resolve()
        if not first_scan.is_file():
            raise RuntimeError(f"First scan does not exist: {first_scan}")
        frame_count = args.frames
        scan_dir = None
    else:
        if args.frames is not None:
            raise ValueError("--frames is only valid with --first-scan")
        scans = collect_scans(args.scan_dir.resolve(), -1)
        if not scans:
            raise RuntimeError(f"No PCD or KITTI bin scans found in {args.scan_dir}")
        first_scan = scans[0]
        frame_count = len(scans)
        scan_dir = str(args.scan_dir.resolve())
    has_timestamps, point_count = scan_schema(first_scan)
    run_reference = reference_is_runtime_feasible(
        point_count, args.max_reference_first_scan_points
    )
    payload = {
        "schema_version": 1,
        "method": "gt_free_reference_runtime_feasibility",
        "ground_truth_used": False,
        "dataset_identity_used": False,
        "signal": "first scan point count only",
        "scan_dir": scan_dir,
        "frames": frame_count,
        "first_scan": str(first_scan),
        "first_scan_has_timestamps": has_timestamps,
        "first_scan_points": point_count,
        "maximum_reference_first_scan_points": args.max_reference_first_scan_points,
        "run_reference": run_reference,
        "fallback_when_reference_skipped": "raw frontend if pose graph is rejected",
    }
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
