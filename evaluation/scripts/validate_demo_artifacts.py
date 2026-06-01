#!/usr/bin/env python3
"""Validate artifacts produced by demo_localization_zoo.sh."""

from __future__ import annotations

import argparse
import json
from html.parser import HTMLParser
from pathlib import Path
import re


REQUIRED_REPORT_SNIPPETS = [
    "Demo Report",
    "Run This Locally",
    "Synthetic trajectory overlay",
    "Committed MCD LiDAR Smoke",
    "Camera-Aware Fixture Smoke",
]


def load_json(path: Path) -> dict:
    if not path.exists():
        raise SystemExit(f"missing required JSON: {path}")
    with path.open() as handle:
        return json.load(handle)


def require_file(path: Path) -> None:
    if not path.exists():
        raise SystemExit(f"missing required artifact: {path}")
    if path.is_file() and path.stat().st_size == 0:
        raise SystemExit(f"empty required artifact: {path}")


def require_ok_methods(label: str, methods: list[dict]) -> None:
    if not methods:
        raise SystemExit(f"{label} has no methods")
    bad = [
        f"{method.get('name', '-')}: {method.get('status', '-')}"
        for method in methods
        if str(method.get("status", "")).lower() != "ok"
    ]
    if bad:
        raise SystemExit(f"{label} has non-OK methods: {', '.join(bad)}")


def normalize_name(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", value.lower())


def split_csv(value: str) -> list[str]:
    return [item.strip() for item in value.split(",") if item.strip()]


def require_expected_methods(label: str, expected: list[str], methods: list[dict]) -> None:
    if not expected:
        return
    expected_names = {normalize_name(name) for name in expected}
    actual_names = {normalize_name(str(method.get("name", ""))) for method in methods}
    missing = sorted(expected_names - actual_names)
    extra = sorted(actual_names - expected_names)
    if missing or extra:
        details = []
        if missing:
            details.append(f"missing={','.join(missing)}")
        if extra:
            details.append(f"extra={','.join(extra)}")
        raise SystemExit(f"{label} method set mismatch: {'; '.join(details)}")


def parse_report(path: Path) -> str:
    require_file(path)
    text = path.read_text()
    HTMLParser().feed(text)
    missing = [snippet for snippet in REQUIRED_REPORT_SNIPPETS if snippet not in text]
    if missing:
        raise SystemExit(f"report missing snippets: {', '.join(missing)}")
    return text


def validate(
    demo_dir: Path,
    *,
    expected_lidar_methods: list[str] | None = None,
    expected_multimodal_methods: list[str] | None = None,
    skip_multimodal: bool = False,
) -> None:
    expected_lidar_methods = expected_lidar_methods or []
    expected_multimodal_methods = expected_multimodal_methods or []
    manifest = load_json(demo_dir / "manifest.json")
    if manifest.get("schema_version") != 1:
        raise SystemExit(f"unsupported manifest schema: {manifest.get('schema_version')}")
    if manifest.get("status") != "ok":
        raise SystemExit(f"demo manifest status is not ok: {manifest.get('status')}")
    run = manifest.get("run", {})
    if not expected_lidar_methods:
        expected_lidar_methods = [str(value) for value in run.get("lidar_methods", [])]
    if not expected_multimodal_methods:
        expected_multimodal_methods = [
            str(value) for value in run.get("multimodal_methods", [])
        ]
    skip_multimodal = skip_multimodal or run.get("multimodal_enabled") is False

    artifacts = manifest.get("artifacts", {})
    report_path = demo_dir / artifacts.get("report_html", "report.html")
    synthetic_log = demo_dir / artifacts.get("synthetic_log", "synthetic_benchmark.log")
    lidar_summary_path = demo_dir / artifacts.get("lidar_summary", "lidar_fixture_summary.json")
    multimodal_summary_path = demo_dir / artifacts.get(
        "multimodal_summary", "multimodal_fixture_summary.json"
    )
    synthetic_trajectories = demo_dir / artifacts.get(
        "synthetic_trajectories", "benchmark_results"
    )
    lidar_trajectories = demo_dir / artifacts.get("lidar_trajectories", "dogfooding_results")

    report = parse_report(report_path)
    require_file(synthetic_log)
    require_file(lidar_summary_path)
    require_file(synthetic_trajectories / "gt.txt")
    require_file(lidar_trajectories / "gt.txt")

    lidar = load_json(lidar_summary_path)
    require_ok_methods("LiDAR fixture", lidar.get("methods", []))
    require_expected_methods("LiDAR fixture", expected_lidar_methods, lidar.get("methods", []))

    manifest_lidar_names = {
        method.get("name") for method in manifest.get("lidar_fixture", {}).get("methods", [])
    }
    summary_lidar_names = {method.get("name") for method in lidar.get("methods", [])}
    if manifest_lidar_names != summary_lidar_names:
        raise SystemExit("manifest LiDAR methods do not match LiDAR summary")

    summary_multimodal_names: set[str | None] = set()
    if not skip_multimodal:
        require_file(multimodal_summary_path)
        multimodal = load_json(multimodal_summary_path)
        require_ok_methods("multimodal fixture", multimodal.get("methods", []))
        require_expected_methods(
            "multimodal fixture",
            expected_multimodal_methods,
            multimodal.get("methods", []),
        )

        manifest_multimodal_names = {
            method.get("name")
            for method in manifest.get("multimodal_fixture", {}).get("methods", [])
        }
        summary_multimodal_names = {
            method.get("name") for method in multimodal.get("methods", [])
        }
        if manifest_multimodal_names != summary_multimodal_names:
            raise SystemExit("manifest multimodal methods do not match multimodal summary")

    for method_name in summary_lidar_names | summary_multimodal_names:
        if method_name and method_name not in report:
            raise SystemExit(f"report does not mention method: {method_name}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--demo-dir",
        default="experiments/results/runs/demo_localization_zoo",
        help="Directory produced by demo_localization_zoo.sh.",
    )
    parser.add_argument(
        "--expected-lidar-methods",
        default="",
        help="Comma-separated LiDAR method list that must exactly match the summary.",
    )
    parser.add_argument(
        "--expected-multimodal-methods",
        default="",
        help="Comma-separated multimodal method list that must exactly match the summary.",
    )
    parser.add_argument(
        "--skip-multimodal",
        action="store_true",
        help="Do not require multimodal fixture artifacts.",
    )
    args = parser.parse_args()

    demo_dir = Path(args.demo_dir)
    validate(
        demo_dir,
        expected_lidar_methods=split_csv(args.expected_lidar_methods),
        expected_multimodal_methods=split_csv(args.expected_multimodal_methods),
        skip_multimodal=args.skip_multimodal,
    )
    print(f"demo artifacts valid: {demo_dir}")


if __name__ == "__main__":
    main()
