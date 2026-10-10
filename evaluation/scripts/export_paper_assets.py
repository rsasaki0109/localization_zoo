#!/usr/bin/env python3
"""Export paper-ready tables and Pareto figures from experiment aggregates."""

from __future__ import annotations

import csv
import json
import math
import re
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import matplotlib
import matplotlib.ticker

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


# IMU-only selectors read no point clouds, so their FPS and ATE are not
# comparable with the LiDAR/visual Pareto figures. They stay in Table 5.
NON_LIDAR_SELECTORS = {"imu_dead_reckoning", "odonet", "nhc_net", "nn_zupt"}


def contract_type(problem: dict[str, Any], dataset: dict[str, Any]) -> str:
    # Only the GT file name is checked: every GT CSV lives under
    # experiments/reference_data/, so matching the full path marks everything.
    problem_id = str(problem.get("id", "")).lower()
    title = str(problem.get("title", "")).lower()
    gt_name = Path(str(dataset.get("gt_csv", ""))).name.lower()
    if "reference" in problem_id or "reference" in title or "reference" in gt_name:
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
        if selector in NON_LIDAR_SELECTORS:
            continue
        ctype = contract_type(problem, dataset)
        dataset_name = short_dataset_name(str(dataset["pcd_dir"]))
        current_default = problem_entry.get("current_default")
        for variant in aggregate["variants"]:
            if variant.get("status") not in {"OK", "ok"}:
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
        writer = csv.writer(handle, lineterminator="\n")
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
        writer = csv.writer(handle, lineterminator="\n")
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


def _pareto_front_indices(xs: list[float], ys: list[float]) -> set[int]:
    """Return indices on the Pareto front (minimize x, maximize y)."""
    indexed = sorted(range(len(xs)), key=lambda i: (xs[i], -ys[i]))
    front: set[int] = set()
    best_y = -float("inf")
    for i in indexed:
        if ys[i] > best_y:
            front.add(i)
            best_y = ys[i]
    return front


# Figure 1 compares every method on one shared benchmark instead of mixing
# windows: KITTI Odometry 07 (full, 1101 frames), pure odometry only.
PARETO_DATASET = "kitti_seq_07_full"
# Diverged variants (RPE in the tens of percent) would stretch the log axis;
# they are counted in the footnote instead.
PARETO_MAX_RPE_PCT = 5.0
GT_SEEDED_NOTE = re.compile(r"GT-seeded|Seeds .* with GT", re.IGNORECASE)
# Categorical slots in fixed order (dataviz reference palette, light mode,
# validated: CVD and normal-vision separation pass). Three slots sit below 3:1
# contrast on white, so every method is also direct-labelled and has its own
# marker shape; only methods on the Pareto front get a direct label so the
# dense lower-left cluster stays readable (the legend names every method).
CATEGORICAL = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100",
               "#e87ba4", "#008300", "#4a3aa7", "#e34948"]
MARKERS = ["o", "s", "^", "D", "v", "P", "X", "h"]
INK = "#1f2328"
INK_MUTED = "#6e7781"
GRID = "#e6e6e3"


@dataclass
class OdometryPoint:
    selector: str
    variant_id: str
    rpe_trans_pct: float
    fps: float
    aggregate_path: str


def load_pareto_points(results_dir: Path, dataset: str = PARETO_DATASET) -> list[OdometryPoint]:
    points: list[OdometryPoint] = []
    for path in sorted(results_dir.glob("*_matrix.json")):
        aggregate = load_json(path)
        if Path(str(aggregate.get("dataset", {}).get("pcd_dir", ""))).name != dataset:
            continue
        selector = str(aggregate["stable_interface"]["methods"])
        for variant in aggregate.get("variants", []):
            rpe, fps = variant.get("rpe_trans_pct"), variant.get("fps")
            # NaN RPE (diverged runs) would also break the sort in pareto_front.
            if rpe is None or fps is None or not math.isfinite(float(rpe)) or not float(fps) > 0:
                continue
            # paper_input variants feed modified scans (e.g. KITTI elevation
            # correction); keep the front on the shared raw-scan input.
            if GT_SEEDED_NOTE.search(str(variant.get("note", ""))) or variant.get(
                    "design_style") in ("ablation", "paper_input"):
                continue
            points.append(OdometryPoint(selector, str(variant["id"]), float(rpe), float(fps), relpath(path)))
    return points


