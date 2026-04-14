#!/usr/bin/env python3

"""Prepare KITTI Odometry inputs for pcd_dogfooding manifests."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_DIR = Path(__file__).resolve().parent


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Convert KITTI Odometry velodyne + poses into dogfooding_results/ and "
            "experiments/reference_data/ inputs for benchmark manifests."
        )
    )
    parser.add_argument(
        "--kitti-root",
        required=True,
        help="Directory containing KITTI odometry sequences/ and poses/.",
    )
    parser.add_argument(
        "--sequence",
        action="append",
        default=[],
        help="Sequence id to export (default: 00 and 07). Can be passed multiple times.",
    )
    parser.add_argument(
        "--window-size",
        action="append",
        type=int,
        default=[],
        help=(
            "Export the first N frames to kitti_seq_<seq>_<N>. "
            "Defaults to 108 when omitted."
        ),
    )
    parser.add_argument(
        "--include-full",
        action="store_true",
        help="Also export the full sequence to kitti_seq_<seq>_full.",
    )
    parser.add_argument(
        "--output-root",
        default=str(REPO_ROOT / "dogfooding_results"),
        help="Root directory for PCD sequence outputs.",
    )
    parser.add_argument(
        "--reference-data-dir",
        default=str(REPO_ROOT / "experiments" / "reference_data"),
        help="Directory for generated GT CSV files.",
    )
    return parser.parse_args()


def ensure_kitti_layout(kitti_root: Path, sequences: list[str]) -> None:
    missing: list[str] = []
    for seq in sequences:
        velodyne_dir = kitti_root / "sequences" / seq / "velodyne"
        poses_file = kitti_root / "poses" / f"{seq}.txt"
        if not velodyne_dir.is_dir():
            missing.append(str(velodyne_dir))
        if not poses_file.is_file():
            missing.append(str(poses_file))
    if missing:
        raise FileNotFoundError(
            "Missing KITTI Odometry inputs:\n- " + "\n- ".join(missing)
        )


def run(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True)


def artifact_stem(sequence: str, *, full: bool, window_size: int | None) -> str:
    if full:
        return f"kitti_seq_{sequence}_full"
    if window_size is None or window_size <= 0:
        raise ValueError("window_size must be positive for non-full exports.")
    return f"kitti_seq_{sequence}_{window_size}"


def export_sequence(
    *,
    kitti_root: Path,
    sequence: str,
    output_root: Path,
    reference_data_dir: Path,
    full: bool,
    window_size: int | None,
) -> tuple[str, Path, Path]:
    stem = artifact_stem(sequence, full=full, window_size=window_size)
    velodyne_dir = kitti_root / "sequences" / sequence / "velodyne"
    poses_file = kitti_root / "poses" / f"{sequence}.txt"
    output_dir = output_root / stem
    gt_csv = reference_data_dir / f"{stem}_gt.csv"
    max_frames = -1 if full else int(window_size or -1)

    bin_cmd = [
        sys.executable,
        str(SCRIPT_DIR / "kitti_bin_to_pcd_dir.py"),
        "--velodyne-dir",
        str(velodyne_dir),
        "--output-dir",
        str(output_dir),
        "--start-frame",
        "0",
        "--max-frames",
        str(max_frames),
    ]
    gt_cmd = [
        sys.executable,
        str(SCRIPT_DIR / "kitti_poses_to_gt_csv.py"),
        "--poses-file",
        str(poses_file),
        "--output",
        str(gt_csv),
        "--start-frame",
        "0",
        "--max-frames",
        str(max_frames),
    ]
    run(bin_cmd)
    run(gt_cmd)
    return stem, output_dir, gt_csv


def count_exported_frames(sequence_dir: Path) -> int:
    return sum(
        1
        for child in sequence_dir.iterdir()
        if child.is_dir() and child.name.isdigit() and len(child.name) == 8
    )


def main() -> int:
    args = parse_args()
    kitti_root = Path(args.kitti_root)
    sequences = args.sequence or ["00", "07"]
    window_sizes = args.window_size or [108]
    output_root = Path(args.output_root)
    reference_data_dir = Path(args.reference_data_dir)

    ensure_kitti_layout(kitti_root, sequences)
    output_root.mkdir(parents=True, exist_ok=True)
    reference_data_dir.mkdir(parents=True, exist_ok=True)

    exports: list[tuple[str, Path, Path]] = []
    for sequence in sequences:
        for window_size in window_sizes:
            exports.append(
                export_sequence(
                    kitti_root=kitti_root,
                    sequence=sequence,
                    output_root=output_root,
                    reference_data_dir=reference_data_dir,
                    full=False,
                    window_size=window_size,
                )
            )
        if args.include_full:
            exports.append(
                export_sequence(
                    kitti_root=kitti_root,
                    sequence=sequence,
                    output_root=output_root,
                    reference_data_dir=reference_data_dir,
                    full=True,
                    window_size=None,
                )
            )

    print("=== KITTI Odometry exports ===")
    for stem, sequence_dir, gt_csv in exports:
        gt_lines = sum(1 for _ in gt_csv.open()) - 1
        pcd_frames = count_exported_frames(sequence_dir)
        print(f"{stem}: {pcd_frames} PCD frames, {gt_lines} GT poses")

    print("")
    print("Next steps:")
    print("  1. Build:  cmake -B build && cmake --build build -j$(nproc)")
    print(
        "  2. Run:    python3 evaluation/scripts/run_experiment_matrix.py "
        "--manifest experiments/pending/<method>_kitti_seq_<seq>_matrix.json"
    )
    print(
        "  3. Full:   rerun with --include-full and point manifests at "
        "kitti_seq_<seq>_full when promoting full-sequence studies"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
