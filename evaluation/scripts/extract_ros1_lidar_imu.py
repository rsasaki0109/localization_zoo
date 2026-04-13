#!/usr/bin/env python3

"""Extract sequential PCDs and IMU CSV from ROS1 bags for pcd_dogfooding."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import numpy as np
from rosbags.rosbag1 import Reader
from rosbags.typesys import Stores, get_typestore


PACKED_POINT_DTYPE = np.dtype(
    [
        ("x", "<f4"),
        ("y", "<f4"),
        ("z", "<f4"),
        ("intensity", "<f4"),
        ("time", "<f4"),
    ]
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract PointCloud2 + IMU from ROS1 bags for Localization Zoo dogfooding.",
    )
    parser.add_argument("--pointcloud-bag", required=True, help="ROS1 bag with PointCloud2.")
    parser.add_argument(
        "--pointcloud-topic",
        default="/lidar/0/pandar_packets",
        help="PointCloud2 topic in the pointcloud bag.",
    )
    parser.add_argument("--imu-bag", required=True, help="ROS1 bag with IMU.")
    parser.add_argument(
        "--imu-topic",
        default="/imu/0/data_raw/corrected",
        help="IMU topic in the IMU bag.",
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
        "--time-mode",
        default="preserve",
        choices=["preserve", "index", "azimuth"],
        help=(
            "How to populate per-point time when the source PointCloud2 lacks a "
            "'time' field. 'preserve' keeps current behavior and errors, "
            "'index' uses firing order, 'azimuth' uses scan-order azimuth."
        ),
    )
    return parser.parse_args()


def get_connection(reader: Reader, topic: str):
    for conn in reader.connections:
        if conn.topic == topic:
            return conn
    raise RuntimeError(f"Topic not found: {topic}")


def stamp_to_sec(stamp) -> float:
    return float(stamp.sec) + float(stamp.nanosec) * 1e-9


def pointcloud_dtype(msg, names: tuple[str, ...]) -> np.dtype:
    offsets = {field.name: field.offset for field in msg.fields}
    missing = [name for name in names if name not in offsets]
    if missing:
        raise RuntimeError(f"PointCloud2 is missing fields: {', '.join(missing)}")
    return np.dtype(
        {
            "names": list(names),
            "formats": ["<f4"] * len(names),
            "offsets": [offsets[name] for name in names],
            "itemsize": msg.point_step,
        }
    )


def synthesize_index_time(count: int) -> np.ndarray:
    if count <= 1:
        return np.zeros(count, dtype=np.float32)
    return np.linspace(0.0, 1.0, count, dtype=np.float32)


def synthesize_azimuth_time(x: np.ndarray, y: np.ndarray) -> np.ndarray:
    azimuth = np.arctan2(y, x)
    unwrapped = np.unwrap(azimuth)
    span = float(unwrapped[-1] - unwrapped[0]) if len(unwrapped) > 1 else 0.0
    if abs(span) <= 1e-9:
        return synthesize_index_time(len(unwrapped))
    normalized = (unwrapped - unwrapped[0]) / span
    return np.clip(normalized.astype(np.float32), 0.0, 1.0)


def extract_points(msg, time_mode: str) -> np.ndarray:
    offsets = {field.name: field.offset for field in msg.fields}
    source_names = ["x", "y", "z"]
    if "intensity" in offsets:
        source_names.append("intensity")
    if "time" in offsets:
        source_names.append("time")

    source_dtype = pointcloud_dtype(msg, tuple(source_names))
    count = int(msg.width) * int(msg.height)
    source = np.frombuffer(msg.data, dtype=source_dtype, count=count)
    packed = np.empty(count, dtype=PACKED_POINT_DTYPE)
    packed["x"] = source["x"]
    packed["y"] = source["y"]
    packed["z"] = source["z"]
    packed["intensity"] = source["intensity"] if "intensity" in source.dtype.names else 0.0
    if "time" in source.dtype.names:
        packed["time"] = source["time"]
    elif time_mode == "index":
        packed["time"] = synthesize_index_time(count)
    elif time_mode == "azimuth":
        packed["time"] = synthesize_azimuth_time(source["x"], source["y"])
    else:
        raise RuntimeError(
            "PointCloud2 is missing fields: time. "
            "Re-run with --time-mode index or --time-mode azimuth to synthesize it."
        )
    return packed


def write_binary_pcd(path: Path, points: np.ndarray) -> None:
    header = "\n".join(
        [
            "# .PCD v0.7 - Point Cloud Data file format",
            "VERSION 0.7",
            "FIELDS x y z intensity time",
            "SIZE 4 4 4 4 4",
            "TYPE F F F F F",
            "COUNT 1 1 1 1 1",
            f"WIDTH {len(points)}",
            "HEIGHT 1",
            "VIEWPOINT 0 0 0 1 0 0 0",
            f"POINTS {len(points)}",
            "DATA binary",
            "",
        ]
    ).encode("ascii")

    with path.open("wb") as f:
        f.write(header)
        f.write(points.tobytes())


def export_pointclouds(
    bag_path: Path,
    topic: str,
    out_dir: Path,
    start_frame: int,
    max_frames: int,
    typestore,
    time_mode: str,
) -> int:
    out_dir.mkdir(parents=True, exist_ok=True)
    timestamps_path = out_dir / "frame_timestamps.csv"
    written = 0

    with Reader(bag_path) as reader, timestamps_path.open("w", newline="") as csv_file:
        conn = get_connection(reader, topic)
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        for frame_idx, (_, _, raw) in enumerate(reader.messages(connections=[conn])):
            if frame_idx < start_frame:
                continue
            if max_frames >= 0 and written >= max_frames:
                break

            msg = typestore.deserialize_ros1(raw, conn.msgtype)
            points = extract_points(msg, time_mode)
            scan_dir = out_dir / f"{written:08d}"
            scan_dir.mkdir(parents=True, exist_ok=True)
            write_binary_pcd(scan_dir / "cloud.pcd", points)

            writer.writerow([written, f"{stamp_to_sec(msg.header.stamp):.9f}", len(points)])
            written += 1
            if written == 1 or written % 100 == 0:
                print(f"[lidar] exported {written} frames")

    return written


def export_imu(bag_path: Path, topic: str, out_dir: Path, typestore) -> int:
    out_dir.mkdir(parents=True, exist_ok=True)
    imu_path = out_dir / "imu.csv"
    written = 0

    with Reader(bag_path) as reader, imu_path.open("w", newline="") as csv_file:
        conn = get_connection(reader, topic)
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

        for _, _, raw in reader.messages(connections=[conn]):
            msg = typestore.deserialize_ros1(raw, conn.msgtype)
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

    return written


def main() -> int:
    args = parse_args()
    out_dir = Path(args.output_dir).resolve()
    typestore = get_typestore(Stores.ROS1_NOETIC)

    lidar_count = export_pointclouds(
        bag_path=Path(args.pointcloud_bag),
        topic=args.pointcloud_topic,
        out_dir=out_dir,
        start_frame=args.start_frame,
        max_frames=args.max_frames,
        typestore=typestore,
        time_mode=args.time_mode,
    )
    imu_count = export_imu(
        bag_path=Path(args.imu_bag),
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
