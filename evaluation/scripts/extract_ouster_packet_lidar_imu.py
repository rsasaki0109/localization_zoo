#!/usr/bin/env python3

"""Extract Ouster PacketMsg LiDAR + IMU from ROS1 bags into PCD dogfooding layout."""

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
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bag", required=True, help="ROS1 bag with Ouster packet topics.")
    parser.add_argument("--output-dir", required=True, help="Output dogfooding PCD directory.")
    parser.add_argument(
        "--imu-topic",
        default="/vectornav_node/uncomp_imu",
        help="IMU topic to export into imu.csv.",
    )
    parser.add_argument(
        "--timestamp-topic",
        default="/sensor_sync_node/trigger_1",
        help=(
            "Header-stamped trigger topic used for LiDAR frame timestamps. "
            "Use an empty string to fall back to SDK packet/column timestamps."
        ),
    )
    parser.add_argument(
        "--intensity-field",
        choices=["SIGNAL", "REFLECTIVITY", "NEAR_IR", "RANGE"],
        default="SIGNAL",
        help="Ouster channel field to write as PCD intensity.",
    )
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First decoded LiDAR scan index to export after optional incomplete-frame filtering.",
    )
    parser.add_argument(
        "--frame-stride",
        type=int,
        default=1,
        help="Export every Nth decoded LiDAR scan after --start-frame. Useful for full-bag scouting.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=200,
        help="Maximum LiDAR frames to export. Use -1 for all frames.",
    )
    parser.add_argument(
        "--skip-incomplete",
        action="store_true",
        help="Skip incomplete decoded scans, usually useful to drop the first partial frame.",
    )
    parser.add_argument(
        "--soft-id-check",
        action="store_true",
        help="Allow Ouster packet init_id mismatches in the SDK source.",
    )
    return parser.parse_args()


def stamp_to_sec(stamp) -> float:
    return float(stamp.sec) + float(stamp.nanosec) * 1e-9


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

    with path.open("wb") as handle:
        handle.write(header)
        handle.write(points.tobytes())


def relative_column_times(scan) -> np.ndarray:
    timestamps = np.asarray(scan.timestamp, dtype=np.float64)
    valid = timestamps > 0
    if not np.any(valid):
        if scan.w <= 1:
            return np.zeros(scan.w, dtype=np.float32)
        return np.linspace(0.0, 1.0, scan.w, dtype=np.float32)
    first = float(timestamps[valid][0])
    rel = (timestamps - first) * 1e-9
    rel[~valid] = 0.0
    return rel.astype(np.float32)


def frame_timestamp_sec(scan, trigger_timestamps: list[float], scan_index: int) -> float:
    if scan_index < len(trigger_timestamps):
        return trigger_timestamps[scan_index]
    packet_stamp = int(scan.get_first_valid_packet_timestamp())
    if packet_stamp > 0:
        return packet_stamp * 1e-9
    column_stamp = int(scan.get_first_valid_column_timestamp())
    return column_stamp * 1e-9


def scan_to_points(scan, xyz_lut, intensity_field: str) -> np.ndarray:
    xyz = xyz_lut(scan).astype(np.float32, copy=False)
    ranges = np.asarray(scan.field("RANGE"))
    if scan.has_field(intensity_field):
        intensity = np.asarray(scan.field(intensity_field), dtype=np.float32)
    else:
        intensity = ranges.astype(np.float32)
    rel_time = np.broadcast_to(relative_column_times(scan), ranges.shape).astype(np.float32, copy=False)

    valid = (ranges > 0) & np.isfinite(xyz).all(axis=2)
    points = np.empty(int(np.count_nonzero(valid)), dtype=PACKED_POINT_DTYPE)
    points["x"] = xyz[:, :, 0][valid]
    points["y"] = xyz[:, :, 1][valid]
    points["z"] = xyz[:, :, 2][valid]
    points["intensity"] = intensity[valid]
    points["time"] = rel_time[valid]
    return points


