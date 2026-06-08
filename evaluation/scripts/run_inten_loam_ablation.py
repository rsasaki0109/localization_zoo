#!/usr/bin/env python3
"""Run InTEn-LOAM full-sequence ablations on KITTI Odometry seq00/seq07."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
BINARY = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"

SEQUENCES = {
    "seq00": {
        "pcd_dir": "dogfooding_results/kitti_seq_00_full",
        "gt_csv": "experiments/reference_data/kitti_seq_00_full_gt.csv",
    },
    "seq07": {
        "pcd_dir": "dogfooding_results/kitti_seq_07_full",
        "gt_csv": "experiments/reference_data/kitti_seq_07_full_gt.csv",
    },
}

# label -> extra CLI flags (dense profile always on)
VARIANTS: dict[str, list[str]] = {
    "full": [],
    "no_mapping": ["--inten-loam-no-mapping"],
    "no_intensity": ["--inten-loam-no-intensity"],
    "no_tvf": ["--inten-loam-no-tvf"],
    "no_dor": ["--inten-loam-no-dor"],
    "no_tvf_no_dor": ["--inten-loam-no-tvf", "--inten-loam-no-dor"],
    "mapping_only": [
        "--inten-loam-no-tvf",
        "--inten-loam-no-dor",
        # mapping + intensity ON (default)
    ],
}


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument(
        "--sequence",
        action="append",
        choices=sorted(SEQUENCES),
        default=[],
    )
    p.add_argument(
        "--variant",
        action="append",
        choices=sorted(VARIANTS),
        default=[],
    )
    p.add_argument(
        "--out-dir",
        default="docs/benchmarks/kitti_full_new_methods",
    )
    return p.parse_args()


def run_variant(seq_key: str, variant: str, out_dir: Path) -> dict:
    cfg = SEQUENCES[seq_key]
    summary = out_dir / f"{seq_key}_inten_loam_{variant}.json"
    cmd = [
        str(BINARY),
        str(REPO_ROOT / cfg["pcd_dir"]),
        str(REPO_ROOT / cfg["gt_csv"]),
        "--methods",
        "inten_loam",
        "--no-gt-seed",
        "--inten-loam-dense-profile",
        *VARIANTS[variant],
        "--summary-json",
        str(summary),
    ]
    print(f"[run] {seq_key} variant={variant}", flush=True)
    print("  " + " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    data = json.loads(summary.read_text())
    method = data["methods"][0]
    method["variant"] = variant
    return method


def merge_bundle(seq_key: str, methods: list[dict], out_dir: Path) -> Path:
    cfg = SEQUENCES[seq_key]
    bundle = out_dir / f"{seq_key}_inten_loam_ablation.json"
    payload = {
        "pcd_dir": cfg["pcd_dir"],
        "gt_csv": cfg["gt_csv"],
        "eval": "KITTI Odometry full, --no-gt-seed, --inten-loam-dense-profile",
        "methods": methods,
    }
    bundle.write_text(json.dumps(payload, indent=2) + "\n")
    return bundle


def main() -> int:
    args = parse_args()
    sequences = args.sequence or sorted(SEQUENCES)
    variants = args.variant or sorted(VARIANTS)
    out_dir = REPO_ROOT / args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    if not BINARY.is_file():
        print(f"Missing binary: {BINARY}", file=sys.stderr)
        return 1

    bundles: list[Path] = []
    for seq_key in sequences:
        methods: list[dict] = []
        for variant in variants:
            methods.append(run_variant(seq_key, variant, out_dir))
        bundles.append(merge_bundle(seq_key, methods, out_dir))

    print(json.dumps({"bundles": [str(p.relative_to(REPO_ROOT)) for p in bundles]}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
