#!/usr/bin/env python3
"""Generate the method x dataset ATE leaderboard injected into README.md.

Scans every aggregate under experiments/results/, collects the best (minimum)
ATE per (method, dataset) cell across all tuned variants and manifests, then
renders a compact markdown leaderboard over the five canonical KITTI Odometry
full sequences (the densest, most recognizable cross-method coverage). The
per-column winner is bolded.

The table is written between the LEADERBOARD markers in README.md so it can be
regenerated as the benchmark grows:

    <!-- LEADERBOARD:START -->
    ...generated table...
    <!-- LEADERBOARD:END -->

Usage:
    python3 evaluation/scripts/generate_leaderboard.py            # rewrite README
    python3 evaluation/scripts/generate_leaderboard.py --check    # fail if stale
    python3 evaluation/scripts/generate_leaderboard.py --print    # stdout only
"""
from __future__ import annotations

import argparse
import glob
import json
import math
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
README_PATH = REPO_ROOT / "README.md"

START_MARKER = "<!-- LEADERBOARD:START -->"
END_MARKER = "<!-- LEADERBOARD:END -->"

# Leaderboard columns: the five canonical KITTI Odometry full sequences. These
# have the broadest, densest multi-method coverage (9-10 methods each) and are
# the most widely recognized LiDAR odometry benchmark.
COLUMNS = [
    ("dogfooding_results/kitti_seq_00_full", "Seq 00", "4542 fr"),
    ("dogfooding_results/kitti_seq_02_full", "Seq 02", "4661 fr"),
    ("dogfooding_results/kitti_seq_05_full", "Seq 05", "2761 fr"),
    ("dogfooding_results/kitti_seq_07_full", "Seq 07", "1101 fr"),
    ("dogfooding_results/kitti_seq_08_full", "Seq 08", "4071 fr"),
]

# Methods initialized from odometry only (no per-frame GT seed). Their ATE
# accumulates drift over the trajectory, unlike GT-seeded scan-to-map methods
# whose ATE is the per-frame registration residual from ground truth. The
# distinction is material, so it is flagged rather than hidden.
ODOMETRY_ONLY = {
    "CT-ICP", "KISS-ICP", "GenZ-ICP", "FAST-LIO2", "Point-LIO", "DLIO", "DLO",
    "RKO-LIO", "A-LOAM", "F-LOAM", "LeGO-LOAM", "MULLS", "SuMa", "LOAM-Livox",
    "LINS", "LIO-SAM", "ISC-LOAM", "FAST-LIO-SLAM", "HDL-Graph-SLAM", "BALM2",
    "VGICP-SLAM", "CT-LIO", "CLINS", "X-ICP",
}


def _dataset_key(agg: dict) -> str:
    ds = agg.get("dataset") or {}
    key = ds.get("pcd_dir")
    if key:
        return key
    return (agg.get("problem", {}).get("dataset", {}) or {}).get("pcd_dir", "")


def collect_cells() -> dict:
    """Return {(method, dataset_dir): best_ate_m} across all aggregates on disk.

    Each cell is the minimum ATE across every tuned variant and every manifest
    for that (method, dataset) pair, i.e. the best result the repository can
    currently produce for that method on that dataset.
    """
    cells: dict[tuple[str, str], float] = {}
    for path in glob.glob(str(RESULTS_DIR / "*.json")):
        if Path(path).name == "index.json":
            continue
        try:
            agg = json.loads(Path(path).read_text())
        except (json.JSONDecodeError, OSError):
            continue
        if not isinstance(agg, dict) or "variants" not in agg:
            continue
        method = agg.get("stable_interface", {}).get("primary_method")
        dataset = _dataset_key(agg)
        if not method or not dataset:
            continue
        for variant in agg.get("variants", []):
            if variant.get("status") != "ok":
                continue
            ate = variant.get("ate_m")
            if ate is None or not isinstance(ate, (int, float)) or not math.isfinite(ate):
                continue
            key = (method, dataset)
            if key not in cells or ate < cells[key]:
                cells[key] = float(ate)
    return cells


