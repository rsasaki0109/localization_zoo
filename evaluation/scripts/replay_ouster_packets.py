#!/usr/bin/env python3

"""Replay NTNU Ouster packet bags into a PointCloud2 ROS1 bag.

The LiDAR degeneracy dataset stores OS0-128 LiDAR as ouster_ros PacketMsg
topics. Localization Zoo needs PointCloud2 PCD inputs, so this helper wraps the
ROS1 ouster_ros replay workflow and records the decoded `/ouster/points` topic
into a new bag that can be consumed by extract_ros1_lidar_imu.py.
"""

from __future__ import annotations

import argparse
import json
import shlex
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DATA_ROOT = REPO_ROOT / "data" / "lidar_degeneracy_datasets"
SCRIPT_DIR = Path(__file__).resolve().parent
SEQUENCE_BAGS = {
    "fog": "fog.bag",
    "tunnel": "tunnel.bag",
}
DEFAULT_LIDAR_PACKET_TOPIC = "/os_cloud_node/lidar_packets"
DEFAULT_IMU_TOPIC = "/vectornav_node/uncomp_imu"
DEFAULT_POINTS_TOPIC = "/ouster/points"
DEFAULT_OUSTER_IMU_TOPIC = "/ouster/imu"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sequence", choices=sorted(SEQUENCE_BAGS), default="fog")
    parser.add_argument("--data-root", type=Path, default=DATA_ROOT)
    parser.add_argument("--bag", type=Path, help="Input packet ROS1 bag. Defaults from --sequence.")
    parser.add_argument(
        "--output-bag",
        type=Path,
        help="Converted output bag. Default: data/lidar_degeneracy_datasets/<sequence>_ouster_points.bag",
    )
    parser.add_argument(
        "--points-topic",
        default=DEFAULT_POINTS_TOPIC,
        help="Decoded PointCloud2 topic expected from ouster_ros replay.",
    )
    parser.add_argument(
        "--record-topic",
        action="append",
        default=[],
        help="Extra topic to record in the converted bag. Repeatable.",
    )
    parser.add_argument(
        "--lidar-packet-topic",
        default=DEFAULT_LIDAR_PACKET_TOPIC,
        help="Packet topic that must exist in the input bag.",
    )
    parser.add_argument(
        "--imu-topic",
        default=DEFAULT_IMU_TOPIC,
        help="Original dataset IMU topic used by the follow-up extractor command.",
    )
    parser.add_argument(
        "--ros-setup",
        action="append",
        default=[],
        help="setup.bash to source before running, e.g. /opt/ros/noetic/setup.bash. Repeatable.",
    )
    parser.add_argument(
        "--ouster-workspace",
        type=Path,
        help="Catkin workspace containing ntnu-arl/ouster-ros. Sources devel/setup.bash.",
    )
    parser.add_argument(
        "--launch-arg",
        action="append",
        default=[],
        help="Extra roslaunch arg for ouster_ros replay.launch, e.g. point_type:=xyzi. Repeatable.",
    )
    parser.add_argument(
        "--record-buffsize",
        type=int,
        default=1024,
        help="rosbag record --buffsize value in MB.",
    )
    parser.add_argument(
        "--startup-sleep",
        type=float,
        default=3.0,
        help="Seconds to wait after starting roscore and recorder.",
    )
    parser.add_argument(
        "--script-out",
        type=Path,
        help="Write the generated shell script to this path.",
    )
    parser.add_argument(
        "--run",
        action="store_true",
        help="Execute the generated script. Without this flag, only validate and print commands.",
    )
    parser.add_argument(
        "--skip-inspect",
        action="store_true",
        help="Skip rosbags inspection of the input packet bag.",
    )
    return parser.parse_args()


def shell_join(items: list[str]) -> str:
    return " ".join(shlex.quote(item) for item in items)


def inspect_ros1_bag(path: Path) -> dict[str, Any]:
    try:
        from rosbags.rosbag1 import Reader
    except ImportError as exc:
        raise RuntimeError("rosbags is required for packet-bag inspection. Install with `pip install rosbags`.") from exc

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
    return {"bag_path": str(path), "topics": topics}


def require_topic(inspection: dict[str, Any], topic: str) -> None:
    present = {item["topic"] for item in inspection["topics"]}
    if topic not in present:
        raise RuntimeError(f"Required topic missing from input bag: {topic}")


def default_output_bag(args: argparse.Namespace) -> Path:
    return args.data_root.expanduser() / f"{args.sequence}_ouster_points.bag"


def input_bag(args: argparse.Namespace) -> Path:
    return args.bag.expanduser() if args.bag else args.data_root.expanduser() / SEQUENCE_BAGS[args.sequence]


