#!/usr/bin/env python3
"""Validate the repository showcase surface used by README and GitHub Pages."""

from __future__ import annotations

import argparse
import json
import sys
from html.parser import HTMLParser
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import validate_demo_artifacts


README_REQUIRED_SNIPPETS = [
    "https://rsasaki0109.github.io/localization_zoo/",
    "docs/assets/hero_seq00.gif",
    "bash evaluation/scripts/demo_localization_zoo.sh",
    "experiments/results/runs/demo_localization_zoo/report.html",
    "manifest.json",
]

INDEX_REQUIRED_SNIPPETS = [
    'const catalogPath = "./methods.json"',
    'const dataPath = "./benchmarks/latest/results.json"',
    'const paperBundlePath = "./benchmarks/paper_ready_bundle.json"',
    "Method Explorer",
    "Odometry Results by Benchmark",
    "rankableOdometryRows",
    "renderOdometryRankings",
    "Benchmark Groups",
    "Overview only. Points from different datasets",
    "demo_localization_zoo.sh",
    "social_card.png",
]

METHOD_REQUIRED_FIELDS = {
    "name",
    "family",
    "scope",
    "signals",
    "href",
    "summary",
    "tags",
}


class HtmlCheckParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__()
        self.images: list[str] = []
        self.links: list[str] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        attr_map = {name: value for name, value in attrs}
        if tag == "img" and attr_map.get("src"):
            self.images.append(attr_map["src"] or "")
        if tag == "a" and attr_map.get("href"):
            self.links.append(attr_map["href"] or "")


def load_json(path: Path) -> dict:
    if not path.exists():
        raise SystemExit(f"missing JSON: {path}")
    with path.open() as handle:
        return json.load(handle)


def require_file(path: Path) -> None:
    if not path.exists():
        raise SystemExit(f"missing file: {path}")
    if path.is_file() and path.stat().st_size == 0:
        raise SystemExit(f"empty file: {path}")


def validate_readme(root: Path) -> None:
    readme_path = root / "README.md"
    require_file(readme_path)
    readme = readme_path.read_text()
    missing = [snippet for snippet in README_REQUIRED_SNIPPETS if snippet not in readme]
    if missing:
        raise SystemExit(f"README missing showcase snippets: {', '.join(missing)}")

    hero_path = root / "docs/assets/hero_seq00.gif"
    require_file(hero_path)
    with hero_path.open("rb") as handle:
        signature = handle.read(6)
    if not signature.startswith(b"GIF8"):
        raise SystemExit(f"README hero is not a GIF: {hero_path}")


def validate_pages_index(root: Path) -> None:
    index_path = root / "docs/index.html"
    require_file(index_path)
    text = index_path.read_text()
    HTMLParser().feed(text)
    missing = [snippet for snippet in INDEX_REQUIRED_SNIPPETS if snippet not in text]
    if missing:
        raise SystemExit(f"docs/index.html missing snippets: {', '.join(missing)}")

    parser = HtmlCheckParser()
    parser.feed(text)
    if not any("github.com/rsasaki0109/localization_zoo" in href for href in parser.links):
        raise SystemExit("docs/index.html is missing the GitHub repository link")


def validate_method_catalog(root: Path) -> None:
    catalog_path = root / "docs/methods.json"
    catalog = load_json(catalog_path)
    if catalog.get("schema_version") != 1:
        raise SystemExit(f"unsupported method catalog schema: {catalog.get('schema_version')}")

    methods = catalog.get("methods", [])
    if not methods:
        raise SystemExit("method catalog has no methods")

    names = [method.get("name") for method in methods]
    hrefs = [method.get("href") for method in methods]
    if len(names) != len(set(names)):
        raise SystemExit("method catalog has duplicate method names")
    if len(hrefs) != len(set(hrefs)):
        raise SystemExit("method catalog has duplicate hrefs")

    paper_dirs = {
        f"papers/{path.name}"
        for path in (root / "papers").iterdir()
        if path.is_dir()
    }
    if set(hrefs) != paper_dirs:
        missing = sorted(paper_dirs - set(hrefs))
        extra = sorted(set(hrefs) - paper_dirs)
        raise SystemExit(f"method catalog drift: missing={missing} extra={extra}")

    for method in methods:
        missing_fields = METHOD_REQUIRED_FIELDS - method.keys()
        if missing_fields:
            raise SystemExit(f"method missing fields: {method.get('name')} {missing_fields}")
        if not method["signals"]:
            raise SystemExit(f"method has no signals: {method['name']}")
        if not (root / method["href"] / "README.md").exists():
            raise SystemExit(f"method README missing: {method['href']}")

    track_refs = {
        method_name
        for track in catalog.get("starter_tracks", [])
        for method_name in track.get("methods", [])
    }
    unknown_refs = track_refs - set(names)
    if unknown_refs:
        raise SystemExit(f"starter tracks reference unknown methods: {sorted(unknown_refs)}")