def build_table(cells: dict) -> str:
    col_dirs = [c[0] for c in COLUMNS]
    methods = sorted({m for (m, d) in cells if d in col_dirs})

    # Per-column winner (minimum ATE) for bolding.
    col_min: dict[str, float] = {}
    for d in col_dirs:
        vals = [cells[(m, d)] for m in methods if (m, d) in cells]
        if vals:
            col_min[d] = min(vals)

    # Sort rows by mean ATE over present cells (lower first); accuracy leaders
    # rise to the top, sparse/odometry rows settle below.
    def mean_ate(m: str) -> float:
        present = [cells[(m, d)] for d in col_dirs if (m, d) in cells]
        return sum(present) / len(present) if present else float("inf")

    methods.sort(key=mean_ate)

    head = ["Method"] + [c[1] for c in COLUMNS]
    sub = [""] + [f"_{c[2]}_" for c in COLUMNS]
    lines = [
        "| " + " | ".join(head) + " |",
        "|" + "|".join(["---"] + ["---:" for _ in COLUMNS]) + "|",
        "| " + " | ".join(sub) + " |",
    ]

    for m in methods:
        label = f"{m} †" if m in ODOMETRY_ONLY else f"**{m}**"
        row = [label]
        for d in col_dirs:
            if (m, d) not in cells:
                row.append("–")
                continue
            ate = cells[(m, d)]
            txt = f"{ate:.3f}"
            if col_min.get(d) is not None and abs(ate - col_min[d]) < 1e-9:
                txt = f"**{txt}**"
            row.append(txt)
        lines.append("| " + " | ".join(row) + " |")

    note = (
        "\n_ATE in metres on KITTI Odometry ground truth; lower is better. "
        "**Bold** = best on that sequence. Each cell is the best ATE across all "
        "tuned variants for that method (see [`docs/experiments.md`](docs/experiments.md)). "
        "† = odometry-only (no per-frame GT seed), so its ATE accumulates drift; "
        "non-† rows use GT-seeded scan-to-map init with weak-update fallback, "
        "where ATE is the per-frame registration residual from ground truth — "
        "the two groups are not directly comparable. "
        "Regenerated by `evaluation/scripts/generate_leaderboard.py`._"
    )
    return "\n".join(lines) + "\n" + note


def render_block(cells: dict) -> str:
    table = build_table(cells)
    return (
        f"{START_MARKER}\n"
        "## Leaderboard\n\n"
        "Best ATE per method on the five canonical KITTI Odometry full "
        "sequences. Other benchmarks (KITTI Raw, NCLT, MCD, MulRan, HDL-400) "
        "and the full method × dataset matrix are browsable in the "
        "[**interactive explorer**](https://rsasaki0109.github.io/localization_zoo/).\n\n"
        f"{table}\n"
        f"{END_MARKER}"
    )


def inject(readme: str, block: str) -> str:
    if START_MARKER in readme and END_MARKER in readme:
        pre = readme.split(START_MARKER)[0]
        post = readme.split(END_MARKER, 1)[1]
        return pre + block + post
    # First-time insertion: after "## Why Localization Zoo?", before "## Scope Note".
    anchor = "\n## Scope Note\n"
    if anchor not in readme:
        raise SystemExit("Could not find insertion anchor ('## Scope Note') in README.md")
    idx = readme.index(anchor)
    return readme[:idx] + "\n" + block + "\n" + readme[idx:]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if README is stale")
    parser.add_argument("--print", dest="print_only", action="store_true",
                        help="print the block to stdout, do not edit README")
    args = parser.parse_args()

    cells = collect_cells()
    block = render_block(cells)

    if args.print_only:
        print(block)
        return 0

    readme = README_PATH.read_text()
    updated = inject(readme, block)

    if args.check:
        if updated != readme:
            print("README leaderboard is stale; run generate_leaderboard.py", file=sys.stderr)
            return 1
        print("README leaderboard up to date.")
        return 0

    if updated != readme:
        README_PATH.write_text(updated)
        print(f"Updated leaderboard in {README_PATH.relative_to(REPO_ROOT)}")
    else:
        print("README leaderboard already up to date.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
