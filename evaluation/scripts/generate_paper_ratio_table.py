#!/usr/bin/env python3
"""Build the original-paper vs repository comparison table (Table 6).

For every method in evaluation/data/paper_reported_numbers.json with per-sequence
KITTI Odometry values, pair each paper number with repository results on the
same full KITTI Odometry sequence:

  - pool: every variant of that method in experiments/results/*_matrix.json whose
    dataset is kitti_seq_<NN>_full, that produced a translational RPE, and that
    is not GT-seeded (pure odometry only)
  - best: the lowest RPE in the pool. This is selected on the evaluated sequence,
    so it is an optimistic bound, reported next to the pool median and size.

The repository RPE averages 100 m segments, while the papers report the official
KITTI metric averaged over 100-800 m segments, so ratios are directional.

Writes under docs/assets/paper/:
  - paper_ratio_table.csv
  - paper_ratio_table.tex
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import re
import statistics
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[2]
PAPER_DATA = REPO_ROOT / "evaluation" / "data" / "paper_reported_numbers.json"
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
ASSETS_DIR = REPO_ROOT / "docs" / "assets" / "paper"

SEQUENCE_KEY = re.compile(r"kitti_(\d\d)")
FULL_DATASET = re.compile(r"kitti_seq_(\d\d)_full")
GT_SEEDED_NOTE = re.compile(r"GT-seeded|Seeds .* with GT", re.IGNORECASE)

CSV_COLUMNS = [
    "method",
    "sequence",
    "paper_rte_pct",
    "paper_source",
    "repo_best_rpe_pct",
    "repo_best_ratio",
    "repo_pool_median_rpe_pct",
    "repo_pool_size",
    "repo_best_variant",
    "repo_best_aggregate",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--paper-data", type=Path, default=PAPER_DATA)
    parser.add_argument("--results-dir", type=Path, default=RESULTS_DIR)
    parser.add_argument("--output-dir", type=Path, default=ASSETS_DIR)
    return parser.parse_args()


def is_gt_seeded(variant: dict[str, Any]) -> bool:
    return bool(GT_SEEDED_NOTE.search(str(variant.get("note", ""))))


def collect_pools(results_dir: Path, selectors: set[str]) -> dict[tuple[str, str], list[dict[str, Any]]]:
    pools: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for path in sorted(results_dir.glob("*_matrix.json")):
        aggregate = json.loads(path.read_text())
        selector = str(aggregate.get("stable_interface", {}).get("methods", ""))
        if selector not in selectors:
            continue
        match = FULL_DATASET.fullmatch(Path(str(aggregate.get("dataset", {}).get("pcd_dir", ""))).name)
        if not match:
            continue
        for variant in aggregate.get("variants", []):
            rpe = variant.get("rpe_trans_pct")
            if rpe is None or not math.isfinite(float(rpe)) or is_gt_seeded(variant):
                continue
            pools.setdefault((selector, match.group(1)), []).append(
                {
                    "rpe": float(rpe),
                    "variant": str(variant["id"]),
                    "aggregate": f"experiments/results/{path.name}",
                }
            )
    return pools


def collect_rows(paper_data: Path, results_dir: Path) -> list[dict[str, str]]:
    methods = json.loads(paper_data.read_text())["methods"]
    targets = {
        selector: info
        for selector, info in methods.items()
        if any(SEQUENCE_KEY.fullmatch(key) for key in info.get("reported_values", {}))
    }
    pools = collect_pools(results_dir, set(targets))
    rows: list[dict[str, str]] = []
    for selector, info in sorted(targets.items()):
        source = info.get("reported_source", {})
        source_text = " ".join(str(source.get(key, "")) for key in ("table", "row")).strip()
        for key, paper_value in sorted(info["reported_values"].items()):
            match = SEQUENCE_KEY.fullmatch(key)
            if not match:
                continue
            sequence = match.group(1)
            pool = pools.get((selector, sequence), [])
            row = {
                "method": selector,
                "sequence": sequence,
                "paper_rte_pct": f"{float(paper_value):.2f}",
                "paper_source": source_text,
                "repo_best_rpe_pct": "",
                "repo_best_ratio": "",
                "repo_pool_median_rpe_pct": "",
                "repo_pool_size": str(len(pool)),
                "repo_best_variant": "",
                "repo_best_aggregate": "",
            }
            if pool:
                best = min(pool, key=lambda item: item["rpe"])
                row.update(
                    {
                        "repo_best_rpe_pct": f"{best['rpe']:.3f}",
                        "repo_best_ratio": f"{best['rpe'] / float(paper_value):.2f}",
                        "repo_pool_median_rpe_pct": f"{statistics.median(item['rpe'] for item in pool):.3f}",
                        "repo_best_variant": best["variant"],
                        "repo_best_aggregate": best["aggregate"],
                    }
                )
            rows.append(row)
    return rows


def geometric_mean_ratio(rows: list[dict[str, str]], method: str) -> float | None:
    ratios = [float(row["repo_best_ratio"]) for row in rows if row["method"] == method and row["repo_best_ratio"]]
    if not ratios:
        return None
    return math.exp(sum(math.log(value) for value in ratios) / len(ratios))


def write_csv(rows: list[dict[str, str]], output_path: Path) -> None:
    with output_path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=CSV_COLUMNS, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def render_tex(rows: list[dict[str, str]]) -> str:
    lines = [
        "% Generated by evaluation/scripts/generate_paper_ratio_table.py; do not edit by hand.",
        "% Paper: official KITTI RTE (100-800 m). Repo: 100 m-segment RPE, best non-GT-seeded",
        "% variant selected on the evaluated sequence (optimistic). Ratios are directional.",
        r"\begin{tabular}{llrrrrr}",
        r"\toprule",
        r"Method & Seq. & Paper [\%] & Repo best [\%] & Ratio & Repo median [\%] & $n$ \\",
        r"\midrule",
    ]
    for row in rows:
        cells = [
            row["method"].replace("_", r"\_"),
            row["sequence"],
            row["paper_rte_pct"],
            row["repo_best_rpe_pct"] or "--",
            f"{row['repo_best_ratio']}$\\times$" if row["repo_best_ratio"] else "--",
            row["repo_pool_median_rpe_pct"] or "--",
            row["repo_pool_size"],
        ]
        lines.append(" & ".join(cells) + r" \\")
    lines += [r"\bottomrule", r"\end{tabular}"]
    return "\n".join(lines) + "\n"


def main() -> None:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    rows = collect_rows(args.paper_data, args.results_dir)
    write_csv(rows, args.output_dir / "paper_ratio_table.csv")
    (args.output_dir / "paper_ratio_table.tex").write_text(render_tex(rows))
    for method in sorted({row["method"] for row in rows}):
        mean = geometric_mean_ratio(rows, method)
        text = f"{mean:.2f}x" if mean is not None else "n/a"
        print(f"[done] {method}: geometric-mean best/paper ratio {text}")


if __name__ == "__main__":
    main()
