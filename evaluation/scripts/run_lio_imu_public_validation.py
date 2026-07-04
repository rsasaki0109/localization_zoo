#!/usr/bin/env python3
"""Run synchronized LiDAR-IMU public validation (IMU-on vs no-imu.csv fallback)."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_BIN = REPO_ROOT / "build" / "evaluation" / "pcd_dogfooding"
BENCH_DIR = REPO_ROOT / "docs" / "benchmarks" / "lio_imu_public"

DATASETS: dict[str, dict[str, Any]] = {
    "hdl_400": {
        "label": "HDL-400 open LiDAR-IMU 120-frame window",
        "pcd_dir": REPO_ROOT / "dogfooding_results" / "hdl_400_open_ct_lio_120_b",
        "gt_csv": REPO_ROOT
        / "experiments"
        / "reference_data"
        / "hdl_400_public_reference_b.csv",
        "frames": 120,
        "association_mode": "legacy-auto",
        "claim_limit": (
            "Synchronized LiDAR-IMU odometry check on a public HDL-400 open "
            "sequence; validates IMU-gated compensation / prior paths but does not "
            "by itself promote methods to full LIO T0 without broader coverage."
        ),
    },
    "nclt_2013_01_10_120": {
        "label": "NCLT 2013-01-10 120-frame window (Velodyne + MS25 IMU)",
        "pcd_dir": REPO_ROOT / "dogfooding_results" / "nclt_2013_01_10_120",
        "gt_csv": REPO_ROOT
        / "experiments"
        / "reference_data"
        / "nclt_2013_01_10_120_gt.csv",
        "frames": 120,
        "association_mode": "strict",
        "claim_limit": (
            "Synchronized LiDAR-IMU odometry check on public NCLT velodyne_sync "
            "+ MS25 IMU; supports IMU mechanism evidence but does not by itself "
            "promote methods to full LIO T0 without broader coverage."
        ),
    },
}

IMU_METHODS = ["d2lio", "degen_sense", "id_lio", "rf_lio", "dlio", "kiss_icp"]
DENSE_PROFILE_FLAGS = ["--id-lio-dense-profile", "--rf-lio-dense-profile"]


def rel(path: Path) -> str:
    return path.relative_to(REPO_ROOT).as_posix()


def run(cmd: list[str]) -> None:
    print("+ " + " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def methods(path: Path) -> dict[str, dict[str, Any]]:
    return {m["name"]: m for m in load(path)["methods"]}


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


def imu_path_active(note: str, *, imu_csv_present: bool) -> bool:
    if not imu_csv_present:
        return False
    lowered = note.lower()
    if "no imu.csv" in lowered:
        return False
    if "imu gyro prior" in lowered:
        return True
    if "lidar-imu" in lowered:
        return True
    if "imu/lidar fusion" in lowered:
        return True
    if "imu preintegration" in lowered:
        return True
    if "imu prior" in lowered:
        return True
    return False


def imu_sample_count(pcd_dir: Path) -> int:
    imu_csv = pcd_dir / "imu.csv"
    if not imu_csv.is_file():
        return 0
    with imu_csv.open() as handle:
        return max(sum(1 for _ in handle) - 1, 0)


def make_no_imu_tree(source_dir: Path) -> Path:
    temp_root = Path(tempfile.mkdtemp(prefix="lio_imu_no_imu_"))
    for child in sorted(source_dir.iterdir()):
        if child.name.isdigit() and len(child.name) == 8:
            (temp_root / child.name).symlink_to(child.resolve())
    for name in ("frame_timestamps.csv",):
        src = source_dir / name
        if src.is_file():
            shutil.copy2(src, temp_root / name)
    return temp_root


def build_conclusion(
    dataset_key: str,
    cfg: dict[str, Any],
    on_data: dict[str, Any],
    pcd_dir: Path,
    d2: dict[str, Any],
    degen: dict[str, Any],
    idlio: dict[str, Any],
    rflio: dict[str, Any],
    kiss: dict[str, Any],
) -> str:
    frames = on_data.get("num_frames")
    imu_n = imu_sample_count(pcd_dir)
    base = (
        f"On {cfg['label']} ({frames} frames, {imu_n} IMU samples), IMU-gated paths "
        f"activate for D2-LIO ({d2['imu_on']['imu_path_active']}), DegenSense "
        f"({degen['imu_on']['imu_path_active']}), ID-LIO "
        f"({idlio['imu_on']['imu_path_active']}), and RF-LIO "
        f"({rflio['imu_on']['imu_path_active']}). "
    )
    if dataset_key == "nclt_2013_01_10_120":
        degen_ate_delta = degen["imu_off_minus_on"].get("ate_m_relative_pct")
        return (
            base
            + f"DegenSense IMU compensation lowers ATE from "
            f"{degen['imu_off']['metrics']['ate_m']:.2f} m to "
            f"{degen['imu_on']['metrics']['ate_m']:.2f} m "
            f"(~{abs(degen_ate_delta or 0):.0f}% worse without IMU); D2-LIO RPE "
            f"{d2['imu_on']['metrics']['rpe_trans_pct']:.2f}% vs "
            f"{d2['imu_off']['metrics']['rpe_trans_pct']:.2f}% without. "
            f"KISS-ICP sanity is poor on this window ({kiss['rpe_trans_pct']:.0f}% RPE) "
            "so the row is mechanism evidence, not a leaderboard claim. "
            "LiDAR-IBA IMU is not wired in pcd_dogfooding yet."
        )
    return (
        base
        + f"D2-LIO RPE {d2['imu_on']['metrics']['rpe_trans_pct']:.3f}% with IMU vs "
        f"{d2['imu_off']['metrics']['rpe_trans_pct']:.3f}% without; DegenSense "
        f"{degen['imu_on']['metrics']['rpe_trans_pct']:.3f}% vs "
        f"{degen['imu_off']['metrics']['rpe_trans_pct']:.3f}%. "
        "LiDAR-IBA IMU residuals are not wired in pcd_dogfooding yet — "
        "BA-on/off KITTI ablation remains the committed IMU-free evidence. "
        "This closes the synchronized LiDAR-IMU validation gap for compensation "
        "and prior paths at T1 mechanism level, not full manuscript LIO claims."
    )


def build_summary(
    dataset_key: str,
    imu_on_json: Path,
    imu_off_json: Path,
    output: Path,
) -> None:
    cfg = DATASETS[dataset_key]
    on_data = load(imu_on_json)
    off_data = load(imu_off_json)
    on_methods = methods(imu_on_json)
    off_methods = methods(imu_off_json)
    kiss = on_methods["KISS-ICP"]

    tracked = [
        ("D2-LIO", "D2-LIO"),
        ("DegenSense", "DegenSense"),
        ("ID-LIO", "ID-LIO"),
        ("RF-LIO", "RF-LIO"),
        ("DLIO", "DLIO"),
    ]
    method_blocks: dict[str, Any] = {}
    for key, name in tracked:
        on_m = on_methods[name]
        off_m = off_methods[name]
        method_blocks[key] = {
            "imu_on": {
                "metrics": metric_subset(on_m),
                "imu_path_active": imu_path_active(on_m.get("note", ""), imu_csv_present=True),
                "note": on_m.get("note", ""),
            },
            "imu_off": {
                "metrics": metric_subset(off_m),
                "imu_path_active": imu_path_active(off_m.get("note", ""), imu_csv_present=False),
                "note": off_m.get("note", ""),
            },
            "imu_off_minus_on": delta(on_m, off_m),
        }

    d2 = method_blocks["D2-LIO"]
    degen = method_blocks["DegenSense"]
    idlio = method_blocks["ID-LIO"]
    rflio = method_blocks["RF-LIO"]

    summary = {
        "schema_version": 1,
        "benchmark": "lio_imu_public_validation",
        "generated_by": "evaluation/scripts/run_lio_imu_public_validation.py",
        "scope": {
            "dataset": cfg["label"],
            "dataset_key": dataset_key,
            "frames": on_data.get("num_frames"),
            "trajectory_length_m": on_data.get("trajectory_length_m"),
            "imu_samples": imu_sample_count(cfg["pcd_dir"]),
            "claim_limit": cfg["claim_limit"],
        },
        "artifacts": {
            "imu_on": rel(imu_on_json),
            "imu_off": rel(imu_off_json),
        },
        "protocol": {
            "seed": "first-pose anchor, --no-gt-seed velocity prediction after frame 0",
            "association_mode": cfg["association_mode"],
            "imu_on_command": (
                "./build/evaluation/pcd_dogfooding "
                f"{rel(cfg['pcd_dir'])} {rel(cfg['gt_csv'])} {cfg['frames']} "
                f"--methods {','.join(IMU_METHODS)} --no-gt-seed "
                f"{' '.join(DENSE_PROFILE_FLAGS)} "
                f"--association-mode {cfg['association_mode']} "
                f"--summary-json {rel(imu_on_json)}"
            ),
            "imu_off_command": (
                "Run the same command on a temporary PCD tree without imu.csv "
                f"(see runner); output -> {rel(imu_off_json)}"
            ),
        },
        "kiss_sanity_reference": metric_subset(kiss),
        "methods": method_blocks,
        "conclusion": build_conclusion(
            dataset_key,
            cfg,
            on_data,
            cfg["pcd_dir"],
            method_blocks["D2-LIO"],
            method_blocks["DegenSense"],
            method_blocks["ID-LIO"],
            method_blocks["RF-LIO"],
            metric_subset(kiss),
        ),
    }
    output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"wrote {rel(output)}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dataset",
        choices=sorted(DATASETS),
        default="hdl_400",
        help="Prepared dogfooding tree with imu.csv (default: hdl_400).",
    )
    parser.add_argument("--pcd-bin", default=str(DEFAULT_BIN))
    parser.add_argument("--summary-only", action="store_true")
    args = parser.parse_args()

    cfg = DATASETS[args.dataset]
    pcd_dir = cfg["pcd_dir"]
    gt_csv = cfg["gt_csv"]
    frames = cfg["frames"]

    pcd_bin = Path(args.pcd_bin)
    if not pcd_bin.is_absolute():
        pcd_bin = REPO_ROOT / pcd_bin
    if not args.summary_only and not pcd_bin.exists():
        raise SystemExit(f"missing pcd_dogfooding binary: {rel(pcd_bin)}")
    if not pcd_dir.is_dir():
        raise SystemExit(f"missing PCD tree: {rel(pcd_dir)}")
    if not gt_csv.is_file():
        raise SystemExit(f"missing GT CSV: {rel(gt_csv)}")
    if not (pcd_dir / "imu.csv").is_file():
        raise SystemExit(f"missing imu.csv under {rel(pcd_dir)}")

    BENCH_DIR.mkdir(parents=True, exist_ok=True)
    prefix = args.dataset
    imu_on_json = BENCH_DIR / f"{prefix}_imu_on.json"
    imu_off_json = BENCH_DIR / f"{prefix}_imu_off.json"
    summary_json = BENCH_DIR / f"{prefix}_lio_imu_validation_summary.json"

    base_cmd = [
        rel(pcd_bin),
        None,
        rel(gt_csv),
        str(frames),
        "--methods",
        ",".join(IMU_METHODS),
        "--no-gt-seed",
        *DENSE_PROFILE_FLAGS,
        "--association-mode",
        cfg["association_mode"],
    ]

    if not args.summary_only:
        on_cmd = list(base_cmd)
        on_cmd[1] = rel(pcd_dir)
        on_cmd.extend(["--summary-json", rel(imu_on_json)])
        run(on_cmd)

        no_imu_dir = make_no_imu_tree(pcd_dir)
        try:
            off_cmd = list(base_cmd)
            off_cmd[1] = str(no_imu_dir.resolve())
            off_cmd.extend(["--summary-json", rel(imu_off_json)])
            run(off_cmd)
        finally:
            shutil.rmtree(no_imu_dir, ignore_errors=True)

    if not imu_on_json.exists() or not imu_off_json.exists():
        raise SystemExit("missing LIO IMU validation artifacts; run without --summary-only")

    build_summary(args.dataset, imu_on_json, imu_off_json, summary_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
