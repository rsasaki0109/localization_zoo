#!/usr/bin/env python3

"""Generate synthetic landmark + reprojection CSVs for multimodal_dogfooding.

This script bootstraps known-landmark visual inputs from GT poses. It does not
read raw images. Instead it:

1. samples anchor poses along the GT trajectory,
2. places 3D landmarks in front of those anchor poses,
3. deduplicates them in world coordinates, and
4. projects the landmarks into every benchmark frame.

Outputs:
  - landmarks.csv
  - visual_observations.csv
"""

from __future__ import annotations

import argparse
import csv
import math
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Pose:
    timestamp: float
    x: float
    y: float
    z: float
    roll: float
    pitch: float
    yaw: float


@dataclass(frozen=True)
class Landmark:
    landmark_id: int
    x: float
    y: float
    z: float


def parse_float_list(csv_text: str) -> list[float]:
    return [float(item.strip()) for item in csv_text.split(",") if item.strip()]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate synthetic landmark observations from GT poses.",
    )
    parser.add_argument(
        "--sequence-dir",
        required=True,
        help="Dogfooding sequence directory. frame_timestamps.csv is read from here when present.",
    )
    parser.add_argument(
        "--gt-csv",
        required=True,
        help="Ground-truth CSV with timestamp,lidar_pose.x/y/z/roll/pitch/yaw columns.",
    )
    parser.add_argument(
        "--output-dir",
        default=None,
        help="Output directory for landmarks.csv and visual_observations.csv. Defaults to sequence-dir.",
    )
    parser.add_argument("--start-frame", type=int, default=0)
    parser.add_argument("--max-frames", type=int, default=-1)
    parser.add_argument(
        "--anchor-stride",
        type=int,
        default=8,
        help="Use every Nth frame pose as an anchor pose for synthetic landmarks.",
    )
    parser.add_argument(
        "--depths",
        default="10,16,22",
        help="Comma-separated forward distances in meters from each anchor pose.",
    )
    parser.add_argument(
        "--laterals",
        default="-6,-3,0,3,6",
        help="Comma-separated lateral offsets in meters from each anchor pose.",
    )
    parser.add_argument(
        "--heights",
        default="1.5,2.5",
        help="Comma-separated landmark heights in meters from each anchor pose.",
    )
    parser.add_argument(
        "--dedup-resolution",
        type=float,
        default=0.75,
        help="World-space voxel size for landmark deduplication.",
    )
    parser.add_argument("--camera-fx", type=float, default=160.0)
    parser.add_argument("--camera-fy", type=float, default=160.0)
    parser.add_argument("--camera-cx", type=float, default=320.0)
    parser.add_argument("--camera-cy", type=float, default=240.0)
    parser.add_argument("--camera-width", type=int, default=640)
    parser.add_argument("--camera-height", type=int, default=480)
    return parser.parse_args()


def trim_csv_token(token: str) -> str:
    return token.strip(" \t\r\n")


def load_gt_poses(path: Path) -> list[Pose]:
    with path.open() as handle:
        reader = csv.DictReader(handle)
        poses: list[Pose] = []
        for row in reader:
            poses.append(
                Pose(
                    timestamp=float(row["timestamp"]),
                    x=float(row["lidar_pose.x"]),
                    y=float(row["lidar_pose.y"]),
                    z=float(row["lidar_pose.z"]),
                    roll=float(row["lidar_pose.roll"]),
                    pitch=float(row["lidar_pose.pitch"]),
                    yaw=float(row["lidar_pose.yaw"]),
                )
            )
    return poses


def list_pcd_frames(sequence_dir: Path) -> int:
    count = 0
    for entry in sequence_dir.iterdir():
        if entry.is_dir() and (entry / "cloud.pcd").exists():
            count += 1
    return count


def load_frame_timestamps(sequence_dir: Path) -> list[float]:
    csv_path = sequence_dir / "frame_timestamps.csv"
    if not csv_path.exists():
        return []

    with csv_path.open() as handle:
        reader = csv.reader(handle)
        try:
            header = next(reader)
        except StopIteration:
            return []
        header = [trim_csv_token(item) for item in header]
        try:
            timestamp_idx = header.index("timestamp")
        except ValueError:
            timestamp_idx = 1 if len(header) > 1 else 0

        timestamps: list[float] = []
        for row in reader:
            if timestamp_idx >= len(row):
                continue
            timestamps.append(float(row[timestamp_idx]))
        return timestamps


