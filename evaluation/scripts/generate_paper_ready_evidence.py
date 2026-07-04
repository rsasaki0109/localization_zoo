#!/usr/bin/env python3
"""Regenerate paper-ready validation evidence snippets in README.md from JSON summaries."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
README_PATH = REPO_ROOT / "README.md"
BUNDLE_PATH = REPO_ROOT / "docs" / "benchmarks" / "paper_ready_bundle.json"

SNIPPETS = {
    "EVIDENCE:M-GCLO-SEQ08": REPO_ROOT
    / "docs/benchmarks/kitti_seq08_public/m_gclo_kitti_seq08_validation_summary.json",
    "EVIDENCE:QUADRIC-SEQ02": REPO_ROOT
    / "docs/benchmarks/kitti_seq02_public/quadric_lo_kitti_seq02_validation_summary.json",
    "EVIDENCE:RF-ID-LIO-SEQ05": REPO_ROOT
    / "docs/benchmarks/kitti_seq05_public/rf_id_lio_kitti_seq05_validation_summary.json",
    "EVIDENCE:M-GCLO-MULRAN": REPO_ROOT
    / "docs/benchmarks/mulran_parkinglot_public/m_gclo_mulran_parkinglot_validation_summary.json",
    "EVIDENCE:LIO-IMU-HDL400": REPO_ROOT
    / "docs/benchmarks/lio_imu_public/hdl_400_lio_imu_validation_summary.json",
    "EVIDENCE:LIO-IMU-NCLT": REPO_ROOT
    / "docs/benchmarks/lio_imu_public/nclt_2013_01_10_120_lio_imu_validation_summary.json",
}


def load_json(path: Path) -> dict:
    return json.loads(path.read_text())


def m_gclo_seq08_block(summary: dict) -> str:
    mg = summary["m_gclo"]
    on = mg["ground_on"]["metrics"]
    delta_block = mg["ground_off_minus_on"]
    artifact = "docs/benchmarks/kitti_seq08_public/m_gclo_kitti_seq08_validation_summary.json"
    return (
        "Public hilly KITTI seq08 full shows the same anchoring pattern: ground off\n"
        f"leaves RPE ~{on['rpe_trans_pct']:.2f}% unchanged but worsens ATE by about "
        f"{abs(delta_block.get('ate_m_relative_pct', 0)):.0f}%\n"
        f"([`m_gclo_kitti_seq08_validation_summary.json`]({artifact}))."
    )


def quadric_seq02_block(summary: dict) -> str:
    delta_block = summary["quadric_lo"]["fallback_off_minus_on"]
    artifact = "docs/benchmarks/kitti_seq02_public/quadric_lo_kitti_seq02_validation_summary.json"
    return (
        "Public residential KITTI seq02 full still uses >99% quadric correspondences, but\n"
        "disabling plane fallback worsens RPE by about "
        f"{abs(delta_block.get('rpe_trans_pct_relative_pct', 0)):.0f}% and ATE by about "
        f"{abs(delta_block.get('ate_m_relative_pct', 0)):.0f}% — the rare\n"
        "fallback carries more weight on suburban/vegetation-rich driving than on highway\n"
        f"seq00/07\n([`quadric_lo_kitti_seq02_validation_summary.json`]({artifact}))."
    )


def rf_id_lio_seq05_block(summary: dict) -> str:
    kiss = summary["kiss_sanity_reference"]
    id_lio = summary["methods"]["id_lio"]["metrics"]
    rf_lio = summary["methods"]["rf_lio_default"]["metrics"]
    artifact = "docs/benchmarks/kitti_seq05_public/rf_id_lio_kitti_seq05_validation_summary.json"
    return (
        "Public urban KITTI seq05 full keeps both paths active but RF-LIO default still\n"
        f"trails ID-LIO ({rf_lio['rpe_trans_pct']:.3f}% vs {id_lio['rpe_trans_pct']:.3f}% RPE) "
        f"and KISS-ICP ({kiss['rpe_trans_pct']:.3f}%)\n"
        f"([`rf_id_lio_kitti_seq05_validation_summary.json`]({artifact}))."
    )


def m_gclo_mulran_block(summary: dict) -> str:
    kiss = summary["kiss_sanity_reference"]
    mg = summary["m_gclo"]
    on = mg["ground_on"]["metrics"]
    off = mg["ground_off"]["metrics"]
    artifact = (
        "docs/benchmarks/mulran_parkinglot_public/"
        "m_gclo_mulran_parkinglot_validation_summary.json"
    )
    return (
        "Public MulRan ParkingLot full (1176 Ouster frames) is committed, but shared "
        f"no-gt-seed odometry diverges (~{kiss['rpe_trans_pct']:.0f}% RPE for KISS-ICP and "
        f"M-GCLO); ground off changes RPE from {on['rpe_trans_pct']:.3f}% to "
        f"{off['rpe_trans_pct']:.3f}% with negligible ATE delta — inconclusive for T0 "
        f"([`m_gclo_mulran_parkinglot_validation_summary.json`]({artifact}))."
    )


def lio_imu_hdl400_block(summary: dict) -> str:
    d2 = summary["methods"]["D2-LIO"]
    degen = summary["methods"]["DegenSense"]
    artifact = "docs/benchmarks/lio_imu_public/hdl_400_lio_imu_validation_summary.json"
    return (
        "On public HDL-400 open (120 frames, 24k IMU samples), IMU-gated paths activate "
        "for D2-LIO, DegenSense, ID-LIO, and RF-LIO; RPE deltas vs no-`imu.csv` fallback "
        f"are small on this window (D2-LIO {d2['imu_on']['metrics']['rpe_trans_pct']:.2f}% "
        f"vs {d2['imu_off']['metrics']['rpe_trans_pct']:.2f}%, DegenSense "
        f"{degen['imu_on']['metrics']['rpe_trans_pct']:.2f}% vs "
        f"{degen['imu_off']['metrics']['rpe_trans_pct']:.2f}%) "
        f"([`hdl_400_lio_imu_validation_summary.json`]({artifact})). "
        "LiDAR-IBA IMU residuals are not wired in `pcd_dogfooding` yet."
    )


def lio_imu_nclt_block(summary: dict) -> str:
    degen = summary["methods"]["DegenSense"]
    kiss = summary["kiss_sanity_reference"]
    artifact = (
        "docs/benchmarks/lio_imu_public/nclt_2013_01_10_120_lio_imu_validation_summary.json"
    )
    ate_delta = degen["imu_off_minus_on"].get("ate_m_relative_pct")
    return (
        "Public NCLT 2013-01-10 (120 frames, MS25 IMU) confirms IMU-gated paths and shows "
        "DegenSense compensation lowering ATE from "
        f"{degen['imu_off']['metrics']['ate_m']:.2f} m to "
        f"{degen['imu_on']['metrics']['ate_m']:.2f} m "
        f"(~{abs(ate_delta or 0):.0f}% worse without IMU); KISS-ICP sanity is poor on this "
        f"window ({kiss['rpe_trans_pct']:.0f}% RPE) so the row is mechanism evidence only "
        f"([`nclt_2013_01_10_120_lio_imu_validation_summary.json`]({artifact}))."
    )


def render_block(name: str, summary_path: Path) -> str:
    summary = load_json(summary_path)
    if name == "EVIDENCE:M-GCLO-SEQ08":
        return m_gclo_seq08_block(summary)
    if name == "EVIDENCE:QUADRIC-SEQ02":
        return quadric_seq02_block(summary)
    if name == "EVIDENCE:RF-ID-LIO-SEQ05":
        return rf_id_lio_seq05_block(summary)
    if name == "EVIDENCE:M-GCLO-MULRAN":
        return m_gclo_mulran_block(summary)
    if name == "EVIDENCE:LIO-IMU-HDL400":
        return lio_imu_hdl400_block(summary)
    if name == "EVIDENCE:LIO-IMU-NCLT":
        return lio_imu_nclt_block(summary)
    raise KeyError(name)


def replace_snippet(text: str, name: str, block: str) -> str:
    pattern = re.compile(
        rf"(<!-- {re.escape(name)}:START -->)(.*?)(<!-- {re.escape(name)}:END -->)",
        re.DOTALL,
    )
    match = pattern.search(text)
    if not match:
        raise SystemExit(f"missing markers for {name} in README.md")
    replacement = f"{match.group(1)}\n{block}\n{match.group(3)}"
    return text[: match.start()] + replacement + text[match.end() :]


def build_readme(text: str) -> str:
    updated = text
    for name, path in SNIPPETS.items():
        if not path.is_file():
            if name == "EVIDENCE:M-GCLO-MULRAN":
                continue
            raise SystemExit(f"missing summary JSON: {path.relative_to(REPO_ROOT)}")
        updated = replace_snippet(updated, name, render_block(name, path))
    return updated


def extract_snippet(text: str, name: str) -> str:
    pattern = re.compile(
        rf"<!-- {re.escape(name)}:START -->\n(.*?)<!-- {re.escape(name)}:END -->",
        re.DOTALL,
    )
    match = pattern.search(text)
    if not match:
        raise SystemExit(f"missing markers for {name} in README.md")
    return match.group(1).strip("\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--print", action="store_true", help="Print generated snippets only.")
    parser.add_argument("--check", action="store_true", help="Exit 1 if README snippets are stale.")
    args = parser.parse_args()

    current = README_PATH.read_text()
    updated = build_readme(current)

    if args.print:
        for name in SNIPPETS:
            if not SNIPPETS[name].is_file():
                continue
            print(f"=== {name} ===")
            print(render_block(name, SNIPPETS[name]))
            print()
        return 0

    if args.check:
        for name, path in SNIPPETS.items():
            if not path.is_file():
                continue
            expected = render_block(name, path)
            actual = extract_snippet(current, name)
            if actual != expected:
                print(f"README snippet {name} is stale.", file=sys.stderr)
                return 1
        return 0

    README_PATH.write_text(updated)
    print(f"updated {README_PATH.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
