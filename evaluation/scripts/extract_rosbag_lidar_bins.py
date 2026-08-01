#!/usr/bin/env python3
"""Extract a PointCloud2 topic into GT-free KITTI-style scan bins."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path

import numpy as np


POINT_FIELD_DTYPES = {
    1: "i1",   # INT8
    2: "u1",   # UINT8
    3: "i2",   # INT16
    4: "u2",   # UINT16
    5: "i4",   # INT32
    6: "u4",   # UINT32
    7: "f4",   # FLOAT32
    8: "f8",   # FLOAT64
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bag", type=Path, required=True)
    parser.add_argument("--topic", required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(8 * 1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def cloud_dtype(fields, point_step: int, is_bigendian: bool) -> np.dtype:
    names = []
    formats = []
    offsets = []
    byte_order = ">" if is_bigendian else "<"
    for field in fields:
        if field.datatype not in POINT_FIELD_DTYPES:
            raise ValueError(f"unsupported PointField datatype {field.datatype}")
        base = np.dtype(byte_order + POINT_FIELD_DTYPES[field.datatype])
        names.append(field.name)
        formats.append(base if field.count == 1 else (base, field.count))
        offsets.append(field.offset)
    return np.dtype(
        {
            "names": names,
            "formats": formats,
            "offsets": offsets,
            "itemsize": point_step,
        }
    )


def cloud_xyzi(message) -> np.ndarray:
    dtype = cloud_dtype(message.fields, message.point_step, message.is_bigendian)
    points = np.ndarray(
        shape=(message.height, message.width),
        dtype=dtype,
        buffer=message.data,
        strides=(message.row_step, message.point_step),
    ).reshape(-1)
    missing = {"x", "y", "z"} - set(dtype.names or ())
    if missing:
        raise ValueError(f"PointCloud2 is missing fields: {sorted(missing)}")
    intensity_name = next(
        (name for name in ("intensity", "reflectivity") if name in dtype.names),
        None,
    )
    intensity = (
        points[intensity_name]
        if intensity_name is not None
        else np.zeros(len(points), dtype=np.float32)
    )
    output = np.column_stack(
        (points["x"], points["y"], points["z"], intensity)
    ).astype("<f4", copy=False)
    return output[np.all(np.isfinite(output[:, :3]), axis=1)]


def main() -> int:
    args = parse_args()
    import rosbag

    scan_dir = args.output_dir / "velodyne"
    scan_dir.mkdir(parents=True, exist_ok=True)
    timestamps: list[float] = []
    total_points = 0
    fields = None
    with rosbag.Bag(str(args.bag), "r") as bag:
        for frame, (_, message, _) in enumerate(
            bag.read_messages(topics=[args.topic])
        ):
            scan = cloud_xyzi(message)
            scan.tofile(scan_dir / f"{frame:06d}.bin")
            timestamps.append(message.header.stamp.to_sec())
            total_points += len(scan)
            if fields is None:
                fields = [
                    {
                        "name": field.name,
                        "offset": field.offset,
                        "datatype": field.datatype,
                        "count": field.count,
                    }
                    for field in message.fields
                ]
    if not timestamps:
        raise ValueError(f"no messages found on {args.topic}")
    if np.any(np.diff(timestamps) <= 0):
        raise ValueError("LiDAR timestamps must be strictly increasing")
    with (args.output_dir / "timestamps.csv").open(
        "w", newline="", encoding="utf-8"
    ) as stream:
        writer = csv.writer(stream)
        writer.writerow(["frame_id", "timestamp"])
        for frame, stamp in enumerate(timestamps):
            writer.writerow([frame, f"{stamp:.9f}"])
    manifest = {
        "schema_version": 1,
        "method": "rosbag_pointcloud2_to_kitti_bins",
        "ground_truth_used": False,
        "source_bag_sha256": sha256(args.bag),
        "topic": args.topic,
        "frames": len(timestamps),
        "total_points": total_points,
        "first_stamp": timestamps[0],
        "last_stamp": timestamps[-1],
        "source_fields": fields,
        "point_layout": "x y z intensity float32",
    }
    (args.output_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
