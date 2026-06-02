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
    "docs/assets/explorer_preview.png",
    "bash evaluation/scripts/demo_localization_zoo.sh",
    "experiments/results/runs/demo_localization_zoo/report.html",
    "manifest.json",
]

INDEX_REQUIRED_SNIPPETS = [
    'const catalogPath = "./methods.json"',
    'const dataPath = "./benchmarks/latest/results.json"',
    "Method Explorer",
    "demo_localization_zoo.sh",
    "explorer_preview.png",
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

    preview_path = root / "docs/assets/explorer_preview.png"
    require_file(preview_path)
    with preview_path.open("rb") as handle:
        signature = handle.read(8)
    if signature != b"\x89PNG\r\n\x1a\n":
        raise SystemExit(f"README preview is not a PNG: {preview_path}")


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
    validate_demo(root, Path(args.demo_dir), args.demo_mode)
    print("showcase valid")


if __name__ == "__main__":
    main()
