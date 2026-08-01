#!/usr/bin/env python3
"""Select a frozen odometry trajectory using GT-free loop support only."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import time
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pose-graph-manifest", type=Path, required=True)
    parser.add_argument("--graph-supported-poses", type=Path, required=True)
    parser.add_argument("--fallback-poses", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--min-loop-clusters", type=int, default=2)
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def pose_count(path: Path) -> int:
    with path.open("r", encoding="utf-8") as stream:
        return sum(1 for line in stream if line.strip())


def select_by_loop_support(
    pose_graph_manifest: dict[str, object], min_loop_clusters: int
) -> str:
    if min_loop_clusters < 1:
        raise ValueError("min_loop_clusters must be positive")
    if pose_graph_manifest.get("ground_truth_used") is not False:
        raise ValueError("pose graph manifest must explicitly be GT-free")
    loop_clusters = int(pose_graph_manifest.get("loop_clusters", -1))
    correction_enabled = pose_graph_manifest.get("correction_enabled") is True
    return (
        "graph_supported"
        if correction_enabled and loop_clusters >= min_loop_clusters
        else "fallback"
    )


def main() -> int:
    args = parse_args()
    pose_graph_manifest = json.loads(
        args.pose_graph_manifest.read_text(encoding="utf-8")
    )
    selected = select_by_loop_support(
        pose_graph_manifest, args.min_loop_clusters
    )
    selected_path = (
        args.graph_supported_poses
        if selected == "graph_supported"
        else args.fallback_poses
    )
    graph_frames = pose_count(args.graph_supported_poses)
    fallback_frames = pose_count(args.fallback_poses)
    if graph_frames != fallback_frames:
        raise ValueError("candidate trajectory frame counts must match")

    start = time.perf_counter()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(selected_path, args.output)
    elapsed = time.perf_counter() - start
    payload = {
        "schema_version": 1,
        "method": "gt_free_loop_support_odometry_selector",
        "ground_truth_used": False,
        "selection_signal": "pose_graph_loop_clusters_and_correction_enabled",
        "min_loop_clusters": args.min_loop_clusters,
        "observed_loop_clusters": int(pose_graph_manifest["loop_clusters"]),
        "observed_correction_enabled": bool(
            pose_graph_manifest["correction_enabled"]
        ),
        "selected": selected,
        "frames": graph_frames,
        "seconds_including_pose_io": elapsed,
        "pose_graph_manifest_sha256": sha256_file(
            args.pose_graph_manifest
        ),
        "graph_supported_sha256": sha256_file(
            args.graph_supported_poses
        ),
        "fallback_sha256": sha256_file(args.fallback_poses),
        "output_sha256": sha256_file(args.output),
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(
        json.dumps(payload, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
