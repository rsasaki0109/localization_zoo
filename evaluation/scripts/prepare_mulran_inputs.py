#!/usr/bin/env python3

"""Prepare MulRan Ouster bins + global_pose.csv for pcd_dogfooding manifests."""

from __future__ import annotations

import argparse
import csv
import importlib.util
import sys
from pathlib import Path

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_DIR = Path(__file__).resolve().parent


def load_mulran_pose_io():
    spec = importlib.util.spec_from_file_location(
        "mulran_pose_io", SCRIPT_DIR / "mulran_pose_io.py"
    )
    if spec is None or spec.loader is None:
        raise RuntimeError("Failed to load mulran_pose_io.py")
    mod = importlib.util.module_from_spec(spec)
    sys.modules["mulran_pose_io"] = mod
    spec.loader.exec_module(mod)
    return mod


def load_kitti_bin_module():
    spec = importlib.util.spec_from_file_location(
        "kitti_bin_to_pcd_dir", SCRIPT_DIR / "kitti_bin_to_pcd_dir.py"
    )
    if spec is None or spec.loader is None:
        raise RuntimeError("Failed to load kitti_bin_to_pcd_dir.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Convert MulRan Ouster/*.bin + global_pose.csv into dogfooding_results/ and "
            "experiments/reference_data/ inputs (same layout as KITTI prep scripts)."
        )
    )
    parser.add_argument(
        "--sequence-root",
        required=True,
        help="MulRan sequence folder (e.g. .../DCC01) containing Ouster/ and global_pose.csv.",
    )
    parser.add_argument(
        "--lidar-subdir",
        default="Ouster",
        help="Subdirectory under sequence-root with Ouster OS1-64 .bin scans (default: Ouster).",
    )
    parser.add_argument(
        "--global-pose",
        default="global_pose.csv",
        help="Filename of global_pose.csv under sequence-root (default: global_pose.csv).",
    )
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First frame index (sorted .bin order) to export.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=120,
        help="Maximum frames to export for the window (default: 120). Use -1 for full sequence.",
    )
    parser.add_argument(
        "--include-full",
        action="store_true",
        help="Also export the full sequence to <stem>_full (uses all aligned bins/poses).",
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
    parser.add_argument(
        "--stem",
        default="",
        help="Output name stem (default: mulran_<sequence>_<N> from folder name and window size).",
    )
    return parser.parse_args()


def ensure_layout(sequence_root: Path, lidar_dir: Path, pose_path: Path) -> None:
    missing: list[str] = []
    if not lidar_dir.is_dir():
        missing.append(str(lidar_dir))
    if not pose_path.is_file():
        missing.append(str(pose_path))
    if missing:
        raise FileNotFoundError("Missing MulRan inputs:\n- " + "\n- ".join(missing))


def count_exported_frames(sequence_dir: Path) -> int:
    return sum(
        1
        for child in sequence_dir.iterdir()
        if child.is_dir() and child.name.isdigit() and len(child.name) == 8
    )


def _write_gt_csv(path: Path, aligned: list[object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "timestamp",
            "lidar_pose.x",
            "lidar_pose.y",
            "lidar_pose.z",
            "lidar_pose.roll",
            "lidar_pose.pitch",
            "lidar_pose.yaw",
        ])
        for p in aligned:
            ts_sec = p.timestamp_ns * 1e-9
            writer.writerow([
                f"{ts_sec:.10f}",
                f"{p.x:.10f}",
                f"{p.y:.10f}",
                f"{p.z:.10f}",
                f"{p.roll:.10f}",
                f"{p.pitch:.10f}",
                f"{p.yaw:.10f}",
            ])


def export_window(
    *,
    sequence_root: Path,
    lidar_dir: Path,
    pose_path: Path,
    output_root: Path,
    reference_data_dir: Path,
    start_frame: int,
    max_frames: int | None,
    stem: str,
    kb: object,
    pose_io: object,
) -> tuple[str, Path, Path]:
    bin_files = sorted(lidar_dir.glob("*.bin"))
    if not bin_files:
        raise RuntimeError(f"No .bin files in {lidar_dir}")

    traj = pose_io.load_global_pose_rows(pose_path)
    if not traj:
        raise RuntimeError(f"No poses parsed from {pose_path}")

    n_bins = len(bin_files)
    end_idx = n_bins if max_frames is None or max_frames < 0 else min(start_frame + max_frames, n_bins)
    selected_bins = bin_files[start_frame:end_idx]
    if not selected_bins:
        raise RuntimeError(f"No frames in range [{start_frame}, {end_idx})")

    if len(traj) != len(bin_files):
        print(
            f"[info] global_pose rows ({len(traj)}) != .bin count ({len(bin_files)}); "
            "using nearest timestamp match per LiDAR frame.",
            file=sys.stderr,
        )

    output_dir = output_root / stem
    output_dir.mkdir(parents=True, exist_ok=True)
    gt_csv = reference_data_dir / f"{stem}_gt.csv"

    aligned: list[object] = []
    timestamps_path = output_dir / "frame_timestamps.csv"
    with timestamps_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        for i, bin_path in enumerate(selected_bins):
            points = kb.load_kitti_bin(bin_path)
            packed = np.empty(
                len(points),
                dtype=np.dtype(
                    [
                        ("x", "<f4"),
                        ("y", "<f4"),
                        ("z", "<f4"),
                        ("intensity", "<f4"),
                    ]
                ),
            )
            packed["x"] = points[:, 0]
            packed["y"] = points[:, 1]
            packed["z"] = points[:, 2]
            packed["intensity"] = points[:, 3]

            frame_dir = output_dir / f"{i:08d}"
            frame_dir.mkdir(parents=True, exist_ok=True)
            kb.write_binary_pcd(frame_dir / "cloud.pcd", packed)

            ts_ns = int(bin_path.stem)
            pose = pose_io.nearest_pose(ts_ns, traj)
            aligned.append(
                pose_io.MulRanPoseRow(
                    ts_ns,
                    pose.x,
                    pose.y,
                    pose.z,
                    pose.roll,
                    pose.pitch,
                    pose.yaw,
                )
            )
            ts_sec = ts_ns * 1e-9
            writer.writerow([i, f"{ts_sec:.10f}", len(packed)])

    _write_gt_csv(gt_csv, aligned)

    return stem, output_dir, gt_csv


def main() -> int:
    args = parse_args()
    sequence_root = Path(args.sequence_root).resolve()
    lidar_dir = sequence_root / args.lidar_subdir
    pose_path = sequence_root / args.global_pose
    output_root = Path(args.output_root)
    reference_data_dir = Path(args.reference_data_dir)

    ensure_layout(sequence_root, lidar_dir, pose_path)
    output_root.mkdir(parents=True, exist_ok=True)
    reference_data_dir.mkdir(parents=True, exist_ok=True)

    kb = load_kitti_bin_module()
    pose_io = load_mulran_pose_io()

    raw_name = sequence_root.name.lower().replace(" ", "_")
    if raw_name.startswith("mulran_"):
        seq_slug = raw_name
    else:
        seq_slug = f"mulran_{raw_name}"
    window_n = args.max_frames if args.max_frames >= 0 else "full"

    exports: list[tuple[str, Path, Path]] = []

    if args.stem:
        stem_w = args.stem
    else:
        stem_w = f"{seq_slug}_{window_n}"

    exports.append(
        export_window(
            sequence_root=sequence_root,
            lidar_dir=lidar_dir,
            pose_path=pose_path,
            output_root=output_root,
            reference_data_dir=reference_data_dir,
            start_frame=args.start_frame,
            max_frames=args.max_frames,
            stem=stem_w,
            kb=kb,
            pose_io=pose_io,
        )
    )

    if args.include_full:
        stem_f = f"{seq_slug}_full"
        exports.append(
            export_window(
                sequence_root=sequence_root,
                lidar_dir=lidar_dir,
                pose_path=pose_path,
                output_root=output_root,
                reference_data_dir=reference_data_dir,
                start_frame=0,
                max_frames=-1,
                stem=stem_f,
                kb=kb,
                pose_io=pose_io,
            )
        )

    print("=== MulRan exports ===")
    for stem, sequence_dir, gt_csv in exports:
        gt_lines = sum(1 for _ in gt_csv.open()) - 1
        pcd_frames = count_exported_frames(sequence_dir)
        print(f"{stem}: {pcd_frames} PCD frames, {gt_lines} GT poses")

    print("")
    print("Next steps:")
    print("  1. Build pcd_dogfooding, then smoke-test:")
    print(
        f"     ./build/evaluation/pcd_dogfooding {exports[0][1]} "
        f"{exports[0][2]} --methods litamin2 --summary-json /tmp/mulran_smoke.json"
    )
    print(
        "  2. Run pending manifests under experiments/pending/ once data is local "
        "(see evaluation/scripts/SETUP_MULRAN_BENCHMARK.md)."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
