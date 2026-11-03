#!/usr/bin/env python3
"""Summarize an LF-GICP --lf-gicp-trace-csv file per block of frames.

For every frame with a valid final Gauss-Newton system the trace stores the
weakest unit eigenvector of the unweighted Hessian in the paper's world twist
(Eq. 2) and in a tangent centred on the sensor. A direction [omega; rho] in the
world twist moves the sensor-local frame by omega and rho - t x omega, with t
the sensor position. It is called rotation-dominated when a 10 m lever arm
moves more under the rotation than under that local translation.

Usage: analyze_trace.py TRACE.csv[.gz] [--block 100] [--output out.json]
"""

from __future__ import annotations

import argparse
import csv
import gzip
import json
import math
from pathlib import Path

LEVER_M = 10.0


def norm(v):
    return math.sqrt(sum(x * x for x in v))


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def rotation_dominated(omega, local_rho) -> bool:
    return LEVER_M * norm(omega) > norm(local_rho)


def summarize(path: Path, block: int) -> dict:
    opener = gzip.open if path.suffix == ".gz" else open
    with opener(path, "rt") as handle:
        rows = list(csv.DictReader(handle))
    blocks: dict[int, dict] = {}
    for row in rows:
        index = int(row["input_index"])
        b = blocks.setdefault(index // block, {
            "first_frame": (index // block) * block, "frames": 0, "gated": 0, "valid": 0,
            "sum_distance_m": 0.0, "world_weak_rotation": 0, "sensor_weak_rotation": 0,
            "max_weight": 0.0, "sum_half_weight_fraction": 0.0})
        b["frames"] += 1
        gated = row["degenerate"] == "1"
        b["gated"] += int(gated)
        if row["diag_valid"] != "1":
            continue
        b["valid"] += 1
        t = tuple(float(row[k]) for k in ("x", "y", "z"))
        b["sum_distance_m"] += norm(t)
        omega = tuple(float(row[f"world_weak_w{k}"]) for k in range(3))
        rho = tuple(float(row[f"world_weak_r{k}"]) for k in range(3))
        tw = cross(t, omega)
        local = tuple(rho[k] - tw[k] for k in range(3))
        b["world_weak_rotation"] += int(rotation_dominated(omega, local))
        s_omega = tuple(float(row[f"sensor_weak_w{k}"]) for k in range(3))
        s_rho = tuple(float(row[f"sensor_weak_r{k}"]) for k in range(3))
        b["sensor_weak_rotation"] += int(rotation_dominated(s_omega, s_rho))
        if gated:
            b["max_weight"] = max(b["max_weight"], float(row["max_weight"]))
            b["sum_half_weight_fraction"] += float(row["half_weight_fraction"])
    out = []
    for key in sorted(blocks):
        b = blocks[key]
        valid = max(b["valid"], 1)
        out.append({
            "first_frame": b["first_frame"], "frames": b["frames"], "gated_frames": b["gated"],
            "mean_distance_from_origin_m": b["sum_distance_m"] / valid,
            "world_weak_rotation_fraction": b["world_weak_rotation"] / valid,
            "sensor_weak_rotation_fraction": b["sensor_weak_rotation"] / valid,
            "max_applied_weight": b["max_weight"] if b["gated"] else None,
            "mean_half_weight_fraction": (b["sum_half_weight_fraction"] / b["gated"]) if b["gated"] else None,
        })
    return {"trace": path.name, "block_frames": block, "lever_m": LEVER_M,
            "frames": len(rows), "gated_frames": sum(b["gated"] for b in blocks.values()), "blocks": out}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("trace", type=Path)
    parser.add_argument("--block", type=int, default=100)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    summary = summarize(args.trace, args.block)
    text = json.dumps(summary, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(f"{summary['trace']}: {summary['gated_frames']}/{summary['frames']} gated frames")
    print("first  gated  mean|t|[m]  world-weak rot  sensor-weak rot  max w  half-weight frac")
    for b in summary["blocks"]:
        mw = "-" if b["max_applied_weight"] is None else f"{b['max_applied_weight']:.1f}"
        hw = "-" if b["mean_half_weight_fraction"] is None else f"{b['mean_half_weight_fraction']:.3f}"
        print(f"{b['first_frame']:5d}  {b['gated_frames']:3d}/{b['frames']:3d}  "
              f"{b['mean_distance_from_origin_m']:9.1f}  {b['world_weak_rotation_fraction']:14.2f}  "
              f"{b['sensor_weak_rotation_fraction']:15.2f}  {mw:>5}  {hw:>6}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
