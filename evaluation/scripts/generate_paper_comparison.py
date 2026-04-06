#!/usr/bin/env python3

"""Generate docs/paper_comparison.md from paper-reported numbers and repo experiment results."""

from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
PAPER_DATA = REPO_ROOT / "evaluation" / "data" / "paper_reported_numbers.json"
DOCS_DIR = REPO_ROOT / "docs"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def gather_repo_defaults(index: dict[str, Any]) -> dict[str, list[dict[str, Any]]]:
    """Group default variants by method selector from index.json."""
    by_selector: dict[str, list[dict[str, Any]]] = {}
    for entry in index.get("problems", []):
        if entry.get("status") != "ready":
            continue
        agg_path = REPO_ROOT / entry["aggregate_path"]
        if not agg_path.exists():
            continue
        agg = load_json(agg_path)
        selector = agg.get("stable_interface", {}).get("methods", "unknown")
        dataset_pcd = agg.get("dataset", {}).get("pcd_dir", "")
        for v in agg.get("variants", []):
            if v.get("decision", "").startswith("Adopt"):
                by_selector.setdefault(selector, []).append({
                    "variant_id": v.get("id", ""),
                    "variant_label": v.get("label", ""),
                    "ate_m": v.get("ate_m"),
                    "fps": v.get("fps"),
                    "dataset": dataset_pcd,
                })
    return by_selector


def fmt(val: float | None, digits: int = 3) -> str:
    if val is None:
        return "-"
    return f"{val:.{digits}f}"


def classify_dataset(pcd_dir: str) -> str:
    if "istanbul" in pcd_dir:
        return "Istanbul"
    if "hdl_400" in pcd_dir:
        return "HDL-400"
    if "kitti" in pcd_dir:
        return "KITTI"
    return pcd_dir


def main() -> int:
    if not PAPER_DATA.exists():
        print(f"[skip] {PAPER_DATA} not found")
        return 1

    paper = load_json(PAPER_DATA)
    methods = paper.get("methods", {})

    index_path = RESULTS_DIR / "index.json"
    if index_path.exists():
        index = load_json(index_path)
        repo_defaults = gather_repo_defaults(index)
    else:
        repo_defaults = {}

    lines: list[str] = []
    ts = datetime.now(timezone.utc).isoformat(timespec="seconds")
    lines.append(f"# Original-Paper Comparison\n")
    lines.append(f"> Generated: {ts}\n")
    lines.append(
        "This document compares paper-reported metrics with the current repository defaults "
        "across each method family. Direct comparison is limited by differences in dataset windows, "
        "hardware, and metric definitions (RPE vs ATE).\n"
    )

    for selector, info in methods.items():
        primary = selector.upper().replace("_", "-")
        if selector == "kiss_icp":
            primary = "KISS-ICP"
        elif selector == "ct_icp":
            primary = "CT-ICP"
        elif selector == "ct_lio":
            primary = "CT-LIO"
        elif selector == "litamin2":
            primary = "LiTAMIN2"

        lines.append(f"## {primary}\n")
        lines.append(f"**Paper**: {info.get('paper', 'N/A')}\n")
        lines.append(f"**Reported dataset**: {info.get('reported_dataset', 'N/A')}")
        lines.append(f"**Reported metric**: {info.get('reported_metric', 'N/A')}")
        if info.get("reported_fps") is not None:
            lines.append(f"**Reported FPS**: ~{info['reported_fps']}")
        lines.append(f"**Hardware**: {info.get('reported_hardware', 'N/A')}")
        lines.append("")

        reported = info.get("reported_values", {})
        if reported:
            lines.append("### Paper-Reported Values\n")
            lines.append("| Sequence | Value |")
            lines.append("|---|---:|")
            for seq, val in sorted(reported.items()):
                lines.append(f"| {seq} | {fmt(val, 2)} |")
            lines.append("")

        defaults = repo_defaults.get(selector, [])
        if defaults:
            lines.append("### Repository Defaults\n")
            lines.append("| Dataset | Variant | ATE [m] | FPS |")
            lines.append("|---|---|---:|---:|")
            for d in defaults:
                ds = classify_dataset(d["dataset"])
                lines.append(
                    f"| {ds} | {d['variant_label']} | {fmt(d['ate_m'], 3)} | {fmt(d['fps'], 2)} |"
                )
            lines.append("")

        notes = info.get("notes", "")
        if notes:
            lines.append(f"**Notes**: {notes}\n")

        lines.append("---\n")

    output_path = DOCS_DIR / "paper_comparison.md"
    output_path.write_text("\n".join(lines))
    print(f"[done] wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
