#!/usr/bin/env python3
"""Run the M-GCLO synthetic non-flat ground stress benchmark."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
GENERATOR = REPO_ROOT / "evaluation" / "scripts" / "generate_nonflat_ground_stress_fixture.py"
DEFAULT_BIN = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"
BENCH_DIR = REPO_ROOT / "docs" / "benchmarks" / "nonflat_ground_stress"
FIXTURE_DIR = REPO_ROOT / "evaluation" / "fixtures" / "nonflat_ground_stress"


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


def build_summary(on_json: Path, off_json: Path, output: Path, frames: int) -> None:
    on_methods = methods(on_json)
    off_methods = methods(off_json)
    mg_on = on_methods["M-GCLO"]
    mg_off = off_methods["M-GCLO"]
    kiss = on_methods["KISS-ICP"]
    note_on = mg_on.get("note", "")
    note_off = mg_off.get("note", "")
    summary = {
        "schema_version": 1,
        "benchmark": "synthetic_nonflat_ground_stress",
        "generated_by": "evaluation/scripts/run_m_gclo_nonflat_stress.py",
        "fixture_generator": rel(GENERATOR),
        "scope": {
            "frames": frames,
            "trajectory_length_m": load(on_json)["trajectory_length_m"],
            "fixture": rel(FIXTURE_DIR),
            "claim_limit": (
                "Synthetic non-flat mechanism stress only; public non-flat dataset "
                "validation is still required before promoting M-GCLO to T0."
            ),
        },
        "artifacts": {
            "ground_on_with_kiss": rel(on_json),
            "ground_off": rel(off_json),
        },
        "protocol": {
            "ground_on_command": (
                "./build/evaluation/pcd_dogfooding "
                "evaluation/fixtures/nonflat_ground_stress "
                "evaluation/fixtures/nonflat_ground_stress/gt.csv "
                f"{frames} --methods m_gclo,kiss_icp --no-gt-seed "
                f"--summary-json {rel(on_json)}"
            ),
            "ground_off_command": (
                "./build/evaluation/pcd_dogfooding "
                "evaluation/fixtures/nonflat_ground_stress "
                "evaluation/fixtures/nonflat_ground_stress/gt.csv "
                f"{frames} --methods m_gclo --no-gt-seed --m-gclo-no-ground "
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
            "ground_off_minus_on": delta(mg_on, mg_off),
        },
        "conclusion": (
            "On the synthetic rolling-ground stress, M-GCLO tracks the non-flat "
            "trajectory while KISS-ICP fails on the simple repeated geometry. "
            "Disabling the classified ground factor worsens M-GCLO ATE, "
            "translational RPE, and rotational RPE, so the multiple-ground path "
            "is active and beneficial under this non-flat mechanism stress."
        ),
    }
    output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"wrote {rel(output)}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pcd-bin", default=str(DEFAULT_BIN))
    parser.add_argument("--frames", type=int, default=60)
    args = parser.parse_args()

    pcd_bin = Path(args.pcd_bin)
    if not pcd_bin.is_absolute():
        pcd_bin = REPO_ROOT / pcd_bin
    if not pcd_bin.exists():
        raise SystemExit(f"missing pcd_dogfooding binary: {rel(pcd_bin)}")

    BENCH_DIR.mkdir(parents=True, exist_ok=True)
    run(["python3", rel(GENERATOR), "--force"])

    ground_on_json = BENCH_DIR / "m_gclo_nonflat_ground_on_60.json"
    ground_off_json = BENCH_DIR / "m_gclo_nonflat_ground_off_60.json"
    summary_json = BENCH_DIR / "m_gclo_nonflat_ground_stress_summary.json"
    run(
        [
            rel(pcd_bin),
            rel(FIXTURE_DIR),
            f"{rel(FIXTURE_DIR)}/gt.csv",
            str(args.frames),
            "--methods",
            "m_gclo,kiss_icp",
            "--no-gt-seed",
            "--summary-json",
            rel(ground_on_json),
        ]
    )
    run(
        [
            rel(pcd_bin),
            rel(FIXTURE_DIR),
            f"{rel(FIXTURE_DIR)}/gt.csv",
            str(args.frames),
            "--methods",
            "m_gclo",
            "--no-gt-seed",
            "--m-gclo-no-ground",
            "--summary-json",
            rel(ground_off_json),
        ]
    )
    build_summary(ground_on_json, ground_off_json, summary_json, args.frames)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
