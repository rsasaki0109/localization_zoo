#!/usr/bin/env python3
"""Run cross-dataset benchmarks on MulRan / NCLT for recent from-paper methods."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
BUILD = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"

DATASETS = {
    "nclt_600": {
        "pcd_dir": "dogfooding_results/nclt_2013_01_10_600",
        "gt_csv": "experiments/reference_data/nclt_2013_01_10_600_gt.csv",
        "max_frames": 600,
    },
    "mulran_120": {
        "pcd_dir": "dogfooding_results/mulran_parkinglot_120",
        "gt_csv": "experiments/reference_data/mulran_parkinglot_120_gt.csv",
        "max_frames": 120,
    },
    "mulran_full": {
        "pcd_dir": "dogfooding_results/mulran_parkinglot_full",
        "gt_csv": "experiments/reference_data/mulran_parkinglot_full_gt.csv",
        "max_frames": 1177,
    },
}

DEFAULT_METHODS = [
    "kiss_icp",
    "i_loam",
    "inten_loam",
    "mcgicp",
    "icpsc",
    "mcc_lo",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dataset",
        action="append",
        choices=sorted(DATASETS),
        default=[],
    )
    parser.add_argument(
        "--methods",
        default=",".join(DEFAULT_METHODS),
        help="Comma-separated method ids for pcd_dogfooding",
    )
    parser.add_argument(
        "--out-dir",
        default="docs/benchmarks/cross_dataset",
        help="Directory for summary JSON files",
    )
    parser.add_argument("--no-gt-seed", action="store_true", default=True)
    return parser.parse_args()


def method_profile(method: str) -> list[str]:
    dense = {
        "i_loam",
        "inten_loam",
        "mcgicp",
        "icpsc",
        "mcc_lo",
        "pl_loam",
        "vlom",
    }
    if method in dense:
        return [f"--{method.replace('_', '-')}-dense-profile"]
    return []


def run_one(dataset_key: str, methods: list[str], out_dir: Path) -> Path:
    cfg = DATASETS[dataset_key]
    summary = out_dir / f"{dataset_key}_{methods[0]}.json"
    if len(methods) > 1:
        summary = out_dir / f"{dataset_key}_bundle.json"

    cmd = [
        str(BUILD),
        cfg["pcd_dir"],
        cfg["gt_csv"],
        str(cfg["max_frames"]),
        "--methods",
        ",".join(methods),
        "--no-gt-seed",
        "--summary-json",
        str(summary),
    ]
    for method in methods:
        cmd.extend(method_profile(method))

    print(f"[run] {' '.join(cmd)}", flush=True)
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    return summary


def main() -> int:
    args = parse_args()
    if not BUILD.exists():
        raise FileNotFoundError(f"Build pcd_dogfooding first: {BUILD}")

    datasets = args.dataset or list(DATASETS)
    methods = [m.strip() for m in args.methods.split(",") if m.strip()]
    out_dir = REPO_ROOT / args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    results: list[str] = []
    for dataset_key in datasets:
        summary = run_one(dataset_key, methods, out_dir)
        results.append(str(summary.relative_to(REPO_ROOT)))

    print(json.dumps({"summaries": results}, indent=2), flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