def pareto_front(points: list[OdometryPoint]) -> list[OdometryPoint]:
    """Points no other point beats on both lower RPE and higher FPS."""
    front: list[OdometryPoint] = []
    best_fps = -1.0
    for point in sorted(points, key=lambda item: (item.rpe_trans_pct, -item.fps)):
        if point.fps > best_fps:
            front.append(point)
            best_fps = point.fps
    return front


def render_odometry_pareto(points: list[OdometryPoint], output_path: Path) -> None:
    if not points:
        return
    # Fixed slot per method, ordered by each method's best RPE so the legend
    # reads best-first; slots never cycle (8 methods fit 8 slots).
    best = {}
    for point in points:
        if point.selector not in best or point.rpe_trans_pct < best[point.selector].rpe_trans_pct:
            best[point.selector] = point
    methods = sorted(best, key=lambda name: best[name].rpe_trans_pct)
    # Categorical slots never cycle: beyond the eighth method (ranked by best
    # RPE) the rest fold into one muted "other" series.
    # Which methods get a slot is decided by rank; the slot itself follows the
    # method name so a method keeps its colour when others are added.
    named = sorted(methods[: len(CATEGORICAL)])
    others = methods[len(CATEGORICAL):]
    front = pareto_front(points)
    shown = [p for p in points if p.rpe_trans_pct <= PARETO_MAX_RPE_PCT]
    hidden = len(points) - len(shown)

    fig, ax = plt.subplots(figsize=(10, 6.5))
    fig.patch.set_facecolor("white")
    ax.set_facecolor("white")
    ax.step([p.rpe_trans_pct for p in front], [p.fps for p in front], where="post",
            color=INK_MUTED, linewidth=1.5, zorder=2, label="Pareto front")
    for index, name in enumerate(named):
        mine = [p for p in shown if p.selector == name]
        total = sum(1 for p in points if p.selector == name)
        ax.scatter([p.rpe_trans_pct for p in mine], [p.fps for p in mine], s=56,
                   marker=MARKERS[index], color=CATEGORICAL[index], edgecolors="white",
                   linewidths=1.5, zorder=3, label=f"{name} ({total})")
    if others:
        mine = [p for p in shown if p.selector in others]
        total = sum(1 for p in points if p.selector in others)
        ax.scatter([p.rpe_trans_pct for p in mine], [p.fps for p in mine], s=40,
                   marker="o", color=INK_MUTED, edgecolors="white", linewidths=1.5,
                   zorder=2, label=f"other: {', '.join(others)} ({total})")
    # One label per front method, at its highest-throughput front point.
    front_top = {point.selector: point for point in front}
    for point in front_top.values():
        ax.annotate(f"{point.selector}  {point.rpe_trans_pct:.2f} %, {point.fps:.0f} FPS",
                    (point.rpe_trans_pct, point.fps), textcoords="offset points",
                    xytext=(10, -3), fontsize=9, color=INK, zorder=4)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xticks([0.5, 0.6, 0.8, 1, 1.5, 2, 3, 4])
    ax.set_yticks([2, 5, 10, 20, 50, 100])
    for axis in (ax.xaxis, ax.yaxis):
        axis.set_major_formatter(matplotlib.ticker.FormatStrFormatter("%g"))
        axis.set_minor_formatter(matplotlib.ticker.NullFormatter())
    ax.set_xlabel("Translational RPE, 100 m segments [%]  (lower is better)", color=INK, fontsize=11)
    ax.set_ylabel("Throughput [FPS]  (higher is better)", color=INK, fontsize=11)
    ax.set_title(
        f"KITTI Odometry 07 (full, 1101 frames): {len(points)} pure-odometry variants of {len(methods)} methods",
        color=INK, fontsize=12, pad=10, loc="left")
    ax.grid(which="major", color=GRID, linewidth=1.0, linestyle="-")
    ax.set_axisbelow(True)
    for spine in ax.spines.values():
        spine.set_color(GRID)
    ax.tick_params(colors=INK_MUTED, labelsize=9)
    legend = ax.legend(title="Method (variants)", fontsize=9, title_fontsize=9, frameon=False,
                       loc="upper left", bbox_to_anchor=(1.01, 1.0))
    for text in legend.get_texts():
        text.set_color(INK)
    footnote = ("GT-seeded, ablation, corrected-input, and diverged (NaN) variants excluded. FPS comes from stored aggregates "
                "and is not normalized across hosts or concurrent load.")
    if hidden:
        footnote += f"\n{hidden} variant(s) with RPE > {PARETO_MAX_RPE_PCT:g} % are off the x-axis (legend counts include them)."
    fig.text(0.01, 0.01, footnote, fontsize=8, color=INK_MUTED, va="bottom")
    fig.tight_layout(rect=(0, 0.05, 1, 1))
    fig.savefig(output_path, dpi=200, facecolor="white")
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
        f"- Pareto plot (Figure 1): [`kitti07_pareto.png`](assets/paper/kitti07_pareto.png)",
        f"- Variant fronts: [`variant_fronts_by_selector.png`](assets/paper/variant_fronts_by_selector.png)",
        f"- Core methods plot: [`manuscript_core_methods.png`](assets/paper/manuscript_core_methods.png)",
        f"- CSV export: [`ready_defaults.csv`](assets/paper/ready_defaults.csv)",
        f"- Manuscript core CSV: [`manuscript_core_defaults.csv`](assets/paper/manuscript_core_defaults.csv)",
        f"- Default matrix (Table 3): [`default_variant_matrix.csv`](assets/paper/default_variant_matrix.csv), "
        f"[`default_variant_matrix_long.csv`](assets/paper/default_variant_matrix_long.csv)",
        f"- Full variant results (Table 5): [`full_variant_results.csv`](assets/paper/full_variant_results.csv), "
        f"[`full_variant_results.tex`](assets/paper/full_variant_results.tex)",
        f"- Continuous-time appendix (Table 7): [`ct_appendix.csv`](assets/paper/ct_appendix.csv), "
        f"[`ct_appendix.tex`](assets/paper/ct_appendix.tex)",
        f"- Original-paper comparison (Table 6): [`paper_ratio_table.csv`](assets/paper/paper_ratio_table.csv), "
        f"[`paper_ratio_table.tex`](assets/paper/paper_ratio_table.tex)",
        f"- Default instability figure: [`default_variant_instability.png`](assets/paper/default_variant_instability.png)",
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
    render_odometry_pareto(load_pareto_points(RESULTS_DIR), ASSETS_DIR / "kitti07_pareto.png")
    render_core_method_figure(core_defaults, ASSETS_DIR / "manuscript_core_methods.png")
    render_variant_fronts(points, ASSETS_DIR / "variant_fronts_by_selector.png")
    (DOCS_DIR / "paper_assets.md").write_text(render_markdown(points, generated_at) + "\n")
    (DOCS_DIR / "paper_captions.md").write_text(
        render_caption_snippets(points, core_defaults, generated_at) + "\n"
    )

    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "evaluation/scripts/generate_default_variant_matrix.py"),
            "--output-dir",
            str(ASSETS_DIR),
        ],
        cwd=str(REPO_ROOT),
        check=True,
    )

    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "evaluation/scripts/generate_full_variant_table.py"),
            "--output-dir",
            str(ASSETS_DIR),
        ],
        cwd=str(REPO_ROOT),
        check=True,
    )
    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "evaluation/scripts/generate_ct_appendix_table.py"),
            "--output-dir",
            str(ASSETS_DIR),
        ],
        cwd=str(REPO_ROOT),
        check=True,
    )
    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "evaluation/scripts/generate_paper_ratio_table.py"),
            "--output-dir",
            str(ASSETS_DIR),
        ],
        cwd=str(REPO_ROOT),
        check=True,
    )

    print(f"[done] wrote {relpath(ASSETS_DIR / 'ready_defaults.csv')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'manuscript_core_defaults.csv')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'kitti07_pareto.png')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'manuscript_core_methods.png')}")
    print(f"[done] wrote {relpath(ASSETS_DIR / 'variant_fronts_by_selector.png')}")
    print(f"[done] wrote {relpath(DOCS_DIR / 'paper_assets.md')}")
    print(f"[done] wrote {relpath(DOCS_DIR / 'paper_captions.md')}")


if __name__ == "__main__":
    main()