def rotation_matrix(roll: float, pitch: float, yaw: float) -> tuple[tuple[float, ...], ...]:
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    rx = ((1.0, 0.0, 0.0), (0.0, cr, -sr), (0.0, sr, cr))
    ry = ((cp, 0.0, sp), (0.0, 1.0, 0.0), (-sp, 0.0, cp))
    rz = ((cy, -sy, 0.0), (sy, cy, 0.0), (0.0, 0.0, 1.0))

    def mm(lhs: tuple[tuple[float, ...], ...], rhs: tuple[tuple[float, ...], ...]) -> tuple[tuple[float, ...], ...]:
        return tuple(
            tuple(sum(lhs[i][k] * rhs[k][j] for k in range(3)) for j in range(3))
            for i in range(3)
        )

    return mm(mm(rz, ry), rx)


def mat_vec_mul(matrix: tuple[tuple[float, ...], ...], vector: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(sum(matrix[i][k] * vector[k] for k in range(3)) for i in range(3))


def transpose(matrix: tuple[tuple[float, ...], ...]) -> tuple[tuple[float, ...], ...]:
    return tuple(tuple(matrix[j][i] for j in range(3)) for i in range(3))


def nearest_pose_index(timestamps: list[float], query_timestamp: float) -> int:
    if not timestamps:
        return 0
    lo, hi = 0, len(timestamps)
    while lo < hi:
        mid = (lo + hi) // 2
        if timestamps[mid] < query_timestamp:
            lo = mid + 1
        else:
            hi = mid
    if lo == len(timestamps):
        return len(timestamps) - 1
    if lo == 0:
        return 0
    prev_idx = lo - 1
    return lo if abs(timestamps[lo] - query_timestamp) < abs(timestamps[prev_idx] - query_timestamp) else prev_idx


def align_frame_poses(
    gt_poses: list[Pose], sequence_dir: Path, start_frame: int, max_frames: int
) -> list[Pose]:
    pcd_frame_count = list_pcd_frames(sequence_dir)
    frame_timestamps = load_frame_timestamps(sequence_dir)

    if frame_timestamps:
        gt_timestamps = [pose.timestamp for pose in gt_poses]
        aligned = [gt_poses[nearest_pose_index(gt_timestamps, ts)] for ts in frame_timestamps]
    elif pcd_frame_count > 0:
        aligned = []
        for frame_idx in range(pcd_frame_count):
            gt_idx = min(
                int(frame_idx / max(pcd_frame_count, 1) * len(gt_poses)),
                len(gt_poses) - 1,
            )
            aligned.append(gt_poses[gt_idx])
    else:
        aligned = list(gt_poses)

    if start_frame < 0:
        start_frame = 0
    end_frame = len(aligned)
    if max_frames >= 0:
        end_frame = min(end_frame, start_frame + max_frames)
    return aligned[start_frame:end_frame]


def dedup_key(x: float, y: float, z: float, resolution: float) -> tuple[int, int, int]:
    return (
        int(round(x / resolution)),
        int(round(y / resolution)),
        int(round(z / resolution)),
    )


def synthesize_landmarks(
    frame_poses: list[Pose],
    anchor_stride: int,
    depths: list[float],
    laterals: list[float],
    heights: list[float],
    dedup_resolution: float,
) -> list[Landmark]:
    if anchor_stride <= 0:
        anchor_stride = 1

    landmarks_by_voxel: dict[tuple[int, int, int], tuple[float, float, float]] = {}
    for pose in frame_poses[::anchor_stride]:
        rotation = rotation_matrix(pose.roll, pose.pitch, pose.yaw)
        translation = (pose.x, pose.y, pose.z)
        for depth in depths:
            for lateral in laterals:
                for height in heights:
                    local_point = (depth, lateral, height)
                    world_delta = mat_vec_mul(rotation, local_point)
                    world_point = (
                        translation[0] + world_delta[0],
                        translation[1] + world_delta[1],
                        translation[2] + world_delta[2],
                    )
                    landmarks_by_voxel.setdefault(
                        dedup_key(*world_point, dedup_resolution), world_point
                    )

    ordered_points = sorted(landmarks_by_voxel.values())
    return [
        Landmark(landmark_id=index, x=point[0], y=point[1], z=point[2])
        for index, point in enumerate(ordered_points)
    ]


def project_landmarks(
    frame_poses: list[Pose],
    landmarks: list[Landmark],
    fx: float,
    fy: float,
    cx: float,
    cy: float,
    width: int,
    height: int,
) -> list[tuple[int, float, int, float, float]]:
    rows: list[tuple[int, float, int, float, float]] = []
    for frame_index, pose in enumerate(frame_poses):
        rotation = rotation_matrix(pose.roll, pose.pitch, pose.yaw)
        rotation_t = transpose(rotation)
        translation = (pose.x, pose.y, pose.z)
        for landmark in landmarks:
            delta = (
                landmark.x - translation[0],
                landmark.y - translation[1],
                landmark.z - translation[2],
            )
            point_camera = mat_vec_mul(rotation_t, delta)
            if point_camera[0] <= 1e-6:
                continue
            u = fx * point_camera[1] / point_camera[0] + cx
            v = fy * point_camera[2] / point_camera[0] + cy
            if 0.0 <= u < width and 0.0 <= v < height:
                rows.append((frame_index, pose.timestamp, landmark.landmark_id, u, v))
    return rows


def write_landmarks(path: Path, landmarks: list[Landmark]) -> None:
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["landmark_id", "x", "y", "z"])
        for landmark in landmarks:
            writer.writerow(
                [
                    landmark.landmark_id,
                    f"{landmark.x:.9f}",
                    f"{landmark.y:.9f}",
                    f"{landmark.z:.9f}",
                ]
            )