def setup_paths(args: argparse.Namespace) -> list[Path]:
    paths = [Path(item).expanduser() for item in args.ros_setup]
    if args.ouster_workspace is not None:
        paths.append(args.ouster_workspace.expanduser() / "devel" / "setup.bash")
    return paths


def generated_script(args: argparse.Namespace, bag_path: Path, output_bag: Path) -> str:
    setup_lines = [f"source {shlex.quote(str(path))}" for path in setup_paths(args)]
    record_topics = [args.points_topic, DEFAULT_OUSTER_IMU_TOPIC, *args.record_topic]
    record_topics = list(dict.fromkeys(record_topics))
    launch_args = ["ouster_ros", "replay.launch", f"bag_file:={bag_path}", *args.launch_arg]
    extractor_cmd = [
        sys.executable,
        str(SCRIPT_DIR / "extract_ros1_lidar_imu.py"),
        "--pointcloud-bag",
        str(output_bag),
        "--pointcloud-topic",
        args.points_topic,
        "--imu-bag",
        str(bag_path),
        "--imu-topic",
        args.imu_topic,
        "--output-dir",
        str(REPO_ROOT / "dogfooding_results" / f"lidar_degeneracy_{args.sequence}_200"),
        "--max-frames",
        "200",
        "--time-mode",
        "azimuth",
    ]

    return "\n".join(
        [
            "#!/usr/bin/env bash",
            "set -euo pipefail",
            *setup_lines,
            f"mkdir -p {shlex.quote(str(output_bag.parent))}",
            f"rm -f {shlex.quote(str(output_bag))} {shlex.quote(str(output_bag))}.active",
            "roscore >/tmp/localization_zoo_ouster_roscore.log 2>&1 &",
            "ROSCORE_PID=$!",
            "RECORDER_PID=",
            "cleanup() {",
            "  if [[ -n \"${RECORDER_PID}\" ]]; then kill \"${RECORDER_PID}\" >/dev/null 2>&1 || true; fi",
            "  kill \"${ROSCORE_PID}\" >/dev/null 2>&1 || true",
            "}",
            "trap cleanup EXIT",
            f"sleep {args.startup_sleep}",
            "rosparam set use_sim_time true",
            shell_join(
                [
                    "rosbag",
                    "record",
                    f"--buffsize={args.record_buffsize}",
                    "-O",
                    str(output_bag),
                    *record_topics,
                ]
            )
            + " &",
            "RECORDER_PID=$!",
            f"sleep {args.startup_sleep}",
            shell_join(["roslaunch", *[str(item) for item in launch_args]]),
            f"sleep {args.startup_sleep}",
            "kill \"${RECORDER_PID}\" >/dev/null 2>&1 || true",
            "wait \"${RECORDER_PID}\" >/dev/null 2>&1 || true",
            "RECORDER_PID=",
            shell_join(["rosbag", "info", str(output_bag)]),
            "",
            "# Follow-up extraction command:",
            "# " + shell_join(extractor_cmd),
            "",
        ]
    )


def validate_runtime(args: argparse.Namespace) -> list[str]:
    missing: list[str] = []
    for path in setup_paths(args):
        if not path.is_file():
            missing.append(str(path))
    if not setup_paths(args):
        for command in ("roscore", "rosbag", "roslaunch", "rosparam"):
            if shutil.which(command) is None:
                missing.append(command)
    return missing


def main() -> int:
    args = parse_args()
    bag_path = input_bag(args).resolve()
    output_bag = (args.output_bag.expanduser() if args.output_bag else default_output_bag(args)).resolve()

    if not bag_path.is_file():
        raise FileNotFoundError(f"Input bag not found: {bag_path}")
    if not args.skip_inspect:
        inspection = inspect_ros1_bag(bag_path)
        require_topic(inspection, args.lidar_packet_topic)
        print(
            json.dumps(
                {
                    "bag_path": str(bag_path),
                    "required_packet_topic": args.lidar_packet_topic,
                    "topics": inspection["topics"],
                },
                indent=2,
            )
        )

    script = generated_script(args, bag_path, output_bag)
    script_path = args.script_out or args.data_root / f"{args.sequence}_ouster_replay.sh"
    script_path.parent.mkdir(parents=True, exist_ok=True)
    script_path.write_text(script)
    script_path.chmod(0o755)
    print(f"[script] {script_path}")
    print(f"[output_bag] {output_bag}")

    missing = validate_runtime(args)
    if missing:
        print("[runtime] missing: " + ", ".join(missing))
        print("[runtime] install/source ROS1 Noetic and ntnu-arl/ouster-ros dev/sensor_sync_replay, then run:")
        print(f"  {script_path}")
        return 2 if args.run else 0
    if not args.run:
        print("[runtime] ready. Re-run with --run to execute conversion.")
        return 0

    subprocess.run([str(script_path)], check=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
