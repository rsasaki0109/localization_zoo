#!/usr/bin/env python3
"""Verify the external-SSD dataset layout for the LiDAR odometry SOTA study."""

from __future__ import annotations

import argparse
import json
import os
import shutil
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REGISTRY = REPO_ROOT / "evaluation" / "data" / "lidar_odometry_sota_suite.json"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--data-root",
        default=os.environ.get("LOCALIZATION_ZOO_DATA_ROOT", ""),
        help="External dataset root. Defaults to LOCALIZATION_ZOO_DATA_ROOT.",
    )
    parser.add_argument(
        "--registry",
        default=str(DEFAULT_REGISTRY),
        help="Dataset suite registry JSON.",
    )
    parser.add_argument(
        "--require-role",
        action="append",
        default=[],
        choices=["development", "validation", "held_out", "robustness_only", "diagnostic"],
        help="Return non-zero when a dataset in this role is unavailable. Repeatable.",
    )
    parser.add_argument("--json", action="store_true", help="Emit machine-readable JSON.")
    return parser.parse_args()


def count_frames(path: Path, frame_format: str = "pcd_directory") -> int:
    if not path.is_dir():
        return 0
    if frame_format == "kitti_bin":
        return sum(
            child.is_file() and child.suffix == ".bin" and child.stem.isdigit()
            for child in path.iterdir()
        )
    if frame_format not in {"pcd", "pcd_directory"}:
        raise ValueError(f"Unsupported frame_format: {frame_format}")
    frame_directories = sum(
        child.is_dir()
        and child.name.isdigit()
        and len(child.name) == 8
        and (
            frame_format == "pcd_directory"
            or any(item.is_file() and item.suffix.lower() == ".pcd" for item in child.iterdir())
        )
        for child in path.iterdir()
    )
    if frame_directories:
        return frame_directories
    return sum(
        child.is_file() and child.suffix.lower() == ".pcd" and child.stem.isdigit()
        for child in path.iterdir()
    )


def inspect_dataset(entry: dict[str, Any], data_root: Path) -> dict[str, Any]:
    processed = data_root / entry["processed_dir"]
    reference_raw = entry.get("reference_csv")
    reference_external_raw = entry.get("reference_csv_external")
    reference = (
        REPO_ROOT / reference_raw
        if reference_raw
        else data_root / reference_external_raw
        if reference_external_raw
        else None
    )
    frame_format = entry.get("frame_format", "pcd_directory")
    frames = count_frames(processed, frame_format)
    expected = entry.get("expected_frames")
    processed_ok = processed.is_dir() and frames > 0
    frame_count_ok = expected is None or frames == expected
    reference_ok = reference is None or reference.is_file()
    available = processed_ok and frame_count_ok and reference_ok
    return {
        "id": entry["id"],
        "family": entry["family"],
        "role": entry["role"],
        "gt_backed": bool(entry["gt_backed"]),
        "available": available,
        "processed_dir": str(processed),
        "processed_exists": processed.is_dir(),
        "frame_format": frame_format,
        "frames": frames,
        "expected_frames": expected,
        "frame_count_ok": frame_count_ok,
        "reference_csv": str(reference) if reference else None,
        "reference_exists": reference_ok,
    }


def build_report(registry: dict[str, Any], data_root: Path) -> dict[str, Any]:
    usage = shutil.disk_usage(data_root)
    datasets = [inspect_dataset(entry, data_root) for entry in registry["datasets"]]
    return {
        "schema_version": 1,
        "data_root": str(data_root),
        "free_gb": round(usage.free / (1024**3), 2),
        "total_gb": round(usage.total / (1024**3), 2),
        "available_count": sum(row["available"] for row in datasets),
        "dataset_count": len(datasets),
        "datasets": datasets,
    }


def print_human(report: dict[str, Any]) -> None:
    print(
        f"LiDAR SOTA data root: {report['data_root']} "
        f"({report['free_gb']:.2f} GiB free / {report['total_gb']:.2f} GiB)"
    )
    print("")
    for row in report["datasets"]:
        status = "READY" if row["available"] else "MISSING"
        expected = row["expected_frames"]
        frame_text = f"{row['frames']} frames"
        if expected is not None:
            frame_text += f" / expected {expected}"
        details: list[str] = []
        if not row["processed_exists"]:
            details.append("processed directory")
        elif not row["frame_count_ok"]:
            details.append("frame count")
        if not row["reference_exists"]:
            details.append("reference CSV")
        suffix = f"; missing: {', '.join(details)}" if details else ""
        print(f"[{status}] {row['id']} ({row['role']}, {frame_text}){suffix}")
    print("")
    print(f"Ready: {report['available_count']}/{report['dataset_count']}")


def main() -> int:
    args = parse_args()
    if not args.data_root:
        raise SystemExit(
            "Set LOCALIZATION_ZOO_DATA_ROOT or pass --data-root "
            "(example on Windows: E:\\datasets\\loc_zoo)."
        )
    data_root = Path(os.path.expanduser(os.path.expandvars(args.data_root))).resolve()
    if not data_root.is_dir():
        raise SystemExit(f"Dataset root does not exist: {data_root}")
    registry_path = Path(args.registry)
    if not registry_path.is_absolute():
        registry_path = REPO_ROOT / registry_path
    registry = json.loads(registry_path.read_text())
    report = build_report(registry, data_root)
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print_human(report)

    required_roles = set(args.require_role)
    missing_required = [
        row for row in report["datasets"]
        if row["role"] in required_roles and not row["available"]
    ]
    return 1 if missing_required else 0


if __name__ == "__main__":
    raise SystemExit(main())
