#!/usr/bin/env python3

"""Generate real visual observations for multimodal_dogfooding from KITTI Raw.

Pipeline:
  1. load a frame-aligned image sequence from a KITTI Raw *_sync drive,
  2. track Shi-Tomasi corners with KLT optical flow,
  3. triangulate persistent tracks with GT poses and a pinhole camera model,
  4. export landmarks.csv + visual_observations.csv.

This uses real 2D image tracks. Landmark 3D positions are GT-backed via
monocular triangulation, so this is stronger than fully synthetic reprojection
but still not a full visual-SLAM frontend benchmark.
"""

from __future__ import annotations

import argparse
import csv
import math
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

import cv2
import numpy as np


DEFAULT_KITTI_RAW_ROOT = Path("/tmp/kitti_real")
KITTI_COLOR_02_INTRINSICS = {
    "fx": 721.5377,
    "fy": 721.5377,
    "cx": 609.5593,
    "cy": 172.8540,
}
REPO_CAMERA_TO_STANDARD = np.array(
    (
        (0.0, 1.0, 0.0),
        (0.0, 0.0, 1.0),
        (1.0, 0.0, 0.0),
    ),
    dtype=np.float64,
)


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
class FrameData:
    frame_index: int
    timestamp: float
    image_path: Path
    pose: Pose


@dataclass
class Track:
    track_id: int
    observations: list[tuple[int, float, float]] = field(default_factory=list)
    last_point: tuple[float, float] = (0.0, 0.0)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate KITTI Raw visual observations for multimodal_dogfooding.",
    )
    parser.add_argument(
        "--sequence-dir",
        required=True,
        help="Benchmark sequence dir with frame_timestamps.csv.",
    )
    parser.add_argument(
        "--gt-csv",
        required=True,
        help="GT CSV matched to the benchmark sequence.",
    )
    parser.add_argument(
        "--drive-dir",
        default=None,
        help="KITTI Raw *_sync drive dir. If omitted, infer from sequence dir name under --kitti-root.",
    )
    parser.add_argument(
        "--kitti-root",
        default=str(DEFAULT_KITTI_RAW_ROOT),
        help="KITTI Raw root used when --drive-dir is omitted.",
    )
    parser.add_argument(
        "--camera-dir",
        default="image_02",
        help="KITTI camera folder under drive dir, e.g. image_02.",
    )
    parser.add_argument(
        "--output-dir",
        default=None,
        help="Output dir for landmarks.csv, visual_observations.csv, camera_args.txt. Defaults to sequence dir.",
    )
    parser.add_argument("--start-frame", type=int, default=0)
    parser.add_argument("--max-frames", type=int, default=-1)
    parser.add_argument("--max-active-tracks", type=int, default=1200)
    parser.add_argument("--detect-stride", type=int, default=3)
    parser.add_argument("--quality-level", type=float, default=0.01)
    parser.add_argument("--min-feature-distance", type=int, default=12)
    parser.add_argument("--lk-window-size", type=int, default=21)
    parser.add_argument("--lk-max-level", type=int, default=3)
    parser.add_argument("--max-forward-backward-error", type=float, default=1.5)
    parser.add_argument("--min-track-length", type=int, default=4)
    parser.add_argument("--min-baseline-m", type=float, default=0.75)
    parser.add_argument("--max-reprojection-error-px", type=float, default=2.5)
    parser.add_argument("--camera-fx", type=float, default=None)
    parser.add_argument("--camera-fy", type=float, default=None)
    parser.add_argument("--camera-cx", type=float, default=None)
    parser.add_argument("--camera-cy", type=float, default=None)
    parser.add_argument("--camera-width", type=int, default=None)
    parser.add_argument("--camera-height", type=int, default=None)
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


def load_frame_timestamps(sequence_dir: Path) -> list[float]:
    csv_path = sequence_dir / "frame_timestamps.csv"
    with csv_path.open() as handle:
        reader = csv.reader(handle)
        header = [trim_csv_token(item) for item in next(reader)]
        try:
            timestamp_idx = header.index("timestamp")
        except ValueError:
            timestamp_idx = 1 if len(header) > 1 else 0
        timestamps: list[float] = []
        for row in reader:
            if timestamp_idx < len(row):
                timestamps.append(float(row[timestamp_idx]))
    return timestamps


