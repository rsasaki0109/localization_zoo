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

SEED_PROTOCOLS: dict[str, dict[str, Any]] = {
    "no-gt-seed": {
        "label": "no-gt-seed velocity prediction after frame 0",
        "seed_flags": ["--no-gt-seed"],
        "artifact_prefix": "m_gclo_no_gt_seed",
        "claim_limit": (
            "Honest negative: shared no-gt-seed odometry diverges on MulRan ParkingLot; "
            "ground on/off delta is inconclusive under this failure mode."
        ),
    },
    "gt-seed": {
        "label": "GT-seeded scan-to-map initialization (requires M-GCLO gt_seeded_frames in note)",
        "seed_flags": [],
        "artifact_prefix": "m_gclo_gt_seed",
        "claim_limit": (
            "Public off-road / multi-beam non-flat check with GT-seeded M-GCLO; "
            "supports ground-factor mechanism evidence but is an oracle-init protocol, "
            "not a blind odometry claim."
        ),
    },
}


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


def gt_seeded_frames(note: str) -> int | None:
    match = re.search(r"gt_seeded_frames=(\d+)", note)
    return int(match.group(1)) if match else None


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


def build_protocol_block(
    protocol_key: str,
    on_json: Path,
    off_json: Path,
) -> dict[str, Any]:
    cfg = SEED_PROTOCOLS[protocol_key]
    on_data = load(on_json)
    on_methods = methods(on_json)
    off_methods = methods(off_json)
    mg_on = on_methods["M-GCLO"]
    mg_off = off_methods["M-GCLO"]
    kiss = on_methods.get("KISS-ICP")
    note_on = mg_on.get("note", "")
    note_off = mg_off.get("note", "")
    delta_block = delta(mg_on, mg_off)
    seed_flags = cfg["seed_flags"]
    seed_suffix = " ".join(seed_flags)
    return {
        "seed_protocol": protocol_key,
        "seed_label": cfg["label"],
        "claim_limit": cfg["claim_limit"],
        "frames": on_data.get("num_frames"),
        "trajectory_length_m": on_data.get("trajectory_length_m"),
        "artifacts": {
            "ground_on_with_kiss": rel(on_json),
            "ground_off": rel(off_json),
        },
        "protocol": {
            "ground_on_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                f"--methods m_gclo,kiss_icp {seed_suffix} --m-gclo-dense-profile "
                f"--summary-json {rel(on_json)}"
            ).strip(),
            "ground_off_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                f"--methods m_gclo {seed_suffix} --m-gclo-dense-profile --m-gclo-no-ground "
                f"--summary-json {rel(off_json)}"
            ).strip(),
        },
        "kiss_sanity_reference": metric_subset(kiss) if kiss else None,
        "m_gclo": {
            "ground_on": {
                "metrics": metric_subset(mg_on),
                "correspondences": ground_nonground(note_on),
                "gt_seeded_frames": gt_seeded_frames(note_on),
                "note": note_on,
            },
            "ground_off": {
                "metrics": metric_subset(mg_off),
                "correspondences": ground_nonground(note_off),
                "gt_seeded_frames": gt_seeded_frames(note_off),
                "note": note_off,
            },
            "ground_off_minus_on": delta_block,
        },
    }


def build_conclusion(summary: dict[str, Any]) -> str:
    gt = summary["protocols"]["gt-seed"]
    no_gt = summary["protocols"]["no-gt-seed"]
    mg_gt_on = gt["m_gclo"]["ground_on"]["metrics"]
    mg_gt_off = gt["m_gclo"]["ground_off"]["metrics"]
    delta_gt = gt["m_gclo"]["ground_off_minus_on"]
    kiss_no = no_gt["kiss_sanity_reference"] or {}
    mg_no_on = no_gt["m_gclo"]["ground_on"]["metrics"]
    return (
        "On public MulRan ParkingLot full (1176 Ouster frames), no-gt-seed odometry "
        f"diverges (~{kiss_no.get('rpe_trans_pct', 0):.1f}% KISS-ICP RPE, "
        f"~{mg_no_on.get('ate_m', 0):.1f} m M-GCLO ATE) — ground on/off is inconclusive "
        "there. With GT-seeded M-GCLO initialization, tracking stays stable and disabling "
        f"the ground factor changes RPE from {mg_gt_on['rpe_trans_pct']:.3f}% to "
        f"{mg_gt_off['rpe_trans_pct']:.3f}% and ATE from {mg_gt_on['ate_m']:.2f} m to "
        f"{mg_gt_off['ate_m']:.2f} m "
        f"(~{abs(delta_gt.get('ate_m_relative_pct', 0) or 0):.1f}% ATE delta). "
        "This is oracle-init mechanism evidence on a dedicated off-road / multi-beam "
        "sequence, not blind odometry or automatic T0 promotion."
    )


