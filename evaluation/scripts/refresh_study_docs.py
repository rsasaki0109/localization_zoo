#!/usr/bin/env python3
"""Refresh experiment-facing and publication-facing docs."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Refresh experiment and publication docs from the current repository state."
    )
    parser.add_argument(
        "--rerun",
        action="store_true",
        help="Rerun experiment manifests instead of reusing existing summaries.",
    )
    parser.add_argument(
        "--manifest",
        action="append",
        default=[],
        help="Optional manifest path passed through to run_experiment_matrix.py. Can be repeated.",
    )
    return parser.parse_args()


def run(cmd: list[str]) -> None:
    print("[run]", " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def main() -> None:
    args = parse_args()

    runner_cmd = ["python3", "evaluation/scripts/run_experiment_matrix.py"]
    if not args.rerun:
        runner_cmd.append("--reuse-existing")
    for manifest in args.manifest:
        runner_cmd.extend(["--manifest", manifest])

    run(runner_cmd)
    run(["python3", "evaluation/scripts/generate_publication_docs.py"])
    run(["python3", "evaluation/scripts/export_paper_assets.py"])
    run(["python3", "evaluation/scripts/generate_paper_comparison.py"])
    print("[done] refreshed experiment, publication, and comparison docs")


if __name__ == "__main__":
    main()
