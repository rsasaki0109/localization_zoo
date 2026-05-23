#!/usr/bin/env python3
"""Run a small CT-ICP optimizer sweep on diagnostic-watch windows."""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
import subprocess
import sys
from collections import Counter
from pathlib import Path
from types import ModuleType
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
RUN_HEALTH = REPO_ROOT / "evaluation" / "scripts" / "run_lidar_degradation_health.py"
SUMMARY_SCRIPT = REPO_ROOT / "evaluation" / "scripts" / "summarize_lidar_degeneracy_health.py"
DEFAULT_COMPARISON_JSON = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "method_health_comparison"
    / "method_health_comparison.json"
)
DEFAULT_OUTPUT_DIR = (
    REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "ct_icp_diagnostic_watch_sweep"
)

SEQUENCE_WINDOWS = {
    "fog_200": REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "fog_200"
    / "window_selection"
    / "degradation_windows.json",
    "tunnel_geom_2700_200": REPO_ROOT
    / "experiments"
    / "results"
    / "lidar_degeneracy"
    / "tunnel_geom_2700_200"
    / "window_selection"
    / "degradation_windows.json",
}

BASE_CT_ICP_ARGS = [
    "--method",
    "ct_icp",
    "--ct-icp-refinement-gate",
    "--ct-icp-multi-scale",
    "--ct-icp-normal-cholesky",
    "--max-step-translation",
    "2.0",
    "--max-step-yaw-deg",
    "20",
]

VARIANTS = [
    {
        "name": "current",
        "description": "Current diagnostic-watch CT-ICP recipe.",
        "args": [],
    },
    {
        "name": "more_iter",
        "description": "Increase CT-ICP outer iterations from 8 to 16.",
        "args": ["--ct-icp-max-iterations", "16"],
    },
    {
        "name": "more_ceres",
        "description": "Increase Ceres inner iterations from 1 to 3.",
        "args": ["--ct-icp-ceres-max-iterations", "3"],
    },
    {
        "name": "relaxed_planarity",
        "description": "Relax keypoint planarity threshold from 0.08 to 0.05.",
        "args": ["--ct-icp-planarity-threshold", "0.05"],
    },
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison-json", type=Path, default=DEFAULT_COMPARISON_JSON)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument(
        "--skip-runs",
        action="store_true",
        help="Reuse existing variant health summaries and only rebuild the sweep report.",
    )
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def load_module(name: str, path: Path) -> ModuleType:
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load module: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def display_path(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def fmt(value: Any, digits: int = 3) -> str:
    if value is None:
        return "n/a"
    try:
        number = float(value)
    except (TypeError, ValueError):
        return "n/a"
    if not math.isfinite(number):
        return "n/a"
    return f"{number:.{digits}f}"


def as_float(value: Any) -> float | None:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def ratio(value: Any, reference: Any) -> float | None:
    value_number = as_float(value)
    reference_number = as_float(reference)
    if value_number is None or reference_number is None or abs(reference_number) < 1e-9:
        return None
    return value_number / reference_number


def mean(values: list[Any]) -> float | None:
    finite = [number for value in values if (number := as_float(value)) is not None]
    if not finite:
        return None
    return float(sum(finite) / len(finite))


def diagnostic_specs(comparison: dict[str, Any]) -> list[dict[str, Any]]:
    specs: list[dict[str, Any]] = []
    for sequence in comparison["sequences"]:
        sequence_name = str(sequence["sequence"])
        for window in sequence["windows"]:
            row = window["methods"].get("ct_icp")
            if not row or row.get("health_state") != "diagnostic_watch":
                continue
            specs.append(
                {
                    "sequence": sequence_name,
                    "window": window["name"],
                    "start": int(window["start"]),
                    "end": int(window["end"]),
                    "expected_stress": window.get("expected_stress"),
                    "healthy_path_median_m": row.get("healthy_path_median_m"),
                    "all_method_path_median_m": row.get("all_method_path_median_m"),
                }
            )
    return specs


def run_variant(variant: dict[str, Any], sequence: str, output_dir: Path) -> Path:
    health_dir = output_dir / "runs" / str(variant["name"]) / sequence / "ct_icp_health"
    cmd = [
        sys.executable,
        str(RUN_HEALTH),
        str(SEQUENCE_WINDOWS[sequence]),
        str(health_dir),
        *BASE_CT_ICP_ARGS,
        *variant["args"],
    ]
    print(f"[run] {variant['name']} {sequence}")
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)
    return health_dir / "odometry_health_summary.json"


