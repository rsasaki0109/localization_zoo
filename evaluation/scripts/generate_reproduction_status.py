#!/usr/bin/env python3

"""Generate docs/reproduction_status.md from tracked paper metadata."""

from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
PAPER_DATA = REPO_ROOT / "evaluation" / "data" / "paper_reported_numbers.json"
DOCS_DIR = REPO_ROOT / "docs"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def display_name(selector: str) -> str:
    if selector == "kiss_icp":
        return "KISS-ICP"
    if selector == "ct_icp":
        return "CT-ICP"
    if selector == "ct_lio":
        return "CT-LIO"
    if selector == "litamin2":
        return "LiTAMIN2"
    return selector.upper().replace("_", "-")


def render_markdown(paper: dict[str, Any], generated_at: str) -> str:
    methods = paper.get("methods", {})
    lines = [
        "# Reproduction Status",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/generate_reproduction_status.py`._",
        "",
        "This page records what the repository can currently claim about reproducing original-paper results.",
        "The tracked subset below is intentionally conservative: if the implementation, metric, dataset, or protocol diverges, the repo should say so explicitly.",
        "",
        "## Tracked Families",
        "",
        "| Method | Repo Scope | Current Claim | Numeric Comparison | Main Blocker | Next Step |",
        "|--------|------------|---------------|--------------------|--------------|-----------|",
    ]

    for selector, info in methods.items():
        lines.append(
            f"| {display_name(selector)} | {info.get('repo_scope_label', '-')} | "
            f"{info.get('reproduction_status_label', '-')} | "
            f"{info.get('numeric_comparison_label', '-')} | "
            f"{info.get('main_blocker', '-')} | "
            f"{info.get('next_step', '-')} |"
        )

    for selector, info in methods.items():
        lines.extend(
            [
                "",
                f"## {display_name(selector)}",
                "",
                f"- **Paper**: {info.get('paper', 'N/A')}",
                f"- **Method README**: `{info.get('method_readme', '')}`",
                f"- **Reported dataset**: {info.get('reported_dataset', 'N/A')}",
                f"- **Reported metric**: {info.get('reported_metric', 'N/A')}",
                f"- **Repo scope**: {info.get('repo_scope_label', 'N/A')}. {info.get('repo_scope_summary', '')}",
                f"- **Current claim**: {info.get('reproduction_status_label', 'N/A')}. {info.get('reproduction_status_summary', '')}",
                f"- **Numeric comparison**: {info.get('numeric_comparison_label', 'N/A')}. {info.get('numeric_comparison_summary', '')}",
                f"- **Main blocker**: {info.get('main_blocker', 'N/A')}",
                f"- **Next step**: {info.get('next_step', 'N/A')}",
            ]
        )
        notes = info.get("notes", "")
        if notes:
            lines.append(f"- **Notes**: {notes}")

    return "\n".join(lines)


def main() -> int:
    if not PAPER_DATA.exists():
        print(f"[skip] {PAPER_DATA} not found")
        return 1

    paper = load_json(PAPER_DATA)
    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    output_path = DOCS_DIR / "reproduction_status.md"
    output_path.write_text(render_markdown(paper, generated_at) + "\n")
    print(f"[done] wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
