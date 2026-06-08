#!/usr/bin/env python3
"""Download KITTI Raw color PNG windows for PL-LOAM/VLOM RGB evaluation.

Extracts image_02 PNGs for benchmark frame indices listed in frame_timestamps.csv.
Uses the public KITTI Raw sync zip (requires ~1–3 GB download per drive).
"""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
import tarfile
import zipfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
S3_BASE = "https://s3.eu-central-1.amazonaws.com/avg-kitti/raw_data"

SEQUENCE_DRIVE = {
    "kitti_raw_0009_200": ("2011_09_26", "0009"),
    "kitti_raw_0009_full": ("2011_09_26", "0009"),
    "kitti_raw_0061_200": ("2011_09_26", "0061"),
    "kitti_raw_0061_full": ("2011_09_26", "0061"),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sequence-dir",
        required=True,
        help="dogfooding_results/kitti_raw_* directory",
    )
    parser.add_argument(
        "--kitti-root",
        default=str(REPO_ROOT.parent / "_kitti"),
        help="Where to extract drive_*_sync folders",
    )
    parser.add_argument("--camera-dir", default="image_02/data")
    parser.add_argument(
        "--force",
        action="store_true",
        help="Re-download even if PNGs already exist",
    )
    return parser.parse_args()


def load_frame_indices(sequence_dir: Path) -> list[int]:
    csv_path = sequence_dir / "frame_timestamps.csv"
    if not csv_path.exists():
        raise FileNotFoundError(f"Missing {csv_path}")
    indices: list[int] = []
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            ts = float(row.get("timestamp", row.get("frame_idx", "0")))
            indices.append(int(round(ts)))
    return indices


def infer_sequence_key(sequence_dir: Path) -> str:
    name = sequence_dir.name
    if name in SEQUENCE_DRIVE:
        return name
    match = re.search(r"kitti_raw_(\d{4})_", name)
    if not match:
        raise RuntimeError(f"Cannot infer drive from {name}")
    drive = match.group(1)
    for key, (_, d) in SEQUENCE_DRIVE.items():
        if d == drive and name.endswith(key.split("_", 3)[-1]):
            return key
    return f"kitti_raw_{drive}_200"


def download_sync_zip(date: str, drive: str, dst: Path) -> Path:
    folder = f"{date}_drive_{drive}"
    zip_name = f"{folder}_sync.zip"
    url = f"{S3_BASE}/{folder}/{zip_name}"
    dst.parent.mkdir(parents=True, exist_ok=True)
    if not dst.exists():
        print(f"[download] {url} -> {dst}", flush=True)
        subprocess.run(
            ["curl", "-L", "--continue-at", "-", "-o", str(dst), url],
            check=True,
        )
    return dst


def extract_color_pngs(zip_path: Path, out_dir: Path, frame_indices: set[int]) -> int:
    out_dir.mkdir(parents=True, exist_ok=True)
    extracted = 0
    prefix = "image_02/data/"
    opener = zipfile.ZipFile if zip_path.suffix == ".zip" else tarfile.open
    with opener(zip_path) as archive:
        members = archive.namelist() if hasattr(archive, "namelist") else archive.getnames()
        wanted = {
            m for m in members if m.endswith(".png") and prefix in m.replace("\\", "/")
        }
        for member in sorted(wanted):
            name = Path(member.replace("\\", "/")).name
            frame_idx = int(name.split(".")[0])
            if frame_idx not in frame_indices:
                continue
            target = out_dir / name
            if target.exists():
                extracted += 1
                continue
            if hasattr(archive, "open"):
                with archive.open(member) as src, target.open("wb") as dst:
                    dst.write(src.read())
            else:
                archive.extract(member, path=out_dir.parent)
                src = out_dir.parent / member.replace("\\", "/")
                if src != target:
                    src.rename(target)
            extracted += 1
    return extracted


def main() -> int:
    args = parse_args()
    sequence_dir = (REPO_ROOT / args.sequence_dir).resolve()
    if not sequence_dir.is_dir():
        raise FileNotFoundError(sequence_dir)

    key = infer_sequence_key(sequence_dir)
    date, drive = SEQUENCE_DRIVE[key]
    frame_indices = set(load_frame_indices(sequence_dir))
    kitti_root = Path(args.kitti_root)
    drive_dir = kitti_root / date / f"{date}_drive_{drive}_sync"
    png_dir = drive_dir / args.camera_dir

    missing = [i for i in sorted(frame_indices) if not (png_dir / f"{i:010d}.png").exists()]
    if not args.force and not missing:
        print(f"[skip] all {len(frame_indices)} PNGs present under {png_dir}", flush=True)
        print(f"[hint] --pl-loam-rgb-root {drive_dir}", flush=True)
        return 0

    zip_path = kitti_root / f"{date}_drive_{drive}_sync.zip"
    download_sync_zip(date, drive, zip_path)
    count = extract_color_pngs(zip_path, png_dir, frame_indices)
    print(f"[done] {count} PNGs under {png_dir}", flush=True)
    print(f"[hint] --pl-loam-rgb-root {drive_dir}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
