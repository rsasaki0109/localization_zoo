#!/usr/bin/env python3

"""Select GT-free LiDAR degradation windows from per-frame diagnostics."""

from __future__ import annotations

import argparse
import csv
import json
import math
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class FrameDiag:
    frame: int
    points: float
    intensity_mean: float
    intensity_std: float
    range_p95_m: float
    scattering: float
    degeneracy_score: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("frame_degeneracy_csv", type=Path)
    parser.add_argument("output_dir", type=Path)
    parser.add_argument("--window-size", type=int, default=30)
    parser.add_argument("--stride", type=int, default=5)
    parser.add_argument(
        "--source-pcd-dir",
        type=Path,
        default=Path("dogfooding_results/lidar_degeneracy_fog_200"),
        help="PCD directory written into the output manifest.",
    )
    return parser.parse_args()


def load_frames(path: Path) -> list[FrameDiag]:
    rows: list[FrameDiag] = []
    with path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            rows.append(
                FrameDiag(
                    frame=int(row["frame"]),
                    points=float(row["points"]),
                    intensity_mean=float(row["intensity_mean"]),
                    intensity_std=float(row["intensity_std"]),
                    range_p95_m=float(row["range_p95_m"]),
                    scattering=float(row["scattering"]),
                    degeneracy_score=float(row["degeneracy_score"]),
                )
            )
    if not rows:
        raise RuntimeError(f"No frame diagnostics found: {path}")
    return rows


def median(values: list[float]) -> float:
    ordered = sorted(values)
    mid = len(ordered) // 2
    if len(ordered) % 2:
        return ordered[mid]
    return 0.5 * (ordered[mid - 1] + ordered[mid])


def percentile(values: list[float], q: float) -> float:
    ordered = sorted(values)
    if not ordered:
        return float("nan")
    pos = (len(ordered) - 1) * q
    lo = math.floor(pos)
    hi = math.ceil(pos)
    if lo == hi:
        return ordered[lo]
    return ordered[lo] * (hi - pos) + ordered[hi] * (pos - lo)


def mean(values: list[float]) -> float:
    return sum(values) / len(values)


def window_stats(frames: list[FrameDiag], start: int, size: int, baseline: dict[str, float]) -> dict[str, float | int]:
    chunk = frames[start : start + size]
    points = [row.points for row in chunk]
    intensity = [row.intensity_mean for row in chunk]
    intensity_std = [row.intensity_std for row in chunk]
    range_p95 = [row.range_p95_m for row in chunk]
    degeneracy = [row.degeneracy_score for row in chunk]
    scattering = [row.scattering for row in chunk]
    point_drop = 1.0 - mean(points) / baseline["points"]
    intensity_drop = 1.0 - mean(intensity) / baseline["intensity_mean"]
    range_drop = 1.0 - mean(range_p95) / baseline["range_p95_m"]
    obscurant_score = max(0.0, point_drop) + max(0.0, intensity_drop) + 0.5 * max(0.0, range_drop)
    geometry_score = mean(degeneracy) + max(0.0, 0.20 - mean(scattering))
    return {
        "start": chunk[0].frame,
        "end": chunk[-1].frame,
        "frames": len(chunk),
        "points_mean": mean(points),
        "points_p05": percentile(points, 0.05),
        "intensity_mean": mean(intensity),
        "intensity_std_mean": mean(intensity_std),
        "range_p95_mean_m": mean(range_p95),
        "degeneracy_score_mean": mean(degeneracy),
        "degeneracy_score_p95": percentile(degeneracy, 0.95),
        "scattering_mean": mean(scattering),
        "geometry_score": geometry_score,
        "point_drop_vs_baseline": point_drop,
        "intensity_drop_vs_baseline": intensity_drop,
        "range_p95_drop_vs_baseline": range_drop,
        "obscurant_score": obscurant_score,
    }


def choose_windows(windows: list[dict[str, float | int]]) -> dict[str, dict[str, float | int]]:
    if not windows:
        raise RuntimeError("No windows available")
    return {
        "baseline": min(windows, key=lambda item: float(item["obscurant_score"])),
        "degraded": max(windows, key=lambda item: float(item["obscurant_score"])),
        "point_count_tail": min(windows, key=lambda item: float(item["points_mean"])),
        "intensity_tail": min(windows, key=lambda item: float(item["intensity_mean"])),
        "geometry_degeneracy": max(windows, key=lambda item: float(item["geometry_score"])),
    }


def write_markdown(path: Path, selected: dict[str, dict[str, float | int]], top_windows: list[dict[str, float | int]]) -> None:
    lines = [
        "# LiDAR Degradation Windows",
        "",
        "Selected windows:",
        "",
        "| Name | Start | End | Points mean | Intensity mean | Degeneracy p95 | Scattering mean | Obscurant | Geometry |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for name, row in selected.items():
        lines.append(
            f"| `{name}` | {row['start']} | {row['end']} | "
            f"{float(row['points_mean']):.1f} | {float(row['intensity_mean']):.1f} | "
            f"{float(row['degeneracy_score_p95']):.3f} | {float(row['scattering_mean']):.3f} | "
            f"{float(row['obscurant_score']):.3f} | {float(row['geometry_score']):.3f} |"
        )
    lines.extend(
        [
            "",
            "Top obscurant windows:",
            "",
            "| Rank | Start | End | Point drop | Intensity drop | Range drop | Score |",
            "|---:|---:|---:|---:|---:|---:|---:|",
        ]
    )
    for rank, row in enumerate(top_windows, start=1):
        lines.append(
            f"| {rank} | {row['start']} | {row['end']} | "
            f"{float(row['point_drop_vs_baseline']):.3f} | "
            f"{float(row['intensity_drop_vs_baseline']):.3f} | "
            f"{float(row['range_p95_drop_vs_baseline']):.3f} | "
            f"{float(row['obscurant_score']):.3f} |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    frames = load_frames(args.frame_degeneracy_csv)
    if args.window_size <= 0 or args.stride <= 0:
        raise ValueError("window-size and stride must be positive")
    if len(frames) < args.window_size:
        raise RuntimeError(f"Need at least {args.window_size} frames; got {len(frames)}")

    baseline_slice = frames[: min(len(frames), max(args.window_size, 30))]
    baseline = {
        "points": median([row.points for row in baseline_slice]),
        "intensity_mean": median([row.intensity_mean for row in baseline_slice]),
        "range_p95_m": median([row.range_p95_m for row in baseline_slice]),
    }
    windows = [
        window_stats(frames, start, args.window_size, baseline)
        for start in range(0, len(frames) - args.window_size + 1, args.stride)
    ]
    selected = choose_windows(windows)
    top_windows = sorted(windows, key=lambda item: float(item["obscurant_score"]), reverse=True)[:10]

    payload = {
        "source_csv": str(args.frame_degeneracy_csv),
        "source_pcd_dir": str(args.source_pcd_dir),
        "window_size": args.window_size,
        "stride": args.stride,
        "baseline": baseline,
        "selected": selected,
        "top_obscurant_windows": top_windows,
        "all_windows": windows,
    }
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / "degradation_windows.json").write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(args.output_dir / "degradation_windows.md", selected, top_windows)
    print(f"[done] windows={len(windows)} output={args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
