#!/usr/bin/env python3

"""Extract sequential PCDs and optional IMU CSV from ROS2 sqlite bags."""

from __future__ import annotations

import argparse
import csv
import sqlite3
from pathlib import Path

import numpy as np
from rosbags.typesys import Stores, get_typestore


FIELD_SPECS = {
    "x": "<f4",
    "y": "<f4",
    "z": "<f4",
    "intensity": "<f4",
    "time": "<f4",
}
SOURCE_FIELD_SPECS = {
    **FIELD_SPECS,
    "t": "<u4",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract PointCloud2 + optional IMU from ROS2 sqlite bags.",
    )
    parser.add_argument(
        "--bag",
        action="append",
        required=True,
        help="ROS2 bag directory or .db3 file. Repeat to concatenate split bags.",
    )
    parser.add_argument(
        "--pointcloud-topic",
        required=True,
        help="PointCloud2 topic to export.",
    )
    parser.add_argument(
        "--imu-topic",
        default="",
        help="Optional IMU topic to export.",
    )
    parser.add_argument("--output-dir", required=True, help="Output directory.")
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First lidar frame index to export.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=-1,
        help="Maximum number of lidar frames to export. -1 means all.",
    )
    parser.add_argument(
        "--ros-distro",
        default="humble",
        choices=["foxy", "galactic", "humble", "iron", "jazzy", "latest"],
        help="Typestore to use for ROS2 message deserialization.",
    )
    return parser.parse_args()


def resolve_typestore(name: str):
    store_name = {
        "foxy": "ROS2_FOXY",
        "galactic": "ROS2_GALACTIC",
        "humble": "ROS2_HUMBLE",
        "iron": "ROS2_IRON",
        "jazzy": "ROS2_JAZZY",
        "latest": "LATEST",
    }[name]
    store = getattr(Stores, store_name, None)
    if store is None:
        available = ", ".join(member.name for member in Stores)
        raise RuntimeError(
            f"This rosbags version has no {store_name} typestore. "
            f"Available stores: {available}"
        )
    return get_typestore(store)


def resolve_db3_files(path: Path) -> list[Path]:
    if path.is_file():
        return [path]
    return sorted(path.glob("*.db3"))


def resolve_db3_inputs(paths: list[str]) -> list[Path]:
    db3_files: list[Path] = []
    for value in paths:
        resolved = resolve_db3_files(Path(value))
        if not resolved:
            raise RuntimeError(f"No .db3 files found under: {value}")
        db3_files.extend(resolved)
    return db3_files


def get_topic_info(cursor: sqlite3.Cursor, topic: str):
    row = cursor.execute(
        "select id, type from topics where name = ?",
        (topic,),
    ).fetchone()
    if row is None:
        raise RuntimeError(f"Topic not found: {topic}")
    return int(row[0]), str(row[1])


def make_field_dtype(msg) -> np.dtype:
    offsets = {field.name: field.offset for field in msg.fields}
    names = [
        name
        for name in ("x", "y", "z", "intensity", "time", "t")
        if name in offsets
    ]
    if not {"x", "y", "z"}.issubset(names):
        raise RuntimeError("PointCloud2 must contain x/y/z fields.")
    return np.dtype(
        {
            "names": names,
            "formats": [SOURCE_FIELD_SPECS[name] for name in names],
            "offsets": [offsets[name] for name in names],
            "itemsize": msg.point_step,
        }
    )


def extract_points(msg) -> tuple[np.ndarray, list[str]]:
    source_dtype = make_field_dtype(msg)
    count = int(msg.width) * int(msg.height)
    source = np.frombuffer(msg.data, dtype=source_dtype, count=count)
    names = ["x", "y", "z", "intensity"]
    if "time" in source_dtype.names or "t" in source_dtype.names:
        names.append("time")
    packed_dtype = np.dtype([(name, FIELD_SPECS[name]) for name in names])
    packed = np.empty(count, dtype=packed_dtype)
    for name in ("x", "y", "z"):
        packed[name] = source[name]
    packed["intensity"] = (
        source["intensity"] if "intensity" in source_dtype.names else 0.0
    )
    if "time" in source_dtype.names:
        packed["time"] = source["time"]
    elif "t" in source_dtype.names:
        packed["time"] = source["t"].astype(np.float32) * 1e-9
    return packed, names


