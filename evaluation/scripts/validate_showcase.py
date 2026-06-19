#!/usr/bin/env python3
"""Validate the repository showcase surface used by README and GitHub Pages."""

from __future__ import annotations

import argparse
import json
import math
import re
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
    "rank_metric",
    "Coverage",
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

PUBLIC_ROW_REQUIRED_FIELDS = {
    "name",
    "status",
    "benchmark_id",
    "benchmark_label",
    "comparison_group_id",
    "sequence_or_window",
    "scope",
    "result_role",
    "evaluation_policy",
    "rankable",
    "rank_metric",
    "frames",
    "trajectory_length_m",
    "generated_at",
    "artifact",
}

RANKABLE_PUBLIC_GROUPS = {"kitti_00_full", "kitti_07_full"}

STABLE_FULL_SEQUENCE_ARTIFACTS = {
    "UA-LIO": {
        "00": {
            "path": "docs/benchmarks/kitti_full_new_methods/seq00_ua_lio.json",
            "frames": 4541,
            "max_rpe_trans_pct": 2.0,
        },
        "07": {
            "path": "docs/benchmarks/kitti_full_new_methods/seq07_ua_lio.json",
            "frames": 1101,
            "max_rpe_trans_pct": 2.0,
        },
    },
    "PL-LOAM": {
        "00": {
            "path": "docs/benchmarks/kitti_full_new_methods/seq00_pl_loam.json",
            "frames": 4541,
            "max_rpe_trans_pct": 100.0,
        },
        "07": {
            "path": "docs/benchmarks/kitti_full_new_methods/seq07_pl_loam.json",
            "frames": 1101,
            "max_rpe_trans_pct": 100.0,
        },
    },
    "InTEn-LOAM": {
        "00": {
            "path": "docs/benchmarks/kitti_full_new_methods/seq00_inten_loam.json",
            "frames": 4541,
            "max_rpe_trans_pct": 70.0,
        },
        "07": {
            "path": "docs/benchmarks/kitti_full_new_methods/seq07_inten_loam.json",
            "frames": 1101,
            "max_rpe_trans_pct": 70.0,
        },
    },
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


def finite_number(value: object) -> bool:
    if value is None or isinstance(value, bool):
        return False
    try:
        return math.isfinite(float(value))
    except (TypeError, ValueError):
        return False


def policy_of(method: dict) -> str:
    if method.get("evaluation_policy"):
        return str(method["evaluation_policy"])
    note = str(method.get("note", "")).lower()
    if "gt-seeded" in note or "ground-truth pose" in note:
        return "gt_seeded_reference"
    if "odometry-only" in note or "first gt pose" in note:
        return "odometry_only"
    return "not_run" if method.get("status") == "skipped" else "unspecified"


def latest_benchmark_label(results: dict) -> str:
    name = str(results.get("dataset", {}).get("name", "Latest"))
    if "Autoware Istanbul" in name:
        return "Autoware 108"
    return name


def latest_sequence_or_window(results: dict) -> str:
    name = str(results.get("dataset", {}).get("name", ""))
    match = re.search(r"frames\s+([0-9-]+)", name, flags=re.IGNORECASE)
    return match.group(1) if match else "latest"


def public_benchmark_rows(results: dict, paper_bundle: dict) -> list[dict]:
    dataset = results.get("dataset", {})
    rows: list[dict] = []
    for method in results.get("methods", []):
        if method.get("status") not in {"ok", "skipped"}:
            continue
        row = {
            **method,
            "benchmark_id": "autoware_istanbul",
            "benchmark_label": latest_benchmark_label(results),
            "comparison_group_id": "autoware_108_reference",
            "sequence_or_window": latest_sequence_or_window(results),
            "scope": "snapshot",
            "result_role": "reference_snapshot" if method.get("status") == "ok" else "not_run",
            "evaluation_policy": policy_of(method),
            "rankable": False,
            "rank_metric": None,
            "trajectory_length_m": dataset.get("trajectory_length_m"),
            "timestamp_source": dataset.get("timestamp_source"),
            "generated_at": results.get("generated_at"),
            "artifact": "docs/benchmarks/latest/results.json",
            "method_artifact": method.get("artifact"),
            "runtime_profile": "latest_snapshot",
        }
        rows.append(row)

    for source in paper_bundle.get("paper_table_rows", []):
        row = {
            "name": source.get("method"),
            "status": "ok",
            "note": f"{source.get('tier')}; {source.get('variant')}",
            **source,
        }
        rows.append(row)
    return rows


def validate_public_result_rows(root: Path, results: dict, paper_bundle: dict) -> None:
    rows = public_benchmark_rows(results, paper_bundle)
    if not rows:
        raise SystemExit("public benchmark rows are empty")

    rankable: list[dict] = []
    for row in rows:
        label = f"{row.get('benchmark_label')} / {row.get('name')}"
        missing = [field for field in PUBLIC_ROW_REQUIRED_FIELDS if field not in row]
        if missing:
            raise SystemExit(f"public row missing fields: {label} {missing}")
        if row.get("status") in {"invalidated", "superseded"}:
            raise SystemExit(f"public row exposes invalidated/superseded value: {label}")
        if row.get("result_role") == "regression_smoke":
            raise SystemExit(f"smoke row leaked into public benchmark rows: {label}")
        if row.get("status") == "ok":
            for field in ("frames", "trajectory_length_m", "ate_m"):
                if not finite_number(row.get(field)) or float(row[field]) <= 0.0:
                    raise SystemExit(f"public row has invalid {field}: {label} {row.get(field)}")
            if row.get("fps") is not None and not finite_number(row.get("fps")):
                raise SystemExit(f"public row has invalid fps: {label} {row.get('fps')}")
            require_file(root / str(row["artifact"]))
        if row.get("rankable"):
            rankable.append(row)
            if row.get("status") != "ok":
                raise SystemExit(f"rankable row is not OK: {label}")
            if row.get("comparison_group_id") not in RANKABLE_PUBLIC_GROUPS:
                raise SystemExit(f"rankable row has unsupported group: {label}")
            if row.get("scope") != "full_sequence":
                raise SystemExit(f"rankable row is not full-sequence: {label}")
            if row.get("result_role") != "paper_ready_full":
                raise SystemExit(f"rankable row has non-paper-ready role: {label}")
            if row.get("evaluation_policy") != "odometry_only":
                raise SystemExit(f"rankable row has non-odometry policy: {label}")
            if row.get("rank_metric") != "rpe_trans_pct":
                raise SystemExit(f"rankable row is not ranked by RPE: {label}")
            if not finite_number(row.get("rpe_trans_pct")):
                raise SystemExit(f"rankable row missing RPE: {label}")
        else:
            if row.get("rank_metric") is not None and row.get("result_role") != "not_run":
                raise SystemExit(f"unranked row unexpectedly has a rank metric: {label}")

    rankable_groups = {row["comparison_group_id"] for row in rankable}
    if rankable_groups != RANKABLE_PUBLIC_GROUPS:
        raise SystemExit(f"rankable groups mismatch: {sorted(rankable_groups)}")

    for group in rankable_groups:
        group_rows = [row for row in rankable if row["comparison_group_id"] == group]
        policies = {row["evaluation_policy"] for row in group_rows}
        scopes = {row["scope"] for row in group_rows}
        metrics = {row["rank_metric"] for row in group_rows}
        if len(group_rows) < 2:
            raise SystemExit(f"rankable group has too few rows: {group}")
        if policies != {"odometry_only"} or scopes != {"full_sequence"} or metrics != {"rpe_trans_pct"}:
            raise SystemExit(f"rankable group is semantically mixed: {group}")


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

    validate_public_result_rows(root, results, paper_bundle)


def validate_kitti_full_artifact_notes(root: Path) -> None:
    artifact_dir = root / "docs/benchmarks/kitti_full_new_methods"
    for path in sorted(artifact_dir.glob("seq[0-9][0-9]_*.json")):
        artifact = load_json(path)
        for method in artifact.get("methods", []):
            if method.get("status") != "ok":
                continue
            note = str(method.get("note", "")).lower()
            if "gt-seeded init" in note or "gt-seeded initialization" in note:
                raise SystemExit(
                    f"KITTI full artifact claims GT-seeded init: "
                    f"{path.relative_to(root)} / {method.get('name')}"
                )


def validate_stable_full_sequence_artifacts(root: Path) -> None:
    for method_name, sequences in STABLE_FULL_SEQUENCE_ARTIFACTS.items():
        for sequence, spec in sequences.items():
            artifact_rel = spec["path"]
            artifact = load_json(root / artifact_rel)
            methods = artifact.get("methods", [])
            if len(methods) != 1:
                raise SystemExit(f"{artifact_rel} must contain exactly one method")
            method = methods[0]
            label = f"{method_name} seq{sequence}"
            if method.get("name") != method_name:
                raise SystemExit(f"{label} artifact has wrong method: {method.get('name')}")
            if method.get("status") != "ok":
                raise SystemExit(f"{label} is not stable OK: {method.get('status')}")
            if artifact.get("schema_version") != 2:
                raise SystemExit(f"{label} artifact is not schema v2")
            if artifact.get("association_mode") != "exact_frame_id":
                raise SystemExit(f"{label} does not use exact frame association")
            if method.get("frames") != spec["frames"] or artifact.get("num_frames") != spec["frames"]:
                raise SystemExit(f"{label} frame count changed from {spec['frames']}")
            for field in ("ate_m", "rpe_trans_pct", "fps"):
                if not finite_number(method.get(field)) or float(method[field]) <= 0.0:
                    raise SystemExit(f"{label} has invalid {field}: {method.get(field)}")
            max_rpe = spec.get("max_rpe_trans_pct")
            if max_rpe is not None and float(method["rpe_trans_pct"]) > float(max_rpe):
                raise SystemExit(
                    f"{label} exceeds stable RPE guard: "
                    f"{method['rpe_trans_pct']} > {max_rpe}"
                )


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
    validate_kitti_full_artifact_notes(root)
    validate_stable_full_sequence_artifacts(root)
    validate_scan2d_benchmarks(root)
    validate_demo(root, Path(args.demo_dir), args.demo_mode)
    print("showcase valid")


if __name__ == "__main__":
    main()