def rotation_matrix(roll: float, pitch: float, yaw: float) -> np.ndarray:
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    rx = np.array(((1.0, 0.0, 0.0), (0.0, cr, -sr), (0.0, sr, cr)))
    ry = np.array(((cp, 0.0, sp), (0.0, 1.0, 0.0), (-sp, 0.0, cp)))
    rz = np.array(((cy, -sy, 0.0), (sy, cy, 0.0), (0.0, 0.0, 1.0)))
    return rz @ ry @ rx


def nearest_pose_index(gt_timestamps: list[float], query_timestamp: float) -> int:
    lo = 0
    hi = len(gt_timestamps)
    while lo < hi:
        mid = (lo + hi) // 2
        if gt_timestamps[mid] < query_timestamp:
            lo = mid + 1
        else:
            hi = mid
    if lo == len(gt_timestamps):
        return len(gt_timestamps) - 1
    if lo == 0:
        return 0
    prev_idx = lo - 1
    return lo if abs(gt_timestamps[lo] - query_timestamp) < abs(gt_timestamps[prev_idx] - query_timestamp) else prev_idx


def infer_drive_dir(sequence_dir: Path, kitti_root: Path) -> Path:
    match = re.search(r"kitti_raw_(\d{4})_", sequence_dir.name)
    if not match:
        raise RuntimeError(
            f"Could not infer KITTI drive id from sequence dir name: {sequence_dir.name}"
        )
    drive_id = match.group(1)
    candidates = sorted(kitti_root.glob(f"**/*drive_{drive_id}_sync"))
    if not candidates:
        raise RuntimeError(f"No KITTI Raw drive_*_{drive_id}_sync found under {kitti_root}")
    return candidates[0]


def build_frame_data(
    *,
    sequence_dir: Path,
    gt_csv: Path,
    drive_dir: Path,
    camera_dir: str,
    start_frame: int,
    max_frames: int,
) -> list[FrameData]:
    frame_timestamps = load_frame_timestamps(sequence_dir)
    gt_poses = load_gt_poses(gt_csv)
    gt_timestamps = [pose.timestamp for pose in gt_poses]

    if start_frame < 0:
        start_frame = 0
    end_frame = len(frame_timestamps)
    if max_frames >= 0:
        end_frame = min(end_frame, start_frame + max_frames)

    image_dir = drive_dir / camera_dir / "data"
    frames: list[FrameData] = []
    for local_index, timestamp in enumerate(frame_timestamps[start_frame:end_frame]):
        global_index = int(round(timestamp))
        image_path = image_dir / f"{global_index:010d}.png"
        if not image_path.exists():
            raise FileNotFoundError(f"Image frame not found: {image_path}")
        pose_idx = nearest_pose_index(gt_timestamps, timestamp)
        frames.append(
            FrameData(
                frame_index=local_index,
                timestamp=timestamp,
                image_path=image_path,
                pose=gt_poses[pose_idx],
            )
        )
    return frames


def camera_matrix_from_args(args: argparse.Namespace, image_shape: tuple[int, int]) -> tuple[np.ndarray, dict[str, float | int]]:
    height, width = image_shape
    fx = args.camera_fx if args.camera_fx is not None else KITTI_COLOR_02_INTRINSICS["fx"]
    fy = args.camera_fy if args.camera_fy is not None else KITTI_COLOR_02_INTRINSICS["fy"]
    cx = args.camera_cx if args.camera_cx is not None else KITTI_COLOR_02_INTRINSICS["cx"]
    cy = args.camera_cy if args.camera_cy is not None else KITTI_COLOR_02_INTRINSICS["cy"]
    width = args.camera_width if args.camera_width is not None else width
    height = args.camera_height if args.camera_height is not None else height
    camera = np.array(
        (
            (fx, 0.0, cx),
            (0.0, fy, cy),
            (0.0, 0.0, 1.0),
        ),
        dtype=np.float64,
    )
    return camera, {
        "fx": fx,
        "fy": fy,
        "cx": cx,
        "cy": cy,
        "width": width,
        "height": height,
    }


def load_grayscale(path: Path) -> np.ndarray:
    image = cv2.imread(str(path), cv2.IMREAD_GRAYSCALE)
    if image is None:
        raise RuntimeError(f"Failed to read image: {path}")
    return image


