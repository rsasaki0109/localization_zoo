#!/usr/bin/env python3
"""Build a GT-free NTU VIRAL ROS bag from extracted LiDAR and IMU binaries."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

import numpy as np
import rosbag
import rospy
from sensor_msgs.msg import Imu, PointCloud2, PointField


IMU_DTYPE = np.dtype(
    [
        ("stamp", "<f8"),
        ("ax", "<f8"),
        ("ay", "<f8"),
        ("az", "<f8"),
        ("gx", "<f8"),
        ("gy", "<f8"),
        ("gz", "<f8"),
    ]
)
SOURCE_POINT_DTYPE = np.dtype(
    [("x", "<f4"), ("y", "<f4"), ("z", "<f4"), ("intensity", "<f4"), ("t_ms", "<f4")]
)
OUSTER_POINT_DTYPE = np.dtype(
    {
        "names": ["x", "y", "z", "intensity", "t", "reflectivity", "ring", "ambient", "range"],
        "formats": ["<f4", "<f4", "<f4", "<f4", "<u4", "<u2", "u1", "<u2", "<u4"],
        "offsets": [0, 4, 8, 12, 16, 20, 22, 24, 28],
        "itemsize": 32,
    }
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lidar-bin", type=Path, required=True)
    parser.add_argument("--imu-bin", type=Path, required=True)
    parser.add_argument("--output-bag", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--max-lidar-frames", type=int)
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def ros_time(stamp: float) -> rospy.Time:
    return rospy.Time.from_sec(float(stamp))


def make_imu(row: np.void, sequence: int) -> Imu:
    message = Imu()
    message.header.seq = sequence
    message.header.stamp = ros_time(row["stamp"])
    message.header.frame_id = "imu"
    message.orientation_covariance[0] = -1.0
    message.linear_acceleration.x = float(row["ax"])
    message.linear_acceleration.y = float(row["ay"])
    message.linear_acceleration.z = float(row["az"])
    message.angular_velocity.x = float(row["gx"])
    message.angular_velocity.y = float(row["gy"])
    message.angular_velocity.z = float(row["gz"])
    return message


def make_cloud(stamp: float, source: np.ndarray, sequence: int) -> PointCloud2:
    points = np.zeros(len(source), dtype=OUSTER_POINT_DTYPE)
    for field in ("x", "y", "z", "intensity"):
        points[field] = source[field]
    points["t"] = np.clip(np.rint(source["t_ms"] * 1.0e6), 0, np.iinfo(np.uint32).max).astype(np.uint32)
    points["range"] = np.clip(
        np.rint(np.sqrt(source["x"] ** 2 + source["y"] ** 2 + source["z"] ** 2) * 1000.0),
        0,
        np.iinfo(np.uint32).max,
    ).astype(np.uint32)
    message = PointCloud2()
    message.header.seq = sequence
    message.header.stamp = ros_time(stamp)
    message.header.frame_id = "os1_sensor"
    message.height = 1
    message.width = len(points)
    message.fields = [
        PointField("x", 0, PointField.FLOAT32, 1),
        PointField("y", 4, PointField.FLOAT32, 1),
        PointField("z", 8, PointField.FLOAT32, 1),
        PointField("intensity", 12, PointField.FLOAT32, 1),
        PointField("t", 16, PointField.UINT32, 1),
        PointField("reflectivity", 20, PointField.UINT16, 1),
        PointField("ring", 22, PointField.UINT8, 1),
        PointField("ambient", 24, PointField.UINT16, 1),
        PointField("range", 28, PointField.UINT32, 1),
    ]
    message.is_bigendian = False
    message.point_step = OUSTER_POINT_DTYPE.itemsize
    message.row_step = message.point_step * message.width
    message.data = points.tobytes()
    message.is_dense = True
    return message


def iter_lidar(path: Path):
    with path.open("rb") as stream:
        while True:
            header = stream.read(16)
            if not header:
                return
            if len(header) != 16:
                raise ValueError("short LiDAR frame header")
            stamp, count, _ = struct.unpack("<dII", header)
            payload = stream.read(count * SOURCE_POINT_DTYPE.itemsize)
            if len(payload) != count * SOURCE_POINT_DTYPE.itemsize:
                raise ValueError("short LiDAR frame payload")
            yield stamp, np.frombuffer(payload, dtype=SOURCE_POINT_DTYPE)


def main() -> int:
    args = parse_args()
    imu = np.fromfile(args.imu_bin, dtype=IMU_DTYPE)
    if not len(imu) or np.any(np.diff(imu["stamp"]) <= 0):
        raise ValueError("IMU timestamps must be strictly increasing")
    args.output_bag.parent.mkdir(parents=True, exist_ok=True)
    lidar_frames = 0
    lidar_points = 0
    imu_index = 0
    first_lidar_stamp = None
    last_lidar_stamp = None
    with rosbag.Bag(str(args.output_bag), "w") as bag:
        for lidar_frames, (stamp, points) in enumerate(iter_lidar(args.lidar_bin), start=1):
            if args.max_lidar_frames is not None and lidar_frames > args.max_lidar_frames:
                lidar_frames -= 1
                break
            while imu_index < len(imu) and imu[imu_index]["stamp"] <= stamp:
                message = make_imu(imu[imu_index], imu_index)
                bag.write("/imu/imu", message, message.header.stamp)
                imu_index += 1
            cloud = make_cloud(stamp, points, lidar_frames - 1)
            bag.write("/os1_cloud_node1/points", cloud, cloud.header.stamp)
            lidar_points += len(points)
            first_lidar_stamp = stamp if first_lidar_stamp is None else first_lidar_stamp
            last_lidar_stamp = stamp
        while imu_index < len(imu) and (
            last_lidar_stamp is None or imu[imu_index]["stamp"] <= last_lidar_stamp
        ):
            message = make_imu(imu[imu_index], imu_index)
            bag.write("/imu/imu", message, message.header.stamp)
            imu_index += 1
    manifest = {
        "schema_version": 1,
        "method": "ntu_viral_extracted_sensor_only_rosbag",
        "ground_truth_used": False,
        "topics": ["/imu/imu", "/os1_cloud_node1/points"],
        "lidar_frames": lidar_frames,
        "lidar_points": lidar_points,
        "imu_samples": imu_index,
        "first_lidar_stamp": first_lidar_stamp,
        "last_lidar_stamp": last_lidar_stamp,
        "lidar_sha256": sha256(args.lidar_bin),
        "imu_sha256": sha256(args.imu_bin),
        "bag_sha256": sha256(args.output_bag),
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
