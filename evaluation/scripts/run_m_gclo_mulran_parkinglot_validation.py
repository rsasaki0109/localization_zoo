#!/usr/bin/env python3
"""Run M-GCLO ground on/off validation on public MulRan ParkingLot full."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_BIN = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"
BENCH_DIR = REPO_ROOT / "docs" / "benchmarks" / "mulran_parkinglot_public"
PCD_DIR = REPO_ROOT / "dogfooding_results" / "mulran_parkinglot_full"
GT_CSV = REPO_ROOT / "experiments" / "reference_data" / "mulran_parkinglot_full_gt.csv"


def rel(path: Path) -> str:
    return path.relative_to(REPO_ROOT).as_posix()


def run(cmd: list[str]) -> None:
    print("+ " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def methods(path: Path) -> dict[str, dict[str, Any]]:
    return {m["name"]: m for m in load(path)["methods"]}


def ground_nonground(note: str) -> dict[str, float | None]:
    match = re.search(r"mean_ground/nonground_corr=([-+0-9.eE]+)/([-+0-9.eE]+)", note)
    if not match:
        return {"ground_per_frame": None, "nonground_per_frame": None}
    return {
        "ground_per_frame": float(match.group(1)),
        "nonground_per_frame": float(match.group(2)),
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


def build_summary(on_json: Path, off_json: Path, output: Path) -> None:
    on_data = load(on_json)
    on_methods = methods(on_json)
    off_methods = methods(off_json)
    mg_on = on_methods["M-GCLO"]
    mg_off = off_methods["M-GCLO"]
    kiss = on_methods["KISS-ICP"]
    note_on = mg_on.get("note", "")
    note_off = mg_off.get("note", "")
    delta_block = delta(mg_on, mg_off)
    ate_rel = delta_block.get("ate_m_relative_pct")
    summary = {
        "schema_version": 1,
        "benchmark": "mulran_parkinglot_public_validation",
        "generated_by": "evaluation/scripts/run_m_gclo_mulran_parkinglot_validation.py",
        "scope": {
            "dataset": "MulRan ParkingLot Ouster full (1176 frames)",
            "frames": on_data.get("num_frames"),
            "trajectory_length_m": on_data.get("trajectory_length_m"),
            "claim_limit": (
                "Public off-road / multi-beam non-flat check on MulRan ParkingLot; "
                "supports M-GCLO ground-factor mechanism evidence but does not by "
                "itself promote the method to T0 without broader non-flat coverage."
            ),
        },
        "artifacts": {
            "ground_on_with_kiss": rel(on_json),
            "ground_off": rel(off_json),
        },
        "protocol": {
            "ground_on_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                "--methods m_gclo,kiss_icp --no-gt-seed --m-gclo-dense-profile "
                f"--summary-json {rel(on_json)}"
            ),
            "ground_off_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                "--methods m_gclo --no-gt-seed --m-gclo-dense-profile --m-gclo-no-ground "
                f"--summary-json {rel(off_json)}"
            ),
        },
        "kiss_sanity_reference": metric_subset(kiss),
        "m_gclo": {
            "ground_on": {
                "metrics": metric_subset(mg_on),
                "correspondences": ground_nonground(note_on),
                "note": note_on,
            },
            "ground_off": {
                "metrics": metric_subset(mg_off),
                "correspondences": ground_nonground(note_off),
                "note": note_off,
            },
            "ground_off_minus_on": delta_block,
        },
        "conclusion": (
            "On public MulRan ParkingLot full (1176 Ouster frames), shared no-gt-seed "
            f"odometry diverges for both KISS-ICP and M-GCLO (~{kiss['rpe_trans_pct']:.1f}% RPE, "
            f"~{kiss['ate_m']:.1f} m ATE). Disabling the classified ground factor leaves "
            f"translational RPE at {mg_on['rpe_trans_pct']:.3f}% vs {mg_off['rpe_trans_pct']:.3f}% "
            f"and whole-run ATE at {mg_on['ate_m']:.2f} m vs {mg_off['ate_m']:.2f} m — deltas are "
            "negligible under this failure mode. This commits the dedicated off-road / multi-beam "
            "benchmark attempt but does not support T0 promotion; GT-seeded or IMU-backed MulRan "
            "protocol work remains open."
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
    ground_on_json = BENCH_DIR / "m_gclo_ground_on.json"
    ground_off_json = BENCH_DIR / "m_gclo_ground_off.json"
    summary_json = BENCH_DIR / "m_gclo_mulran_parkinglot_validation_summary.json"

    if not args.summary_only:
        run(
            [
                rel(pcd_bin),
                rel(PCD_DIR),
                rel(GT_CSV),
                "--methods",
                "m_gclo,kiss_icp",
                "--no-gt-seed",
                "--m-gclo-dense-profile",
                "--summary-json",
                rel(ground_on_json),
            ]
        )
        run(
            [
                rel(pcd_bin),
                rel(PCD_DIR),
                rel(GT_CSV),
                "--methods",
                "m_gclo",
                "--no-gt-seed",
                "--m-gclo-dense-profile",
                "--m-gclo-no-ground",
                "--summary-json",
                rel(ground_off_json),
            ]
        )

    if not ground_on_json.exists() or not ground_off_json.exists():
        raise SystemExit("missing MulRan validation artifacts; run without --summary-only")

    build_summary(ground_on_json, ground_off_json, summary_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