def export_lidar(
    *,
    bag_path: Path,
    output_dir: Path,
    start_frame: int,
    frame_stride: int,
    max_frames: int,
    skip_incomplete: bool,
    soft_id_check: bool,
    intensity_field: str,
    trigger_timestamps: list[float],
) -> int:
    try:
        from ouster.sdk import core
        from ouster.sdk.bag import BagScanSource
    except ImportError as exc:
        raise RuntimeError("ouster-sdk is required. Install with `pip install ouster-sdk`.") from exc

    source = BagScanSource(str(bag_path), soft_id_check=soft_id_check)
    xyz_lut = core.XYZLut(source.sensor_info[0])
    output_dir.mkdir(parents=True, exist_ok=True)
    timestamps_path = output_dir / "frame_timestamps.csv"
    written = 0
    considered = 0
    if frame_stride <= 0:
        raise ValueError("frame_stride must be positive")

    with timestamps_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points", "source_frame_id", "complete", "source_scan_index"])
        for scan_set in source:
            scan = scan_set[0]
            if skip_incomplete and not scan.complete():
                continue
            if considered < start_frame:
                considered += 1
                continue
            if (considered - start_frame) % frame_stride != 0:
                considered += 1
                continue
            if max_frames >= 0 and written >= max_frames:
                break

            points = scan_to_points(scan, xyz_lut, intensity_field)
            scan_dir = output_dir / f"{written:08d}"
            scan_dir.mkdir(parents=True, exist_ok=True)
            write_binary_pcd(scan_dir / "cloud.pcd", points)
            writer.writerow(
                [
                    written,
                    f"{frame_timestamp_sec(scan, trigger_timestamps, considered):.9f}",
                    len(points),
                    int(scan.frame_id),
                    bool(scan.complete()),
                    int(considered),
                ]
            )
            written += 1
            considered += 1
            if written == 1 or written % 100 == 0:
                print(f"[lidar] exported {written} frames")

    return written


def get_connection(reader: Reader, topic: str):
    for conn in reader.connections:
        if conn.topic == topic:
            return conn
    raise RuntimeError(f"Topic not found: {topic}")


def read_header_timestamps(bag_path: Path, topic: str, typestore) -> list[float]:
    if not topic:
        return []
    stamps: list[float] = []
    with Reader(bag_path) as reader:
        conn = get_connection(reader, topic)
        for _, _, raw in reader.messages(connections=[conn]):
            msg = typestore.deserialize_ros1(raw, conn.msgtype)
            if hasattr(msg, "header"):
                stamps.append(stamp_to_sec(msg.header.stamp))
            elif hasattr(msg, "stamp"):
                stamps.append(stamp_to_sec(msg.stamp))
            else:
                raise RuntimeError(f"Topic {topic} has no header/stamp field")
    print(f"[timestamps] loaded {len(stamps)} stamps from {topic}")
    return stamps


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
                    msg.angular_velocity.x,
                    msg.angular_velocity.y,
                    msg.angular_velocity.z,
                    msg.linear_acceleration.x,
                    msg.linear_acceleration.y,
                    msg.linear_acceleration.z,
                ]
            )
            written += 1

    print(f"[imu] exported {written} messages")
    return written


def main() -> int:
    args = parse_args()
    bag_path = Path(args.bag)
    output_dir = Path(args.output_dir)
    typestore = get_typestore(Stores.ROS1_NOETIC)
    trigger_timestamps = read_header_timestamps(bag_path, args.timestamp_topic, typestore)

    lidar_count = export_lidar(
        bag_path=bag_path,
        output_dir=output_dir,
        start_frame=args.start_frame,
        frame_stride=args.frame_stride,
        max_frames=args.max_frames,
        skip_incomplete=args.skip_incomplete,
        soft_id_check=args.soft_id_check,
        intensity_field=args.intensity_field,
        trigger_timestamps=trigger_timestamps,
    )
    imu_count = export_imu(bag_path, args.imu_topic, output_dir, typestore)
    print(f"[done] lidar_frames={lidar_count} imu_messages={imu_count} output={output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