def create_detection_mask(
    image_shape: tuple[int, int],
    active_points: Iterable[tuple[float, float]],
    radius: int,
) -> np.ndarray:
    height, width = image_shape
    mask = np.full((height, width), 255, dtype=np.uint8)
    border = max(radius, 8)
    mask[:border, :] = 0
    mask[-border:, :] = 0
    mask[:, :border] = 0
    mask[:, -border:] = 0
    for x, y in active_points:
        cv2.circle(mask, (int(round(x)), int(round(y))), radius, 0, -1)
    return mask


def add_new_tracks(
    *,
    image: np.ndarray,
    frame_index: int,
    active_tracks: list[Track],
    next_track_id: int,
    max_active_tracks: int,
    quality_level: float,
    min_feature_distance: int,
) -> tuple[list[Track], int]:
    needed = max_active_tracks - len(active_tracks)
    if needed <= 0:
        return active_tracks, next_track_id

    mask = create_detection_mask(
        image.shape,
        (track.last_point for track in active_tracks),
        radius=min_feature_distance,
    )
    features = cv2.goodFeaturesToTrack(
        image,
        maxCorners=needed,
        qualityLevel=quality_level,
        minDistance=float(min_feature_distance),
        blockSize=7,
        mask=mask,
    )
    if features is None:
        return active_tracks, next_track_id

    for feature in features.reshape(-1, 2):
        track = Track(track_id=next_track_id)
        track.last_point = (float(feature[0]), float(feature[1]))
        track.observations.append((frame_index, float(feature[0]), float(feature[1])))
        active_tracks.append(track)
        next_track_id += 1
    return active_tracks, next_track_id


def track_features(
    frames: list[FrameData],
    *,
    max_active_tracks: int,
    detect_stride: int,
    quality_level: float,
    min_feature_distance: int,
    lk_window_size: int,
    lk_max_level: int,
    max_forward_backward_error: float,
) -> list[Track]:
    if not frames:
        return []

    previous_image = load_grayscale(frames[0].image_path)
    active_tracks: list[Track] = []
    completed_tracks: list[Track] = []
    next_track_id = 0
    active_tracks, next_track_id = add_new_tracks(
        image=previous_image,
        frame_index=frames[0].frame_index,
        active_tracks=active_tracks,
        next_track_id=next_track_id,
        max_active_tracks=max_active_tracks,
        quality_level=quality_level,
        min_feature_distance=min_feature_distance,
    )

    lk_criteria = (
        cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT,
        30,
        0.01,
    )

    for i in range(1, len(frames)):
        current_image = load_grayscale(frames[i].image_path)

        if active_tracks:
            previous_points = np.array(
                [track.last_point for track in active_tracks],
                dtype=np.float32,
            ).reshape(-1, 1, 2)
            current_points, status, _ = cv2.calcOpticalFlowPyrLK(
                previous_image,
                current_image,
                previous_points,
                None,
                winSize=(lk_window_size, lk_window_size),
                maxLevel=lk_max_level,
                criteria=lk_criteria,
            )
            back_points, back_status, _ = cv2.calcOpticalFlowPyrLK(
                current_image,
                previous_image,
                current_points,
                None,
                winSize=(lk_window_size, lk_window_size),
                maxLevel=lk_max_level,
                criteria=lk_criteria,
            )

            survivors: list[Track] = []
            width = current_image.shape[1]
            height = current_image.shape[0]
            for track, point_prev, point_curr, point_back, ok, ok_back in zip(
                active_tracks,
                previous_points.reshape(-1, 2),
                current_points.reshape(-1, 2),
                back_points.reshape(-1, 2),
                status.reshape(-1),
                back_status.reshape(-1),
                strict=True,
            ):
                if not ok or not ok_back:
                    completed_tracks.append(track)
                    continue
                fb_error = float(np.linalg.norm(point_prev - point_back))
                x, y = float(point_curr[0]), float(point_curr[1])
                if fb_error > max_forward_backward_error:
                    completed_tracks.append(track)
                    continue
                if x < 4.0 or x >= width - 4.0 or y < 4.0 or y >= height - 4.0:
                    completed_tracks.append(track)
                    continue

                track.last_point = (x, y)
                track.observations.append((frames[i].frame_index, x, y))
                survivors.append(track)
            active_tracks = survivors

        if i % max(detect_stride, 1) == 0 or len(active_tracks) < max_active_tracks // 2:
            active_tracks, next_track_id = add_new_tracks(
                image=current_image,
                frame_index=frames[i].frame_index,
                active_tracks=active_tracks,
                next_track_id=next_track_id,
                max_active_tracks=max_active_tracks,
                quality_level=quality_level,
                min_feature_distance=min_feature_distance,
            )

        previous_image = current_image

    completed_tracks.extend(active_tracks)
    return completed_tracks


