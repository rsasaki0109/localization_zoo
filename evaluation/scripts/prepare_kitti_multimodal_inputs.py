#!/usr/bin/env python3

"""Prepare real-image multimodal inputs for standard KITTI benchmark windows."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]

SEQUENCE_CONFIG = {
    "kitti_raw_0009_200": {
        "sequence_dir": "dogfooding_results/kitti_raw_0009_200",
        "gt_csv": "experiments/reference_data/kitti_raw_0009_200_gt.csv",
    },
    "kitti_raw_0061_200": {
        "sequence_dir": "dogfooding_results/kitti_raw_0061_200",
        "gt_csv": "experiments/reference_data/kitti_raw_0061_200_gt.csv",
    },
    "kitti_raw_0009_full": {
        "sequence_dir": "dogfooding_results/kitti_raw_0009_full",
        "gt_csv": "experiments/reference_data/kitti_raw_0009_full_gt.csv",
    },
    "kitti_raw_0061_full": {
        "sequence_dir": "dogfooding_results/kitti_raw_0061_full",
        "gt_csv": "experiments/reference_data/kitti_raw_0061_full_gt.csv",
    },
}

DEFAULT_SEQUENCES = ("kitti_raw_0009_200", "kitti_raw_0061_200")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate real-image landmarks.csv and visual_observations.csv for KITTI windows.",
    )
    parser.add_argument(
        "--sequence",
        action="append",
        choices=sorted(SEQUENCE_CONFIG),
        default=[],
        help=(
            "Sequence key to prepare. Can be repeated. "
            "Defaults to the two 200-frame benchmark windows."
        ),
    )
    parser.add_argument(
        "--include-full",
        action="store_true",
        help="When no --sequence is given, also prepare the two full KITTI windows.",
    )
    parser.add_argument(
        "--kitti-root",
        default="/tmp/kitti_real",
        help="KITTI Raw root passed through to generate_kitti_visual_observations.py.",
    )
    parser.add_argument(
        "--camera-dir",
        default="image_02",
        help="KITTI camera folder under each drive dir.",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Regenerate outputs even if landmarks.csv and visual_observations.csv already exist.",
    )
    return parser.parse_args()


def output_paths(sequence_dir: Path) -> list[Path]:
    return [
        sequence_dir / "landmarks.csv",
        sequence_dir / "visual_observations.csv",
        sequence_dir / "camera_args.txt",
    ]


def selected_sequences(args: argparse.Namespace) -> list[str]:
    if args.sequence:
        return list(args.sequence)

    sequences = list(DEFAULT_SEQUENCES)
    if args.include_full:
        sequences.extend(["kitti_raw_0009_full", "kitti_raw_0061_full"])
    return sequences


def build_command(config: dict[str, str], args: argparse.Namespace) -> list[str]:
    return [
        sys.executable,
        "evaluation/scripts/generate_kitti_visual_observations.py",
        "--sequence-dir",
        config["sequence_dir"],
        "--gt-csv",
        config["gt_csv"],
        "--kitti-root",
        args.kitti_root,
        "--camera-dir",
        args.camera_dir,
    ]


def main() -> int:
    args = parse_args()
    prepared = 0
    skipped = 0

    for sequence_key in selected_sequences(args):
        config = SEQUENCE_CONFIG[sequence_key]
        sequence_dir = REPO_ROOT / config["sequence_dir"]
        outputs = output_paths(sequence_dir)

        if not args.force and all(path.exists() for path in outputs):
            print(
                f"[skip] {sequence_key}: outputs already exist under {sequence_dir}",
                flush=True,
            )
            skipped += 1
            continue

        command = build_command(config, args)
        print(f"[run] {sequence_key}: {' '.join(command)}", flush=True)
        subprocess.run(command, cwd=REPO_ROOT, check=True)
        prepared += 1

    print(f"[done] prepared={prepared} skipped={skipped}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
