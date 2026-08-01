#!/usr/bin/env python3
"""Convert Boreas [x,y,z,i,r,t] scans to ordered KITTI-style XYZI bins."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sequence-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def convert_scan(source: Path, output: Path) -> int:
    values = np.fromfile(source, dtype=np.float32)
    if values.size % 6 != 0:
        raise ValueError(f"{source} does not contain Boreas 6-float points")
    points = values.reshape(-1, 6)
    output.parent.mkdir(parents=True, exist_ok=True)
    np.ascontiguousarray(points[:, :4]).tofile(output)
    return len(points)


def main() -> int:
    args = parse_args()
    scans = sorted((args.sequence_dir / "lidar").glob("*.bin"))
    if not scans:
        raise RuntimeError(f"No Boreas scans under {args.sequence_dir / 'lidar'}")
    args.output_dir.mkdir(parents=True, exist_ok=True)
    records = []
    total_points = 0
    for index, source in enumerate(scans):
        output = args.output_dir / f"{index:06d}.bin"
        points = convert_scan(source, output)
        total_points += points
        records.append(
            {
                "index": index,
                "timestamp_us": int(source.stem),
                "source": str(source),
                "source_sha256": sha256(source),
                "output": str(output),
                "output_sha256": sha256(output),
                "points": points,
            }
        )
        if index % 50 == 0:
            print(f"[boreas-xyzi-bin] {index}/{len(scans)}", flush=True)
    manifest = {
        "schema_version": 1,
        "method": "boreas_six_float_to_ordered_xyzi_bin",
        "ground_truth_used": False,
        "point_policy": "retain every point in source order; copy x,y,z,intensity exactly; omit ring and relative time fields",
        "frames": len(records),
        "points": total_points,
        "records": records,
    }
    manifest_path = args.output_dir / "conversion_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(
        json.dumps(
            {
                "frames": len(records),
                "points": total_points,
                "manifest": str(manifest_path),
                "ground_truth_used": False,
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
