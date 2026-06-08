#!/usr/bin/env python3
"""Prepare 2D laser scan fixtures for scan_dogfooding.

Exports the repository layout:
  <output_dir>/scan_meta.json
  <output_dir>/00000000/scan.csv   (comma-separated ranges)
  <output_dir>/gt.csv              (timestamp,x,y,yaw)
  <output_dir>/frame_timestamps.csv (optional audit trail)

Sources:
  - ROS1 bag with sensor_msgs/LaserScan (--ros1-bag)
  - Existing scan tree validation (--validate-only)

Ground truth:
  - CSV with timestamp,x,y,yaw (or x,y,yaw only) interpolated to scan times
  - TUM format (--gt-tum): timestamp tx ty tz qx qy qz qw (planar yaw extracted)

Requires `rosbags` for bag import: pip install rosbags
"""

from __future__ import annotations

import argparse
import bisect
import csv
import json
import math
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Prepare 2D LaserScan inputs for scan_dogfooding.",
    )
    parser.add_argument(
        "--output-dir",
        required=True,
        help="Directory to write scan_meta.json, NNNNNNNN/scan.csv, gt.csv.",
    )
    parser.add_argument(
        "--ros1-bag",
        default="",
        help="ROS1 .bag file containing sensor_msgs/LaserScan.",
    )
    parser.add_argument(
        "--scan-topic",
        default="/scan",
        help="LaserScan topic in the ROS1 bag.",
    )
    parser.add_argument(
        "--gt-csv",
        default="",
        help="Ground-truth CSV (timestamp,x,y,yaw). Interpolated to scan times.",
    )
    parser.add_argument(
        "--gt-tum",
        default="",
        help="Ground-truth TUM file (timestamp tx ty tz qx qy qz qw).",
    )
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First scan frame index to export.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=-1,
        help="Maximum scan frames to export (-1 = all).",
    )
    parser.add_argument(
        "--scan-rate-hz",
        type=float,
        default=0.0,
        help="Override scan_rate_hz in scan_meta.json (0 = infer from stamps).",
    )
    parser.add_argument(
        "--inspect",
        action="store_true",
        help="List LaserScan topics in --ros1-bag and exit.",
    )
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="Validate an existing output-dir layout without exporting.",
    )
    return parser.parse_args()


def stamp_to_sec(stamp) -> float:
    return float(stamp.sec) + float(stamp.nanosec) * 1e-9


def yaw_from_quat(qx: float, qy: float, qz: float, qw: float) -> float:
    siny_cosp = 2.0 * (qw * qz + qx * qy)
    cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz)
    return math.atan2(siny_cosp, cosy_cosp)


def load_gt_csv(path: Path) -> list[tuple[float, float, float, float]]:
    rows: list[tuple[float, float, float, float]] = []
    with path.open(newline="") as f:
        reader = csv.reader(f)
        header = next(reader, None)
        if header is None:
            return rows
        cols = [c.strip() for c in header]
        has_ts = "timestamp" in cols
        ix = cols.index("x") if "x" in cols else 1
        iy = cols.index("y") if "y" in cols else 2
        iyaw = cols.index("yaw") if "yaw" in cols else 3
        its = cols.index("timestamp") if has_ts else -1
        for i, row in enumerate(reader):
            if len(row) < 3:
                continue
            try:
                ts = float(row[its]) if its >= 0 else float(i)
                x = float(row[ix])
                y = float(row[iy])
                yaw = float(row[iyaw])
            except (ValueError, IndexError):
                continue
            rows.append((ts, x, y, yaw))
    rows.sort(key=lambda r: r[0])
    return rows


def load_gt_tum(path: Path) -> list[tuple[float, float, float, float]]:
    rows: list[tuple[float, float, float, float]] = []
    with path.open() as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            if len(parts) < 8:
                continue
            ts = float(parts[0])
            x, y = float(parts[1]), float(parts[2])
            qx, qy, qz, qw = (float(parts[4]), float(parts[5]), float(parts[6]), float(parts[7]))
            rows.append((ts, x, y, yaw_from_quat(qx, qy, qz, qw)))
    rows.sort(key=lambda r: r[0])
    return rows


