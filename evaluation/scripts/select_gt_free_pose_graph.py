#!/usr/bin/env python3
"""Select a causal pose graph only when GT-free support and safety gates pass."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw", type=Path, required=True)
    parser.add_argument("--corrected", type=Path, required=True)
    parser.add_argument("--pose-graph-manifest", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--min-loop-clusters", type=int, default=2)
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
    translation = np.linalg.norm(
        corrected[:, :3, 3] - raw[:, :3, 3], axis=1
    )
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


def main() -> int:
    args = parse_args()
    graph_manifest = json.loads(args.pose_graph_manifest.read_text(encoding="utf-8"))
    raw = load_kitti_poses(args.raw)
    corrected = load_kitti_poses(args.corrected)
    max_translation, final_translation, max_rotation, final_rotation = (
        correction_envelope(raw, corrected)
    )
    loop_clusters = int(graph_manifest["loop_clusters"])
    cluster_gate = loop_clusters >= args.min_loop_clusters
    translation_gate = max_translation <= args.max_translation_correction_m
    rotation_gate = max_rotation <= args.max_rotation_correction_deg
    correction_selected = cluster_gate and translation_gate and rotation_gate
    selected = args.corrected if correction_selected else args.raw
    args.output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(selected, args.output)
    manifest = {
        "schema_version": 1,
        "method": "gt_free_loop_support_and_correction_envelope_selector",
        "ground_truth_used": False,
        "loop_clusters": loop_clusters,
        "min_loop_clusters": args.min_loop_clusters,
        "max_translation_correction_m": max_translation,
        "translation_correction_limit_m": args.max_translation_correction_m,
        "final_translation_correction_m": final_translation,
        "max_rotation_correction_deg": max_rotation,
        "rotation_correction_limit_deg": args.max_rotation_correction_deg,
        "final_rotation_correction_deg": final_rotation,
        "cluster_gate_passed": cluster_gate,
        "translation_envelope_gate_passed": translation_gate,
        "rotation_envelope_gate_passed": rotation_gate,
        "correction_selected": correction_selected,
        "selected_source": str(selected),
        "selected_sha256": sha256(args.output),
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