def write_observations(
    path: Path, rows: list[tuple[int, float, int, float, float]]
) -> None:
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["frame_index", "timestamp", "landmark_id", "u", "v"])
        for frame_index, timestamp, landmark_id, u, v in rows:
            writer.writerow(
                [
                    frame_index,
                    f"{timestamp:.9f}",
                    landmark_id,
                    f"{u:.6f}",
                    f"{v:.6f}",
                ]
            )


def main() -> int:
    args = parse_args()
    sequence_dir = Path(args.sequence_dir)
    output_dir = Path(args.output_dir) if args.output_dir else sequence_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    gt_poses = load_gt_poses(Path(args.gt_csv))
    if not gt_poses:
        raise RuntimeError(f"No GT poses loaded from {args.gt_csv}")

    frame_poses = align_frame_poses(
        gt_poses,
        sequence_dir=sequence_dir,
        start_frame=args.start_frame,
        max_frames=args.max_frames,
    )
    if not frame_poses:
        raise RuntimeError("No aligned frame poses available for projection.")

    landmarks = synthesize_landmarks(
        frame_poses=frame_poses,
        anchor_stride=args.anchor_stride,
        depths=parse_float_list(args.depths),
        laterals=parse_float_list(args.laterals),
        heights=parse_float_list(args.heights),
        dedup_resolution=args.dedup_resolution,
    )
    if not landmarks:
        raise RuntimeError("No landmarks synthesized.")

    observations = project_landmarks(
        frame_poses=frame_poses,
        landmarks=landmarks,
        fx=args.camera_fx,
        fy=args.camera_fy,
        cx=args.camera_cx,
        cy=args.camera_cy,
        width=args.camera_width,
        height=args.camera_height,
    )
    if not observations:
        raise RuntimeError("No projected observations generated.")

    landmarks_path = output_dir / "landmarks.csv"
    observations_path = output_dir / "visual_observations.csv"
    write_landmarks(landmarks_path, landmarks)
    write_observations(observations_path, observations)

    counts_by_frame = [0 for _ in frame_poses]
    for frame_index, _, _, _, _ in observations:
        if 0 <= frame_index < len(counts_by_frame):
            counts_by_frame[frame_index] += 1

    print(f"[done] wrote {len(landmarks)} landmarks to {landmarks_path}")
    print(f"[done] wrote {len(observations)} observations to {observations_path}")
    print(
        "[stats] "
        f"frames={len(frame_poses)} "
        f"min_obs_per_frame={min(counts_by_frame)} "
        f"max_obs_per_frame={max(counts_by_frame)} "
        f"mean_obs_per_frame={sum(counts_by_frame) / len(counts_by_frame):.1f}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