def validate_benchmark_snapshot(root: Path) -> None:
    results_path = root / "docs/benchmarks/latest/results.json"
    results = load_json(results_path)
    dataset = results.get("dataset", {})
    if not dataset.get("name") or not dataset.get("frames"):
        raise SystemExit("benchmark snapshot is missing dataset name or frame count")

    methods = results.get("methods", [])
    ok_methods = [method for method in methods if method.get("status") == "ok"]
    if not ok_methods:
        raise SystemExit("benchmark snapshot has no OK methods")

    plot = results.get("artifacts", {}).get("trajectory_plot")
    if plot:
        require_file(root / "docs/benchmarks/latest" / plot)

    paper_bundle = load_json(root / "docs/benchmarks/paper_ready_bundle.json")
    paper_rows = paper_bundle.get("paper_table_rows")
    if not paper_rows:
        raise SystemExit("paper-ready benchmark bundle has no paper_table_rows")
    for row in paper_rows:
        missing = [
            field
            for field in ("method", "sequence", "frames", "ate_m", "rpe_trans_pct", "fps", "artifact")
            if row.get(field) is None
        ]
        if missing:
            raise SystemExit(f"paper-ready row missing required fields: {row.get('method')} {missing}")

    smoke = results.get("strict_smoke_baseline", {})
    for method in smoke.get("methods", []):
        if method.get("status") == "invalidated":
            raise SystemExit(f"strict smoke baseline contains invalidated method: {method.get('name')}")


def validate_scan2d_benchmarks(root: Path) -> None:
    bundle_path = root / "docs/benchmarks/scan2d/public_bundle.json"
    bundle = load_json(bundle_path)
    if bundle.get("schema_version") != 2:
        raise SystemExit(f"unsupported scan2d bundle schema: {bundle.get('schema_version')}")

    methods_order = bundle.get("methods_order", [])
    if not methods_order:
        raise SystemExit("scan2d bundle has no methods_order")
    require_file(root / bundle.get("readme", "docs/benchmarks/scan2d/README.md"))

    fixtures = bundle.get("fixtures", [])
    if not fixtures:
        raise SystemExit("scan2d bundle has no fixtures")

    known = set(methods_order)
    for fixture in fixtures:
        fid = fixture.get("id", "<unnamed>")
        artifact_rel = fixture.get("artifact")
        if not artifact_rel:
            raise SystemExit(f"scan2d fixture missing artifact path: {fid}")
        artifact = load_json(root / artifact_rel)
        artifact_drift = {
            method.get("name"): round(float(method["drift_pct"]), 3)
            for method in artifact.get("methods", [])
            if "drift_pct" in method
        }

        drift = fixture.get("drift_pct", {})
        if not drift:
            raise SystemExit(f"scan2d fixture has no drift_pct: {fid}")
        unknown = set(drift) - known
        if unknown:
            raise SystemExit(f"scan2d fixture {fid} has methods outside methods_order: {sorted(unknown)}")
        if fixture.get("best") not in drift:
            raise SystemExit(f"scan2d fixture {fid} best={fixture.get('best')} not in drift_pct")
        if fixture["best"] != min(drift, key=drift.get):
            raise SystemExit(f"scan2d fixture {fid} best={fixture['best']} does not match min drift")

        stale = {
            name: (value, artifact_drift.get(name))
            for name, value in drift.items()
            if round(float(value), 3) != artifact_drift.get(name)
        }
        if stale:
            raise SystemExit(
                f"scan2d bundle drift out of sync with {artifact_rel}: {stale} "
                "(refresh docs/benchmarks/scan2d/public_bundle.json from the fixture JSONs)"
            )


def validate_demo(root: Path, demo_dir: Path, mode: str) -> None:
    resolved_demo_dir = demo_dir if demo_dir.is_absolute() else root / demo_dir
    if mode == "skip":
        return
    if not resolved_demo_dir.exists():
        if mode == "optional":
            return
        raise SystemExit(f"required demo directory missing: {resolved_demo_dir}")
    validate_demo_artifacts.validate(
        resolved_demo_dir,
        expected_lidar_methods=[],
        expected_multimodal_methods=[],
        skip_multimodal=False,
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".", help="Repository root.")
    parser.add_argument(
        "--demo-dir",
        default="experiments/results/runs/demo_localization_zoo",
        help="Directory produced by demo_localization_zoo.sh.",
    )
    demo_group = parser.add_mutually_exclusive_group()
    demo_group.add_argument(
        "--require-demo",
        action="store_const",
        dest="demo_mode",
        const="require",
        help="Fail if demo artifacts are missing.",
    )
    demo_group.add_argument(
        "--skip-demo",
        action="store_const",
        dest="demo_mode",
        const="skip",
        help="Do not validate demo artifacts.",
    )
    parser.set_defaults(demo_mode="optional")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    validate_readme(root)
    validate_pages_index(root)
    validate_method_catalog(root)
    validate_benchmark_snapshot(root)
    validate_scan2d_benchmarks(root)
    validate_demo(root, Path(args.demo_dir), args.demo_mode)
    print("showcase valid")


if __name__ == "__main__":
    main()
