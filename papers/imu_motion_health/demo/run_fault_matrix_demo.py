#!/usr/bin/env python3
"""One-command deterministic fault-matrix replay and HTML dashboard demo.

This is a thin orchestration layer around the standard-library tools in
``evaluation``.  It is useful for a first run after building the CLI::

    python papers/imu_motion_health/demo/run_fault_matrix_demo.py \
      --output-dir build/imu_demo \
      --cli build/imu_motion_health/imu_motion_health_cli.exe

The command always generates CSV/truth/manifest files.  With ``--cli`` it
also writes JSONL/summary/report and a self-contained HTML replay dashboard.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import sys
from typing import Sequence

try:
    from ..evaluation import (evaluate_fault_matrix, generate_fault_matrix,
                              render_dashboard)
except ImportError:  # direct execution from the repository checkout
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[3]))
    from papers.imu_motion_health.evaluation import (
        evaluate_fault_matrix, generate_fault_matrix, render_dashboard)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", required=True, type=pathlib.Path)
    parser.add_argument("--cli", type=pathlib.Path,
                        help="optional imu_motion_health_cli executable")
    parser.add_argument("--scenario", action="append", default=[],
                        help="scenario(s) to generate; default all")
    parser.add_argument("--seed", type=int, default=0)
    parser.add_argument("--dashboard-scenario",
                        help="scenario for HTML (default impact)")
    args = parser.parse_args(argv)

    try:
        names = generate_fault_matrix._parse_names(args.scenario or ["all"])
        manifest = generate_fault_matrix.generate(args.output_dir, names, args.seed)
        print(f"generated {len(manifest['scenarios'])} scenario(s) in {args.output_dir}")
        if args.cli is None:
            print("CLI not supplied; CSV/truth/manifest generation complete")
            return 0
        if not args.cli.is_file():
            print(f"run_fault_matrix_demo: CLI not found: {args.cli}", file=sys.stderr)
            return 2
        report = evaluate_fault_matrix.evaluate_manifest(
            args.output_dir / "manifest.json", cli=args.cli,
            output_dir=args.output_dir / "runs")
        report_path = args.output_dir / "report.json"
        report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n",
                               encoding="utf-8", newline="\n")
        selected = args.dashboard_scenario or (
            "impact" if "impact" in names else names[0]
        )
        if selected not in names:
            raise ValueError(f"dashboard scenario '{selected}' was not generated")
        render_dashboard.render(
            args.output_dir / "runs" / f"{selected}.jsonl",
            args.output_dir / f"{selected}.html",
            summary_path=args.output_dir / "runs" / f"{selected}.summary.json",
            truth_path=args.output_dir / f"{selected}.truth.json")
        print(f"report: {report_path}")
        print(f"dashboard: {args.output_dir / (selected + '.html')}")
        return 0 if report["aggregate"]["pass"] else 1
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"run_fault_matrix_demo: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
