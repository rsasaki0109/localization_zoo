#!/usr/bin/env python3
"""Select a causal pose graph only when GT-free support and safety gates pass."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import time
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw", type=Path, required=True)
    parser.add_argument("--corrected", type=Path, required=True)
    parser.add_argument(
        "--fallback",
        type=Path,
        help="Frozen fallback trajectory used when correction gates fail.",
    )
    parser.add_argument(
        "--reference",
        type=Path,
        help=(
            "Independent GT-free reference trajectory used only to test "
            "whether its rotation correction corroborates the pose graph."
        ),
    )
    parser.add_argument(
        "--corroborated-fallback",
        type=Path,
        help=(
            "Frozen two-of-three rotation-majority trajectory selected when "
            "the graph translation is unsafe but graph and reference "
            "rotation corrections agree."
        ),
    )
    parser.add_argument("--pose-graph-manifest", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--min-loop-clusters", type=int, default=2)
    parser.add_argument(
        "--strong-loop-clusters",
        type=int,
        default=6,
        help=(
            "Independent loop-cluster count treated as overwhelming support; "
            "the default is three times the minimum support count."
        ),
    )
    parser.add_argument(
        "--allow-strong-loop-override",
        action="store_true",
        help=(
            "Permit overwhelming loop support to override accumulated "
            "correction envelopes. Disabled by default for LIVO datasets."
        ),
    )
    parser.add_argument("--max-translation-correction-m", type=float, default=0.5)
    parser.add_argument("--max-rotation-correction-deg", type=float, default=5.0)
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load_kitti_poses(path: Path) -> np.ndarray:
    values = np.loadtxt(path)
    values = np.atleast_2d(values)
    if values.shape[1] != 12 or not np.all(np.isfinite(values)):
        raise ValueError(f"{path} must contain finite KITTI 3x4 poses")
    poses = np.repeat(np.eye(4)[None, :, :], len(values), axis=0)
    poses[:, :3, :] = values.reshape(-1, 3, 4)
    return poses


def correction_envelope(
    raw: np.ndarray, corrected: np.ndarray
) -> tuple[float, float, float, float]:
    if raw.shape != corrected.shape:
        raise ValueError("raw and corrected pose counts must match")
    translation = np.linalg.norm(corrected[:, :3, 3] - raw[:, :3, 3], axis=1)
    rotation_degrees = []
    for raw_pose, corrected_pose in zip(raw, corrected):
        relative = raw_pose[:3, :3].T @ corrected_pose[:3, :3]
        cosine = float(np.clip((np.trace(relative) - 1.0) / 2.0, -1.0, 1.0))
        rotation_degrees.append(float(np.degrees(np.arccos(cosine))))
    return (
        float(np.max(translation)),
        float(translation[-1]),
        float(np.max(rotation_degrees)),
        float(rotation_degrees[-1]),
    )


def log_rotation(rotation: np.ndarray) -> np.ndarray:
    cosine = float(np.clip((np.trace(rotation) - 1.0) * 0.5, -1.0, 1.0))
    angle = float(np.arccos(cosine))
    skew_vector = np.array(
        [
            rotation[2, 1] - rotation[1, 2],
            rotation[0, 2] - rotation[2, 0],
            rotation[1, 0] - rotation[0, 1],
        ]
    )
    if angle < 1e-10:
        return 0.5 * skew_vector
    return angle / (2.0 * np.sin(angle)) * skew_vector


def rotation_correction_agreement(
    raw: np.ndarray,
    corrected: np.ndarray,
    reference: np.ndarray,
) -> tuple[bool, int, float | None, float | None]:
    if not (raw.shape == corrected.shape == reference.shape):
        raise ValueError("raw, corrected, and reference pose counts must match")

    graph_vectors = []
    reference_vectors = []
    for raw_pose, corrected_pose, reference_pose in zip(raw, corrected, reference):
        graph_vector = log_rotation(raw_pose[:3, :3].T @ corrected_pose[:3, :3])
        if np.linalg.norm(graph_vector) <= 1e-8:
            continue
        graph_vectors.append(graph_vector)
        reference_vectors.append(
            log_rotation(raw_pose[:3, :3].T @ reference_pose[:3, :3])
        )
    if not graph_vectors:
        return False, 0, None, None

    graph_array = np.asarray(graph_vectors)
    reference_array = np.asarray(reference_vectors)
    dots = np.einsum("ij,ij->i", graph_array, reference_array)
    denominator = float(np.linalg.norm(graph_array) * np.linalg.norm(reference_array))
    cosine = float(np.sum(dots) / denominator) if denominator > 0.0 else None
    positive_fraction = float(np.mean(dots > 0.0))
    return (
        bool(cosine is not None and cosine > 0.0),
        len(dots),
        cosine,
        positive_fraction,
    )


def select_source(
    raw: Path,
    corrected: Path,
    fallback: Path | None,
    correction_selected: bool,
    corroborated_fallback: Path | None = None,
    corroborated_fallback_selected: bool = False,
) -> Path:
    if correction_selected:
        return corrected
    if corroborated_fallback_selected and corroborated_fallback is not None:
        return corroborated_fallback
    return fallback if fallback is not None else raw


def correction_is_selected(
    cluster_gate: bool,
    strong_cluster_gate: bool,
    translation_gate: bool,
    rotation_gate: bool,
    allow_strong_loop_override: bool = False,
) -> bool:
    return (allow_strong_loop_override and strong_cluster_gate) or (
        cluster_gate and translation_gate and rotation_gate
    )


def main() -> int:
    started = time.perf_counter()
    args = parse_args()
    if (args.reference is None) != (args.corroborated_fallback is None):
        raise ValueError(
            "--reference and --corroborated-fallback must be supplied together"
        )
    graph_manifest = json.loads(args.pose_graph_manifest.read_text(encoding="utf-8"))
    raw = load_kitti_poses(args.raw)
    corrected = load_kitti_poses(args.corrected)
    max_translation, final_translation, max_rotation, final_rotation = (
        correction_envelope(raw, corrected)
    )
    loop_clusters = int(graph_manifest["loop_clusters"])
    cluster_gate = loop_clusters >= args.min_loop_clusters
    strong_cluster_gate = loop_clusters >= args.strong_loop_clusters
    translation_gate = max_translation <= args.max_translation_correction_m
    rotation_gate = max_rotation <= args.max_rotation_correction_deg
    correction_selected = correction_is_selected(
        cluster_gate,
        strong_cluster_gate,
        translation_gate,
        rotation_gate,
        args.allow_strong_loop_override,
    )
    direction_agrees = False
    active_correction_frames = 0
    correction_direction_cosine = None
    positive_direction_fraction = None
    if args.reference is not None:
        reference = load_kitti_poses(args.reference)
        (
            direction_agrees,
            active_correction_frames,
            correction_direction_cosine,
            positive_direction_fraction,
        ) = rotation_correction_agreement(raw, corrected, reference)
    corroborated_fallback_selected = bool(
        args.corroborated_fallback is not None
        and cluster_gate
        and rotation_gate
        and not translation_gate
        and direction_agrees
    )
    selected = select_source(
        args.raw,
        args.corrected,
        args.fallback,
        correction_selected,
        args.corroborated_fallback,
        corroborated_fallback_selected,
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(selected, args.output)
    elapsed = time.perf_counter() - started
    manifest = {
        "schema_version": 1,
        "method": "gt_free_loop_support_and_correction_envelope_selector",
        "ground_truth_used": False,
        "loop_clusters": loop_clusters,
        "min_loop_clusters": args.min_loop_clusters,
        "strong_loop_clusters": args.strong_loop_clusters,
        "strong_loop_override_enabled": args.allow_strong_loop_override,
        "max_translation_correction_m": max_translation,
        "translation_correction_limit_m": args.max_translation_correction_m,
        "final_translation_correction_m": final_translation,
        "max_rotation_correction_deg": max_rotation,
        "rotation_correction_limit_deg": args.max_rotation_correction_deg,
        "final_rotation_correction_deg": final_rotation,
        "cluster_gate_passed": cluster_gate,
        "strong_cluster_gate_passed": strong_cluster_gate,
        "translation_envelope_gate_passed": translation_gate,
        "rotation_envelope_gate_passed": rotation_gate,
        "correction_selected": correction_selected,
        "fallback_available": args.fallback is not None,
        "reference_available": args.reference is not None,
        "reference_sha256": sha256(args.reference) if args.reference else None,
        "corroborated_fallback_available": args.corroborated_fallback is not None,
        "rotation_correction_active_frames": active_correction_frames,
        "rotation_correction_direction_cosine": correction_direction_cosine,
        "rotation_correction_positive_direction_fraction": positive_direction_fraction,
        "rotation_correction_direction_agrees": direction_agrees,
        "corroborated_fallback_selected": corroborated_fallback_selected,
        "selected_source": str(selected),
        "selected_sha256": sha256(args.output),
        "seconds_including_pose_io": elapsed,
        "fps_including_pose_io": len(raw) / elapsed if elapsed > 0.0 else None,
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
