#!/usr/bin/env python3
"""Rate-limit a causal pose-graph correction using past odometry motion only."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import time
from collections import deque
from pathlib import Path

import numpy as np

from fuse_odometry_incremental_rotation import load_kitti_poses, write_kitti_poses


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw-poses", required=True)
    parser.add_argument("--corrected-poses", required=True)
    parser.add_argument(
        "--correction-source-poses",
        help=(
            "Optional raw trajectory corresponding to --corrected-poses. "
            "Its causal left corrections are transferred onto --raw-poses."
        ),
    )
    parser.add_argument("--output", required=True)
    parser.add_argument("--manifest")
    parser.add_argument("--motion-window-frames", type=int, default=100)
    parser.add_argument("--translation-rate-fraction", type=float, default=0.005)
    parser.add_argument("--rotation-rate-fraction", type=float, default=0.005)
    parser.add_argument(
        "--integration-policy",
        choices=(
            "left_correction",
            "local_increment",
            "first_correction_latch",
            "first_correction_bias_rate",
            "causal_correction_bias_rate_updates",
            "causal_interval_bias_rate_updates",
            "causal_interval_rotation_first_translation_latch",
            "causal_interval_consensus_rotation_bias",
            "causal_interval_yaw_consensus_bias",
            "causal_interval_robust_rotation_bias",
            "causal_interval_robust_consistent_direction_bias",
            "causal_interval_robust_pair_distance_direction_bias",
            "causal_interval_robust_yaw_direction_bias",
            "causal_interval_robust_motion_yaw_output_bias",
            "causal_interval_robust_persistent_yaw_bias",
            "causal_interval_robust_rotation_scale_bias",
            "causal_interval_delayed_median_rotation_bias",
            "causal_interval_distance_weighted_rotation_bias",
            "causal_interval_distance_weighted_se3_bias",
            "causal_interval_distance_weighted_tilt_robust_yaw_bias",
            "causal_interval_median_vector_rotation_bias",
            "causal_interval_hampel_vector_rotation_bias",
            "causal_interval_hampel_vector_persistent_yaw_bias",
        ),
        default="left_correction",
    )
    parser.add_argument("--correction-latch-threshold", type=float, default=1e-4)
    parser.add_argument("--bias-translation-gain", type=float, default=1.0)
    parser.add_argument("--bias-rotation-gain", type=float, default=1.0)
    parser.add_argument("--bias-update-threshold", type=float, default=0.01)
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def rotation_angle(rotation: np.ndarray) -> float:
    cosine = float(np.clip((np.trace(rotation) - 1.0) * 0.5, -1.0, 1.0))
    return math.acos(cosine)


def rotation_axis(rotation: np.ndarray, angle: float) -> np.ndarray:
    sine = math.sin(angle)
    if abs(sine) > 1e-8:
        axis = np.array(
            [
                rotation[2, 1] - rotation[1, 2],
                rotation[0, 2] - rotation[2, 0],
                rotation[1, 0] - rotation[0, 1],
            ]
        ) / (2.0 * sine)
    else:
        values, vectors = np.linalg.eig(rotation)
        axis = np.real(vectors[:, int(np.argmin(np.abs(values - 1.0)))])
    norm = float(np.linalg.norm(axis))
    if norm <= 1e-12:
        return np.array([1.0, 0.0, 0.0])
    return axis / norm


def axis_angle_rotation(axis: np.ndarray, angle: float) -> np.ndarray:
    x, y, z = axis
    skew = np.array([[0.0, -z, y], [z, 0.0, -x], [-y, x, 0.0]])
    return np.eye(3) + math.sin(angle) * skew + (1.0 - math.cos(angle)) * (skew @ skew)


def step_rotation_toward(
    current: np.ndarray, target: np.ndarray, maximum_step_rad: float
) -> np.ndarray:
    relative = current.T @ target
    angle = rotation_angle(relative)
    if angle <= maximum_step_rad or angle <= 1e-12:
        return target.copy()
    stepped = current @ axis_angle_rotation(
        rotation_axis(relative, angle), maximum_step_rad
    )
    left, _, right = np.linalg.svd(stepped)
    orthonormal = left @ right
    if np.linalg.det(orthonormal) < 0.0:
        left[:, -1] *= -1.0
        orthonormal = left @ right
    return orthonormal


def smooth_causal_pose_graph_correction(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    motion_window_frames: int = 100,
    translation_rate_fraction: float = 0.005,
    rotation_rate_fraction: float = 0.005,
) -> list[np.ndarray]:
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if motion_window_frames <= 0:
        raise ValueError("motion_window_frames must be positive")
    if translation_rate_fraction < 0.0 or rotation_rate_fraction < 0.0:
        raise ValueError("rate fractions must be non-negative")
    if not raw:
        return []

    translation_history: deque[float] = deque(maxlen=motion_window_frames)
    rotation_history: deque[float] = deque(maxlen=motion_window_frames)
    current_correction = corrected[0] @ np.linalg.inv(raw[0])
    output = [current_correction @ raw[0]]

    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        translation_history.append(float(np.linalg.norm(raw_increment[:3, 3])))
        rotation_history.append(rotation_angle(raw_increment[:3, :3]))

        target_correction = corrected[index] @ np.linalg.inv(raw[index])
        translation_scale = float(np.median(translation_history))
        rotation_scale = float(np.median(rotation_history))
        maximum_translation_step = translation_rate_fraction * translation_scale
        maximum_rotation_step = rotation_rate_fraction * rotation_scale

        translation_delta = target_correction[:3, 3] - current_correction[:3, 3]
        translation_distance = float(np.linalg.norm(translation_delta))
        if translation_distance <= maximum_translation_step or translation_distance <= 1e-12:
            next_translation = target_correction[:3, 3].copy()
        else:
            next_translation = (
                current_correction[:3, 3]
                + maximum_translation_step * translation_delta / translation_distance
            )
        next_rotation = step_rotation_toward(
            current_correction[:3, :3],
            target_correction[:3, :3],
            maximum_rotation_step,
        )
        current_correction = np.eye(4)
        current_correction[:3, :3] = next_rotation
        current_correction[:3, 3] = next_translation
        output.append(current_correction @ raw[index])
    return output


def smooth_causal_pose_graph_local_increment(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    motion_window_frames: int = 100,
    translation_rate_fraction: float = 0.005,
    rotation_rate_fraction: float = 0.005,
) -> list[np.ndarray]:
    """Integrate raw local increments and rate-limit attraction to graph poses.

    Unlike smoothing a world-frame left correction, this policy never rotates
    the already accumulated position about the world origin. A changing graph
    rotation therefore cannot create a lever-arm translation jump.
    """
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if motion_window_frames <= 0:
        raise ValueError("motion_window_frames must be positive")
    if translation_rate_fraction < 0.0 or rotation_rate_fraction < 0.0:
        raise ValueError("rate fractions must be non-negative")
    if not raw:
        return []

    translation_history: deque[float] = deque(maxlen=motion_window_frames)
    rotation_history: deque[float] = deque(maxlen=motion_window_frames)
    output = [corrected[0].copy()]
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        raw_translation = raw_increment[:3, 3]
        raw_rotation = raw_increment[:3, :3]
        translation_history.append(float(np.linalg.norm(raw_translation)))
        rotation_history.append(rotation_angle(raw_rotation))

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_rotation
        predicted[:3, 3] = (
            output[-1][:3, 3] + output[-1][:3, :3] @ raw_translation
        )

        maximum_translation_step = (
            translation_rate_fraction * float(np.median(translation_history))
        )
        maximum_rotation_step = (
            rotation_rate_fraction * float(np.median(rotation_history))
        )
        translation_delta = corrected[index][:3, 3] - predicted[:3, 3]
        translation_distance = float(np.linalg.norm(translation_delta))
        if translation_distance <= maximum_translation_step or translation_distance <= 1e-12:
            predicted[:3, 3] = corrected[index][:3, 3]
        else:
            predicted[:3, 3] += (
                maximum_translation_step * translation_delta / translation_distance
            )
        predicted[:3, :3] = step_rotation_toward(
            predicted[:3, :3],
            corrected[index][:3, :3],
            maximum_rotation_step,
        )
        output.append(predicted)
    return output


def latch_first_causal_pose_graph_correction(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    correction_latch_threshold: float = 1e-4,
) -> list[np.ndarray]:
    """Latch the first corroborated non-identity graph correction."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if correction_latch_threshold < 0.0:
        raise ValueError("correction_latch_threshold must be non-negative")
    if not raw:
        return []

    latched = np.eye(4)
    has_latched = False
    output: list[np.ndarray] = []
    for raw_pose, corrected_pose in zip(raw, corrected):
        target = corrected_pose @ np.linalg.inv(raw_pose)
        correction_size = max(
            float(np.linalg.norm(target[:3, 3])),
            rotation_angle(target[:3, :3]),
        )
        if not has_latched and correction_size > correction_latch_threshold:
            latched = target
            has_latched = True
        output.append(latched @ raw_pose)
    return output


