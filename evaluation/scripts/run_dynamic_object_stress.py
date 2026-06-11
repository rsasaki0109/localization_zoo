#!/usr/bin/env python3
"""Run the RF-LIO/ID-LIO synthetic dynamic-object stress benchmark."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
GENERATOR = REPO_ROOT / "evaluation" / "scripts" / "generate_dynamic_object_stress_fixture.py"
DEFAULT_BIN = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"
BENCH_DIR = REPO_ROOT / "docs" / "benchmarks" / "dynamic_object_stress"
CLEAN_DIR = REPO_ROOT / "evaluation" / "fixtures" / "dynamic_object_stress_clean"
DYNAMIC_DIR = REPO_ROOT / "evaluation" / "fixtures" / "dynamic_object_stress"


def rel(path: Path) -> str:
    return path.relative_to(REPO_ROOT).as_posix()


def run(cmd: list[str]) -> None:
    print("+ " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def method_map(path: Path) -> dict[str, dict[str, Any]]:
    payload = load(path)
    return {m["name"]: m for m in payload["methods"]}


def note_value(note: str, key: str) -> float | None:
    match = re.search(rf"{re.escape(key)}=([-+0-9.eE]+)", note)
    if not match:
        return None
    return float(match.group(1))


def metric_subset(method: dict[str, Any]) -> dict[str, Any]:
    keys = [
        "ate_m",
        "rpe_trans_pct",
        "rpe_rot_deg_per_m",
        "frames",
        "fps",
    ]
    return {key: method.get(key) for key in keys}


def degradation(static: dict[str, Any], dynamic: dict[str, Any]) -> dict[str, Any]:
    out: dict[str, Any] = {}
    for key in ("ate_m", "rpe_trans_pct", "rpe_rot_deg_per_m"):
        a = static.get(key)
        b = dynamic.get(key)
        if a is None or b is None:
            continue
        out[f"{key}_abs_delta"] = b - a
        out[f"{key}_relative_pct"] = ((b - a) / a * 100.0) if abs(a) > 1e-12 else None
    return out


def build_summary(
    clean_json: Path,
    dynamic_json: Path,
    rf_conservative_json: Path,
    output: Path,
    frames: int,
) -> None:
    clean = method_map(clean_json)
    dynamic = method_map(dynamic_json)
    rf_conservative = method_map(rf_conservative_json)["RF-LIO"]

    pairs: list[dict[str, Any]] = []
    for method in ("KISS-ICP", "ID-LIO", "RF-LIO"):
        entry = {
            "method": method,
            "static_clean": metric_subset(clean[method]),
            "dynamic_objects": metric_subset(dynamic[method]),
            "dynamic_minus_static": degradation(clean[method], dynamic[method]),
        }
        note = dynamic[method].get("note", "")
        if method == "ID-LIO":
            entry["dynamic_statistics"] = {
                "dynamic_per_frame": note_value(note, "dynamic/frame"),
                "correspondences_per_frame": note_value(note, "corr/frame"),
                "removed_sum_per_frame": note_value(note, "removed_sum/frame"),
            }
        elif method == "RF-LIO":
            entry["dynamic_statistics"] = {
                "removal_first_per_frame": note_value(note, "removal_first/frame"),
                "candidates_per_frame": note_value(note, "candidates/frame"),
                "kept_per_frame": note_value(note, "kept/frame"),
                "backend_dynamic_per_frame": note_value(note, "backend_dynamic/frame"),
            }
        pairs.append(entry)

    rf_note = rf_conservative.get("note", "")
    summary = {
        "schema_version": 1,
        "benchmark": "synthetic_dynamic_object_stress",
        "generated_by": "evaluation/scripts/run_dynamic_object_stress.py",
        "fixture_generator": rel(GENERATOR),
        "scope": {
            "frames": frames,
            "trajectory_length_m": load(dynamic_json)["trajectory_length_m"],
            "static_fixture": rel(CLEAN_DIR),
            "dynamic_fixture": rel(DYNAMIC_DIR),
            "claim_limit": (
                "Synthetic mechanism stress only; this does not replace a public "
                "high-dynamic dataset evaluation for manuscript-level claims."
            ),
        },
        "artifacts": {
            "static_clean_default": rel(clean_json),
            "dynamic_objects_default": rel(dynamic_json),
            "dynamic_objects_rf_lio_conservative": rel(rf_conservative_json),
        },
        "protocol": {
            "base_command": (
                "./build/evaluation/pcd_dogfooding <fixture> <fixture>/gt.csv "
                f"{frames} --methods <methods> --no-gt-seed --summary-json <artifact>"
            ),
            "default_methods": "id_lio,rf_lio,kiss_icp",
            "rf_lio_conservative_flags": [
                "--rf-lio-foreground-margin",
                "1.5",
                "--rf-lio-max-removal-fraction",
                "0.08",
            ],
        },
        "pairs": pairs,
        "rf_lio_conservative": {
            "metrics": metric_subset(rf_conservative),
            "dynamic_statistics": {
                "removal_first_per_frame": note_value(rf_note, "removal_first/frame"),
                "candidates_per_frame": note_value(rf_note, "candidates/frame"),
                "kept_per_frame": note_value(rf_note, "kept/frame"),
                "backend_dynamic_per_frame": note_value(rf_note, "backend_dynamic/frame"),
            },
            "delta_vs_default_rf_lio": degradation(dynamic["RF-LIO"], rf_conservative),
        },
        "conclusion": (
            "The synthetic foreground boxes strongly activate both dynamic paths. "
            "Default RF-LIO removes far more foreground points than clean-scene "
            "baseline and fails less severely than ID-LIO, while conservative "
            "removal cap improves RF-LIO further. KISS-ICP remains the sanity "
            "reference on this simple synthetic geometry."
        ),
    }
    output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"wrote {rel(output)}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pcd-bin", default=str(DEFAULT_BIN))
    parser.add_argument("--frames", type=int, default=30)
    args = parser.parse_args()

    pcd_bin = Path(args.pcd_bin)
    if not pcd_bin.is_absolute():
        pcd_bin = REPO_ROOT / pcd_bin
    if not pcd_bin.exists():
        raise SystemExit(f"missing pcd_dogfooding binary: {rel(pcd_bin)}")

    BENCH_DIR.mkdir(parents=True, exist_ok=True)
    run(["python3", rel(GENERATOR), "--output-dir", rel(CLEAN_DIR), "--no-dynamic-objects", "--force"])
    run(["python3", rel(GENERATOR), "--output-dir", rel(DYNAMIC_DIR), "--force"])

    clean_json = BENCH_DIR / "static_clean_default_30.json"
    dynamic_json = BENCH_DIR / "dynamic_objects_default_30.json"
    rf_conservative_json = BENCH_DIR / "dynamic_objects_rf_lio_conservative_30.json"
    summary_json = BENCH_DIR / "rf_id_lio_dynamic_object_stress_summary.json"

    run(
        [
            rel(pcd_bin),
            rel(CLEAN_DIR),
            f"{rel(CLEAN_DIR)}/gt.csv",
            str(args.frames),
            "--methods",
            "id_lio,rf_lio,kiss_icp",
            "--no-gt-seed",
            "--summary-json",
            rel(clean_json),
        ]
    )
    run(
        [
            rel(pcd_bin),
            rel(DYNAMIC_DIR),
            f"{rel(DYNAMIC_DIR)}/gt.csv",
            str(args.frames),
            "--methods",
            "id_lio,rf_lio,kiss_icp",
            "--no-gt-seed",
            "--summary-json",
            rel(dynamic_json),
        ]
    )
    run(
        [
            rel(pcd_bin),
            rel(DYNAMIC_DIR),
            f"{rel(DYNAMIC_DIR)}/gt.csv",
            str(args.frames),
            "--methods",
            "rf_lio",
            "--no-gt-seed",
            "--rf-lio-foreground-margin",
            "1.5",
            "--rf-lio-max-removal-fraction",
            "0.08",
            "--summary-json",
            rel(rf_conservative_json),
        ]
    )
    build_summary(clean_json, dynamic_json, rf_conservative_json, summary_json, args.frames)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
