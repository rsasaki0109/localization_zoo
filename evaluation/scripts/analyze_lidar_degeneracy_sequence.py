#!/usr/bin/env python3

"""Compute GT-free LiDAR degeneracy diagnostics from dogfooding PCD frames."""

from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path

import numpy as np


PCD_TYPE_TO_DTYPE = {
    ("F", 4): "<f4",
    ("F", 8): "<f8",
    ("I", 1): "<i1",
    ("I", 2): "<i2",
    ("I", 4): "<i4",
    ("I", 8): "<i8",
    ("U", 1): "<u1",
    ("U", 2): "<u2",
    ("U", 4): "<u4",
    ("U", 8): "<u8",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pcd_dir", type=Path, help="dogfooding_results sequence directory.")
    parser.add_argument("output_dir", type=Path, help="Directory for CSV/JSON/Markdown outputs.")
    parser.add_argument(
        "--max-frames",
        type=int,
        default=-1,
        help="Maximum frames to analyze. -1 means all contiguous frames.",
    )
    parser.add_argument(
        "--sample-points",
        type=int,
        default=50000,
        help="Deterministic stride sample cap per frame for covariance diagnostics.",
    )
    return parser.parse_args()


def collect_pcds(pcd_dir: Path, max_frames: int) -> list[Path]:
    paths: list[Path] = []
    for idx in range(10_000_000):
        if max_frames >= 0 and len(paths) >= max_frames:
            break
        path = pcd_dir / f"{idx:08d}" / "cloud.pcd"
        if not path.is_file():
            break
        paths.append(path)
    if not paths:
        raise FileNotFoundError(f"No contiguous 00000000/cloud.pcd frames found under {pcd_dir}")
    return paths


def read_pcd_header(path: Path) -> tuple[dict[str, list[str]], int]:
    values: dict[str, list[str]] = {}
    offset = 0
    with path.open("rb") as handle:
        while True:
            line = handle.readline()
            if not line:
                raise RuntimeError(f"PCD header missing DATA line: {path}")
            offset += len(line)
            text = line.decode("ascii", errors="replace").strip()
            if not text or text.startswith("#"):
                continue
            parts = text.split()
            key = parts[0].upper()
            values[key] = parts[1:]
            if key == "DATA":
                return values, offset


def pcd_dtype(header: dict[str, list[str]]) -> np.dtype:
    fields = header.get("FIELDS", [])
    sizes = [int(v) for v in header.get("SIZE", [])]
    types = header.get("TYPE", [])
    counts = [int(v) for v in header.get("COUNT", ["1"] * len(fields))]
    if not (len(fields) == len(sizes) == len(types) == len(counts)):
        raise RuntimeError("Malformed PCD header: FIELDS/SIZE/TYPE/COUNT length mismatch")

    dtype_fields: list[tuple[str, str] | tuple[str, str, tuple[int, ...]]] = []
    for name, typ, size, count in zip(fields, types, sizes, counts):
        dtype_str = PCD_TYPE_TO_DTYPE.get((typ, size))
        if dtype_str is None:
            raise RuntimeError(f"Unsupported PCD field type: {name} TYPE={typ} SIZE={size}")
        if count == 1:
            dtype_fields.append((name, dtype_str))
        else:
            dtype_fields.append((name, dtype_str, (count,)))
    return np.dtype(dtype_fields)


def load_pcd_points(path: Path) -> tuple[np.ndarray, np.ndarray | None]:
    header, data_offset = read_pcd_header(path)
    data_kind = header.get("DATA", [""])[0].lower()
    point_count = int(header.get("POINTS", header.get("WIDTH", ["0"]))[0])
    dtype = pcd_dtype(header)
    required = {"x", "y", "z"}
    if not required.issubset(dtype.names or ()):
        raise RuntimeError(f"PCD is missing x/y/z fields: {path}")

    if data_kind == "binary":
        with path.open("rb") as handle:
            handle.seek(data_offset)
            data = np.frombuffer(handle.read(point_count * dtype.itemsize), dtype=dtype, count=point_count)
    elif data_kind == "ascii":
        data = np.genfromtxt(path, dtype=dtype, skip_header=count_header_lines(path), max_rows=point_count)
        data = np.atleast_1d(data)
    else:
        raise RuntimeError(f"Unsupported PCD DATA mode `{data_kind}` in {path}")

    xyz = np.column_stack([data["x"], data["y"], data["z"]]).astype(np.float64, copy=False)
    intensity = data["intensity"].astype(np.float64, copy=False) if "intensity" in (dtype.names or ()) else None
    finite = np.isfinite(xyz).all(axis=1)
    if intensity is not None:
        finite &= np.isfinite(intensity)
    xyz = xyz[finite]
    if intensity is not None:
        intensity = intensity[finite]
    return xyz, intensity


def count_header_lines(path: Path) -> int:
    count = 0
    with path.open("rb") as handle:
        for line in handle:
            count += 1
            if line.decode("ascii", errors="replace").strip().upper().startswith("DATA"):
                return count
    raise RuntimeError(f"PCD header missing DATA line: {path}")


def deterministic_sample(points: np.ndarray, max_points: int) -> np.ndarray:
    if max_points <= 0 or len(points) <= max_points:
        return points
    step = max(1, math.ceil(len(points) / max_points))
    return points[::step][:max_points]


def frame_metrics(index: int, path: Path, sample_points: int) -> dict[str, float | int | str]:
    xyz, intensity = load_pcd_points(path)
    sampled = deterministic_sample(xyz, sample_points)
    if len(sampled) < 3:
        eig = np.array([0.0, 0.0, 0.0])
    else:
        centered = sampled - sampled.mean(axis=0)
        cov = np.cov(centered, rowvar=False)
        eig = np.sort(np.linalg.eigvalsh(cov))[::-1]
        eig = np.maximum(eig, 0.0)

    l1, l2, l3 = (float(v) for v in eig)
    denom = max(l1, 1e-12)
    linearity = (l1 - l2) / denom
    planarity = (l2 - l3) / denom
    scattering = l3 / denom
    degeneracy_score = max(linearity, planarity)
    ranges = np.linalg.norm(sampled, axis=1) if len(sampled) else np.array([], dtype=np.float64)

    row: dict[str, float | int | str] = {
        "frame": index,
        "path": str(path),
        "points": int(len(xyz)),
        "sampled_points": int(len(sampled)),
        "lambda1": l1,
        "lambda2": l2,
        "lambda3": l3,
        "linearity": linearity,
        "planarity": planarity,
        "scattering": scattering,
        "degeneracy_score": degeneracy_score,
        "range_mean_m": float(np.mean(ranges)) if len(ranges) else 0.0,
        "range_p95_m": float(np.percentile(ranges, 95)) if len(ranges) else 0.0,
    }
    if intensity is not None and len(intensity):
        row.update(
            {
                "intensity_mean": float(np.mean(intensity)),
                "intensity_std": float(np.std(intensity)),
                "intensity_p05": float(np.percentile(intensity, 5)),
                "intensity_p95": float(np.percentile(intensity, 95)),
            }
        )
    else:
        row.update(
            {
                "intensity_mean": 0.0,
                "intensity_std": 0.0,
                "intensity_p05": 0.0,
                "intensity_p95": 0.0,
            }
        )
    return row


def percentile(values: list[float], q: float) -> float:
    return float(np.percentile(np.asarray(values, dtype=np.float64), q)) if values else 0.0


def summarize(rows: list[dict[str, float | int | str]]) -> dict[str, object]:
    numeric_keys = [
        "points",
        "linearity",
        "planarity",
        "scattering",
        "degeneracy_score",
        "range_mean_m",
        "range_p95_m",
        "intensity_mean",
        "intensity_std",
    ]
    summary: dict[str, object] = {"frames": len(rows)}
    for key in numeric_keys:
        values = [float(row[key]) for row in rows]
        summary[key] = {
            "mean": float(np.mean(values)) if values else 0.0,
            "median": percentile(values, 50),
            "p95": percentile(values, 95),
            "max": max(values) if values else 0.0,
        }
    high_degeneracy = [row for row in rows if float(row["degeneracy_score"]) >= 0.90]
    summary["high_degeneracy_frames"] = len(high_degeneracy)
    summary["high_degeneracy_fraction"] = len(high_degeneracy) / len(rows) if rows else 0.0
    return summary


def write_csv(path: Path, rows: list[dict[str, float | int | str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = list(rows[0].keys()) if rows else []
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def write_markdown(path: Path, pcd_dir: Path, summary: dict[str, object]) -> None:
    def metric_line(key: str) -> str:
        stats = summary[key]
        assert isinstance(stats, dict)
        return (
            f"| `{key}` | {stats['mean']:.4f} | {stats['median']:.4f} | "
            f"{stats['p95']:.4f} | {stats['max']:.4f} |"
        )

    lines = [
        "# LiDAR Degeneracy Diagnostics",
        "",
        f"- PCD sequence: `{pcd_dir}`",
        f"- Frames: `{summary['frames']}`",
        f"- High-degeneracy frames (`score >= 0.90`): `{summary['high_degeneracy_frames']}` "
        f"({summary['high_degeneracy_fraction']:.1%})",
        "",
        "| Metric | Mean | Median | P95 | Max |",
        "|---|---:|---:|---:|---:|",
    ]
    for key in [
        "points",
        "linearity",
        "planarity",
        "scattering",
        "degeneracy_score",
        "range_mean_m",
        "range_p95_m",
        "intensity_mean",
        "intensity_std",
    ]:
        lines.append(metric_line(key))
    lines.extend(
        [
            "",
            "Interpretation:",
            "",
            "- `linearity` near 1 means points are mostly distributed along one dominant axis.",
            "- `planarity` near 1 means a dominant plane with weak orthogonal structure.",
            "- `scattering` near 0 with high `linearity` or `planarity` is a degeneracy warning.",
            "- This is a GT-free diagnostic; use it before odometry metrics to locate difficult windows.",
            "",
        ]
    )
    path.write_text("\n".join(lines))


def main() -> int:
    args = parse_args()
    pcds = collect_pcds(args.pcd_dir, args.max_frames)
    rows = [frame_metrics(i, path, args.sample_points) for i, path in enumerate(pcds)]
    summary = summarize(rows)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_csv(args.output_dir / "frame_degeneracy.csv", rows)
    (args.output_dir / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    write_markdown(args.output_dir / "summary.md", args.pcd_dir, summary)
    print(f"[done] frames={len(rows)} output_dir={args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
