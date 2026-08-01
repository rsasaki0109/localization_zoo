#!/usr/bin/env python3
"""Fuse causal local-frame translation increments at their midpoint."""

from __future__ import annotations

import argparse
import hashlib
import json
import time
from pathlib import Path

import numpy as np

from fuse_odometry_incremental_rotation import load_kitti_poses, write_kitti_poses


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--primary-poses", required=True)
    parser.add_argument("--reference-poses", required=True)
    parser.add_argument("--rotation-poses", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--manifest")
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def local_translation(previous: np.ndarray, current: np.ndarray) -> np.ndarray:
    return previous[:3, :3].T @ (current[:3, 3] - previous[:3, 3])


def fuse_causal_local_increment_midpoint(
    primary: list[np.ndarray],
    reference: list[np.ndarray],
    rotation: list[np.ndarray],
) -> list[np.ndarray]:
    if not (len(primary) == len(reference) == len(rotation)):
        raise ValueError("primary, reference, and rotation pose counts must match")
    if not primary:
        return []

    first = rotation[0].copy()
    first[:3, 3] = 0.5 * (primary[0][:3, 3] + reference[0][:3, 3])
    output = [first]
    for index in range(1, len(primary)):
        primary_delta = local_translation(primary[index - 1], primary[index])
        reference_delta = local_translation(reference[index - 1], reference[index])
        fused_delta = 0.5 * (primary_delta + reference_delta)

        fused = rotation[index].copy()
        fused[:3, 3] = (
            output[-1][:3, 3] + output[-1][:3, :3] @ fused_delta
        )
        output.append(fused)
    return output


def main() -> int:
    args = parse_args()
    primary_path = Path(args.primary_poses)
    reference_path = Path(args.reference_poses)
    rotation_path = Path(args.rotation_poses)
    output_path = Path(args.output)

    started = time.perf_counter()
    primary = load_kitti_poses(primary_path)
    reference = load_kitti_poses(reference_path)
    rotation = load_kitti_poses(rotation_path)
    output = fuse_causal_local_increment_midpoint(primary, reference, rotation)
    write_kitti_poses(output_path, output)
    elapsed = time.perf_counter() - started

    payload = {
        "schema_version": 1,
        "method": "causal_local_increment_midpoint_translation",
        "ground_truth_used": False,
        "frames": len(output),
        "translation_policy": (
            "arithmetic midpoint of primary and reference same-step translations "
            "expressed in each frontend's previous sensor-local frame"
        ),
        "translation_weight_primary": 0.5,
        "translation_weight_reference": 0.5,
        "rotation_policy": "retain the supplied frozen causal rotation trajectory",
        "integration_policy": "rotate the fused local increment by the previous published rotation",
        "causality": "output i uses component poses only through i",
        "seconds_including_pose_io": elapsed,
        "fps_including_pose_io": len(output) / elapsed if elapsed > 0.0 else None,
        "primary_sha256": sha256_file(primary_path),
        "reference_sha256": sha256_file(reference_path),
        "rotation_sha256": sha256_file(rotation_path),
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
