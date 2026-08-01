#!/usr/bin/env python3
"""Summarize GT-free finite-pose and inter-frame health for KITTI pose files."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--poses", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--max-step-translation-m", type=float, default=2.0)
    parser.add_argument("--max-step-rotation-deg", type=float, default=20.0)
    parser.add_argument(
        "--metrics-note",
        default="GT-free health only; no absolute accuracy claim.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    poses_path = Path(args.poses)
    values = np.loadtxt(poses_path, dtype=np.float64)
    if values.ndim == 1:
        values = values.reshape(1, -1)
    if values.shape[1] != 12:
        raise RuntimeError(f"Expected 12 KITTI pose values per row: {poses_path}")

    poses = values.reshape(-1, 3, 4)
    rotations = poses[:, :, :3]
    translations = poses[:, :, 3]
    delta_translation = np.linalg.norm(np.diff(translations, axis=0), axis=1)
    relative_rotation = np.einsum(
        "nij,njk->nik",
        np.transpose(rotations[:-1], (0, 2, 1)),
        rotations[1:],
    )
    cos_angle = np.clip(
        (np.trace(relative_rotation, axis1=1, axis2=2) - 1.0) / 2.0,
        -1.0,
        1.0,
    )
    delta_rotation_deg = np.degrees(np.arccos(cos_angle))

    health = {
        "schema_version": 1,
        "poses": str(poses_path),
        "metrics_note": args.metrics_note,
        "frames": int(len(poses)),
        "finite_pose_fraction": float(
            np.isfinite(poses).all(axis=(1, 2)).mean()
        ),
        "path_length_m": float(delta_translation.sum()),
        "net_displacement_m": float(
            np.linalg.norm(translations[-1] - translations[0])
        ),
        "step_translation_m": {
            "median": float(np.median(delta_translation)),
            "p95": float(np.percentile(delta_translation, 95)),
            "max": float(delta_translation.max()),
            "threshold": args.max_step_translation_m,
            "violations": int(
                np.count_nonzero(
                    delta_translation > args.max_step_translation_m
                )
            ),
        },
        "step_rotation_deg": {
            "median": float(np.median(delta_rotation_deg)),
            "p95": float(np.percentile(delta_rotation_deg, 95)),
            "max": float(delta_rotation_deg.max()),
            "threshold": args.max_step_rotation_deg,
            "violations": int(
                np.count_nonzero(
                    delta_rotation_deg > args.max_step_rotation_deg
                )
            ),
        },
    }
    health["health_gate"] = (
        "pass"
        if health["finite_pose_fraction"] == 1.0
        and health["step_translation_m"]["violations"] == 0
        and health["step_rotation_deg"]["violations"] == 0
        else "fail"
    )

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(health, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(health, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