def pose_projection_matrix(
    pose: Pose, camera_matrix: np.ndarray
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    rotation_wc = rotation_matrix(pose.roll, pose.pitch, pose.yaw)
    translation_wc = np.array((pose.x, pose.y, pose.z), dtype=np.float64)
    rotation_cw_repo = rotation_wc.T
    translation_cw_repo = -rotation_cw_repo @ translation_wc
    rotation_cw_standard = REPO_CAMERA_TO_STANDARD @ rotation_cw_repo
    translation_cw_standard = REPO_CAMERA_TO_STANDARD @ translation_cw_repo
    projection = camera_matrix @ np.hstack(
        (rotation_cw_standard, translation_cw_standard.reshape(3, 1))
    )
    return projection, rotation_cw_standard, translation_cw_standard


def triangulate_track(
    track: Track,
    frames_by_index: dict[int, FrameData],
    camera_matrix: np.ndarray,
    min_baseline_m: float,
    max_reprojection_error_px: float,
) -> np.ndarray | None:
    if len(track.observations) < 2:
        return None

    camera_centers = []
    rows = []
    projection_matrices = {}
    for frame_index, u, v in track.observations:
        frame = frames_by_index[frame_index]
        projection, rotation_cw, translation_cw = pose_projection_matrix(frame.pose, camera_matrix)
        projection_matrices[frame_index] = (projection, rotation_cw, translation_cw)
        center = np.array((frame.pose.x, frame.pose.y, frame.pose.z), dtype=np.float64)
        camera_centers.append(center)
        rows.append(u * projection[2] - projection[0])
        rows.append(v * projection[2] - projection[1])

    max_baseline = 0.0
    for i in range(len(camera_centers)):
        for j in range(i + 1, len(camera_centers)):
            max_baseline = max(max_baseline, float(np.linalg.norm(camera_centers[i] - camera_centers[j])))
    if max_baseline < min_baseline_m:
        return None

    design = np.stack(rows, axis=0)
    _, _, vh = np.linalg.svd(design)
    homogeneous = vh[-1]
    if abs(homogeneous[3]) < 1e-9:
        return None
    point_world = homogeneous[:3] / homogeneous[3]

    errors = []
    positive_depth = 0
    for frame_index, u, v in track.observations:
        _, rotation_cw, translation_cw = projection_matrices[frame_index]
        point_camera = rotation_cw @ point_world + translation_cw
        if point_camera[2] <= 1e-6:
            continue
        positive_depth += 1
        u_hat = camera_matrix[0, 0] * point_camera[0] / point_camera[2] + camera_matrix[0, 2]
        v_hat = camera_matrix[1, 1] * point_camera[1] / point_camera[2] + camera_matrix[1, 2]
        errors.append(math.hypot(u - u_hat, v - v_hat))

    if positive_depth < max(2, int(0.8 * len(track.observations))):
        return None
    if not errors:
        return None
    if float(np.mean(errors)) > max_reprojection_error_px:
        return None
    return point_world


def write_outputs(
    *,
    output_dir: Path,
    accepted_landmarks: list[tuple[int, np.ndarray]],
    accepted_tracks: list[Track],
    frames_by_index: dict[int, FrameData],
    camera_values: dict[str, float | int],
) -> None:
    landmarks_path = output_dir / "landmarks.csv"
    observations_path = output_dir / "visual_observations.csv"
    camera_args_path = output_dir / "camera_args.txt"

    with landmarks_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(("landmark_id", "x", "y", "z"))
        for landmark_id, point_world in accepted_landmarks:
            writer.writerow(
                (
                    landmark_id,
                    f"{point_world[0]:.9f}",
                    f"{point_world[1]:.9f}",
                    f"{point_world[2]:.9f}",
                )
            )

    with observations_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(("frame_index", "timestamp", "landmark_id", "u", "v"))
        for track in accepted_tracks:
            for frame_index, u, v in track.observations:
                frame = frames_by_index[frame_index]
                writer.writerow(
                    (
                        frame_index,
                        f"{frame.timestamp:.9f}",
                        track.track_id,
                        f"{u:.6f}",
                        f"{v:.6f}",
                    )
                )

    camera_args_path.write_text(
        " ".join(
            [
                f"--camera-fx {camera_values['fx']}",
                f"--camera-fy {camera_values['fy']}",
                f"--camera-cx {camera_values['cx']}",
                f"--camera-cy {camera_values['cy']}",
                f"--camera-width {camera_values['width']}",
                f"--camera-height {camera_values['height']}",
            ]
        )
        + "\n"
    )


def main() -> int:
    args = parse_args()
    sequence_dir = Path(args.sequence_dir)
    output_dir = Path(args.output_dir) if args.output_dir else sequence_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    drive_dir = Path(args.drive_dir) if args.drive_dir else infer_drive_dir(
        sequence_dir, Path(args.kitti_root)
    )
    frames = build_frame_data(
        sequence_dir=sequence_dir,
        gt_csv=Path(args.gt_csv),
        drive_dir=drive_dir,
        camera_dir=args.camera_dir,
        start_frame=args.start_frame,
        max_frames=args.max_frames,
    )
    if len(frames) < 2:
        raise RuntimeError("Need at least 2 frames to build visual tracks.")

    first_image = load_grayscale(frames[0].image_path)
    camera_matrix, camera_values = camera_matrix_from_args(args, first_image.shape)

    tracks = track_features(
        frames,
        max_active_tracks=args.max_active_tracks,
        detect_stride=args.detect_stride,
        quality_level=args.quality_level,
        min_feature_distance=args.min_feature_distance,
        lk_window_size=args.lk_window_size,
        lk_max_level=args.lk_max_level,
        max_forward_backward_error=args.max_forward_backward_error,
    )

    frames_by_index = {frame.frame_index: frame for frame in frames}
    accepted_landmarks: list[tuple[int, np.ndarray]] = []
    accepted_tracks: list[Track] = []
    next_landmark_id = 0
    for track in tracks:
        if len(track.observations) < args.min_track_length:
            continue
        point_world = triangulate_track(
            track,
            frames_by_index=frames_by_index,
            camera_matrix=camera_matrix,
            min_baseline_m=args.min_baseline_m,
            max_reprojection_error_px=args.max_reprojection_error_px,
        )
        if point_world is None:
            continue
        accepted_track = Track(track_id=next_landmark_id)
        accepted_track.observations = list(track.observations)
        accepted_track.last_point = track.last_point
        accepted_tracks.append(accepted_track)
        accepted_landmarks.append((next_landmark_id, point_world))
        next_landmark_id += 1

    if not accepted_tracks:
        raise RuntimeError("No valid landmarks survived tracking + triangulation.")

    write_outputs(
        output_dir=output_dir,
        accepted_landmarks=accepted_landmarks,
        accepted_tracks=accepted_tracks,
        frames_by_index=frames_by_index,
        camera_values=camera_values,
    )

    observations_count = sum(len(track.observations) for track in accepted_tracks)
    per_frame = [0 for _ in frames]
    for track in accepted_tracks:
        for frame_index, _, _ in track.observations:
            per_frame[frame_index] += 1

    print(f"[done] drive_dir={drive_dir}")
    print(f"[done] frames={len(frames)} tracked={len(tracks)} accepted={len(accepted_tracks)}")
    print(f"[done] wrote {len(accepted_landmarks)} landmarks to {output_dir / 'landmarks.csv'}")
    print(
        f"[done] wrote {observations_count} observations to "
        f"{output_dir / 'visual_observations.csv'}"
    )
    print(
        "[stats] "
        f"min_obs_per_frame={min(per_frame)} "
        f"max_obs_per_frame={max(per_frame)} "
        f"mean_obs_per_frame={sum(per_frame) / len(per_frame):.1f}"
    )
    print(f"[done] camera args saved to {output_dir / 'camera_args.txt'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
