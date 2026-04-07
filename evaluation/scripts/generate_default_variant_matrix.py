#!/usr/bin/env python3
"""Build method × dataset default-variant matrix and instability heatmap for the paper.

Reads experiments/results/index.json plus each aggregate JSON for (methods, pcd_dir).
Writes under docs/assets/paper/:
  - default_variant_matrix.csv       — wide; methods as rows, dataset slugs as columns
  - default_variant_matrix_long.csv   — long form for pivot tools
  - default_variant_instability.png  — green = matches row plurality default, red = differs, gray = empty
"""

from __future__ import annotations

import argparse
import csv
import json
from collections import Counter
from pathlib import Path
from typing import Any

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

REPO_ROOT = Path(__file__).resolve().parents[2]
INDEX_PATH = REPO_ROOT / "experiments" / "results" / "index.json"
ASSETS_DIR = REPO_ROOT / "docs" / "assets" / "paper"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=ASSETS_DIR,
        help="Directory for CSV/PNG (default: docs/assets/paper)",
    )
    parser.add_argument("--no-plot", action="store_true", help="Skip PNG heatmap")
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def dataset_slug(aggregate: dict[str, Any]) -> str:
    pcd = str(aggregate.get("dataset", {}).get("pcd_dir", ""))
    return Path(pcd).name or pcd or "unknown"


def method_selector(aggregate: dict[str, Any]) -> str:
    return str(aggregate["stable_interface"]["methods"])


def dataset_sort_key(slug: str) -> tuple:
    s = slug.lower()
    if s.startswith("autoware_istanbul"):
        return (0, slug)
    if s.startswith("hdl") or "hdl_400" in s:
        return (1, slug)
    if "kitti" in s:
        return (2, slug)
    if s.startswith("mcd") or "_mcd_" in s:
        return (3, slug)
    return (9, slug)


def build_matrix() -> dict[str, dict[str, str]]:
    """Return matrix[method][dataset_slug] -> elected default variant id."""
    index = load_json(INDEX_PATH)
    matrix: dict[str, dict[str, str]] = {}
    for entry in index["problems"]:
        if entry["status"] != "ready":
            continue
        default = entry.get("current_default")
        if not default:
            continue
        agg_path = REPO_ROOT / entry["aggregate_path"]
        if not agg_path.is_file():
            continue
        aggregate = load_json(agg_path)
        m = method_selector(aggregate)
        slug = dataset_slug(aggregate)
        matrix.setdefault(m, {})[slug] = str(default)
    return matrix


def write_wide_csv(matrix: dict[str, dict[str, str]], columns: list[str], out: Path) -> None:
    methods = sorted(matrix.keys())
    with out.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["method", *columns])
        for m in methods:
            row = [m] + [matrix[m].get(c, "") for c in columns]
            writer.writerow(row)


def write_long_csv(matrix: dict[str, dict[str, str]], out: Path) -> None:
    with out.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["method", "dataset_slug", "default_variant_id"])
        for m in sorted(matrix.keys()):
            for slug in sorted(matrix[m].keys(), key=dataset_sort_key):
                writer.writerow([m, slug, matrix[m][slug]])


def plurality_default(variants: list[str]) -> str | None:
    counts = Counter(variants)
    if not counts:
        return None
    return counts.most_common(1)[0][0]


def render_instability_heatmap(
    matrix: dict[str, dict[str, str]],
    columns: list[str],
    out: Path,
) -> None:
    methods = sorted(matrix.keys())
    n_rows, n_cols = len(methods), len(columns)
    # RGBA image [H, W, 4]; origin top for imshow with method0 at top
    img = np.ones((n_rows, n_cols, 4), dtype=float)
    # empty: light gray
    empty = np.array([0.92, 0.92, 0.92, 1.0])
    match_c = np.array([0.65, 0.85, 0.65, 1.0])  # green
    diff_c = np.array([0.95, 0.55, 0.55, 1.0])  # red

    for i, m in enumerate(methods):
        row_vals = [matrix[m].get(c) for c in columns]
        present = [v for v in row_vals if v]
        mode = plurality_default(present)
        for j, v in enumerate(row_vals):
            if not v:
                img[i, j] = empty
            elif mode and v == mode:
                img[i, j] = match_c
            else:
                img[i, j] = diff_c

    fig_w = max(10.0, 0.22 * n_cols + 3.0)
    fig_h = max(4.0, 0.35 * n_rows + 1.5)
    fig, ax = plt.subplots(figsize=(fig_w, fig_h))
    ax.imshow(img, aspect="auto", interpolation="nearest")
    ax.set_xticks(np.arange(n_cols))
    ax.set_yticks(np.arange(n_rows))
    ax.set_xticklabels(columns, rotation=75, ha="right", fontsize=7)
    ax.set_yticklabels(methods, fontsize=8)
    ax.set_xlabel("Dataset (pcd_dir basename)")
    ax.set_ylabel("Method --methods selector")

    for i, m in enumerate(methods):
        for j, c in enumerate(columns):
            v = matrix[m].get(c)
            if not v:
                continue
            ax.text(
                j,
                i,
                v.replace("_", "\n") if len(v) > 18 else v,
                ha="center",
                va="center",
                color="black",
                fontsize=5,
                linespacing=0.85,
            )

    ax.set_title(
        "Elected default variant: green = matches row plurality; red = differs; gray = no run",
        fontsize=9,
    )
    fig.tight_layout()
    fig.savefig(out, dpi=160)
    plt.close(fig)


def main() -> int:
    args = parse_args()
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    matrix = build_matrix()
    all_slugs = sorted(
        {s for rows in matrix.values() for s in rows},
        key=dataset_sort_key,
    )

    wide_path = out_dir / "default_variant_matrix.csv"
    long_path = out_dir / "default_variant_matrix_long.csv"
    png_path = out_dir / "default_variant_instability.png"

    write_wide_csv(matrix, all_slugs, wide_path)
    write_long_csv(matrix, long_path)
    if not args.no_plot:
        render_instability_heatmap(matrix, all_slugs, png_path)

    print(f"[done] wrote {wide_path.relative_to(REPO_ROOT)} ({len(matrix)} methods × {len(all_slugs)} datasets)")
    print(f"[done] wrote {long_path.relative_to(REPO_ROOT)}")
    if not args.no_plot:
        print(f"[done] wrote {png_path.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
