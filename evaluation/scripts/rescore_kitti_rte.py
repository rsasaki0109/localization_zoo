#!/usr/bin/env python3
"""Re-run the Table 6 best variants to record the official KITTI RTE.

Historical aggregates only store the repository 100 m-segment RPE. For each row
of docs/assets/paper/paper_ratio_table.csv this script re-executes the selected
variant (same binary, method selector, dataset extra args, and variant args) on
the full KITTI Odometry sequence and records:

  - kitti_rte_trans_pct / kitti_rte_rot_deg_per_100m (official devkit metric)
  - the re-run 100 m RPE next to the stored value, as a determinism check

Writes experiments/results/kitti_rte_rescore.json, which
generate_paper_ratio_table.py overlays onto the aggregates.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[2]
TABLE = REPO_ROOT / "docs" / "assets" / "paper" / "paper_ratio_table.csv"
OUTPUT = REPO_ROOT / "experiments" / "results" / "kitti_rte_rescore.json"

sys.path.insert(0, str(Path(__file__).resolve().parent))
from capture_benchmark_environment import capture_run_host  # noqa: E402


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=REPO_ROOT / "build/evaluation/pcd_dogfooding")
    parser.add_argument(
        "--data-root",
        type=Path,
        default=Path(os.environ.get("LOCALIZATION_ZOO_DATA_ROOT", REPO_ROOT)),
        help="Directory containing dogfooding_results/kitti_seq_<NN>_full",
    )
    parser.add_argument("--work-dir", type=Path, required=True, help="Scratch directory for summaries")
    parser.add_argument("--table", type=Path, default=TABLE)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--method", action="append", default=[], help="Limit to these methods")
    return parser.parse_args()


def find_variant(aggregate: dict[str, Any], variant_id: str) -> dict[str, Any]:
    for variant in aggregate["variants"]:
        if variant["id"] == variant_id:
            return variant
    raise KeyError(variant_id)


def primary_result(summary: dict[str, Any], primary_method: str) -> dict[str, Any]:
    for item in summary["methods"]:
        name = str(item["name"])
        if name == primary_method or (
            name.startswith(primary_method) and not name[len(primary_method)].isalnum()
        ):
            return item
    raise KeyError(primary_method)


def rescore_row(row: dict[str, str], args: argparse.Namespace) -> dict[str, Any]:
    aggregate_path = row["repo_best_aggregate"]
    aggregate = json.loads((REPO_ROOT / aggregate_path).read_text())
    variant = find_variant(aggregate, row["repo_best_variant"])
    dataset = aggregate["dataset"]
    pcd_dir = args.data_root / dataset["pcd_dir"]
    gt_csv = REPO_ROOT / dataset["gt_csv"]
    summary_path = args.work_dir / f"{Path(aggregate_path).stem}__{variant['id']}.json"
    command = [
        str(args.binary),
        str(pcd_dir),
        str(gt_csv),
        "--methods",
        str(aggregate["stable_interface"]["methods"]),
        "--summary-json",
        str(summary_path),
        *dataset.get("extra_args", []),
        *variant["args"],
    ]
    subprocess.run(command, cwd=REPO_ROOT, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    result = primary_result(
        json.loads(summary_path.read_text()), str(aggregate["stable_interface"]["primary_method"])
    )
    stored = float(variant["rpe_trans_pct"])
    rerun = result.get("rpe_trans_pct")
    return {
        "method": row["method"],
        "sequence": row["sequence"],
        "aggregate_path": aggregate_path,
        "variant_id": variant["id"],
        "args": variant["args"],
        "kitti_rte_trans_pct": result.get("kitti_rte_trans_pct"),
        "kitti_rte_rot_deg_per_100m": result.get("kitti_rte_rot_deg_per_100m"),
        "stored_rpe_trans_pct": stored,
        "rerun_rpe_trans_pct": rerun,
        "rerun_rpe_abs_delta": abs(float(rerun) - stored) if rerun is not None else None,
        "frames": result.get("frames"),
        "fps": result.get("fps"),
    }


def main() -> int:
    args = parse_args()
    args.work_dir.mkdir(parents=True, exist_ok=True)
    rows = [row for row in csv.DictReader(args.table.open()) if row["repo_best_variant"]]
    if args.method:
        rows = [row for row in rows if row["method"] in args.method]
    existing = json.loads(args.output.read_text())["rows"] if args.output.is_file() else []
    by_key = {(item["aggregate_path"], item["variant_id"]): item for item in existing}
    for row in rows:
        print(f"[run] {row['method']} seq {row['sequence']} {row['repo_best_variant']}", flush=True)
        item = rescore_row(row, args)
        by_key[(item["aggregate_path"], item["variant_id"])] = item
        print(
            f"      KITTI RTE {item['kitti_rte_trans_pct']:.3f} %  "
            f"100 m RPE rerun {item['rerun_rpe_trans_pct']:.3f} vs stored {item['stored_rpe_trans_pct']:.3f}",
            flush=True,
        )
    payload = {
        "schema_version": 1,
        "generated_at": datetime.now(timezone.utc).replace(microsecond=0).isoformat(),
        "metric": "official KITTI odometry RTE (100-800 m segments, every 10th frame)",
        "host": capture_run_host(),
        "rows": sorted(by_key.values(), key=lambda item: (item["method"], item["sequence"])),
    }
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"[done] wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
