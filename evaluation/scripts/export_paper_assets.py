#!/usr/bin/env python3
"""Export paper-ready tables and Pareto figures from experiment aggregates."""

from __future__ import annotations

import csv
import json
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
DOCS_DIR = REPO_ROOT / "docs"
ASSETS_DIR = DOCS_DIR / "assets" / "paper"


@dataclass
class VariantPoint:
    problem_title: str
    selector: str
    dataset_name: str
    contract_type: str
    variant_id: str
    variant_label: str
    design_style: str
    ate_m: float
    fps: float
    decision: str
    aggregate_path: str
    is_default: bool


def relpath(path: Path | str) -> str:
    candidate = Path(path)
    try:
        return str(candidate.relative_to(REPO_ROOT))
    except ValueError:
        return str(candidate)


def short_dataset_name(dataset_path: str) -> str:
    return Path(dataset_path).name or dataset_path


def contract_type(problem: dict[str, Any], dataset: dict[str, Any]) -> str:
    problem_id = str(problem.get("id", "")).lower()
    title = str(problem.get("title", "")).lower()
    gt_csv = str(dataset.get("gt_csv", "")).lower()
    if "reference" in problem_id or "reference" in title or "reference" in gt_csv:
        return "reference-based"
    return "gt-backed"


def render_metric(value: float | None, digits: int = 3) -> str:
    if value is None:
        return "-"
    return f"{value:.{digits}f}"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def load_variant_points() -> list[VariantPoint]:
    index = load_json(RESULTS_DIR / "index.json")
    points: list[VariantPoint] = []
    for problem_entry in index["problems"]:
        if problem_entry["status"] != "ready":
            continue
        aggregate = load_json(REPO_ROOT / problem_entry["aggregate_path"])
        problem = aggregate["problem"]
        dataset = aggregate["dataset"]
        selector = str(aggregate["stable_interface"]["methods"])
        ctype = contract_type(problem, dataset)
        dataset_name = short_dataset_name(str(dataset["pcd_dir"]))
        current_default = problem_entry.get("current_default")
        for variant in aggregate["variants"]:
            if variant.get("status") != "OK":
                continue
            ate_m = variant.get("ate_m")
            fps = variant.get("fps")
            if ate_m is None or fps is None:
                continue
            points.append(
                VariantPoint(
                    problem_title=str(problem["title"]),
                    selector=selector,
                    dataset_name=dataset_name,
                    contract_type=ctype,
                    variant_id=str(variant["id"]),
                    variant_label=str(variant["label"]),
                    design_style=str(variant["design_style"]),
                    ate_m=float(ate_m),
                    fps=float(fps),
                    decision=str(variant["decision"]),
                    aggregate_path=problem_entry["aggregate_path"],
                    is_default=str(variant["id"]) == current_default,
                )
            )
    return points


def write_ready_defaults_csv(points: list[VariantPoint], output_path: Path) -> None:
    defaults = sorted(
        [point for point in points if point.is_default],
        key=lambda item: (item.selector, item.dataset_name, item.problem_title),
    )
    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            [
                "selector",
                "dataset",
                "contract_type",
                "variant_id",
                "variant_label",
                "ate_m",
                "fps",
                "aggregate_path",
            ]
        )
        for item in defaults:
            writer.writerow(
                [
                    item.selector,
                    item.dataset_name,
                    item.contract_type,
                    item.variant_id,
                    item.variant_label,
                    f"{item.ate_m:.6f}",
                    f"{item.fps:.6f}",
                    item.aggregate_path,
                ]
            )


def choose_core_default(points: list[VariantPoint], selector: str) -> VariantPoint | None:
    defaults = [point for point in points if point.is_default and point.selector == selector]
    if not defaults:
        return None
    gt_backed = [point for point in defaults if point.contract_type == "gt-backed"]
    source = gt_backed if gt_backed else defaults
    return min(source, key=lambda item: (item.ate_m, -item.fps, item.dataset_name))


def write_manuscript_core_csv(points: list[VariantPoint], output_path: Path) -> list[VariantPoint]:
    selectors = sorted({point.selector for point in points if point.is_default})
    chosen = [choose_core_default(points, selector) for selector in selectors]
    chosen = [item for item in chosen if item is not None]
    with output_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            [
                "selector",
                "dataset",
                "contract_type",
                "variant_id",
                "ate_m",
                "fps",
                "aggregate_path",
            ]
        )
        for item in chosen:
            writer.writerow(
                [
                    item.selector,
                    item.dataset_name,
                    item.contract_type,
                    item.variant_id,
                    f"{item.ate_m:.6f}",
                    f"{item.fps:.6f}",
                    item.aggregate_path,
                ]
            )
    return chosen


