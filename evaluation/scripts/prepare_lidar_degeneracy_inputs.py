#!/usr/bin/env python3

"""Download, inspect, and optionally extract NTNU LiDAR degeneracy bags.

The upstream release is a ROS1 bag dataset. The bags may contain Ouster packet
topics rather than ready-made PointCloud2 topics, so this script is intentionally
split into explicit stages:

1. download bags from Hugging Face,
2. inspect topic availability,
3. extract PointCloud2 topics only when they already exist.

If only packet topics are present, replay the bag through an Ouster driver first,
then run the extractor on the generated PointCloud2 bag.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import shutil
import subprocess
import sys
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_DIR = Path(__file__).resolve().parent
HF_BASE = "https://huggingface.co/datasets/ntnu-arl/lidar_degeneracy_datasets/resolve/main"


@dataclass(frozen=True)
class SequenceSpec:
    name: str
    bag_file: str
    environment: str
    failure_mode: str


SEQUENCES = {
    "tunnel": SequenceSpec(
        name="tunnel",
        bag_file="tunnel.bag",
        environment="Fyllingsdalen biking tunnel",
        failure_mode="geometric self-similarity in a long straight tunnel",
    ),
    "fog": SequenceSpec(
        name="fog",
        bag_file="fog.bag",
        environment="fog-filled NTNU university corridor",
        failure_mode="obscurants and degraded LiDAR returns",
    ),
}

KNOWN_TOPICS = {
    "lidar_packets": "/os_cloud_node/lidar_packets",
    "lidar_imu_packets": "/os_cloud_node/imu_packets",
    "lidar_metadata": "/os_cloud_node/metadata",
    "imu": "/vectornav_node/uncomp_imu",
    "radar": "/radar/cloud",
}

EXTRINSICS_IMU_FRAME = {
    "lidar": {
        "translation_xyz_m": [-0.00171, 0.02149, 0.0358],
        "orientation_xyzw": [0.000462, 0.0008483, 0.0028835, 0.9999954],
    },
    "radar": {
        "translation_xyz_m": [0.07771, 0.02141, -0.03631],
        "orientation_xyzw": [0.953717, 0.0, -0.3007058, 0.0],
    },
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sequence",
        choices=sorted(SEQUENCES),
        action="append",
        help="Sequence to handle. Repeat for both; default handles tunnel and fog.",
    )
    parser.add_argument(
        "--data-root",
        type=Path,
        default=REPO_ROOT / "data" / "lidar_degeneracy_datasets",
        help="Directory for downloaded bags and manifests.",
    )
    parser.add_argument(
        "--download",
        action="store_true",
        help="Download missing bags from Hugging Face.",
    )
    parser.add_argument(
        "--download-tool",
        choices=["auto", "parallel", "wget", "curl", "urllib"],
        default="auto",
        help="Downloader backend. auto prefers the parallel range downloader for large files.",
    )
    parser.add_argument(
        "--download-connections",
        type=int,
        default=6,
        help="Number of range requests for --download-tool parallel/auto.",
    )
    parser.add_argument(
        "--download-chunk-mb",
        type=int,
        default=64,
        help="Chunk size for --download-tool parallel/auto.",
    )
    parser.add_argument(
        "--inspect",
        action="store_true",
        help="Inspect ROS1 bag topics with rosbags.",
    )
    parser.add_argument(
        "--extract",
        action="store_true",
        help="Extract a PointCloud2 topic into dogfooding_results layout.",
    )
    parser.add_argument(
        "--pointcloud-topic",
        default="",
        help="PointCloud2 topic to extract. If omitted, the first detected PointCloud2 topic is used.",
    )
    parser.add_argument(
        "--imu-topic",
        default=KNOWN_TOPICS["imu"],
        help="IMU topic to extract when present.",
    )
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First LiDAR frame index to export.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=200,
        help="Maximum frames to export. Use -1 for all frames.",
    )
    parser.add_argument(
        "--time-mode",
        choices=["preserve", "index", "azimuth"],
        default="azimuth",
        help="Per-point time synthesis mode passed to extract_ros1_lidar_imu.py.",
    )
    parser.add_argument(
        "--output-root",
        type=Path,
        default=REPO_ROOT / "dogfooding_results",
        help="Root directory for extracted PCD sequences.",
    )
    parser.add_argument(
        "--manifest-only",
        action="store_true",
        help="Only write manifests; do not fail if bags are missing.",
    )
    return parser.parse_args()


def selected_specs(args: argparse.Namespace) -> list[SequenceSpec]:
    names = args.sequence or sorted(SEQUENCES)
    return [SEQUENCES[name] for name in names]


def select_download_tool(requested: str) -> str:
    if requested != "auto":
        if requested != "urllib" and shutil.which(requested) is None:
            if requested == "parallel":
                return requested
            raise RuntimeError(f"--download-tool {requested!r} requested but not found in PATH")
        return requested
    return "parallel"


def resolve_download(url: str) -> tuple[str, int, bool]:
    request = urllib.request.Request(url, headers={"Range": "bytes=0-0"})
    with urllib.request.urlopen(request, timeout=60) as response:
        final_url = response.geturl()
        content_range = response.headers.get("Content-Range", "")
        if "/" in content_range:
            return final_url, int(content_range.rsplit("/", 1)[1]), True
        content_length = response.headers.get("Content-Length")
        if content_length and content_length.isdigit():
            return final_url, int(content_length), False
    raise RuntimeError(f"Could not resolve download size for {url}")


def fetch_range(url: str, path: Path, start: int, end: int) -> None:
    expected_size = end - start + 1
    existing_size = path.stat().st_size if path.exists() else 0
    if existing_size == expected_size:
        return
    if existing_size > expected_size:
        path.unlink()
        existing_size = 0
    request_start = start + existing_size
    headers = {"Range": f"bytes={request_start}-{end}"}
    for attempt in range(1, 6):
        try:
            request = urllib.request.Request(url, headers=headers)
            with urllib.request.urlopen(request, timeout=120) as response, path.open("ab") as handle:
                if getattr(response, "status", None) != 206:
                    path.unlink(missing_ok=True)
                    raise RuntimeError(
                        f"Server ignored Range header for bytes {request_start}-{end}: "
                        f"HTTP {getattr(response, 'status', 'unknown')}"
                    )
                while True:
                    chunk = response.read(1024 * 1024)
                    if not chunk:
                        break
                    handle.write(chunk)
            if path.stat().st_size == expected_size:
                return
        except Exception:
            if attempt == 5:
                raise
    raise RuntimeError(f"Failed to download byte range {start}-{end}")


def download_file_parallel(url: str, dst: Path, *, connections: int, chunk_mb: int) -> None:
    final_url, total_size, supports_ranges = resolve_download(url)
    if not supports_ranges:
        raise RuntimeError(f"Server does not support ranged downloads: {url}")
    if dst.exists() and dst.stat().st_size == total_size:
        print(f"[download] exists: {dst}")
        return

    prefix_size = dst.stat().st_size if dst.exists() else 0
    if prefix_size > total_size:
        dst.unlink()
        prefix_size = 0

    chunk_size = max(1, chunk_mb) * 1024 * 1024
    parts_dir = dst.parent / f".{dst.name}.parts"
    parts_dir.mkdir(parents=True, exist_ok=True)
    ranges: list[tuple[int, int, Path]] = []
    for start in range(prefix_size, total_size, chunk_size):
        end = min(start + chunk_size - 1, total_size - 1)
        ranges.append((start, end, parts_dir / f"{start:016d}-{end:016d}.part"))

    print(
        f"[download] parallel ranges={len(ranges)} connections={connections} "
        f"resume_prefix={prefix_size / 1e6:.1f} MB total={total_size / 1e9:.2f} GB",
        flush=True,
    )
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, connections)) as executor:
        futures = [executor.submit(fetch_range, final_url, part, start, end) for start, end, part in ranges]
        for index, future in enumerate(concurrent.futures.as_completed(futures), start=1):
            future.result()
            if index == 1 or index == len(futures) or index % 4 == 0:
                downloaded = prefix_size + sum(part.stat().st_size for _, _, part in ranges if part.exists())
                print(f"[download] {dst.name}: {downloaded / 1e9:.2f} / {total_size / 1e9:.2f} GB", flush=True)

    assembling = dst.with_suffix(dst.suffix + ".assembling")
    with assembling.open("wb") as out:
        if prefix_size:
            with dst.open("rb") as prefix:
                shutil.copyfileobj(prefix, out, length=1024 * 1024)
        for start, end, part in sorted(ranges):
            expected_size = end - start + 1
            if part.stat().st_size != expected_size:
                raise RuntimeError(f"Part has wrong size: {part}")
            with part.open("rb") as handle:
                shutil.copyfileobj(handle, out, length=1024 * 1024)
    if assembling.stat().st_size != total_size:
        raise RuntimeError(f"Assembled file has wrong size: {assembling}")
    assembling.replace(dst)
    shutil.rmtree(parts_dir)
    print(f"[download] wrote: {dst}")


def download_file(url: str, dst: Path, *, tool: str, connections: int, chunk_mb: int) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    selected_tool = select_download_tool(tool)
    if selected_tool == "parallel":
        download_file_parallel(url, dst, connections=connections, chunk_mb=chunk_mb)
        return
    partial = dst.with_suffix(dst.suffix + ".part")
    if partial.exists() and not dst.exists() and selected_tool in {"wget", "curl"}:
        partial.rename(dst)
        print(f"[download] resumed partial as: {dst}")
    if dst.exists() and selected_tool == "urllib":
        print(f"[download] exists: {dst}")
        return

    if selected_tool == "wget":
        cmd = ["wget", "-c", "--progress=dot:giga", "-O", str(dst), url]
        print("[download] " + " ".join(cmd))
        subprocess.run(cmd, check=True)
        return
    if selected_tool == "curl":
        cmd = ["curl", "-L", "-C", "-", "-o", str(dst), url]
        print("[download] " + " ".join(cmd))
        subprocess.run(cmd, check=True)
        return

    print(f"[download] {url}")
    with urllib.request.urlopen(url) as response, partial.open("wb") as handle:
        total = response.headers.get("Content-Length")
        total_bytes = int(total) if total and total.isdigit() else 0
        downloaded = 0
        while True:
            chunk = response.read(1024 * 1024)
            if not chunk:
                break
            handle.write(chunk)
            downloaded += len(chunk)
            if total_bytes and downloaded % (128 * 1024 * 1024) < len(chunk):
                pct = 100.0 * downloaded / total_bytes
                print(f"[download] {dst.name}: {downloaded / 1e9:.2f} GB ({pct:.1f}%)")
    partial.rename(dst)
    print(f"[download] wrote: {dst}")


def inspect_ros1_bag(path: Path) -> dict[str, Any]:
    try:
        from rosbags.rosbag1 import Reader
    except ImportError as exc:
        raise RuntimeError(
            "rosbags is required for --inspect. Install with `pip install rosbags`."
        ) from exc

    topics: list[dict[str, Any]] = []
    with Reader(path) as reader:
        for conn in sorted(reader.connections, key=lambda c: c.topic):
            topics.append(
                {
                    "topic": conn.topic,
                    "msgtype": conn.msgtype,
                    "msgcount": int(getattr(conn, "msgcount", 0) or 0),
                }
            )
    pointcloud_topics = [
        item["topic"]
        for item in topics
        if item["msgtype"] in {"sensor_msgs/msg/PointCloud2", "sensor_msgs/PointCloud2"}
    ]
    lidar_pointcloud_topics = [
        topic
        for topic in pointcloud_topics
        if topic != KNOWN_TOPICS["radar"] and any(token in topic.lower() for token in ("lidar", "points", "cloud"))
    ]
    non_lidar_pointcloud_topics = [
        topic for topic in pointcloud_topics if topic not in set(lidar_pointcloud_topics)
    ]
    packet_topics = [
        item["topic"]
        for item in topics
        if "Packet" in item["msgtype"] or item["topic"].endswith("_packets")
    ]
    return {
        "bag_path": str(path),
        "topics": topics,
        "pointcloud_topics": pointcloud_topics,
        "lidar_pointcloud_topics": lidar_pointcloud_topics,
        "non_lidar_pointcloud_topics": non_lidar_pointcloud_topics,
        "packet_topics": packet_topics,
        "known_topics_present": {
            key: any(item["topic"] == topic for item in topics)
            for key, topic in KNOWN_TOPICS.items()
        },
    }


def write_manifest(
    *,
    path: Path,
    spec: SequenceSpec,
    bag_path: Path,
    inspection: dict[str, Any] | None,
    output_dir: Path | None,
    start_frame: int,
    max_frames: int,
) -> None:
    pointcloud_topics = [] if inspection is None else inspection["pointcloud_topics"]
    lidar_pointcloud_topics = [] if inspection is None else inspection.get("lidar_pointcloud_topics", pointcloud_topics)
    non_lidar_pointcloud_topics = [] if inspection is None else inspection.get("non_lidar_pointcloud_topics", [])
    packet_topics = [] if inspection is None else inspection["packet_topics"]
    manifest = {
        "schema_version": 1,
        "dataset": "ntnu_lidar_degeneracy_datasets",
        "upstream": {
            "github": "https://github.com/ntnu-arl/lidar_degeneracy_datasets",
            "huggingface": "https://huggingface.co/datasets/ntnu-arl/lidar_degeneracy_datasets",
            "bag_file": spec.bag_file,
        },
        "sequence": {
            "name": spec.name,
            "environment": spec.environment,
            "failure_mode": spec.failure_mode,
        },
        "bag_path": str(bag_path),
        "dogfooding_pcd_dir": str(output_dir) if output_dir is not None else None,
        "window": {
            "start_frame": start_frame,
            "max_frames": max_frames,
        },
        "topics": {
            "known": KNOWN_TOPICS,
            "pointcloud_detected": pointcloud_topics,
            "lidar_pointcloud_detected": lidar_pointcloud_topics,
            "non_lidar_pointcloud_detected": non_lidar_pointcloud_topics,
            "packet_detected": packet_topics,
        },
        "extrinsics_imu_frame": EXTRINSICS_IMU_FRAME,
        "benchmark_contract": {
            "ground_truth": "not provided by upstream README; use odometry diagnostics and degeneracy metrics first",
            "primary_metrics": [
                "point_count",
                "linearity",
                "planarity",
                "scattering",
                "degeneracy_score",
                "intensity_mean",
                "intensity_std",
            ],
            "next_methods": ["kiss_icp", "ct_icp", "keyframe_correction_replay"],
        },
        "conversion_status": conversion_status(lidar_pointcloud_topics, packet_topics),
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"[manifest] {path}")


def conversion_status(lidar_pointcloud_topics: list[str], packet_topics: list[str]) -> str:
    if lidar_pointcloud_topics:
        return "ready_for_pointcloud2_extract"
    if packet_topics:
        return "requires_ouster_packet_replay_before_extract"
    return "bag_missing_or_no_lidar_topic_detected"


def extract_pointcloud(
    *,
    bag_path: Path,
    output_dir: Path,
    pointcloud_topic: str,
    imu_topic: str,
    start_frame: int,
    max_frames: int,
    time_mode: str,
) -> None:
    cmd = [
        sys.executable,
        str(SCRIPT_DIR / "extract_ros1_lidar_imu.py"),
        "--pointcloud-bag",
        str(bag_path),
        "--pointcloud-topic",
        pointcloud_topic,
        "--imu-bag",
        str(bag_path),
        "--imu-topic",
        imu_topic,
        "--output-dir",
        str(output_dir),
        "--start-frame",
        str(start_frame),
        "--max-frames",
        str(max_frames),
        "--time-mode",
        time_mode,
    ]
    print("[extract] " + " ".join(cmd))
    subprocess.run(cmd, check=True)


def handle_sequence(args: argparse.Namespace, spec: SequenceSpec) -> None:
    data_root = args.data_root.resolve()
    bag_path = data_root / spec.bag_file
    manifest_path = data_root / f"{spec.name}_manifest.json"
    output_dir = args.output_root / f"lidar_degeneracy_{spec.name}_{args.max_frames}"

    if args.download:
        download_file(
            f"{HF_BASE}/{spec.bag_file}?download=true",
            bag_path,
            tool=args.download_tool,
            connections=args.download_connections,
            chunk_mb=args.download_chunk_mb,
        )

    inspection: dict[str, Any] | None = None
    if args.inspect and bag_path.is_file():
        inspection = inspect_ros1_bag(bag_path)
        inspect_path = data_root / f"{spec.name}_topics.json"
        inspect_path.write_text(json.dumps(inspection, indent=2) + "\n")
        print(f"[inspect] {inspect_path}")
        for topic in inspection["topics"]:
            print(f"  {topic['topic']} [{topic['msgtype']}] count={topic['msgcount']}")
    elif args.inspect and not args.manifest_only:
        raise FileNotFoundError(f"Bag not found: {bag_path}")

    write_manifest(
        path=manifest_path,
        spec=spec,
        bag_path=bag_path,
        inspection=inspection,
        output_dir=output_dir if args.extract else None,
        start_frame=args.start_frame,
        max_frames=args.max_frames,
    )

    if not args.extract:
        return
    if not bag_path.is_file():
        raise FileNotFoundError(f"Bag not found: {bag_path}")
    if inspection is None:
        inspection = inspect_ros1_bag(bag_path)
    pointcloud_topics = inspection.get("lidar_pointcloud_topics", inspection["pointcloud_topics"])
    pointcloud_topic = args.pointcloud_topic or (pointcloud_topics[0] if pointcloud_topics else "")
    if not pointcloud_topic:
        raise RuntimeError(
            f"{bag_path} does not expose a LiDAR PointCloud2 topic. "
            "Replay /os_cloud_node/lidar_packets through an Ouster driver first, "
            "then pass the generated PointCloud2 bag to extract_ros1_lidar_imu.py."
        )
    if pointcloud_topic in inspection.get("non_lidar_pointcloud_topics", []):
        raise RuntimeError(
            f"{pointcloud_topic} is not classified as a LiDAR PointCloud2 topic. "
            "Pass an explicit LiDAR pointcloud topic after Ouster packet replay."
        )
    extract_pointcloud(
        bag_path=bag_path,
        output_dir=output_dir,
        pointcloud_topic=pointcloud_topic,
        imu_topic=args.imu_topic,
        start_frame=args.start_frame,
        max_frames=args.max_frames,
        time_mode=args.time_mode,
    )


def main() -> int:
    args = parse_args()
    for spec in selected_specs(args):
        handle_sequence(args, spec)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
