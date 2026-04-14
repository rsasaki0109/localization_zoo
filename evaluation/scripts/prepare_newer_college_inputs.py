#!/usr/bin/env python3

"""Prepare Newer College flat-file LiDAR (PCD) + GT CSV for pcd_dogfooding."""

from __future__ import annotations

import argparse
import csv
import importlib.util
import shutil
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_DIR = Path(__file__).resolve().parent


def load_pose_io():
    spec = importlib.util.spec_from_file_location(
        "newer_college_pose_io", SCRIPT_DIR / "newer_college_pose_io.py"
    )
    if spec is None or spec.loader is None:
        raise RuntimeError("Failed to load newer_college_pose_io.py")
    mod = importlib.util.module_from_spec(spec)
    sys.modules["newer_college_pose_io"] = mod
    spec.loader.exec_module(mod)
    return mod


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Convert Newer College stereo-cam raw export (flat PCD + GT CSV) into "
            "dogfooding_results/<stem>/00000000/cloud.pcd and "
            "experiments/reference_data/<stem>_gt.csv."
        )
    )
    parser.add_argument(
        "--sequence-dir",
        required=True,
        help="Directory containing LiDAR PCDs and ground-truth CSV (see SETUP_NEWER_COLLEGE_BENCHMARK.md).",
    )
    parser.add_argument(
        "--lidar-subdir",
        default="os1_cloud_node/points",
        help="Subdirectory under sequence-dir with one .pcd per scan (default: os1_cloud_node/points).",
    )
    parser.add_argument(
        "--ground-truth",
        default="",
        help="Path to GT CSV (sec,nsec,x,y,z,qx,qy,qz,qw). Default: <sequence-dir>/ground_truth.csv",
    )
    parser.add_argument(
        "--start-frame",
        type=int,
        default=0,
        help="First frame index after sorting PCD filenames.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=120,
        help="Maximum frames to export (-1 for all).",
    )
    parser.add_argument(
        "--include-full",
        action="store_true",
        help="Also export the full sequence as <stem>_full.",
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
        help="Output stem (default: newer_college_<seq>_<N>).",
    )
    return parser.parse_args()


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
    sequence_dir: Path,
    lidar_dir: Path,
    gt_path: Path,
    output_root: Path,
    reference_data_dir: Path,
    start_frame: int,
    max_frames: int | None,
    stem: str,
    pose_io: object,
) -> tuple[str, Path, Path]:
    pcd_files = sorted(lidar_dir.glob("*.pcd"))
    if not pcd_files:
        raise RuntimeError(f"No .pcd files in {lidar_dir}")

    traj = pose_io.load_ground_truth_csv(gt_path)
    if not traj:
        raise RuntimeError(f"No poses parsed from {gt_path}")

    n_all = len(pcd_files)
    end_idx = n_all if max_frames is None or max_frames < 0 else min(start_frame + max_frames, n_all)
    selected = pcd_files[start_frame:end_idx]
    if not selected:
        raise RuntimeError(f"No frames in range [{start_frame}, {end_idx})")

    output_dir = output_root / stem
    output_dir.mkdir(parents=True, exist_ok=True)
    gt_csv = reference_data_dir / f"{stem}_gt.csv"

    aligned: list[object] = []
    ts_path = output_dir / "frame_timestamps.csv"
    with ts_path.open("w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["frame_idx", "timestamp", "points"])

        for i, src in enumerate(selected):
            stem_name = src.stem
            ts_ns = pose_io.parse_pcd_stem_to_timestamp_ns(stem_name)
            pose = pose_io.nearest_pose(ts_ns, traj)
            dst = output_dir / f"{i:08d}"
            dst.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst / "cloud.pcd")

            aligned.append(
                pose_io.NewerCollegePoseRow(
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
            writer.writerow([i, f"{ts_sec:.10f}", 0])

    _write_gt_csv(gt_csv, aligned)
    return stem, output_dir, gt_csv


def resolve_gt_path(sequence_dir: Path, ground_truth_arg: str) -> Path:
    if ground_truth_arg:
        p = Path(ground_truth_arg)
        return p if p.is_absolute() else (sequence_dir / p).resolve()
    for name in ("ground_truth.csv", "gt.csv", "lidar_gt.csv"):
        cand = sequence_dir / name
        if cand.is_file():
            return cand.resolve()
    raise FileNotFoundError(
        f"No ground-truth CSV found under {sequence_dir}. "
        "Pass --ground-truth with sec,nsec,x,y,z,qx,qy,qz,qw columns."
    )


def main() -> int:
    args = parse_args()
    sequence_dir = Path(args.sequence_dir).resolve()
    lidar_dir = (sequence_dir / args.lidar_subdir).resolve()
    gt_path = resolve_gt_path(sequence_dir, args.ground_truth)
    output_root = Path(args.output_root)
    reference_data_dir = Path(args.reference_data_dir)

    if not lidar_dir.is_dir():
        raise FileNotFoundError(f"LiDAR directory not found: {lidar_dir}")
    if not gt_path.is_file():
        raise FileNotFoundError(f"Ground-truth CSV not found: {gt_path}")

    output_root.mkdir(parents=True, exist_ok=True)
    reference_data_dir.mkdir(parents=True, exist_ok=True)

    pose_io = load_pose_io()

    raw_name = sequence_dir.name.lower().replace(" ", "_")
    if raw_name.startswith("newer_college_"):
        seq_slug = raw_name
    else:
        seq_slug = f"newer_college_{raw_name}"
    window_n = args.max_frames if args.max_frames >= 0 else "full"

    exports: list[tuple[str, Path, Path]] = []

    stem_w = args.stem or f"{seq_slug}_{window_n}"
    exports.append(
        export_window(
            sequence_dir=sequence_dir,
            lidar_dir=lidar_dir,
            gt_path=gt_path,
            output_root=output_root,
            reference_data_dir=reference_data_dir,
            start_frame=args.start_frame,
            max_frames=args.max_frames,
            stem=stem_w,
            pose_io=pose_io,
        )
    )

    if args.include_full:
        stem_f = f"{seq_slug}_full"
        exports.append(
            export_window(
                sequence_dir=sequence_dir,
                lidar_dir=lidar_dir,
                gt_path=gt_path,
                output_root=output_root,
                reference_data_dir=reference_data_dir,
                start_frame=0,
                max_frames=-1,
                stem=stem_f,
                pose_io=pose_io,
            )
        )

    print("=== Newer College exports ===")
    for stem, seq_dir, gcsv in exports:
        gt_lines = sum(1 for _ in gcsv.open()) - 1
        pcd_frames = count_exported_frames(seq_dir)
        print(f"{stem}: {pcd_frames} PCD frames, {gt_lines} GT poses")

    print("")
    print("Next steps:")
    print(
        f"     ./build/evaluation/pcd_dogfooding {exports[0][1]} "
        f"{exports[0][2]} --methods litamin2 --summary-json /tmp/newer_college_smoke.json"
    )
    print("  See evaluation/scripts/SETUP_NEWER_COLLEGE_BENCHMARK.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