def index_summary(summary: dict[str, Any]) -> dict[tuple[str, int, int], dict[str, Any]]:
    return {
        (
            str(row["name"]),
            int(row["window"]["start"]),
            int(row["window"]["end"]),
        ): row
        for row in summary["windows"]
    }


def build_rows(
    *,
    variants: list[dict[str, Any]],
    specs: list[dict[str, Any]],
    output_dir: Path,
    summarize_module: ModuleType,
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    specs_by_sequence: dict[str, list[dict[str, Any]]] = {}
    for spec in specs:
        specs_by_sequence.setdefault(str(spec["sequence"]), []).append(spec)

    for variant in variants:
        variant_name = str(variant["name"])
        for sequence, sequence_specs in specs_by_sequence.items():
            summary_path = (
                output_dir
                / "runs"
                / variant_name
                / sequence
                / "ct_icp_health"
                / "odometry_health_summary.json"
            )
            summary = load_json(summary_path)
            summary_rows = index_summary(summary)
            parameters = summary.get("parameters") or {}
            for spec in sequence_specs:
                key = (str(spec["window"]), int(spec["start"]), int(spec["end"]))
                source = summary_rows[key]
                path_vs_healthy = ratio(
                    source.get("used_path_length_m"),
                    spec.get("healthy_path_median_m"),
                )
                path_vs_all = ratio(
                    source.get("used_path_length_m"),
                    spec.get("all_method_path_median_m"),
                )
                status_row = {
                    "health_state": "diagnostic_watch",
                    "accepted_rate": source.get("accepted_rate"),
                    "ct_icp_refinement_gate_rate": source.get("ct_icp_refinement_gate_rate"),
                    "ct_icp_iterations_mean": source.get("ct_icp_iterations_mean"),
                    "ct_icp_max_iterations": parameters.get("ct_icp_max_iterations"),
                    "path_vs_healthy_median": path_vs_healthy,
                    "path_vs_all_median": path_vs_all,
                }
                status = summarize_module.ct_icp_watch_clear_status(status_row)
                rows.append(
                    {
                        "variant": variant_name,
                        "variant_description": variant["description"],
                        "sequence": sequence,
                        "window": spec["window"],
                        "start": spec["start"],
                        "end": spec["end"],
                        "expected_stress": spec["expected_stress"],
                        "accepted_rate": source.get("accepted_rate"),
                        "converged_rate": source.get("converged_rate"),
                        "ct_icp_refinement_gate_rate": source.get("ct_icp_refinement_gate_rate"),
                        "ct_icp_iterations_mean": source.get("ct_icp_iterations_mean"),
                        "ct_icp_max_iterations": parameters.get("ct_icp_max_iterations"),
                        "ct_icp_ceres_max_iterations": parameters.get("ct_icp_ceres_max_iterations"),
                        "ct_icp_planarity_threshold": parameters.get("ct_icp_planarity_threshold"),
                        "used_path_length_m": source.get("used_path_length_m"),
                        "path_vs_healthy_median": path_vs_healthy,
                        "path_vs_all_median": path_vs_all,
                        "runtime_s": source.get("runtime_s"),
                        "health_flags": source.get("health_flags"),
                        **status,
                    }
                )
    return rows


def aggregate_rows(rows: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    aggregates: dict[str, dict[str, Any]] = {}
    for variant in sorted({str(row["variant"]) for row in rows}):
        variant_rows = [row for row in rows if row["variant"] == variant]
        blockers = Counter(
            blocker
            for row in variant_rows
            for blocker in row.get("watch_clear_blockers", [])
        )
        actions = Counter(str(row.get("watch_action") or "n/a") for row in variant_rows)
        first = variant_rows[0]
        aggregates[variant] = {
            "rows": len(variant_rows),
            "ct_icp_max_iterations": first.get("ct_icp_max_iterations"),
            "ct_icp_ceres_max_iterations": first.get("ct_icp_ceres_max_iterations"),
            "ct_icp_planarity_threshold": first.get("ct_icp_planarity_threshold"),
            "mean_accepted_rate": mean([row.get("accepted_rate") for row in variant_rows]),
            "mean_converged_rate": mean([row.get("converged_rate") for row in variant_rows]),
            "mean_refinement_gate_rate": mean(
                [row.get("ct_icp_refinement_gate_rate") for row in variant_rows]
            ),
            "mean_iterations": mean([row.get("ct_icp_iterations_mean") for row in variant_rows]),
            "mean_path_vs_healthy": mean([row.get("path_vs_healthy_median") for row in variant_rows]),
            "mean_path_vs_all": mean([row.get("path_vs_all_median") for row in variant_rows]),
            "clear_candidates": sum(1 for row in variant_rows if row.get("watch_clear_candidate")),
            "action_counts": dict(sorted(actions.items())),
            "blocker_counts": dict(sorted(blockers.items())),
        }
    return aggregates


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    lines = [
        "# CT-ICP Diagnostic Watch Sweep",
        "",
        f"Comparison source: `{payload['comparison_json']}`",
        "",
        "## Variants",
        "",
        "| Variant | max iter | ceres iter | planarity | Rows | Accepted | Converged | CT gate | CT iter | Path/healthy | Path/all | Clear | Actions | Blockers |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |",
    ]
    for variant in [variant["name"] for variant in VARIANTS]:
        row = payload["aggregates"][variant]
        blockers = ", ".join(
            f"{name}:{count}" for name, count in row["blocker_counts"].items()
        ) or "none"
        actions = ", ".join(
            f"{name}:{count}" for name, count in row["action_counts"].items()
        ) or "none"
        lines.append(
            f"| `{variant}` | {row['ct_icp_max_iterations']} | "
            f"{row['ct_icp_ceres_max_iterations']} | "
            f"{fmt(row['ct_icp_planarity_threshold'])} | {row['rows']} | "
            f"{fmt(row['mean_accepted_rate'])} | {fmt(row['mean_converged_rate'])} | "
            f"{fmt(row['mean_refinement_gate_rate'])} | {fmt(row['mean_iterations'])} | "
            f"{fmt(row['mean_path_vs_healthy'])} | {fmt(row['mean_path_vs_all'])} | "
            f"{row['clear_candidates']} | {actions} | {blockers} |"
        )

    lines.extend(
        [
            "",
            "## Rows",
            "",
            "| Variant | Sequence | Window | Accepted | Converged | CT gate | CT iter | Path/healthy | Path/all | Clear? | Action | Blockers |",
            "| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- |",
        ]
    )
    for row in payload["rows"]:
        blockers = ", ".join(row.get("watch_clear_blockers", [])) or "none"
        clear = "yes" if row.get("watch_clear_candidate") else "no"
        action = row.get("watch_action") or "n/a"
        lines.append(
            f"| `{row['variant']}` | `{row['sequence']}` | "
            f"`{row['window']}` {row['start']}-{row['end']} | "
            f"{fmt(row['accepted_rate'])} | {fmt(row['converged_rate'])} | "
            f"{fmt(row['ct_icp_refinement_gate_rate'])} | "
            f"{fmt(row['ct_icp_iterations_mean'])} | "
            f"{fmt(row['path_vs_healthy_median'])} | "
            f"{fmt(row['path_vs_all_median'])} | {clear} | `{action}` | {blockers} |"
        )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- This sweep only probes the diagnostic-watch CT-ICP windows; it does not change the main policy gate.",
            "- `iterations_pinned` means the mean CT-ICP iteration count remains within 0.5 of the configured max iterations.",
            "- A row can be a clear candidate only when acceptance, refinement-gate rate, iteration headroom, and path-ratio checks all pass.",
            "- `fallback_required` means CT-ICP is accepted but its trajectory path disagrees with the healthy/all-method path reference enough that a production component should reject, downweight, or fall back instead of silently trusting it.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    comparison = load_json(args.comparison_json)
    specs = diagnostic_specs(comparison)
    if not specs:
        raise RuntimeError("No CT-ICP diagnostic-watch rows found in comparison report")

    if not args.skip_runs:
        for variant in VARIANTS:
            for sequence in sorted({str(spec["sequence"]) for spec in specs}):
                run_variant(variant, sequence, args.output_dir)

    summarize_module = load_module("summarize_lidar_degeneracy_health", SUMMARY_SCRIPT)
    rows = build_rows(
        variants=VARIANTS,
        specs=specs,
        output_dir=args.output_dir,
        summarize_module=summarize_module,
    )
    payload = {
        "comparison_json": display_path(args.comparison_json),
        "output_dir": display_path(args.output_dir),
        "variants": VARIANTS,
        "diagnostic_windows": specs,
        "aggregates": aggregate_rows(rows),
        "rows": rows,
    }

    output_json = args.output_dir / "ct_icp_diagnostic_watch_sweep.json"
    output_md = args.output_dir / "ct_icp_diagnostic_watch_sweep.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {display_path(output_json)} and {display_path(output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