def write_binary_pcd(path: Path, points: np.ndarray, field_names: list[str]) -> None:
    sizes = " ".join(["4"] * len(field_names))
    types = " ".join(["F"] * len(field_names))
    counts = " ".join(["1"] * len(field_names))
    fields = " ".join(field_names)
    header = "\n".join(
        [
            "# .PCD v0.7 - Point Cloud Data file format",
            "VERSION 0.7",
            f"FIELDS {fields}",
            f"SIZE {sizes}",
            f"TYPE {types}",
            f"COUNT {counts}",
            f"WIDTH {len(points)}",
            "HEIGHT 1",
            "VIEWPOINT 0 0 0 1 0 0 0",
            f"POINTS {len(points)}",
            "DATA binary",
            "",
        ]
    ).encode("ascii")

    with path.open("wb") as handle:
        handle.write(header)
        handle.write(points.tobytes())


def stamp_to_sec(stamp) -> float:
    return float(stamp.sec) + float(stamp.nanosec) * 1e-9


def export_pointclouds(
    db3_files: list[Path],
    topic: str,
    out_dir: Path,
    start_frame: int,
    max_frames: int,
    typestore,
) -> int:
    out_dir.mkdir(parents=True, exist_ok=True)
    timestamps_path = out_dir / "frame_timestamps.csv"
    written = 0
    seen = 0

    with timestamps_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        for db3_path in db3_files:
            conn = sqlite3.connect(db3_path)
            cursor = conn.cursor()
            topic_id, msgtype = get_topic_info(cursor, topic)
            query = (
                "select timestamp, data from messages "
                "where topic_id = ? order by timestamp"
            )
            for _, raw in cursor.execute(query, (topic_id,)):
                if seen < start_frame:
                    seen += 1
                    continue
                if max_frames >= 0 and written >= max_frames:
                    break

                msg = typestore.deserialize_cdr(raw, msgtype)
                points, field_names = extract_points(msg)
                scan_dir = out_dir / f"{written:08d}"
                scan_dir.mkdir(parents=True, exist_ok=True)
                write_binary_pcd(scan_dir / "cloud.pcd", points, field_names)

                writer.writerow(
                    [written, f"{stamp_to_sec(msg.header.stamp):.9f}", len(points)]
                )
                written += 1
                seen += 1
                if written == 1 or written % 100 == 0:
                    print(f"[lidar] exported {written} frames")

            conn.close()
            if max_frames >= 0 and written >= max_frames:
                break

    return written


def export_imu(db3_files: list[Path], topic: str, out_dir: Path, typestore) -> int:
    if not topic:
        return 0

    out_dir.mkdir(parents=True, exist_ok=True)
    imu_path = out_dir / "imu.csv"
    written = 0

    with imu_path.open("w", newline="") as csv_file:
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

        for db3_path in db3_files:
            conn = sqlite3.connect(db3_path)
            cursor = conn.cursor()
            try:
                topic_id, msgtype = get_topic_info(cursor, topic)
            except RuntimeError:
                conn.close()
                continue

            query = (
                "select timestamp, data from messages "
                "where topic_id = ? order by timestamp"
            )
            for _, raw in cursor.execute(query, (topic_id,)):
                msg = typestore.deserialize_cdr(raw, msgtype)
                writer.writerow(
                    [
                        f"{stamp_to_sec(msg.header.stamp):.9f}",
                        f"{msg.angular_velocity.x:.12f}",
                        f"{msg.angular_velocity.y:.12f}",
                        f"{msg.angular_velocity.z:.12f}",
                        f"{msg.linear_acceleration.x:.12f}",
                        f"{msg.linear_acceleration.y:.12f}",
                        f"{msg.linear_acceleration.z:.12f}",
                    ]
                )
                written += 1
            conn.close()

    return written


def main() -> int:
    args = parse_args()
    out_dir = Path(args.output_dir).resolve()
    db3_files = resolve_db3_inputs(args.bag)

    typestore = resolve_typestore(args.ros_distro)
    lidar_count = export_pointclouds(
        db3_files=db3_files,
        topic=args.pointcloud_topic,
        out_dir=out_dir,
        start_frame=args.start_frame,
        max_frames=args.max_frames,
        typestore=typestore,
    )
    imu_count = export_imu(
        db3_files=db3_files,
        topic=args.imu_topic,
        out_dir=out_dir,
        typestore=typestore,
    )

    print(f"[done] output_dir={out_dir}")
    print(f"[done] lidar_frames={lidar_count}")
    print(f"[done] imu_samples={imu_count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