def render_default_pareto(points: list[VariantPoint], output_path: Path) -> None:
    defaults = [point for point in points if point.is_default]
    if not defaults:
        return
    fig, ax = plt.subplots(figsize=(11, 7))
    color_map = {
        "gt-backed": "#2563eb",
        "reference-based": "#dc2626",
    }
    marker_map = {
        "ct_icp": "o",
        "ct_lio": "s",
        "gicp": "^",
        "kiss_icp": "D",
        "litamin2": "P",
        "ndt": "X",
    }
    for item in defaults:
        ax.scatter(
            item.ate_m,
            item.fps,
            s=130,
            color=color_map.get(item.contract_type, "#475569"),
            marker=marker_map.get(item.selector, "o"),
            edgecolors="#111827",
            linewidths=0.8,
            alpha=0.9,
        )
        ax.annotate(
            f"{item.selector}@{item.dataset_name}",
            (item.ate_m, item.fps),
            textcoords="offset points",
            xytext=(6, 5),
            fontsize=8,
        )
    ax.set_title("Ready Default Variants: Accuracy / Throughput Pareto View")
    ax.set_xlabel("ATE [m] (lower is better)")
    ax.set_ylabel("FPS (higher is better)")
    ax.grid(alpha=0.25)
    ax.set_xscale("symlog", linthresh=1.0)
    contract_handles = [
        plt.Line2D([0], [0], marker="o", color="w", label=label, markerfacecolor=color, markersize=10)
        for label, color in color_map.items()
    ]
    ax.legend(handles=contract_handles, title="Contract", loc="best")
    fig.tight_layout()
    fig.savefig(output_path, dpi=180)
    plt.close(fig)


def render_core_method_figure(points: list[VariantPoint], output_path: Path) -> None:
    if not points:
        return
    fig, ax = plt.subplots(figsize=(10, 6.5))
    color_map = {
        "gt-backed": "#2563eb",
        "reference-based": "#dc2626",
    }
    for item in points:
        ax.scatter(
            item.ate_m,
            item.fps,
            s=160,
            color=color_map.get(item.contract_type, "#475569"),
            edgecolors="#111827",
            linewidths=0.9,
            alpha=0.92,
        )
        ax.annotate(
            f"{item.selector}\n{item.variant_id}",
            (item.ate_m, item.fps),
            textcoords="offset points",
            xytext=(7, 5),
            fontsize=8,
        )
    ax.set_title("Core Method Defaults for Manuscript Figures")
    ax.set_xlabel("ATE [m] (lower is better)")
    ax.set_ylabel("FPS (higher is better)")
    ax.grid(alpha=0.25)
    ax.set_xscale("symlog", linthresh=1.0)
    handles = [
        plt.Line2D([0], [0], marker="o", color="w", label=label, markerfacecolor=color, markersize=10)
        for label, color in color_map.items()
    ]
    ax.legend(handles=handles, title="Contract", loc="best")
    fig.tight_layout()
    fig.savefig(output_path, dpi=180)
    plt.close(fig)


def render_variant_fronts(points: list[VariantPoint], output_path: Path) -> None:
    if not points:
        return
    selectors = sorted({point.selector for point in points})
    ncols = 2
    nrows = (len(selectors) + ncols - 1) // ncols
    fig, axes = plt.subplots(nrows, ncols, figsize=(12, 4.5 * nrows))
    axes = axes.flatten()
    color_map = {
        "Adopt as current default": "#2563eb",
        "Keep as active challenger": "#f59e0b",
        "Keep as reference variant": "#64748b",
    }
    for ax, selector in zip(axes, selectors):
        subset = [point for point in points if point.selector == selector]
        for item in subset:
            ax.scatter(
                item.ate_m,
                item.fps,
                s=120 if item.is_default else 70,
                color=color_map.get(item.decision, "#94a3b8"),
                alpha=0.9 if item.is_default else 0.7,
                edgecolors="#111827" if item.is_default else "none",
                linewidths=0.8,
            )
            ax.annotate(
                f"{item.variant_id}@{item.dataset_name}",
                (item.ate_m, item.fps),
                textcoords="offset points",
                xytext=(5, 4),
                fontsize=7,
            )
        ax.set_title(selector)
        ax.set_xlabel("ATE [m]")
        ax.set_ylabel("FPS")
        ax.set_xscale("symlog", linthresh=1.0)
        ax.grid(alpha=0.25)
    for ax in axes[len(selectors) :]:
        ax.axis("off")
    handles = [
        plt.Line2D([0], [0], marker="o", color="w", label=label, markerfacecolor=color, markersize=9)
        for label, color in color_map.items()
    ]
    fig.legend(handles=handles, loc="upper center", ncol=3, frameon=False)
    fig.suptitle("Variant Fronts by Method Family", y=0.995)
    fig.tight_layout(rect=(0, 0, 1, 0.97))
    fig.savefig(output_path, dpi=180)
    plt.close(fig)


