#!/usr/bin/env python3
"""Build the original-paper vs repository comparison table (Table 6).

For every method in evaluation/data/paper_reported_numbers.json with per-sequence
KITTI Odometry values, pair each paper number with repository results on the
same full KITTI Odometry sequence:

  - pool: every variant of that method in experiments/results/*_matrix.json whose
    dataset is kitti_seq_<NN>_full, that produced a translational RPE, and that
    is not GT-seeded (pure odometry only)
  - metric: the official KITTI RTE (kitti_rte_trans_pct, 100-800 m) when any
    pool variant has it (from the aggregate or from the re-run evidence in
    experiments/results/kitti_rte_rescore.json), otherwise the repository
    100 m-segment RPE
  - best: the lowest error in the pool. Variants with KITTI RTE only from the
    re-run evidence were themselves the 100 m-RPE best of the full sweep, whose
    size is reported as selected_from_n. This is selected on the evaluated sequence,
    so it is an optimistic bound, reported next to the pool median and size.

Rows on the 100 m RPE fallback are not the paper's metric and are directional only.

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
RESCORE_PATH = RESULTS_DIR / "kitti_rte_rescore.json"
ASSETS_DIR = REPO_ROOT / "docs" / "assets" / "paper"

SEQUENCE_KEY = re.compile(r"kitti_(\d\d)")
FULL_DATASET = re.compile(r"kitti_seq_(\d\d)_full")
GT_SEEDED_NOTE = re.compile(r"GT-seeded|Seeds .* with GT", re.IGNORECASE)

CSV_COLUMNS = [
    "method",
    "sequence",
    "paper_rte_pct",
    "paper_source",
    "repo_metric",
    "repo_best_rpe_pct",
    "repo_best_ratio",
    "repo_pool_median_rpe_pct",
    "repo_pool_size",
    "selected_from_n",
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


def load_rescore(path: Path) -> dict[tuple[str, str], float]:
    if not path.is_file():
        return {}
    return {
        (Path(item["aggregate_path"]).name, item["variant_id"]): float(item["kitti_rte_trans_pct"])
        for item in json.loads(path.read_text())["rows"]
        if item.get("kitti_rte_trans_pct") is not None
    }


def collect_pools(
    results_dir: Path, selectors: set[str], rescore: dict[tuple[str, str], float] | None = None
) -> dict[tuple[str, str], list[dict[str, Any]]]:
    rescore = rescore or {}
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
            rte = variant.get("kitti_rte_trans_pct")
            if rte is None:
                rte = rescore.get((path.name, str(variant["id"])))
            pools.setdefault((selector, match.group(1)), []).append(
                {
                    "rpe": float(rpe),
                    "rte": float(rte) if rte is not None and math.isfinite(float(rte)) else None,
                    "variant": str(variant["id"]),
                    "aggregate": f"experiments/results/{path.name}",
                }
            )
    return pools


def collect_rows(
    paper_data: Path, results_dir: Path, rescore_path: Path | None = None
) -> list[dict[str, str]]:
    methods = json.loads(paper_data.read_text())["methods"]
    targets = {
        selector: info
        for selector, info in methods.items()
        if any(SEQUENCE_KEY.fullmatch(key) for key in info.get("reported_values", {}))
    }
    pools = collect_pools(results_dir, set(targets), load_rescore(rescore_path or results_dir / RESCORE_PATH.name))
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
            selected_from = len(pool)
            metric = "rpe_100m"
            if any(item["rte"] is not None for item in pool):
                metric = "kitti_rte"
                pool = [{**item, "rpe": item["rte"]} for item in pool if item["rte"] is not None]
            row = {
                "method": selector,
                "sequence": sequence,
                "paper_rte_pct": f"{float(paper_value):.2f}",
                "paper_source": source_text,
                "repo_metric": metric if pool else "",
                "repo_best_rpe_pct": "",
                "repo_best_ratio": "",
                "repo_pool_median_rpe_pct": "",
                "repo_pool_size": str(len(pool)),
                "selected_from_n": str(selected_from),
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
        "% Paper: official KITTI RTE (100-800 m). Repo: best non-GT-seeded variant selected on the",
        "% evaluated sequence (optimistic); KITTI RTE where recorded, else 100 m RPE (marked *).",
        r"\begin{tabular}{llrrrrr}",
        r"\toprule",
        r"Method & Seq. & Paper [\%] & Repo best [\%] & Ratio & Repo median [\%] & $n$ / sweep \\",
        r"\midrule",
    ]
    for row in rows:
        cells = [
            row["method"].replace("_", r"\_"),
            row["sequence"],
            row["paper_rte_pct"],
            (row["repo_best_rpe_pct"] + ("*" if row["repo_metric"] == "rpe_100m" else "")) or "--",
            f"{row['repo_best_ratio']}$\\times$" if row["repo_best_ratio"] else "--",
            row["repo_pool_median_rpe_pct"] or "--",
            f"{row['repo_pool_size']} / {row['selected_from_n']}",
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