def apply_first_causal_correction_as_bias_rate(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    correction_latch_threshold: float = 1e-4,
    bias_translation_gain: float = 1.0,
    bias_rotation_gain: float = 1.0,
) -> list[np.ndarray]:
    """Use the first loop correction as a future per-metre drift estimate."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if correction_latch_threshold < 0.0:
        raise ValueError("correction_latch_threshold must be non-negative")
    if bias_translation_gain < 0.0 or bias_rotation_gain < 0.0:
        raise ValueError("bias gains must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    travelled_distance = 0.0
    has_bias = False
    translation_bias_per_m = np.zeros(3)
    rotation_axis_bias = np.array([1.0, 0.0, 0.0])
    rotation_bias_per_m = 0.0
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        travelled_distance += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
        )
        if has_bias and distance > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_axis_bias,
                    bias_rotation_gain * rotation_bias_per_m * distance,
                )
                @ predicted[:3, :3]
            )
            predicted[:3, 3] += (
                bias_translation_gain * translation_bias_per_m * distance
            )
        output.append(predicted)

        if not has_bias and travelled_distance > 1e-9:
            target = corrected[index] @ np.linalg.inv(raw[index])
            correction_rotation_angle = rotation_angle(target[:3, :3])
            correction_size = max(
                float(np.linalg.norm(target[:3, 3])), correction_rotation_angle
            )
            if correction_size > correction_latch_threshold:
                translation_bias_per_m = target[:3, 3] / travelled_distance
                rotation_axis_bias = rotation_axis(
                    target[:3, :3], correction_rotation_angle
                )
                rotation_bias_per_m = correction_rotation_angle / travelled_distance
                has_bias = True
    return output


def transfer_causal_left_corrections(
    target_raw: list[np.ndarray],
    correction_source_raw: list[np.ndarray],
    correction_source_corrected: list[np.ndarray],
) -> list[np.ndarray]:
    if not (
        len(target_raw)
        == len(correction_source_raw)
        == len(correction_source_corrected)
    ):
        raise ValueError("target and correction-source pose counts must match")
    return [
        corrected @ np.linalg.inv(source) @ target
        for target, source, corrected in zip(
            target_raw, correction_source_raw, correction_source_corrected
        )
    ]


def apply_causal_correction_as_updated_bias_rate(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
    bias_translation_gain: float = 1.0,
    bias_rotation_gain: float = 1.0,
) -> list[np.ndarray]:
    """Update a future-only drift-rate estimate at causal graph changes."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if bias_translation_gain < 0.0 or bias_rotation_gain < 0.0:
        raise ValueError("bias gains must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    travelled_distance = 0.0
    translation_bias_per_m = np.zeros(3)
    rotation_axis_bias = np.array([1.0, 0.0, 0.0])
    rotation_bias_per_m = 0.0
    last_accepted_correction = np.eye(4)
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        travelled_distance += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
            + bias_translation_gain * translation_bias_per_m * distance
        )
        if distance > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_axis_bias,
                    bias_rotation_gain * rotation_bias_per_m * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        update = target @ np.linalg.inv(last_accepted_correction)
        update_size = max(
            float(np.linalg.norm(update[:3, 3])),
            rotation_angle(update[:3, :3]),
        )
        if travelled_distance > 1e-9 and update_size > bias_update_threshold:
            target_angle = rotation_angle(target[:3, :3])
            translation_bias_per_m = target[:3, 3] / travelled_distance
            rotation_axis_bias = rotation_axis(target[:3, :3], target_angle)
            rotation_bias_per_m = target_angle / travelled_distance
            last_accepted_correction = target
    return output


