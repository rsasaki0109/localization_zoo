#!/usr/bin/env python3
"""Fail when a fresh public IMU benchmark run drifts from the committed result.

Checks two things against files in the repository:
  1. `tune` output (profile YAML values and policy JSON) equals the committed
     wearable-public-v1 configuration, so the frozen thresholds are still what
     the CGU-BES training subjects select.
  2. `benchmark` group metrics equal public_benchmark_expected.json within its
     tolerance (counts and CLI failures must match exactly).
"""

from __future__ import annotations

import argparse
import json
import math
import pathlib
import sys

HERE = pathlib.Path(__file__).resolve().parent
CONFIG_DIR = HERE.parent / "config"
EXPECTED = HERE / "public_benchmark_expected.json"
EXACT_KEYS = ("recordings", "cli_failures")


def parse_yaml_scalars(text: str) -> dict[str, str]:
    """Read the flat `key: value` lines of a profile YAML, ignoring comments."""
    values = {}
    for line in text.splitlines():
        line = line.split("#", 1)[0].strip()
        if ":" in line:
            key, value = (part.strip() for part in line.split(":", 1))
            if value:
                values[key] = value
    return values


def compare_tuning(tuned_dir: pathlib.Path, config_dir: pathlib.Path) -> list[str]:
    errors = []
    tuned_profile = parse_yaml_scalars((tuned_dir / "wearable-public-v1.yaml").read_text())
    frozen_profile = parse_yaml_scalars((config_dir / "wearable-public-v1.yaml").read_text())
    if tuned_profile != frozen_profile:
        errors.append(f"tuned profile {tuned_profile} != committed {frozen_profile}")
    tuned_policy = json.loads((tuned_dir / "wearable-public-v1-policy.json").read_text())
    frozen_policy = json.loads((config_dir / "wearable-public-v1-policy.json").read_text())
    for policy in (tuned_policy, frozen_policy):
        policy.pop("description", None)
    if tuned_policy != frozen_policy:
        errors.append(f"tuned policy {tuned_policy} != committed {frozen_policy}")
    return errors


def compare_groups(actual: dict, expected: dict) -> list[str]:
    errors = []
    tolerance = float(expected["tolerance"])
    if actual.get("passed") != expected["passed"]:
        errors.append(f"acceptance passed={actual.get('passed')} expected {expected['passed']}")
    actual_groups = actual.get("groups", {})
    for group, metrics in expected["groups"].items():
        if group not in actual_groups:
            errors.append(f"{group}: missing from benchmark output")
            continue
        for key, want in metrics.items():
            got = actual_groups[group].get(key)
            if want is None or got is None or key in EXACT_KEYS:
                ok = got == want
            else:
                ok = math.isclose(float(got), float(want), rel_tol=0.0, abs_tol=tolerance)
            if not ok:
                errors.append(f"{group}.{key}: got {got}, expected {want}")
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", type=pathlib.Path, required=True,
                        help="Directory holding tune output and benchmark.json")
    parser.add_argument("--expected", type=pathlib.Path, default=EXPECTED)
    parser.add_argument("--config-dir", type=pathlib.Path, default=CONFIG_DIR)
    args = parser.parse_args(argv)

    errors = compare_tuning(args.results, args.config_dir)
    errors += compare_groups(
        json.loads((args.results / "benchmark.json").read_text()),
        json.loads(args.expected.read_text()),
    )
    for error in errors:
        print(f"regression: {error}", file=sys.stderr)
    if errors:
        return 1
    print("public IMU benchmark matches committed configuration and expected metrics")
    return 0


if __name__ == "__main__":
    sys.exit(main())