def build_summary(protocol_runs: dict[str, tuple[Path, Path]], output: Path) -> None:
    protocols = {
        key: build_protocol_block(key, on_json, off_json)
        for key, (on_json, off_json) in protocol_runs.items()
    }
    summary = {
        "schema_version": 2,
        "benchmark": "mulran_parkinglot_public_validation",
        "generated_by": "evaluation/scripts/run_m_gclo_mulran_parkinglot_validation.py",
        "scope": {
            "dataset": "MulRan ParkingLot Ouster full (1176 frames)",
            "frames": protocols["gt-seed"]["frames"],
            "trajectory_length_m": protocols["gt-seed"]["trajectory_length_m"],
        },
        "protocols": protocols,
        "conclusion": "",
    }
    summary["conclusion"] = build_conclusion(summary)
    output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"wrote {rel(output)}")


def run_protocol(
    pcd_bin: Path,
    protocol_key: str,
    *,
    summary_only: bool,
) -> tuple[Path, Path]:
    cfg = SEED_PROTOCOLS[protocol_key]
    prefix = cfg["artifact_prefix"]
    ground_on_json = BENCH_DIR / f"{prefix}_ground_on.json"
    ground_off_json = BENCH_DIR / f"{prefix}_ground_off.json"
    if summary_only:
        return ground_on_json, ground_off_json

    base = [
        rel(pcd_bin),
        rel(PCD_DIR),
        rel(GT_CSV),
        "--methods",
    ]
    run(
        base
        + ["m_gclo,kiss_icp"]
        + cfg["seed_flags"]
        + ["--m-gclo-dense-profile", "--summary-json", rel(ground_on_json)]
    )
    run(
        base
        + ["m_gclo"]
        + cfg["seed_flags"]
        + [
            "--m-gclo-dense-profile",
            "--m-gclo-no-ground",
            "--summary-json",
            rel(ground_off_json),
        ]
    )
    return ground_on_json, ground_off_json


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pcd-bin", default=str(DEFAULT_BIN))
    parser.add_argument(
        "--seed-protocol",
        choices=sorted(SEED_PROTOCOLS),
        action="append",
        dest="seed_protocols",
        help="Which seed protocol(s) to run (default: gt-seed only).",
    )
    parser.add_argument("--summary-only", action="store_true")
    args = parser.parse_args()

    seed_protocols = args.seed_protocols or ["gt-seed"]
    pcd_bin = Path(args.pcd_bin)
    if not pcd_bin.is_absolute():
        pcd_bin = REPO_ROOT / pcd_bin
    if not args.summary_only and not pcd_bin.exists():
        raise SystemExit(f"missing pcd_dogfooding binary: {rel(pcd_bin)}")

    BENCH_DIR.mkdir(parents=True, exist_ok=True)
    summary_json = BENCH_DIR / "m_gclo_mulran_parkinglot_validation_summary.json"

    protocol_runs: dict[str, tuple[Path, Path]] = {}
    for protocol_key in seed_protocols:
        on_json, off_json = run_protocol(
            pcd_bin, protocol_key, summary_only=args.summary_only
        )
        if not on_json.exists() or not off_json.exists():
            raise SystemExit(
                f"missing MulRan validation artifacts for {protocol_key}; "
                "run without --summary-only"
            )
        protocol_runs[protocol_key] = (on_json, off_json)

    if "no-gt-seed" not in protocol_runs:
        no_gt_on = BENCH_DIR / "m_gclo_no_gt_seed_ground_on.json"
        no_gt_off = BENCH_DIR / "m_gclo_no_gt_seed_ground_off.json"
        legacy_on = BENCH_DIR / "m_gclo_ground_on.json"
        legacy_off = BENCH_DIR / "m_gclo_ground_off.json"
        if no_gt_on.is_file() and no_gt_off.is_file():
            protocol_runs["no-gt-seed"] = (no_gt_on, no_gt_off)
        elif legacy_on.is_file() and legacy_off.is_file():
            protocol_runs["no-gt-seed"] = (legacy_on, legacy_off)

    if "gt-seed" not in protocol_runs:
        raise SystemExit("gt-seed MulRan artifacts missing; run with --seed-protocol gt-seed")

    build_summary(protocol_runs, summary_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