def interpolate_gt(
    gt: list[tuple[float, float, float, float]], t: float
) -> tuple[float, float, float]:
    if not gt:
        return 0.0, 0.0, 0.0
    times = [g[0] for g in gt]
    if t <= times[0]:
        return gt[0][1], gt[0][2], gt[0][3]
    if t >= times[-1]:
        return gt[-1][1], gt[-1][2], gt[-1][3]
    j = bisect.bisect_right(times, t)
    t0, x0, y0, yaw0 = gt[j - 1]
    t1, x1, y1, yaw1 = gt[j]
    if abs(t1 - t0) < 1e-12:
        return x0, y0, yaw0
    alpha = (t - t0) / (t1 - t0)
    x = x0 + alpha * (x1 - x0)
    y = y0 + alpha * (y1 - y0)
    dyaw = yaw1 - yaw0
    dyaw = (dyaw + math.pi) % (2 * math.pi) - math.pi
    yaw = yaw0 + alpha * dyaw
    return x, y, yaw


def inspect_ros1_bag(bag_path: Path) -> int:
    try:
        from rosbags.rosbag1 import Reader
    except ImportError as exc:
        raise RuntimeError(
            "rosbags is required for --inspect. Install with `pip install rosbags`."
        ) from exc

    print(f"Inspecting {bag_path}")
    with Reader(bag_path) as reader:
        topics: dict[str, tuple[str, int]] = {}
        for conn in reader.connections:
            topics[conn.topic] = (conn.msgtype, topics.get(conn.topic, (conn.msgtype, 0))[1] + 1)
        for topic, (msgtype, _count) in sorted(topics.items()):
            marker = " *" if "LaserScan" in msgtype else ""
            print(f"  {topic}  [{msgtype}]{marker}")
    return 0


def export_ros1_laserscan(
    bag_path: Path,
    topic: str,
    output_dir: Path,
    gt: list[tuple[float, float, float, float]],
    start_frame: int,
    max_frames: int,
    scan_rate_hz: float,
) -> int:
    try:
        from rosbags.rosbag1 import Reader
        from rosbags.typesys import Stores, get_typestore
    except ImportError as exc:
        raise RuntimeError(
            "rosbags is required for --ros1-bag. Install with `pip install rosbags`."
        ) from exc

    typestore = get_typestore(Stores.ROS1_NOETIC)
    output_dir.mkdir(parents=True, exist_ok=True)

    scans: list[tuple[float, object]] = []
    with Reader(bag_path) as reader:
        conn = None
        for c in reader.connections:
            if c.topic == topic:
                conn = c
                break
        if conn is None:
            raise RuntimeError(f"Topic not found: {topic}")

        for _conn, _ts, raw in reader.messages(connections=[conn]):
            msg = typestore.deserialize_ros1(raw, conn.msgtype)
            t = stamp_to_sec(msg.header.stamp)
            scans.append((t, msg))

    if not scans:
        raise RuntimeError(f"No LaserScan messages on {topic}")

    if start_frame > 0:
        scans = scans[start_frame:]
    if max_frames >= 0:
        scans = scans[:max_frames]

    stamps = [s[0] for s in scans]
    if scan_rate_hz <= 0.0 and len(stamps) > 1:
        dt = (stamps[-1] - stamps[0]) / max(len(stamps) - 1, 1)
        scan_rate_hz = 1.0 / dt if dt > 1e-9 else 10.0
    if scan_rate_hz <= 0.0:
        scan_rate_hz = 10.0

    first_msg = scans[0][1]
    meta = {
        "angle_min": float(first_msg.angle_min),
        "angle_max": float(first_msg.angle_max),
        "angle_increment": float(first_msg.angle_increment),
        "range_min": float(first_msg.range_min),
        "range_max": float(first_msg.range_max),
        "scan_rate_hz": scan_rate_hz,
    }
    (output_dir / "scan_meta.json").write_text(json.dumps(meta, indent=2) + "\n")

    gt_rows = [["timestamp", "x", "y", "yaw"]]
    ts_rows = [["frame", "timestamp"]]
    for i, (t, msg) in enumerate(scans):
        frame_dir = output_dir / f"{i:08d}"
        frame_dir.mkdir(exist_ok=True)
        ranges = []
        for r in msg.ranges:
            rv = float(r)
            if not math.isfinite(rv) or rv < meta["range_min"] or rv > meta["range_max"]:
                ranges.append("0.0")
            else:
                ranges.append(f"{rv:.6f}")
        (frame_dir / "scan.csv").write_text(",".join(ranges) + "\n")

        if gt:
            x, y, yaw = interpolate_gt(gt, t)
        else:
            x, y, yaw = 0.0, 0.0, 0.0
        gt_rows.append([f"{t:.6f}", f"{x:.6f}", f"{y:.6f}", f"{yaw:.6f}"])
        ts_rows.append([str(i), f"{t:.6f}"])

    with (output_dir / "gt.csv").open("w", newline="") as f:
        csv.writer(f).writerows(gt_rows)
    with (output_dir / "frame_timestamps.csv").open("w", newline="") as f:
        csv.writer(f).writerows(ts_rows)

    print(f"Exported {len(scans)} scans to {output_dir}")
    print(f"  scan_rate_hz={scan_rate_hz:.3f}, beams={len(scans[0][1].ranges)}")
    return 0


