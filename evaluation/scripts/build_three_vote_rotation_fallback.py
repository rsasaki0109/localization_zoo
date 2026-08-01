#!/usr/bin/env python3
"""Build the frozen two-of-three GT-free rotation fallback.

The output is eligible only when the selector independently verifies that the
pose-graph correction and the reference correction lie in the same rotation
hemisphere.  Primary rotation receives one vote; the independent reference and
the corroborating pose graph receive two votes, fixing the reference geodesic
blend at 2/3 without a tunable accuracy parameter.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import time
from pathlib import Path

from fuse_odometry_incremental_rotation import (
    exp_so3,
    fuse_incremental_rotations,
    log_so3,
    load_kitti_poses,
    write_kitti_poses,
)

REFERENCE_MAJORITY_BLEND = 2.0 / 3.0
NUMERICAL_DISAGREEMENT_GUARD_DEG = 5.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--primary-poses", type=Path, required=True)
    parser.add_argument("--reference-poses", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def build_three_vote_rotation_fallback(primary, reference):
    return fuse_incremental_rotations(
        primary,
        reference,
        REFERENCE_MAJORITY_BLEND,
        math.radians(NUMERICAL_DISAGREEMENT_GUARD_DEG),
    )


def main() -> int:
    args = parse_args()
    started = time.perf_counter()
    primary = load_kitti_poses(args.primary_poses)
    reference = load_kitti_poses(args.reference_poses)
    output, gated = build_three_vote_rotation_fallback(primary, reference)
    write_kitti_poses(args.output, output)
    elapsed = time.perf_counter() - started
    manifest = {
        "schema_version": 1,
        "method": "gt_free_two_of_three_rotation_majority_fallback",
        "ground_truth_used": False,
        "eligibility": (
            "selector must observe safe graph rotation, unsafe graph "
            "translation, and positive graph/reference rotation-correction "
            "cosine"
        ),
        "votes": {
            "primary_rotation": 1,
            "independent_reference_rotation": 1,
            "pose_graph_corroboration": 1,
        },
        "reference_rotation_blend": REFERENCE_MAJORITY_BLEND,
        "translation_policy": "retain primary sensor-local translation increments",
        "numerical_disagreement_guard_deg": NUMERICAL_DISAGREEMENT_GUARD_DEG,
        "gated_increments": gated,
        "frames": len(output),
        "causality": "each candidate pose uses primary/reference increments through i; eligibility is selected after the GT-free graph-support pass",
        "primary_sha256": sha256(args.primary_poses),
        "reference_sha256": sha256(args.reference_poses),
        "output_sha256": sha256(args.output),
        "seconds_including_pose_io": elapsed,
        "fps_including_pose_io": len(output) / elapsed if elapsed > 0.0 else None,
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
