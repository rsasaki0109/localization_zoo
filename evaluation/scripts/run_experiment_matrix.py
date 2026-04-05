#!/usr/bin/env python3
"""Run experiment manifests and regenerate experiment-facing docs."""

from __future__ import annotations

import argparse
import json
import math
import shlex
import subprocess
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]


@dataclass
class VariantResult:
    id: str
    label: str
    design_style: str
    intent: str
    args: list[str]
    command: str
    summary_path: str
    log_path: str
    status: str
    ate_m: float | None
    fps: float | None
    time_ms: float | None
    frames: int | None
    note: str
    benchmark_score: float
    readability_score: float
    readability_note: str
    extensibility_score: float
    extensibility_note: str
    decision: str = ""
    decision_reason: str = ""


@dataclass
class ProblemRun:
    manifest_path: str
    manifest_stem: str
    problem_id: str
    title: str
    question: str
    problem_state: str
    blocker: str
    next_step: str
    dataset_pcd_dir: str
    dataset_gt_csv: str
    binary_path: str
    method_selector: str
    primary_method: str
    same_metrics: list[str]
    aggregate_relpath: str
    results: list[VariantResult]
    planned_variants: list[dict[str, Any]]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run experiment manifests against pcd_dogfooding and regenerate docs."
    )
    parser.add_argument(
        "--manifest",
        action="append",
        default=[],
        help="Manifest path relative to repo root unless absolute. Can be passed multiple times. Defaults to all experiments/*_matrix.json.",
    )
    parser.add_argument(
        "--binary",
        default=None,
        help="Benchmark binary path override applied to every manifest.",
    )
    parser.add_argument(
        "--pcd-dir",
        default=None,
        help="Override PCD directory for every manifest in the current run.",
    )
    parser.add_argument(
        "--gt-csv",
        default=None,
        help="Override reference CSV for every manifest in the current run.",
    )
    parser.add_argument(
        "--output-dir",
        default="experiments/results",
        help="Directory for aggregate results and per-run artifacts.",
    )
    parser.add_argument(
        "--docs-dir",
        default="docs",
        help="Directory where experiments.md, decisions.md, and interfaces.md are written.",
    )
    parser.add_argument(
        "--reuse-existing",
        action="store_true",
        help="Reuse existing per-variant summary.json files when present instead of rerunning the benchmark.",
    )
    return parser.parse_args()


def resolve_path(path_str: str | None) -> Path | None:
    if path_str is None:
        return None
    path = Path(path_str)
    if path.is_absolute():
        return path
    return REPO_ROOT / path


def relpath(path: Path | str) -> str:
    candidate = Path(path)
    try:
        return str(candidate.relative_to(REPO_ROOT))
    except ValueError:
        return str(candidate)


def discover_manifest_paths(manifest_args: list[str]) -> list[Path]:
    if manifest_args:
        paths = [resolve_path(item) for item in manifest_args]
    else:
        paths = sorted((REPO_ROOT / "experiments").glob("*_matrix.json"))
    resolved = []
    for path in paths:
        if path is None or not path.exists():
            raise FileNotFoundError(f"Manifest not found: {path}")
        resolved.append(path)
    if not resolved:
        raise FileNotFoundError("No experiment manifests found under experiments/*_matrix.json")
    return resolved


def load_manifest(path: Path) -> dict[str, Any]:
    data = json.loads(path.read_text())
    required_top = ["problem", "stable_interface", "variants"]
    for key in required_top:
        if key not in data:
            raise ValueError(f"Manifest is missing top-level key: {key}")
    if not data["variants"]:
        raise ValueError("Manifest must contain at least one variant.")
    return data