def render_caption_snippets(points: list[VariantPoint], core_defaults: list[VariantPoint], generated_at: str) -> str:
    defaults = [point for point in points if point.is_default]
    gt_backed_defaults = [point for point in defaults if point.contract_type == "gt-backed"]
    reference_defaults = [point for point in defaults if point.contract_type == "reference-based"]
    fastest = max(gt_backed_defaults, key=lambda item: item.fps) if gt_backed_defaults else None
    most_accurate = min(gt_backed_defaults, key=lambda item: item.ate_m) if gt_backed_defaults else None
    lines = [
        "# Paper Caption Snippets",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/export_paper_assets.py`._",
        "",
        "## Table Caption",
        "",
        "Table X. Current default variants selected from the experiment-driven benchmark contract.",
        f"The table summarizes {len(defaults)} ready defaults across {len(gt_backed_defaults)} GT-backed and {len(reference_defaults)} reference-based problem instances.",
        "",
        "## Pareto Figure Caption",
        "",
        "Figure X. Accuracy/throughput Pareto view of ready default variants under the shared benchmark contract.",
    ]
    if fastest and most_accurate:
        lines.append(
            f"In the current GT-backed subset, the fastest default is `{fastest.selector}:{fastest.variant_id}` at {fastest.fps:.1f} FPS, while the lowest ATE is `{most_accurate.selector}:{most_accurate.variant_id}` at {most_accurate.ate_m:.3f} m."
        )
    lines.extend(
        [
            "",
            "## Core Figure Caption",
            "",
            "Figure Y. Manuscript-facing core defaults, choosing one representative default per method family.",
            f"The core set currently contains {len(core_defaults)} method families and keeps `reference-based` outputs visually distinct from `gt-backed` outputs.",
            "",
            "## Variant Fronts Caption",
            "",
            "Figure Z. Variant fronts by method family. Each subplot shows how current defaults, active challengers, and reference variants move across the ATE/FPS plane under a single stable interface.",
        ]
    )
    return "\n".join(lines)


def render_markdown(points: list[VariantPoint], generated_at: str) -> str:
    defaults = sorted(
        [point for point in points if point.is_default],
        key=lambda item: (item.selector, item.dataset_name),
    )
    lines = [
        "# Paper Assets",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/export_paper_assets.py`._",
        "",
        "This page is the paper-facing cut of the experiment state.",
        "It keeps only comparable ready-problem outputs and highlights default variants first.",
        "",
        "## Files",
        "",
        f"- Pareto plot: [`ready_defaults_pareto.png`](assets/paper/ready_defaults_pareto.png)",
        f"- Variant fronts: [`variant_fronts_by_selector.png`](assets/paper/variant_fronts_by_selector.png)",
        f"- Core methods plot: [`manuscript_core_methods.png`](assets/paper/manuscript_core_methods.png)",
        f"- CSV export: [`ready_defaults.csv`](assets/paper/ready_defaults.csv)",
        f"- Manuscript core CSV: [`manuscript_core_defaults.csv`](assets/paper/manuscript_core_defaults.csv)",
        f"- Caption snippets: [`paper_captions.md`](paper_captions.md)",
        "",
        "## Ready Defaults",
        "",
        "| Method family | Dataset | Contract | Default variant | ATE [m] | FPS | Aggregate |",
        "|---------------|---------|----------|-----------------|---------|-----|-----------|",
    ]
    for item in defaults:
        lines.append(
            f"| {item.selector} | `{item.dataset_name}` | {item.contract_type} | `{item.variant_id}` | "
            f"{render_metric(item.ate_m)} | {render_metric(item.fps, digits=1)} | `{item.aggregate_path}` |"
        )
    lines.extend(
        [
            "",
            "## Notes",
            "",
            "- `reference-based` means the trajectory CSV is a shared reference export rather than an independently curated GT file.",
            "- `gt-backed` means the trajectory CSV is treated as the benchmark reference for that sequence.",
            "- Blocked problems are intentionally excluded from the Pareto views.",
        ]
    )
    return "\n".join(lines)


def main() -> None:
    points = load_variant_points()
    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    ASSETS_DIR.mkdir(parents=True, exist_ok=True)
    write_ready_defaults_csv(points, ASSETS_DIR / "ready_defaults.csv")
    core_defaults = write_manuscript_core_csv(points, ASSETS_DIR / "manuscript_core_defaults.csv")
    render_default_pareto(points, ASSETS_DIR / "ready_defaults_pareto.png")
    render_core_method_figure(core_defaults, ASSETS_DIR / "manuscript_core_methods.png")
    render_variant_fronts(points, ASSETS_DIR / "variant_fronts_by_selector.png")
    (DOCS_DIR / "paper_assets.md").write_text(render_markdown(points, generated_at) + "\n")
    (DOCS_DIR / "paper_captions.md").write_text(
        render_caption_snippets(points, core_defaults, generated_at) + "\n"
    )

    print(f"[done] wrote {relpath(ASSETS_DIR / 'ready_defaults.csv')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'manuscript_core_defaults.csv')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'ready_defaults_pareto.png')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'manuscript_core_methods.png')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'variant_fronts_by_selector.png')}")
    print(f"[done] wrote {relpath(DOCS_DIR / 'paper_assets.md')}")
    print(f"[done] wrote {relpath(DOCS_DIR / 'paper_captions.md')}")


if __name__ == "__main__":
    main()
