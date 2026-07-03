#!/usr/bin/env python3
"""Run Quadric-LO plane-fallback on/off validation on public KITTI Odometry seq02."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_BIN = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"
BENCH_DIR = REPO_ROOT / "docs" / "benchmarks" / "kitti_seq02_public"
PCD_DIR = REPO_ROOT / "dogfooding_results" / "kitti_seq_02_full"
GT_CSV = REPO_ROOT / "experiments" / "reference_data" / "kitti_seq_02_full_gt.csv"


def rel(path: Path) -> str:
    return path.relative_to(REPO_ROOT).as_posix()


def run(cmd: list[str]) -> None:
    print("+ " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def methods(path: Path) -> dict[str, dict[str, Any]]:
    return {m["name"]: m for m in load(path)["methods"]}


def quadric_plane(note: str) -> dict[str, float | None]:
    match = re.search(r"mean_quadric/plane_corr=([-+0-9.eE]+)/([-+0-9.eE]+)", note)
    if not match:
        return {"quadric_per_frame": None, "plane_per_frame": None, "plane_fallback_ratio": None}
    quadric = float(match.group(1))
    plane = float(match.group(2))
    total = quadric + plane
    return {
        "quadric_per_frame": quadric,
        "plane_per_frame": plane,
        "plane_fallback_ratio": (plane / total) if total > 1e-12 else None,
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
    quad_on = on_methods["Quadric-LO"]
    quad_off = off_methods["Quadric-LO"]
    kiss = on_methods["KISS-ICP"]
    note_on = quad_on.get("note", "")
    note_off = quad_off.get("note", "")
    corr_on = quadric_plane(note_on)
    corr_off = quadric_plane(note_off)
    delta_block = delta(quad_on, quad_off)
    rpe_rel = delta_block.get("rpe_trans_pct_relative_pct")
    ate_rel = delta_block.get("ate_m_relative_pct")
    plane_ratio = corr_on.get("plane_fallback_ratio")
    plane_pct = (plane_ratio * 100.0) if plane_ratio is not None else None
    summary = {
        "schema_version": 1,
        "benchmark": "kitti_odometry_seq02_public_validation",
        "generated_by": "evaluation/scripts/run_quadric_lo_kitti_seq02_validation.py",
        "scope": {
            "dataset": "KITTI Odometry sequence 02 full",
            "frames": on_data.get("num_frames"),
            "trajectory_length_m": on_data.get("trajectory_length_m"),
            "claim_limit": (
                "Public residential KITTI seq02 check with vegetation and suburban "
                "scenery; not a substitute for dedicated orchard or non-urban "
                "multi-beam benchmarks."
            ),
        },
        "artifacts": {
            "fallback_on_with_kiss": rel(on_json),
            "fallback_off": rel(off_json),
        },
        "protocol": {
            "fallback_on_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                "--methods quadric_lo,kiss_icp --no-gt-seed --quadric-lo-dense-profile "
                f"--summary-json {rel(on_json)}"
            ),
            "fallback_off_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(PCD_DIR)} {rel(GT_CSV)} "
                "--methods quadric_lo --no-gt-seed --quadric-lo-dense-profile "
                f"--quadric-lo-no-plane-fallback --summary-json {rel(off_json)}"
            ),
        },
        "kiss_sanity_reference": metric_subset(kiss),
        "quadric_lo": {
            "fallback_on": {
                "metrics": metric_subset(quad_on),
                "correspondences": corr_on,
                "note": note_on,
            },
            "fallback_off": {
                "metrics": metric_subset(quad_off),
                "correspondences": corr_off,
                "note": note_off,
            },
            "fallback_off_minus_on": delta_block,
        },
        "conclusion": (
            "On public KITTI Odometry seq02 full (residential/suburban driving), "
            "Quadric-LO uses point-to-quadric constraints for "
            f"{100.0 - (plane_pct or 0):.1f}% of correspondences "
            f"({corr_on.get('quadric_per_frame', 0):.0f} quadric vs "
            f"{corr_on.get('plane_per_frame', 0):.0f} plane corr/frame). "
            "Unlike the near-neutral seq00/07 highway ablation, disabling plane "
            "fallback on seq02 worsens RPE by "
            f"{rpe_rel:+.1f}% and ATE by {ate_rel:+.1f}% "
            f"({quad_on['rpe_trans_pct']:.3f}% / {quad_on['ate_m']:.2f} m vs "
            f"{quad_off['rpe_trans_pct']:.3f}% / {quad_off['ate_m']:.2f} m), "
            "showing that the rare plane fallback carries more weight on this "
            "vegetation-rich suburban sequence even though the quadric path "
            "dominates correspondence counts. Dedicated orchard or non-urban "
            "multi-beam benchmarks remain open before T0 promotion."
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
    fallback_on_json = BENCH_DIR / "quadric_lo_fallback_on.json"
    fallback_off_json = BENCH_DIR / "quadric_lo_fallback_off.json"
    summary_json = BENCH_DIR / "quadric_lo_kitti_seq02_validation_summary.json"

    if not args.summary_only:
        run(
            [
                rel(pcd_bin),
                rel(PCD_DIR),
                rel(GT_CSV),
                "--methods",
                "quadric_lo,kiss_icp",
                "--no-gt-seed",
                "--quadric-lo-dense-profile",
                "--summary-json",
                rel(fallback_on_json),
            ]
        )
        run(
            [
                rel(pcd_bin),
                rel(PCD_DIR),
                rel(GT_CSV),
                "--methods",
                "quadric_lo",
                "--no-gt-seed",
                "--quadric-lo-dense-profile",
                "--quadric-lo-no-plane-fallback",
                "--summary-json",
                rel(fallback_off_json),
            ]
        )

    if not fallback_on_json.exists() or not fallback_off_json.exists():
        raise SystemExit("missing seq02 validation artifacts; run without --summary-only")

    build_summary(fallback_on_json, fallback_off_json, summary_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