def apply_causal_interval_correction_as_bias_rate(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
    bias_translation_gain: float = 1.0,
    bias_rotation_gain: float = 1.0,
    latch_first_translation: bool = False,
) -> list[np.ndarray]:
    """Learn future drift from each correction increment since the last loop."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if bias_translation_gain < 0.0 or bias_rotation_gain < 0.0:
        raise ValueError("bias gains must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    translation_bias_per_m = np.zeros(3)
    rotation_axis_bias = np.array([1.0, 0.0, 0.0])
    rotation_bias_per_m = 0.0
    last_accepted_correction = np.eye(4)
    translation_latched = False
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
            + bias_translation_gain * translation_bias_per_m * distance
        )
        if distance > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_axis_bias,
                    bias_rotation_gain * rotation_bias_per_m * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            if latch_first_translation and not translation_latched:
                output[-1][:3, 3] += target[:3, 3]
                translation_latched = True
            translation_bias_per_m = (
                correction_increment[:3, 3] / distance_since_update
            )
            rotation_axis_bias = rotation_axis(
                correction_increment[:3, :3], increment_angle
            )
            rotation_bias_per_m = increment_angle / distance_since_update
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_consensus_rotation_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
) -> list[np.ndarray]:
    """Amplify interval rotation drift only when consecutive axes agree."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    rotation_rate_vector = np.zeros(3)
    previous_observed_rate: np.ndarray | None = None
    last_accepted_correction = np.eye(4)
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
        )
        rate_norm = float(np.linalg.norm(rotation_rate_vector))
        if distance > 0.0 and rate_norm > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_rate_vector / rate_norm,
                    rate_norm * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            observed_rate = (
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
                / distance_since_update
            )
            consensus = 0.0
            observed_norm = float(np.linalg.norm(observed_rate))
            if previous_observed_rate is not None and observed_norm > 0.0:
                previous_norm = float(np.linalg.norm(previous_observed_rate))
                if previous_norm > 0.0:
                    consensus = max(
                        0.0,
                        float(
                            np.dot(previous_observed_rate, observed_rate)
                            / (previous_norm * observed_norm)
                        ),
                    )
            rotation_rate_vector = (1.0 + consensus) * observed_rate
            previous_observed_rate = observed_rate
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_yaw_consensus_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
) -> list[np.ndarray]:
    """Apply only corroborated z-up yaw drift from causal loop intervals."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    yaw_rate_per_m = 0.0
    previous_observed_yaw_rate: float | None = None
    last_accepted_correction = np.eye(4)
    z_axis = np.array([0.0, 0.0, 1.0])
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
        )
        if distance > 0.0 and yaw_rate_per_m != 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(z_axis, yaw_rate_per_m * distance)
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            rotation_vector = (
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
            )
            observed_yaw_rate = float(rotation_vector[2]) / distance_since_update
            corroborated = (
                previous_observed_yaw_rate is not None
                and previous_observed_yaw_rate * observed_yaw_rate > 0.0
            )
            yaw_rate_per_m = (2.0 if corroborated else 1.0) * observed_yaw_rate
            previous_observed_yaw_rate = observed_yaw_rate
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_robust_rotation_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
    learn_first_scale: bool = False,
    reject_direction_reversal: bool = False,
    pair_distance_weight_direction: bool = False,
    yaw_only: bool = False,
    preserve_yaw_sign: bool = False,
) -> list[np.ndarray]:
    """Double interval rotation drift while clipping short-interval outliers."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    rotation_rate_vector = np.zeros(3)
    observed_rate_norms: list[float] = []
    previous_observed_rate: np.ndarray | None = None
    previous_observed_distance = 0.0
    last_accepted_correction = np.eye(4)
    translation_scale = 1.0
    scale_learned = False
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3]
            @ (translation_scale * raw_increment[:3, 3])
        )
        rate_norm = float(np.linalg.norm(rotation_rate_vector))
        if distance > 0.0 and rate_norm > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_rate_vector / rate_norm,
                    rate_norm * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            if learn_first_scale and not scale_learned:
                rotated_raw_position = (
                    target[:3, :3] @ raw[index][:3, 3]
                )
                denominator = float(
                    np.dot(rotated_raw_position, rotated_raw_position)
                )
                if denominator > 1e-9:
                    inferred_scale = float(
                        np.dot(corrected[index][:3, 3], rotated_raw_position)
                        / denominator
                    )
                    translation_scale = float(
                        np.clip(inferred_scale, 0.95, 1.05)
                    )
                    scale_learned = True
            observed_rate = (
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
                / distance_since_update
            )
            if yaw_only:
                observed_rate[:2] = 0.0
            if (
                preserve_yaw_sign
                and rotation_rate_vector[2] != 0.0
                and observed_rate[2] * rotation_rate_vector[2] < 0.0
            ):
                observed_rate[2] *= -1.0
            observed_norm = float(np.linalg.norm(observed_rate))
            direction_rate = observed_rate
            if (
                pair_distance_weight_direction
                and previous_observed_rate is not None
            ):
                direction_rate = (
                    previous_observed_rate * previous_observed_distance
                    + observed_rate * distance_since_update
                ) / (previous_observed_distance + distance_since_update)
            applied_norm = 2.0 * observed_norm
            if observed_rate_norms:
                robust_median = float(np.median(observed_rate_norms))
                applied_norm = min(applied_norm, 2.0 * robust_median)
            if len(observed_rate_norms) >= 3:
                median_absolute_deviation = float(
                    np.median(
                        np.abs(np.asarray(observed_rate_norms) - robust_median)
                    )
                )
                hampel_upper_bound = (
                    robust_median
                    + 3.0 * 1.4826 * median_absolute_deviation
                )
                applied_norm = min(applied_norm, hampel_upper_bound)
            observed_rate_norms.append(observed_norm)
            direction_norm = float(np.linalg.norm(direction_rate))
            if direction_norm > 0.0:
                applied_direction = direction_rate / direction_norm
                current_norm = float(np.linalg.norm(rotation_rate_vector))
                if (
                    reject_direction_reversal
                    and current_norm > 0.0
                    and float(
                        np.dot(applied_direction, rotation_rate_vector)
                    ) < 0.0
                ):
                    applied_direction = rotation_rate_vector / current_norm
                rotation_rate_vector = applied_direction * applied_norm
            else:
                rotation_rate_vector = np.zeros(3)
            previous_observed_rate = observed_rate
            previous_observed_distance = distance_since_update
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_median_vector_rotation_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
) -> list[np.ndarray]:
    """Use twice the causal component-wise median interval rotation rate."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    rotation_rate_vector = np.zeros(3)
    observed_rate_vectors: list[np.ndarray] = []
    last_accepted_correction = np.eye(4)
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
        )
        rate_norm = float(np.linalg.norm(rotation_rate_vector))
        if distance > 0.0 and rate_norm > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_rate_vector / rate_norm,
                    rate_norm * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            observed_rate_vectors.append(
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
                / distance_since_update
            )
            rotation_rate_vector = 2.0 * np.median(
                np.stack(observed_rate_vectors), axis=0
            )
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_robust_motion_yaw_output_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
) -> list[np.ndarray]:
    """Use full robust attitude for motion, but publish robust z-up yaw."""
    motion = apply_causal_interval_robust_rotation_bias(
        raw, corrected, bias_update_threshold=bias_update_threshold
    )
    published = apply_causal_interval_robust_rotation_bias(
        raw,
        corrected,
        bias_update_threshold=bias_update_threshold,
        reject_direction_reversal=True,
        yaw_only=True,
    )
    output: list[np.ndarray] = []
    for motion_pose, published_pose in zip(motion, published):
        pose = published_pose.copy()
        pose[:3, 3] = motion_pose[:3, 3]
        output.append(pose)
    return output


def apply_causal_interval_delayed_median_rotation_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
) -> list[np.ndarray]:
    """Use bounded latest drift until three samples make a median robust."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    rotation_rate_vector = np.zeros(3)
    observed_rate_vectors: list[np.ndarray] = []
    last_accepted_correction = np.eye(4)
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
        )
        rate_norm = float(np.linalg.norm(rotation_rate_vector))
        if distance > 0.0 and rate_norm > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_rate_vector / rate_norm,
                    rate_norm * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            observed_rate = (
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
                / distance_since_update
            )
            observed_rate_vectors.append(observed_rate)
            if len(observed_rate_vectors) >= 3:
                rotation_rate_vector = 2.0 * np.median(
                    np.stack(observed_rate_vectors), axis=0
                )
            else:
                observed_norm = float(np.linalg.norm(observed_rate))
                applied_norm = 2.0 * observed_norm
                if len(observed_rate_vectors) == 2:
                    first_norm = float(np.linalg.norm(observed_rate_vectors[0]))
                    applied_norm = min(applied_norm, 2.0 * first_norm)
                rotation_rate_vector = (
                    observed_rate / observed_norm * applied_norm
                    if observed_norm > 0.0
                    else np.zeros(3)
                )
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_distance_weighted_rotation_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
    learn_translation_bias: bool = False,
) -> list[np.ndarray]:
    """Average interval rotation drift in proportion to travelled distance."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    total_observed_distance = 0.0
    cumulative_rotation_vector = np.zeros(3)
    cumulative_translation_vector = np.zeros(3)
    rotation_rate_vector = np.zeros(3)
    translation_rate_vector = np.zeros(3)
    last_accepted_correction = np.eye(4)
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
            + translation_rate_vector * distance
        )
        rate_norm = float(np.linalg.norm(rotation_rate_vector))
        if distance > 0.0 and rate_norm > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_rate_vector / rate_norm,
                    rate_norm * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            cumulative_rotation_vector += (
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
            )
            if learn_translation_bias:
                cumulative_translation_vector += correction_increment[:3, 3]
            total_observed_distance += distance_since_update
            rotation_rate_vector = (
                2.0 * cumulative_rotation_vector / total_observed_distance
            )
            if learn_translation_bias:
                translation_rate_vector = (
                    cumulative_translation_vector / total_observed_distance
                )
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_distance_weighted_tilt_robust_yaw_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
) -> list[np.ndarray]:
    """Use long-baseline tilt drift and independently robust interval yaw."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    total_observed_distance = 0.0
    cumulative_tilt_rotation = np.zeros(2)
    yaw_rate_magnitudes: list[float] = []
    rotation_rate_vector = np.zeros(3)
    last_accepted_correction = np.eye(4)
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
        )
        rate_norm = float(np.linalg.norm(rotation_rate_vector))
        if distance > 0.0 and rate_norm > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_rate_vector / rate_norm,
                    rate_norm * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            rotation_vector = (
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
            )
            cumulative_tilt_rotation += rotation_vector[:2]
            total_observed_distance += distance_since_update
            rotation_rate_vector[:2] = (
                2.0 * cumulative_tilt_rotation / total_observed_distance
            )

            observed_yaw_rate = float(rotation_vector[2]) / distance_since_update
            observed_yaw_magnitude = abs(observed_yaw_rate)
            applied_yaw_magnitude = 2.0 * observed_yaw_magnitude
            if yaw_rate_magnitudes:
                robust_median = float(np.median(yaw_rate_magnitudes))
                applied_yaw_magnitude = min(
                    applied_yaw_magnitude, 2.0 * robust_median
                )
            if len(yaw_rate_magnitudes) >= 3:
                median_absolute_deviation = float(
                    np.median(
                        np.abs(
                            np.asarray(yaw_rate_magnitudes) - robust_median
                        )
                    )
                )
                applied_yaw_magnitude = min(
                    applied_yaw_magnitude,
                    robust_median
                    + 3.0 * 1.4826 * median_absolute_deviation,
                )
            rotation_rate_vector[2] = (
                np.copysign(applied_yaw_magnitude, observed_yaw_rate)
                if observed_yaw_magnitude > 0.0
                else 0.0
            )
            yaw_rate_magnitudes.append(observed_yaw_magnitude)
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def apply_causal_interval_hampel_vector_rotation_bias(
    raw: list[np.ndarray],
    corrected: list[np.ndarray],
    *,
    bias_update_threshold: float = 0.01,
    preserve_yaw_sign: bool = False,
) -> list[np.ndarray]:
    """Winsorize only outlying components of causal interval rotation drift."""
    if len(raw) != len(corrected):
        raise ValueError("raw and corrected pose counts must match")
    if bias_update_threshold < 0.0:
        raise ValueError("bias_update_threshold must be non-negative")
    if not raw:
        return []

    output = [raw[0].copy()]
    distance_since_update = 0.0
    rotation_rate_vector = np.zeros(3)
    observed_rate_vectors: list[np.ndarray] = []
    last_accepted_correction = np.eye(4)
    for index in range(1, len(raw)):
        raw_increment = np.linalg.inv(raw[index - 1]) @ raw[index]
        distance = float(np.linalg.norm(raw_increment[:3, 3]))
        distance_since_update += distance

        predicted = np.eye(4)
        predicted[:3, :3] = output[-1][:3, :3] @ raw_increment[:3, :3]
        predicted[:3, 3] = (
            output[-1][:3, 3]
            + output[-1][:3, :3] @ raw_increment[:3, 3]
        )
        rate_norm = float(np.linalg.norm(rotation_rate_vector))
        if distance > 0.0 and rate_norm > 0.0:
            predicted[:3, :3] = (
                axis_angle_rotation(
                    rotation_rate_vector / rate_norm,
                    rate_norm * distance,
                )
                @ predicted[:3, :3]
            )
        output.append(predicted)

        target = corrected[index] @ np.linalg.inv(raw[index])
        correction_increment = target @ np.linalg.inv(last_accepted_correction)
        increment_angle = rotation_angle(correction_increment[:3, :3])
        update_size = max(
            float(np.linalg.norm(correction_increment[:3, 3])),
            increment_angle,
        )
        if distance_since_update > 1e-9 and update_size > bias_update_threshold:
            observed_rate = (
                rotation_axis(correction_increment[:3, :3], increment_angle)
                * increment_angle
                / distance_since_update
            )
            if (
                preserve_yaw_sign
                and rotation_rate_vector[2] != 0.0
                and observed_rate[2] * rotation_rate_vector[2] < 0.0
            ):
                observed_rate[2] *= -1.0
            adjusted_rate = observed_rate.copy()
            if len(observed_rate_vectors) >= 3:
                history = np.stack(observed_rate_vectors)
                median = np.median(history, axis=0)
                mad = np.median(np.abs(history - median), axis=0)
                radius = 3.0 * 1.4826 * mad
                for component in range(3):
                    if radius[component] <= 1e-15:
                        if abs(adjusted_rate[component] - median[component]) > 1e-15:
                            adjusted_rate[component] = median[component]
                    else:
                        adjusted_rate[component] = np.clip(
                            adjusted_rate[component],
                            median[component] - radius[component],
                            median[component] + radius[component],
                        )
            adjusted_norm = float(np.linalg.norm(adjusted_rate))
            applied_norm = 2.0 * adjusted_norm
            if observed_rate_vectors:
                prior_norms = [
                    float(np.linalg.norm(vector))
                    for vector in observed_rate_vectors
                ]
                applied_norm = min(
                    applied_norm, 2.0 * float(np.median(prior_norms))
                )
            rotation_rate_vector = (
                adjusted_rate / adjusted_norm * applied_norm
                if adjusted_norm > 0.0
                else np.zeros(3)
            )
            observed_rate_vectors.append(observed_rate)
            last_accepted_correction = target
            distance_since_update = 0.0
    return output


