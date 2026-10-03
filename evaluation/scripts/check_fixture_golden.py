#!/usr/bin/env python3
"""Fail CI when a method's result on the committed smoke fixture changes.

smoke_ci_fixture.sh only proves that every method still runs. This compares
each method's ATE / RPE / frame count / status on the 3-frame MCD fixture
against a committed golden file, so a code change that silently moves results
(as the 2026-08-02 KISS-ICP correspondence-search change did) fails the PR
that introduces it instead of surfacing months later.

The golden file must be produced in the CI environment, because compiler and
library versions shift the last digits. Every run writes the actual values to
--actual-out; when a change is intentional, copy that file over the golden
file (the CI artifact `fixture-golden-actual`) and explain the change in the PR.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path
from typing import Any

METRICS = ("ate_m", "rpe_trans_pct")
EXACT = ("frames", "status")


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--summaries", type=Path, required=True, help="Directory of <method>.json summaries")
    parser.add_argument("--golden", type=Path, required=True)
    parser.add_argument("--actual-out", type=Path, required=True)
    parser.add_argument("--rel-tol", type=float, default=1e-3,
                        help="Relative tolerance for metrics (default 0.1 %%)")
    parser.add_argument("--abs-tol", type=float, default=1e-6)
    return parser.parse_args(argv)


def collect(summaries: Path) -> dict[str, dict[str, Any]]:
    actual: dict[str, dict[str, Any]] = {}
    for path in sorted(summaries.glob("*.json")):
        try:
            methods = json.loads(path.read_text())["methods"]
        except (json.JSONDecodeError, KeyError):
            continue
        for item in methods:
            actual[f"{path.stem}/{item['name']}"] = {key: item.get(key) for key in (*METRICS, *EXACT)}
    return actual


def same_metric(got: Any, want: Any, rel_tol: float, abs_tol: float) -> bool:
    if got is None or want is None:
        return got is want
    got, want = float(got), float(want)
    if math.isnan(got) or math.isnan(want):
        return math.isnan(got) and math.isnan(want)
    return math.isclose(got, want, rel_tol=rel_tol, abs_tol=abs_tol)


def compare(actual: dict, golden: dict, rel_tol: float, abs_tol: float) -> list[str]:
    errors = []
    for key in sorted(set(golden) - set(actual)):
        errors.append(f"{key}: missing from this run")
    for key in sorted(set(actual) - set(golden)):
        errors.append(f"{key}: not in golden file (new method? update the golden file)")
    for key in sorted(set(actual) & set(golden)):
        got, want = actual[key], golden[key]
        for field in EXACT:
            if got.get(field) != want.get(field):
                errors.append(f"{key}.{field}: {got.get(field)!r} != golden {want.get(field)!r}")
        for field in METRICS:
            if not same_metric(got.get(field), want.get(field), rel_tol, abs_tol):
                errors.append(f"{key}.{field}: {got.get(field)} != golden {want.get(field)}")
    return errors


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    actual = collect(args.summaries)
    args.actual_out.parent.mkdir(parents=True, exist_ok=True)
    args.actual_out.write_text(json.dumps(actual, indent=2, sort_keys=True) + "\n")
    if not actual:
        print("fixture golden: no summaries found", file=sys.stderr)
        return 1
    if not args.golden.is_file():
        print(f"fixture golden: {args.golden} does not exist; actual values written to {args.actual_out}",
              file=sys.stderr)
        return 1
    errors = compare(actual, json.loads(args.golden.read_text()), args.rel_tol, args.abs_tol)
    for error in errors:
        print(f"fixture drift: {error}", file=sys.stderr)
    if errors:
        print(f"fixture golden: {len(errors)} difference(s). If intentional, replace {args.golden} with "
              f"{args.actual_out} (CI artifact 'fixture-golden-actual') and explain the change in the PR.",
              file=sys.stderr)
        return 1
    print(f"fixture golden: {len(actual)} method results match {args.golden}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