def validate_scan_tree(output_dir: Path) -> int:
    meta_path = output_dir / "scan_meta.json"
    if not meta_path.is_file():
        raise RuntimeError(f"Missing {meta_path}")
    meta = json.loads(meta_path.read_text())
    frames = sorted(
        p for p in output_dir.iterdir()
        if p.is_dir() and re.fullmatch(r"\d{8}", p.name)
    )
    if not frames:
        raise RuntimeError("No NNNNNNNN frame directories found")
    for frame in frames[:3]:
        scan_path = frame / "scan.csv"
        if not scan_path.is_file():
            raise RuntimeError(f"Missing {scan_path}")
        n_beams = len(scan_path.read_text().strip().split(","))
        if n_beams < 10:
            raise RuntimeError(f"Too few beams in {scan_path}: {n_beams}")
    gt_path = output_dir / "gt.csv"
    if gt_path.is_file():
        gt = load_gt_csv(gt_path)
        if len(gt) != len(frames):
            print(
                f"WARNING: gt.csv rows ({len(gt)}) != frame count ({len(frames)}); "
                "scan_dogfooding indexes GT by frame number."
            )
    print(f"OK: {len(frames)} frames, meta keys={list(meta.keys())}")
    return 0


def main() -> int:
    args = parse_args()
    output_dir = Path(args.output_dir)

    if args.inspect:
        if not args.ros1_bag:
            raise SystemExit("--inspect requires --ros1-bag")
        return inspect_ros1_bag(Path(args.ros1_bag))

    if args.validate_only:
        return validate_scan_tree(output_dir)

    gt: list[tuple[float, float, float, float]] = []
    if args.gt_csv:
        gt = load_gt_csv(Path(args.gt_csv))
    elif args.gt_tum:
        gt = load_gt_tum(Path(args.gt_tum))

    if args.ros1_bag:
        return export_ros1_laserscan(
            Path(args.ros1_bag),
            args.scan_topic,
            output_dir,
            gt,
            args.start_frame,
            args.max_frames,
            args.scan_rate_hz,
        )

    raise SystemExit(
        "Specify --ros1-bag to export, or --validate-only to check an existing tree."
    )


if __name__ == "__main__":
    raise SystemExit(main())
