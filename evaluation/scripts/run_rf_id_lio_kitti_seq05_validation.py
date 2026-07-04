#!/usr/bin/env python3
"""Run RF-LIO/ID-LIO public validation on urban KITTI Odometry seq05."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_BIN = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"
BENCH_DIR = REPO_ROOT / "docs" / "benchmarks" / "kitti_seq05_public"
PCD_DIR = REPO_ROOT / "dogfooding_results" / "kitti_seq_05_full"
GT_CSV = REPO_ROOT / "experiments" / "reference_data" / "kitti_seq_05_full_gt.csv"
DENSE_PROFILE_FLAGS = [
    "--id-lio-dense-profile",
    "--rf-lio-dense-profile",
]
RF_CONSERVATIVE_FLAGS = [
    "--rf-lio-foreground-margin",
    "1.5",
    "--rf-lio-max-removal-fraction",
    "0.08",
]


def rel(path: Path) -> str:
    return path.relative_to(REPO_ROOT).as_posix()


def run(cmd: list[str]) -> None:
    print("+ " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def methods(path: Path) -> dict[str, dict[str, Any]]:
    return {m["name"]: m for m in load(path)["methods"]}


def note_value(note: str, key: str) -> float | None:
    match = re.search(rf"{re.escape(key)}=([-+0-9.eE]+)", note)
    return float(match.group(1)) if match else None


def id_lio_stats(note: str) -> dict[str, float | None]:
    return {
        "dynamic_per_frame": note_value(note, "dynamic/frame"),
        "correspondences_per_frame": note_value(note, "corr/frame"),
        "removed_sum_per_frame": note_value(note, "removed_sum/frame"),
    }


def rf_lio_stats(note: str) -> dict[str, float | None]:
    return {
        "removal_first_per_frame": note_value(note, "removal_first/frame"),
        "candidates_per_frame": note_value(note, "candidates/frame"),
        "kept_per_frame": note_value(note, "kept/frame"),
        "backend_dynamic_per_frame": note_value(note, "backend_dynamic/frame"),
        "correspondences_per_frame": note_value(note, "corr/frame"),
        "backend_removed_sum_per_frame": note_value(note, "backend_removed_sum/frame"),
    }


def metric_subset(method: dict[str, Any]) -> dict[str, Any]:
    return {
        key: method.get(key)
        for key in ("ate_m", "rpe_trans_pct", "rpe_rot_deg_per_m", "frames", "fps")
    }


def delta(base: dict[str, Any], variant: dict[str, Any]) -> dict[str, Any]:
    out: dict[str, Any] = {}
    for key in ("ate_m", "rpe_trans_pct", "rpe_rot_deg_per_m"):
        a = base.get(key)
        b = variant.get(key)
        if a is None or b is None:
            continue
        out[f"{key}_abs_delta"] = b - a
        out[f"{key}_relative_pct"] = ((b - a) / a * 100.0) if abs(a) > 1e-12 else None
    return out


def build_summary(
    default_json: Path,
    rf_conservative_json: Path,
    output: Path,
) -> None:
    on_data = load(default_json)
    default_methods = methods(default_json)
    rf_conservative = methods(rf_conservative_json)["RF-LIO"]
    kiss = default_methods["KISS-ICP"]
    id_lio = default_methods["ID-LIO"]
    rf_lio = default_methods["RF-LIO"]
    id_note = id_lio.get("note", "")
    rf_note = rf_lio.get("note", "")
    rf_cons_note = rf_conservative.get("note", "")
    rf_vs_id_rpe = delta(id_lio, rf_lio).get("rpe_trans_pct_relative_pct")
    cons_vs_default = delta(rf_lio, rf_conservative)
    summary = {
        "schema_version": 1,
        "benchmark": "kitti_odometry_seq05_public_validation",
        "generated_by": "evaluation/scripts/run_rf_id_lio_kitti_seq05_validation.py",
        "scope": {
            "dataset": "KITTI Odometry sequence 05 full",
            "frames": on_data.get("num_frames"),
            "trajectory_length_m": on_data.get("trajectory_length_m"),
            "claim_limit": (
                "Public urban KITTI seq05 check with more city traffic than seq00/07; "
                "not a substitute for dedicated high-dynamic multi-beam benchmarks."
            ),
        },
        "artifacts": {
            "default_methods": rel(default_json),
            "rf_lio_conservative": rel(rf_conservative_json),
        },
        "protocol": {
            "default_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                "--methods id_lio,rf_lio,kiss_icp --no-gt-seed "
                + " ".join(DENSE_PROFILE_FLAGS)
                + f" --summary-json {rel(default_json)}"
            ),
            "rf_lio_conservative_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                "--methods rf_lio --no-gt-seed "
                + " ".join(DENSE_PROFILE_FLAGS)
                + " "
                + " ".join(RF_CONSERVATIVE_FLAGS)
                + f" --summary-json {rel(rf_conservative_json)}"
            ),
        },
        "kiss_sanity_reference": metric_subset(kiss),
        "methods": {
            "id_lio": {
                "metrics": metric_subset(id_lio),
                "dynamic_statistics": id_lio_stats(id_note),
                "note": id_note,
            },
            "rf_lio_default": {
                "metrics": metric_subset(rf_lio),
                "dynamic_statistics": rf_lio_stats(rf_note),
                "note": rf_note,
            },
            "rf_lio_conservative": {
                "metrics": metric_subset(rf_conservative),
                "dynamic_statistics": rf_lio_stats(rf_cons_note),
                "note": rf_cons_note,
                "conservative_vs_default": cons_vs_default,
            },
            "rf_lio_default_minus_id_lio": delta(id_lio, rf_lio),
        },
        "conclusion": (
            "On public KITTI Odometry seq05 full (urban driving), both dynamic "
            f"paths stay active (ID-LIO dynamic/frame "
            f"{id_lio_stats(id_note).get('dynamic_per_frame', 0):.0f}, RF-LIO "
            f"removal_first/frame {rf_lio_stats(rf_note).get('removal_first_per_frame', 0):.0f}). "
            f"Default RF-LIO RPE is {rf_lio['rpe_trans_pct']:.3f}% vs ID-LIO "
            f"{id_lio['rpe_trans_pct']:.3f}% ({rf_vs_id_rpe:+.1f}% relative) and "
            f"KISS-ICP {kiss['rpe_trans_pct']:.3f}%. Conservative RF removal "
            f"changes RPE by {cons_vs_default.get('rpe_trans_pct_relative_pct', 0):+.1f}% "
            f"vs default RF-LIO. This records mechanism behavior on a more dynamic "
            "public KITTI sequence but does not replace dedicated high-dynamic "
            "multi-beam benchmarks for manuscript-level dynamic-scene claims."
        ),
    }
    output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"wrote {rel(output)}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pcd-bin", default=str(DEFAULT_BIN))
    parser.add_argument("--summary-only", action="store_true")
    args = parser.parse_args()

    pcd_bin = Path(args.pcd_bin)
    if not pcd_bin.is_absolute():
        pcd_bin = REPO_ROOT / pcd_bin
    if not args.summary_only and not pcd_bin.exists():
        raise SystemExit(f"missing pcd_dogfooding binary: {rel(pcd_bin)}")

    BENCH_DIR.mkdir(parents=True, exist_ok=True)
    default_json = BENCH_DIR / "rf_id_lio_default.json"
    rf_conservative_json = BENCH_DIR / "rf_lio_conservative.json"
    summary_json = BENCH_DIR / "rf_id_lio_kitti_seq05_validation_summary.json"

    if not args.summary_only:
        run(
            [
                rel(pcd_bin),
                rel(PCD_DIR),
                rel(GT_CSV),
                "--methods",
                "id_lio,rf_lio,kiss_icp",
                "--no-gt-seed",
                *DENSE_PROFILE_FLAGS,
                "--summary-json",
                rel(default_json),
            ]
        )
        run(
            [
                rel(pcd_bin),
                rel(PCD_DIR),
                rel(GT_CSV),
                "--methods",
                "rf_lio",
                "--no-gt-seed",
                *DENSE_PROFILE_FLAGS,
                *RF_CONSERVATIVE_FLAGS,
                "--summary-json",
                rel(rf_conservative_json),
            ]
        )

    if not default_json.exists() or not rf_conservative_json.exists():
        raise SystemExit("missing seq05 validation artifacts; run without --summary-only")

    build_summary(default_json, rf_conservative_json, summary_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
