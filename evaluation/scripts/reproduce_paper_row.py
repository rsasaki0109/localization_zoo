#!/usr/bin/env python3
"""Re-run one row of the paper-number check (Table 6) in one command.

For a method and KITTI Odometry sequence this script

1. finds the variant Table 6 uses for that row (docs/assets/paper/paper_ratio_table.csv)
   and its command-line arguments (from the recorded aggregate),
2. fetches the sequence's Velodyne scans if they are not on disk
   (fetch_kitti_odometry_sequences.py, HTTP range requests, no 80 GB ZIP),
3. runs pcd_dogfooding with those arguments against the committed ground truth, and
4. prints the official KITTI RTE of this run next to the recorded repo value and
   the paper's value.

Examples:
  python3 evaluation/scripts/reproduce_paper_row.py --list
  python3 evaluation/scripts/reproduce_paper_row.py --method litamin2 --sequence 07 \\
      --kitti-root data/kitti --fetch
"""

from __future__ import annotations

import argparse
import csv
import json
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[2]
RATIO_TABLE = REPO_ROOT / "docs" / "assets" / "paper" / "paper_ratio_table.csv"
REFERENCE_DIR = REPO_ROOT / "experiments" / "reference_data"
DEFAULT_BINARY = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--list", action="store_true", help="List the rows that can be re-run.")
    parser.add_argument("--method", help="Method selector as in Table 6 (e.g. litamin2, mulls, ct_icp).")
    parser.add_argument("--sequence", help="Two-digit KITTI Odometry sequence (e.g. 07).")
    parser.add_argument("--variant", help="Variant id to run instead of the one Table 6 uses.")
    parser.add_argument(
        "--kitti-root",
        type=Path,
        default=REPO_ROOT / "data" / "kitti",
        help="Root holding dataset/sequences/NN/velodyne (default: data/kitti).",
    )
    parser.add_argument("--fetch", action="store_true", help="Download the sequence if it is missing.")
    parser.add_argument("--binary", type=Path, default=DEFAULT_BINARY)
    parser.add_argument("--table", type=Path, default=RATIO_TABLE)
    parser.add_argument("--dry-run", action="store_true", help="Print the command without running it.")
    return parser.parse_args(argv)


def load_rows(table: Path) -> list[dict[str, str]]:
    with table.open(newline="") as handle:
        return list(csv.DictReader(handle))


def runnable_rows(rows: list[dict[str, str]]) -> list[dict[str, str]]:
    return [row for row in rows if row.get("repo_best_variant") and row.get("repo_best_aggregate")]


def find_row(rows: list[dict[str, str]], method: str, sequence: str) -> dict[str, str]:
    for row in rows:
        if row["method"] == method and row["sequence"] == sequence:
            return row
    raise SystemExit(f"no Table 6 row for method={method} sequence={sequence}; see --list")


def variant_args(aggregate_path: Path, variant_id: str) -> tuple[str, list[str]]:
    """Return (method selector, args) for a variant of a recorded aggregate."""
    aggregate = json.loads(aggregate_path.read_text())
    selector = str(aggregate.get("stable_interface", {}).get("methods", ""))
    for variant in aggregate.get("variants", []):
        if str(variant.get("id")) == variant_id:
            return selector, [str(arg) for arg in variant.get("args", [])]
    raise SystemExit(f"variant {variant_id} not found in {aggregate_path}")


def velodyne_dir(kitti_root: Path, sequence: str) -> Path:
    return kitti_root / "dataset" / "sequences" / sequence / "velodyne"


def build_command(binary: Path, scans: Path, gt_csv: Path, selector: str, args: list[str],
                  summary: Path) -> list[str]:
    return [str(binary), str(scans), str(gt_csv), "--methods", selector, *args, "--summary-json", str(summary)]


def fmt(value: Any) -> str:
    try:
        return f"{float(value):.3f} %"
    except (TypeError, ValueError):
        return "n/a"


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    rows = load_rows(args.table)
    if args.list:
        print("method        seq  paper     repo(recorded)  variant")
        for row in runnable_rows(rows):
            print(f"{row['method']:<13} {row['sequence']}   {fmt(row['paper_rte_pct']):<9} "
                  f"{fmt(row['repo_best_rpe_pct']):<15} {row['repo_best_variant']}")
        return 0
    if not args.method or not args.sequence:
        raise SystemExit("--method and --sequence are required (or use --list)")

    row = find_row(rows, args.method, args.sequence)
    variant = args.variant or row.get("repo_best_variant")
    aggregate = row.get("repo_best_aggregate")
    if not variant or not aggregate:
        raise SystemExit(f"Table 6 has no repo run for {args.method} seq {args.sequence}")
    selector, run_args = variant_args(REPO_ROOT / aggregate, variant)

    scans = velodyne_dir(args.kitti_root, args.sequence)
    gt_csv = REFERENCE_DIR / f"kitti_seq_{args.sequence}_full_gt.csv"
    if not scans.is_dir():
        if not args.fetch:
            raise SystemExit(f"{scans} not found; pass --fetch to download KITTI seq {args.sequence}")
        if not args.dry_run:
            subprocess.run(
                [sys.executable, str(REPO_ROOT / "evaluation" / "scripts" / "fetch_kitti_odometry_sequences.py"),
                 "--output-root", str(args.kitti_root), "--sequence", args.sequence],
                check=True,
            )

    with tempfile.TemporaryDirectory() as tmp:
        summary = Path(tmp) / "summary.json"
        command = build_command(args.binary, scans, gt_csv, selector, run_args, summary)
        print("$", " ".join(command))
        if args.dry_run:
            return 0
        subprocess.run(command, check=True, cwd=REPO_ROOT)
        method = json.loads(summary.read_text())["methods"][0]

    rte = method.get("kitti_rte_trans_pct")
    print()
    print(f"{args.method} KITTI {args.sequence}, variant {variant}")
    print(f"  this run (official RTE):  {fmt(rte)}")
    print(f"  Table 6 recorded value:   {fmt(row.get('repo_best_rpe_pct'))}")
    print(f"  paper ({row.get('paper_source', '')}): {fmt(row.get('paper_rte_pct'))}")
    try:
        print(f"  ratio this run / paper:   {float(rte) / float(row['paper_rte_pct']):.2f}x")
    except (TypeError, ValueError, ZeroDivisionError):
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