def main() -> int:
    args = parse_args()
    raw_path = Path(args.raw_poses)
    corrected_path = Path(args.corrected_poses)
    output_path = Path(args.output)

    started = time.perf_counter()
    raw = load_kitti_poses(raw_path)
    corrected = load_kitti_poses(corrected_path)
    correction_source_path = (
        Path(args.correction_source_poses)
        if args.correction_source_poses
        else None
    )
    if correction_source_path is not None:
        correction_source = load_kitti_poses(correction_source_path)
        corrected = transfer_causal_left_corrections(
            raw, correction_source, corrected
        )
    if args.integration_policy == "first_correction_latch":
        output = latch_first_causal_pose_graph_correction(
            raw,
            corrected,
            correction_latch_threshold=args.correction_latch_threshold,
        )
    elif args.integration_policy == "first_correction_bias_rate":
        output = apply_first_causal_correction_as_bias_rate(
            raw,
            corrected,
            correction_latch_threshold=args.correction_latch_threshold,
            bias_translation_gain=args.bias_translation_gain,
            bias_rotation_gain=args.bias_rotation_gain,
        )
    elif args.integration_policy == "causal_correction_bias_rate_updates":
        output = apply_causal_correction_as_updated_bias_rate(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            bias_translation_gain=args.bias_translation_gain,
            bias_rotation_gain=args.bias_rotation_gain,
        )
    elif args.integration_policy == "causal_interval_bias_rate_updates":
        output = apply_causal_interval_correction_as_bias_rate(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            bias_translation_gain=args.bias_translation_gain,
            bias_rotation_gain=args.bias_rotation_gain,
        )
    elif args.integration_policy == "causal_interval_rotation_first_translation_latch":
        output = apply_causal_interval_correction_as_bias_rate(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            bias_translation_gain=0.0,
            bias_rotation_gain=1.0,
            latch_first_translation=True,
        )
    elif args.integration_policy == "causal_interval_consensus_rotation_bias":
        output = apply_causal_interval_consensus_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy == "causal_interval_yaw_consensus_bias":
        output = apply_causal_interval_yaw_consensus_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy == "causal_interval_robust_rotation_bias":
        output = apply_causal_interval_robust_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy == "causal_interval_robust_consistent_direction_bias":
        output = apply_causal_interval_robust_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            reject_direction_reversal=True,
        )
    elif args.integration_policy == "causal_interval_robust_pair_distance_direction_bias":
        output = apply_causal_interval_robust_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            pair_distance_weight_direction=True,
        )
    elif args.integration_policy == "causal_interval_robust_yaw_direction_bias":
        output = apply_causal_interval_robust_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            reject_direction_reversal=True,
            yaw_only=True,
        )
    elif args.integration_policy == "causal_interval_robust_motion_yaw_output_bias":
        output = apply_causal_interval_robust_motion_yaw_output_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy == "causal_interval_robust_persistent_yaw_bias":
        output = apply_causal_interval_robust_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            preserve_yaw_sign=True,
        )
    elif args.integration_policy == "causal_interval_robust_rotation_scale_bias":
        output = apply_causal_interval_robust_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            learn_first_scale=True,
        )
    elif args.integration_policy == "causal_interval_delayed_median_rotation_bias":
        output = apply_causal_interval_delayed_median_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy == "causal_interval_distance_weighted_rotation_bias":
        output = apply_causal_interval_distance_weighted_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy == "causal_interval_distance_weighted_se3_bias":
        output = apply_causal_interval_distance_weighted_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            learn_translation_bias=True,
        )
    elif (
        args.integration_policy
        == "causal_interval_distance_weighted_tilt_robust_yaw_bias"
    ):
        output = apply_causal_interval_distance_weighted_tilt_robust_yaw_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy == "causal_interval_median_vector_rotation_bias":
        output = apply_causal_interval_median_vector_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
        )
    elif args.integration_policy in (
        "causal_interval_hampel_vector_rotation_bias",
        "causal_interval_hampel_vector_persistent_yaw_bias",
    ):
        output = apply_causal_interval_hampel_vector_rotation_bias(
            raw,
            corrected,
            bias_update_threshold=args.bias_update_threshold,
            preserve_yaw_sign=(
                args.integration_policy
                == "causal_interval_hampel_vector_persistent_yaw_bias"
            ),
        )
    else:
        smoother = (
            smooth_causal_pose_graph_local_increment
            if args.integration_policy == "local_increment"
            else smooth_causal_pose_graph_correction
        )
        output = smoother(
            raw,
            corrected,
            motion_window_frames=args.motion_window_frames,
            translation_rate_fraction=args.translation_rate_fraction,
            rotation_rate_fraction=args.rotation_rate_fraction,
        )
    write_kitti_poses(output_path, output)
    elapsed = time.perf_counter() - started

    payload = {
        "schema_version": 1,
        "method": "causal_motion_scaled_pose_graph_correction_rate_limit",
        "ground_truth_used": False,
        "frames": len(output),
        "motion_window_frames": args.motion_window_frames,
        "translation_rate_fraction": args.translation_rate_fraction,
        "rotation_rate_fraction": args.rotation_rate_fraction,
        "integration_policy": args.integration_policy,
        "correction_latch_threshold": args.correction_latch_threshold,
        "bias_translation_gain": args.bias_translation_gain,
        "bias_rotation_gain": args.bias_rotation_gain,
        "bias_update_threshold": args.bias_update_threshold,
        "policy": (
            "At frame i, move the published pose toward the pose-graph correction "
            "by at most the configured fraction of the median raw-odometry motion "
            "over frames <= i; local_increment integrates raw sensor-local motion "
            "without rotating accumulated position about the world origin."
        ),
        "causality": "output i uses raw and pose-graph poses only through i",
        "seconds_including_pose_io": elapsed,
        "fps_including_pose_io": len(output) / elapsed if elapsed > 0.0 else None,
        "raw_sha256": sha256_file(raw_path),
        "corrected_sha256": sha256_file(corrected_path),
        "correction_source_sha256": (
            sha256_file(correction_source_path)
            if correction_source_path is not None
            else None
        ),
        "correction_transfer_policy": (
            "T_target_corrected_i = T_source_corrected_i * "
            "inverse(T_source_raw_i) * T_target_raw_i"
            if correction_source_path is not None
            else "none; corrected poses correspond directly to raw poses"
        ),
        "output_sha256": sha256_file(output_path),
    }
    if args.manifest:
        manifest_path = Path(args.manifest)
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
