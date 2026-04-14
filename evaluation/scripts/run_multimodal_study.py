#!/usr/bin/env python3

"""Prepare KITTI multimodal inputs and run the selected multimodal manifests."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
METHODS = (
    "vins_fusion",
    "okvis",
    "orb_slam3",
    "lvi_sam",
    "fast_livo2",
    "r2live",
)
SEQUENCE_CONFIG = {
    "kitti_raw_0009_200": "kitti_raw_0009_matrix.json",
    "kitti_raw_0061_200": "kitti_raw_0061_matrix.json",
    "kitti_raw_0009_full": "kitti_raw_0009_full_matrix.json",
    "kitti_raw_0061_full": "kitti_raw_0061_full_matrix.json",
}
DEFAULT_SEQUENCES = ("kitti_raw_0009_200", "kitti_raw_0061_200")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run the multimodal KITTI study with optional real-image input preparation.",
    )
    parser.add_argument(
        "--method",
        action="append",
        choices=METHODS,
        default=[],
        help="Method family to include. Can be repeated. Defaults to all multimodal methods.",
    )
    parser.add_argument(
        "--sequence",
        action="append",
        choices=sorted(SEQUENCE_CONFIG),
        default=[],
        help=(
            "KITTI window to include. Can be repeated. "
            "Defaults to the two 200-frame windows unless --include-full is used."
        ),
    )
    parser.add_argument(
        "--include-full",
        action="store_true",
        help="When no --sequence is given, also include the two full KITTI windows.",
    )
    parser.add_argument(
        "--skip-prepare-inputs",
        action="store_true",
        help="Skip running prepare_kitti_multimodal_inputs.py before the study.",
    )
    parser.add_argument(
        "--kitti-root",
        default="/tmp/kitti_real",
        help="KITTI Raw root passed through when preparing real-image inputs.",
    )
    parser.add_argument(
        "--camera-dir",
        default="image_02",
        help="KITTI camera folder passed through when preparing real-image inputs.",
    )
    parser.add_argument(
        "--output-dir",
        default="/tmp/localization_zoo_multimodal_results",
        help="Aggregate output dir passed to run_experiment_matrix.py.",
    )
    parser.add_argument(
        "--docs-dir",
        default="/tmp/localization_zoo_multimodal_docs",
        help="Docs output dir passed to run_experiment_matrix.py.",
    )
    parser.add_argument(
        "--reuse-existing",
        action="store_true",
        help="Reuse existing per-variant summaries when present.",
    )
    parser.add_argument(
        "--variant-timeout-seconds",
        type=float,
        default=None,
        help="Optional timeout override passed through to run_experiment_matrix.py.",
    )
    parser.add_argument(
        "--list-manifests",
        action="store_true",
        help="Print the selected manifests and exit without preparing or running.",
    )
    return parser.parse_args()


def selected_methods(args: argparse.Namespace) -> list[str]:
    return list(args.method) if args.method else list(METHODS)


def selected_sequences(args: argparse.Namespace) -> list[str]:
    if args.sequence:
        return list(args.sequence)

    sequences = list(DEFAULT_SEQUENCES)
    if args.include_full:
        sequences.extend(["kitti_raw_0009_full", "kitti_raw_0061_full"])
    return sequences


def manifest_paths(methods: list[str], sequences: list[str]) -> list[str]:
    paths = []
    for method in methods:
        for sequence in sequences:
            path = Path("experiments") / f"{method}_{SEQUENCE_CONFIG[sequence]}"
            if not (REPO_ROOT / path).exists():
                raise FileNotFoundError(f"Missing multimodal manifest: {path}")
            paths.append(str(path))
    return paths


def run(cmd: list[str]) -> None:
    print("[run]", " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def main() -> int:
    args = parse_args()
    methods = selected_methods(args)
    sequences = selected_sequences(args)
    manifests = manifest_paths(methods, sequences)

    if args.list_manifests:
        for manifest in manifests:
            print(manifest)
        return 0

    if not args.skip_prepare_inputs:
        prepare_cmd = [
            sys.executable,
            "evaluation/scripts/prepare_kitti_multimodal_inputs.py",
            "--kitti-root",
            args.kitti_root,
            "--camera-dir",
            args.camera_dir,
        ]
        for sequence in sequences:
            prepare_cmd.extend(["--sequence", sequence])
        run(prepare_cmd)

    runner_cmd = [
        sys.executable,
        "evaluation/scripts/run_experiment_matrix.py",
        "--output-dir",
        args.output_dir,
        "--docs-dir",
        args.docs_dir,
    ]
    if args.reuse_existing:
        runner_cmd.append("--reuse-existing")
    if args.variant_timeout_seconds is not None:
        runner_cmd.extend(
            ["--variant-timeout-seconds", str(args.variant_timeout_seconds)]
        )
    for manifest in manifests:
        runner_cmd.extend(["--manifest", manifest])
    run(runner_cmd)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