def split_flag_surface(args: list[str]) -> tuple[int, int]:
    flag_count = 0
    valued_flag_count = 0
    i = 0
    while i < len(args):
        arg = args[i]
        if arg.startswith("--"):
            flag_count += 1
            if "=" in arg:
                valued_flag_count += 1
            elif i + 1 < len(args) and not args[i + 1].startswith("--"):
                valued_flag_count += 1
                i += 1
        i += 1
    return flag_count, valued_flag_count


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def compute_proxy_scores(args: list[str]) -> tuple[float, str, float, str]:
    flag_count, valued_flag_count = split_flag_surface(args)
    readability = clamp(5.0 - 0.35 * flag_count - 0.25 * valued_flag_count, 1.0, 5.0)
    extensibility = clamp(5.0 - 0.25 * flag_count - 0.15 * valued_flag_count, 1.0, 5.0)

    if flag_count == 0:
        readability_note = "Uses the default CLI surface only."
        extensibility_note = "No extra profile knobs beyond the stable core contract."
    elif valued_flag_count == 0:
        readability_note = "Adds only boolean toggles on top of the stable CLI."
        extensibility_note = "Still stays inside the stable CLI, but expands the toggle surface."
    else:
        readability_note = "Adds extra tuning knobs and therefore more command complexity."
        extensibility_note = "Still stable-interface compatible, but with a larger parameter surface."
    return readability, readability_note, extensibility, extensibility_note


def build_command(
    binary: Path,
    pcd_dir: Path,
    gt_csv: Path,
    methods: str,
    args: list[str],
    summary_path: Path,
) -> list[str]:
    return [
        str(binary),
        str(pcd_dir),
        str(gt_csv),
        "--methods",
        methods,
        "--summary-json",
        str(summary_path),
        *args,
    ]


def display_command(
    binary: Path,
    pcd_dir: Path,
    gt_csv: Path,
    methods: str,
    args: list[str],
    summary_path: Path,
) -> str:
    tokens = [
        relpath(binary),
        relpath(pcd_dir),
        relpath(gt_csv),
        "--methods",
        methods,
        "--summary-json",
        relpath(summary_path),
        *args,
    ]
    return " ".join(shlex.quote(token) for token in tokens)


def method_name_matches(name: str, primary_method: str) -> bool:
    if name == primary_method:
        return True
    if not name.startswith(primary_method):
        return False
    if len(name) == len(primary_method):
        return True
    suffix_start = name[len(primary_method)]
    return not suffix_start.isalnum()


def variant_result_from_summary(
    *,
    variant: dict[str, Any],
    primary_method: str,
    binary: Path,
    pcd_dir: Path,
    gt_csv: Path,
    methods: str,
    summary_path: Path,
    log_path: Path,
) -> VariantResult:
    summary = json.loads(summary_path.read_text())
    method_result = None
    for item in summary["methods"]:
        if method_name_matches(str(item["name"]), primary_method):
            method_result = item
            break
    if method_result is None:
        raise RuntimeError(
            f"Primary method {primary_method} not found in summary for {variant['id']}"
        )

    readability, readability_note, extensibility, extensibility_note = compute_proxy_scores(
        variant["args"]
    )
    return VariantResult(
        id=variant["id"],
        label=variant["label"],
        design_style=variant["design_style"],
        intent=variant["intent"],
        args=list(variant["args"]),
        command=display_command(binary, pcd_dir, gt_csv, methods, variant["args"], summary_path),
        summary_path=relpath(summary_path),
        log_path=relpath(log_path),
        status=str(method_result["status"]).upper(),
        ate_m=method_result["ate_m"],
        fps=method_result["fps"],
        time_ms=method_result["time_ms"],
        frames=method_result["frames"],
        note=method_result["note"],
        benchmark_score=0.0,
        readability_score=round(readability, 2),
        readability_note=readability_note,
        extensibility_score=round(extensibility, 2),
        extensibility_note=extensibility_note,
    )


def run_variant(
    manifest_stem: str,
    binary: Path,
    pcd_dir: Path,
    gt_csv: Path,
    methods: str,
    primary_method: str,
    output_dir: Path,
    variant: dict[str, Any],
    reuse_existing: bool,
) -> VariantResult:
    variant_dir = output_dir / "runs" / manifest_stem / variant["id"]
    variant_dir.mkdir(parents=True, exist_ok=True)
    summary_path = variant_dir / "summary.json"
    log_path = variant_dir / "run.log"
    if reuse_existing and summary_path.exists():
        return variant_result_from_summary(
            variant=variant,
            primary_method=primary_method,
            binary=binary,
            pcd_dir=pcd_dir,
            gt_csv=gt_csv,
            methods=methods,
            summary_path=summary_path,
            log_path=log_path,
        )
    command = build_command(binary, pcd_dir, gt_csv, methods, variant["args"], summary_path)
    completed = subprocess.run(
        command,
        cwd=REPO_ROOT,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    log_path.write_text(completed.stdout)
    return variant_result_from_summary(
        variant=variant,
        primary_method=primary_method,
        binary=binary,
        pcd_dir=pcd_dir,
        gt_csv=gt_csv,
        methods=methods,
        summary_path=summary_path,
        log_path=log_path,
    )


def compute_benchmark_scores(results: list[VariantResult]) -> None:
    valid = [
        result
        for result in results
        if result.status == "OK" and result.ate_m is not None and result.fps is not None
    ]
    if not valid:
        return
    best_ate = min(result.ate_m for result in valid if result.ate_m is not None)
    best_fps = max(result.fps for result in valid if result.fps is not None)
    for result in results:
        if result.status != "OK" or result.ate_m is None or result.fps is None:
            result.benchmark_score = 0.0
            continue
        ate_ratio = best_ate / max(result.ate_m, 1e-9)
        fps_ratio = result.fps / max(best_fps, 1e-9)
        result.benchmark_score = round(100.0 * (0.5 * ate_ratio + 0.5 * fps_ratio), 2)


def assign_decisions(results: list[VariantResult]) -> None:
    if not results:
        return
    ranked = sorted(results, key=lambda item: item.benchmark_score, reverse=True)
    best_score = ranked[0].benchmark_score
    for index, result in enumerate(ranked):
        if result.status != "OK":
            result.decision = "Rejected for this run"
            result.decision_reason = "The variant did not produce a valid benchmark result."
            continue
        if index == 0:
            result.decision = "Adopt as current default"
            result.decision_reason = (
                "Best combined benchmark score on the shared dataset and interface."
            )
            continue
        if best_score > 0 and result.benchmark_score >= best_score * 0.9:
            result.decision = "Keep as active challenger"
            result.decision_reason = (
                "Close enough to the current default to keep as a live alternative."
            )
        else:
            result.decision = "Keep as reference variant"
            result.decision_reason = (
                "Useful for comparison, but not strong enough to replace the current default."
            )


def variant_result_to_dict(result: VariantResult) -> dict[str, Any]:
    return {
        "id": result.id,
        "label": result.label,
        "design_style": result.design_style,
        "intent": result.intent,
        "args": result.args,
        "command": result.command,
        "summary_path": result.summary_path,
        "log_path": result.log_path,
        "status": result.status,
        "ate_m": result.ate_m,
        "fps": result.fps,
        "time_ms": result.time_ms,
        "frames": result.frames,
        "note": result.note,
        "benchmark_score": result.benchmark_score,
        "readability_score": result.readability_score,
        "readability_note": result.readability_note,
        "extensibility_score": result.extensibility_score,
        "extensibility_note": result.extensibility_note,
        "decision": result.decision,
        "decision_reason": result.decision_reason,
    }


def render_metric(value: float | None, digits: int = 3) -> str:
    if value is None or (isinstance(value, float) and not math.isfinite(value)):
        return "-"
    return f"{value:.{digits}f}"


def adopted_variant(problem_run: ProblemRun) -> VariantResult | None:
    for result in problem_run.results:
        if result.decision == "Adopt as current default":
            return result
    return None


def fastest_variant(problem_run: ProblemRun) -> VariantResult | None:
    valid = [result for result in problem_run.results if result.fps is not None]
    return max(valid, key=lambda item: item.fps or 0.0) if valid else None


def most_accurate_variant(problem_run: ProblemRun) -> VariantResult | None:
    valid = [result for result in problem_run.results if result.ate_m is not None]
    return min(valid, key=lambda item: item.ate_m or float("inf")) if valid else None


def render_experiments_md(
    problem_runs: list[ProblemRun], generated_at: str, index_relpath: str
) -> str:
    lines = [
        "# Experiment Results",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/run_experiment_matrix.py`. Source index: `{index_relpath}`._",
        "",
        "## Overview",
        "",
        "| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |",
        "|---------|--------|-----------------|--------------|----------|-----------|",
    ]
    for problem_run in problem_runs:
        default = adopted_variant(problem_run)
        accurate = most_accurate_variant(problem_run)
        fastest = fastest_variant(problem_run)
        lines.append(
            f"| {problem_run.title} | `{problem_run.problem_state}` | `{default.id if default else '-'}` | "
            f"{render_metric(accurate.ate_m if accurate else None)} | "
            f"{render_metric(fastest.fps if fastest else None, digits=1)} | "
            f"`{problem_run.aggregate_relpath}` |"
        )

    for problem_run in problem_runs:
        default = adopted_variant(problem_run)
        accurate = most_accurate_variant(problem_run)
        fastest = fastest_variant(problem_run)
        lines.extend(
            [
                "",
                f"## {problem_run.title}",
                "",
                f"- **Problem ID**: `{problem_run.problem_id}`",
                f"- **Question**: {problem_run.question}",
                f"- **Status**: `{problem_run.problem_state}`",
                f"- **Dataset PCD directory**: `{problem_run.dataset_pcd_dir}`",
                f"- **Reference CSV**: `{problem_run.dataset_gt_csv}`",
                f"- **Stable binary**: `{problem_run.binary_path}`",
                f"- **Shared method selector**: `{problem_run.method_selector}`",
                f"- **Shared metrics**: {', '.join(problem_run.same_metrics)}",
                f"- **Aggregate result**: `{problem_run.aggregate_relpath}`",
            ]
        )
        if problem_run.problem_state != "ready":
            lines.extend(
                [
                    f"- **Blocker**: {problem_run.blocker}",
                    f"- **Next step**: {problem_run.next_step}",
                    "",
                    "### Planned Variants",
                    "",
                    "| Variant | Style | Intent | Args |",
                    "|---------|-------|--------|------|",
                ]
            )
            for variant in problem_run.planned_variants:
                arg_text = " ".join(variant.get("args", [])) or "(default flags only)"
                lines.append(
                    f"| {variant['label']} | {variant['design_style']} | {variant['intent']} | `{arg_text}` |"
                )
            continue
        lines.extend(
            [
                "",
                "| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |",
                "|---------|-------|---------|-----|-----------|-------------|---------------|----------|",
            ]
        )
        for result in problem_run.results:
            lines.append(
                f"| {result.label} | {result.design_style} | {render_metric(result.ate_m)} | "
                f"{render_metric(result.fps, digits=1)} | {render_metric(result.benchmark_score, digits=1)} | "
                f"{render_metric(result.readability_score, digits=2)} | "
                f"{render_metric(result.extensibility_score, digits=2)} | {result.decision} |"
            )
        lines.extend(
            [
                "",
                "### Observations",
                "",
                f"1. `{default.id if default else '-'}` is the current default for this problem.",
                f"2. `{fastest.id if fastest else '-'}` is the fastest observed variant at {render_metric(fastest.fps if fastest else None, digits=1)} FPS.",
                f"3. `{accurate.id if accurate else '-'}` is the most accurate observed variant at {render_metric(accurate.ate_m if accurate else None)} m ATE.",
                "",
                "### Variant Notes",
                "",
            ]
        )
        for result in problem_run.results:
            arg_text = " ".join(result.args) if result.args else "(default flags only)"
            lines.extend(
                [
                    f"#### `{result.id}`",
                    "",
                    f"- Intent: {result.intent}",
                    f"- CLI args: `{arg_text}`",
                    f"- Command: `{result.command}`",
                    f"- Summary: `{result.summary_path}`",
                    f"- Log: `{result.log_path}`",
                    f"- Readability proxy: {result.readability_score:.2f} / 5.00. {result.readability_note}",
                    f"- Extensibility proxy: {result.extensibility_score:.2f} / 5.00. {result.extensibility_note}",
                    f"- Method note: {result.note or 'No extra method note.'}",
                    "",
                ]
            )
    return "\n".join(lines)


def render_decisions_md(
    problem_runs: list[ProblemRun], generated_at: str, index_relpath: str
) -> str:
    lines = [
        "# Decisions",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/run_experiment_matrix.py`. Source index: `{index_relpath}`._",
        "",
        "## Rules",
        "",
        "1. Keep at least three concrete variants alive for each active problem.",
        "2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.",
        "3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.",
        "4. Do not delete weaker variants unless they stop being informative.",
    ]
    for problem_run in problem_runs:
        if problem_run.problem_state != "ready":
            lines.extend(
                [
                    "",
                    f"## {problem_run.title}",
                    "",
                    "- Current default: `-`.",
                    f"- Status: `{problem_run.problem_state}`.",
                    f"- Blocker: {problem_run.blocker}",
                    f"- Next step: {problem_run.next_step}",
                    f"- Aggregate result: `{problem_run.aggregate_relpath}`",
                ]
            )
            continue
        adopted = adopted_variant(problem_run)
        challengers = [
            result for result in problem_run.results if result.decision == "Keep as active challenger"
        ]
        references = [
            result for result in problem_run.results if result.decision == "Keep as reference variant"
        ]
        lines.extend(
            [
                "",
                f"## {problem_run.title}",
                "",
                f"- Current default: `{adopted.id if adopted else '-'}`.",
            ]
        )
        if challengers:
            lines.append(
                "- Active challengers: "
                + ", ".join(f"`{result.id}`" for result in challengers)
                + "."
            )
        if references:
            lines.append(
                "- Reference variants: "
                + ", ".join(f"`{result.id}`" for result in references)
                + "."
            )
        lines.extend(
            [
                f"- Aggregate result: `{problem_run.aggregate_relpath}`",
                "",
                "| Variant | Decision | Why |",
                "|---------|----------|-----|",
            ]
        )
        for result in problem_run.results:
            lines.append(f"| {result.id} | {result.decision} | {result.decision_reason} |")
    return "\n".join(lines)


def render_interfaces_md(
    problem_runs: list[ProblemRun], generated_at: str, index_relpath: str
) -> str:
    binaries = sorted({problem_run.binary_path for problem_run in problem_runs})
    selectors = sorted({problem_run.method_selector for problem_run in problem_runs})
    lines = [
        "# Minimal Interfaces",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/run_experiment_matrix.py`. Source index: `{index_relpath}`._",
        "",
        "## Stable Core",
        "",
        "### CLI",
        "",
        "`build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] --methods <selector> --summary-json <path> [variant flags...]`",
        "",
        f"Current active binaries: {', '.join(f'`{item}`' for item in binaries)}",
        "",
        "### Summary JSON Contract",
        "",
        "| Field | Type | Meaning |",
        "|-------|------|---------|",
        "| `pcd_dir` | string | Input PCD sequence directory. |",
        "| `gt_csv` | string | Reference trajectory CSV matched to the sequence. |",
        "| `num_frames` | integer | Number of frames evaluated. |",
        "| `trajectory_length_m` | number | Total GT path length in meters. |",
        "| `timestamp_source` | string | Timestamp source used by the evaluator. |",
        "| `methods[]` | array | Per-method results emitted by the benchmark. |",
        "| `methods[].name` | string | Human-readable method name. |",
        "| `methods[].status` | string | `OK` or `SKIPPED`. |",
        "| `methods[].ate_m` | number or null | Absolute trajectory error in meters. |",
        "| `methods[].frames` | integer | Number of poses evaluated for the method. |",
        "| `methods[].time_ms` | number or null | End-to-end runtime in milliseconds. |",
        "| `methods[].fps` | number or null | Effective frames per second. |",
        "| `methods[].note` | string | Free-form method note or skip reason. |",
        "",
        "## Experimental Layer",
        "",
        "### Manifest Contract",
        "",
        "Every active search problem lives in `experiments/*.json` and must define:",
        "",
        "| Field | Type | Meaning |",
        "|-------|------|---------|",
        "| `problem.id` | string | Stable identifier for the search problem. |",
        "| `problem.state` | string | `ready` or `blocked`. Missing means `ready`. |",
        "| `problem.blocker` | string | Why the problem cannot be benchmarked yet. Optional for blocked problems. |",
        "| `problem.next_step` | string | The next concrete step to unblock the problem. Optional for blocked problems. |",
        "| `problem.dataset` | object | Shared dataset paths for every variant. |",
        "| `stable_interface.binary` | string | Stable benchmark entrypoint. |",
        "| `stable_interface.methods` | string | Shared method selector for comparability. |",
        "| `stable_interface.primary_method` | string | Result key to extract from summary JSON. |",
        "| `variants[]` | array | Concrete variants to compare, keep, or discard. |",
        "| `variants[].args` | array | Extra CLI flags layered on the stable core. |",
        "",
        f"Current active selectors: {', '.join(f'`{item}`' for item in selectors)}",
        "",
        "### Runner Contract",
        "",
        "`python3 evaluation/scripts/run_experiment_matrix.py [--manifest <path>]... [--reuse-existing]`",
        "",
        "If no manifest is specified, the runner executes every `experiments/*_matrix.json` file.",
        "",
        "The runner is responsible for:",
        "",
        "- running every variant against the same dataset and method selector",
        "- saving per-run summary JSON and logs under `experiments/results/runs/`",
        "- regenerating `docs/experiments.md`, `docs/decisions.md`, and `docs/interfaces.md`",
        "- writing per-problem aggregate JSON files plus `experiments/results/index.json`",
        "- optionally reusing existing per-variant summaries to avoid rerunning expensive variants",
        "",
        "## Stability Boundary",
        "",
        "- Stable core: `build/evaluation/pcd_dogfooding` plus the `--summary-json` result contract.",
        "- Experimental surface: manifests, run logs, aggregate results, and generated decision docs.",
        "- Promotion rule: a new default must emerge from shared data and shared metrics, not from a separate code path.",
        "",
        "## Active Problems",
        "",
        "| Problem | Status | Manifest | Selector | Current Default | Aggregate |",
        "|---------|--------|----------|----------|-----------------|-----------|",
    ]
    for problem_run in problem_runs:
        default = adopted_variant(problem_run)
        lines.append(
            f"| {problem_run.title} | `{problem_run.problem_state}` | `{problem_run.manifest_path}` | `{problem_run.method_selector}` | "
            f"`{default.id if default else '-'}` | `{problem_run.aggregate_relpath}` |"
        )
    return "\n".join(lines)


def run_problem(
    manifest_path: Path,
    args: argparse.Namespace,
    output_dir: Path,
    generated_at: str,
) -> ProblemRun:
    manifest = load_manifest(manifest_path)
    problem_cfg = manifest["problem"]
    problem_state = str(problem_cfg.get("state", "ready"))
    stable_interface = manifest["stable_interface"]
    binary = resolve_path(args.binary or stable_interface["binary"])
    if binary is None or not binary.exists():
        raise FileNotFoundError(f"Benchmark binary not found: {binary}")

    dataset_cfg = problem_cfg["dataset"]
    dataset_pcd_raw = args.pcd_dir or dataset_cfg.get("pcd_dir", "")
    dataset_gt_raw = args.gt_csv or dataset_cfg.get("gt_csv", "")
    pcd_dir = resolve_path(dataset_pcd_raw)
    gt_csv = resolve_path(dataset_gt_raw)

    manifest_stem = manifest_path.stem
    blocker = str(problem_cfg.get("blocker", ""))
    next_step = str(problem_cfg.get("next_step", ""))
    if problem_state != "ready":
        aggregate_path = output_dir / f"{manifest_stem}.json"
        aggregate = {
            "generated_at": generated_at,
            "manifest_path": relpath(manifest_path),
            "problem": manifest["problem"],
            "stable_interface": manifest["stable_interface"],
            "dataset": {
                "pcd_dir": str(dataset_pcd_raw),
                "gt_csv": str(dataset_gt_raw),
            },
            "status": problem_state,
            "blocker": blocker,
            "next_step": next_step,
            "planned_variants": manifest["variants"],
            "variants": [],
        }
        aggregate_path.write_text(json.dumps(aggregate, indent=2) + "\n")
        return ProblemRun(
            manifest_path=relpath(manifest_path),
            manifest_stem=manifest_stem,
            problem_id=problem_cfg["id"],
            title=problem_cfg["title"],
            question=problem_cfg["question"],
            problem_state=problem_state,
            blocker=blocker,
            next_step=next_step,
            dataset_pcd_dir=str(dataset_pcd_raw),
            dataset_gt_csv=str(dataset_gt_raw),
            binary_path=relpath(binary),
            method_selector=stable_interface["methods"],
            primary_method=stable_interface["primary_method"],
            same_metrics=list(problem_cfg["same_metrics"]),
            aggregate_relpath=relpath(aggregate_path),
            results=[],
            planned_variants=list(manifest["variants"]),
        )

    if pcd_dir is None or not pcd_dir.exists():
        raise FileNotFoundError(f"PCD directory not found: {pcd_dir}")
    if gt_csv is None or not gt_csv.exists():
        raise FileNotFoundError(f"Reference CSV not found: {gt_csv}")

    results: list[VariantResult] = []
    for variant in manifest["variants"]:
        print(f"[run] {manifest_stem}:{variant['id']}")
        results.append(
            run_variant(
                manifest_stem=manifest_stem,
                binary=binary,
                pcd_dir=pcd_dir,
                gt_csv=gt_csv,
                methods=stable_interface["methods"],
                primary_method=stable_interface["primary_method"],
                output_dir=output_dir,
                variant=variant,
                reuse_existing=args.reuse_existing,
            )
        )

    compute_benchmark_scores(results)
    assign_decisions(results)

    aggregate_path = output_dir / f"{manifest_stem}.json"
    aggregate = {
        "generated_at": generated_at,
        "manifest_path": relpath(manifest_path),
        "problem": manifest["problem"],
        "stable_interface": manifest["stable_interface"],
        "dataset": {
            "pcd_dir": relpath(pcd_dir),
            "gt_csv": relpath(gt_csv),
        },
        "status": problem_state,
        "variants": [variant_result_to_dict(result) for result in results],
    }
    aggregate_path.write_text(json.dumps(aggregate, indent=2) + "\n")

    return ProblemRun(
        manifest_path=relpath(manifest_path),
        manifest_stem=manifest_stem,
        problem_id=problem_cfg["id"],
        title=problem_cfg["title"],
        question=problem_cfg["question"],
        problem_state=problem_state,
        blocker=blocker,
        next_step=next_step,
        dataset_pcd_dir=relpath(pcd_dir),
        dataset_gt_csv=relpath(gt_csv),
        binary_path=relpath(binary),
        method_selector=stable_interface["methods"],
        primary_method=stable_interface["primary_method"],
        same_metrics=list(problem_cfg["same_metrics"]),
        aggregate_relpath=relpath(aggregate_path),
        results=results,
        planned_variants=list(manifest["variants"]),
    )


def main() -> None:
    args = parse_args()
    manifest_paths = discover_manifest_paths(args.manifest)
    output_dir = resolve_path(args.output_dir)
    docs_dir = resolve_path(args.docs_dir)
    assert output_dir is not None
    assert docs_dir is not None
    output_dir.mkdir(parents=True, exist_ok=True)
    docs_dir.mkdir(parents=True, exist_ok=True)

    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    problem_runs = [
        run_problem(
            manifest_path=manifest_path,
            args=args,
            output_dir=output_dir,
            generated_at=generated_at,
        )
        for manifest_path in manifest_paths
    ]

    index_path = output_dir / "index.json"
    index_payload = {
        "generated_at": generated_at,
        "problems": [
            {
                "problem_id": problem_run.problem_id,
                "title": problem_run.title,
                "status": problem_run.problem_state,
                "blocker": problem_run.blocker,
                "manifest_path": problem_run.manifest_path,
                "aggregate_path": problem_run.aggregate_relpath,
                "current_default": adopted_variant(problem_run).id
                if adopted_variant(problem_run)
                else None,
            }
            for problem_run in problem_runs
        ],
    }
    index_path.write_text(json.dumps(index_payload, indent=2) + "\n")
    index_relpath = relpath(index_path)

    (docs_dir / "experiments.md").write_text(
        render_experiments_md(problem_runs, generated_at, index_relpath) + "\n"
    )
    (docs_dir / "decisions.md").write_text(
        render_decisions_md(problem_runs, generated_at, index_relpath) + "\n"
    )
    (docs_dir / "interfaces.md").write_text(
        render_interfaces_md(problem_runs, generated_at, index_relpath) + "\n"
    )

    print(f"[done] wrote {index_relpath}")
    for problem_run in problem_runs:
        print(f"[done] wrote {problem_run.aggregate_relpath}")
    print(f"[done] wrote {relpath(docs_dir / 'experiments.md')}")
    print(f"[done] wrote {relpath(docs_dir / 'decisions.md')}")
    print(f"[done] wrote {relpath(docs_dir / 'interfaces.md')}")


if __name__ == "__main__":
    main()
