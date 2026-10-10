#!/usr/bin/env python3
"""Summarize the KITTI elevation-correction study.

KITTI's Velodyne HDL-64E has a known intrinsic error that upstream KISS-ICP and
CT-ICP undo in their KITTI loaders by rotating every point up by 0.205 deg. The
repository's KITTI runs used raw scans. This script pairs every
``*_kitti_seq_NN_full_elevation_matrix.json`` aggregate variant with its
``_elevation`` twin, adds the official KISS-ICP runs, and writes:

  - experiments/results/kitti_elevation_correction.json
  - docs/kitti_elevation_correction.md

Official KISS-ICP runs come from run_official_kiss_baseline.py. Record them
once with ``ingest-official`` (local paths are dropped); ``report`` then works
from committed files only.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
OFFICIAL_PATH = RESULTS_DIR / "official_kiss_icp_kitti_elevation.json"
REPORT_JSON = RESULTS_DIR / "kitti_elevation_correction.json"
REPORT_MD = REPO_ROOT / "docs" / "kitti_elevation_correction.md"
PAPER_NUMBERS = REPO_ROOT / "evaluation" / "data" / "paper_reported_numbers.json"
AGGREGATE = re.compile(r"(?P<method>.+)_kitti_seq_(?P<seq>\d{2})_full_elevation_matrix\.json")
SUFFIX = "_elevation"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    ingest = sub.add_parser("ingest-official", help="Record an official KISS-ICP run.")
    ingest.add_argument("--manifest", type=Path, required=True,
                        help="zoo_manifest.json written by run_official_kiss_baseline.py")
    ingest.add_argument("--sequence", required=True)
    sub.add_parser("report", help="Write the JSON summary and Markdown report.")
    return parser.parse_args()


def ingest_official(manifest_path: Path, sequence: str) -> None:
    manifest = json.loads(manifest_path.read_text())
    corrected = manifest["configuration"]["kitti_elevation_correction"] != "none"
    metrics = manifest["metrics"]
    entry = {
        "sequence": sequence.zfill(2),
        "elevation_correction": corrected,
        "kiss_icp_version": manifest["versions"]["kiss_icp"],
        "config_sha256": manifest["configuration"]["config_sha256"],
        "estimate_sha256": manifest["artifacts"]["estimate_sha256"],
        "reference_sha256": manifest["artifacts"]["reference_sha256"],
        "max_threads": next(
            (int(manifest["command"][i + 1]) for i, arg in enumerate(manifest["command"])
             if arg == "--max-threads"), None),
        "metrics": {key: metrics.get(key) for key in (
            "frames", "ate_m", "rpe_trans_pct", "kitti_rte_trans_pct",
            "kitti_rte_rot_deg_per_100m", "kitti_rte_segments", "algorithm_fps")},
    }
    payload = {"schema_version": 1, "runs": []}
    if OFFICIAL_PATH.is_file():
        payload = json.loads(OFFICIAL_PATH.read_text())
    payload["runs"] = [
        run for run in payload["runs"]
        if (run["sequence"], run["elevation_correction"])
        != (entry["sequence"], entry["elevation_correction"])
    ] + [entry]
    payload["runs"].sort(key=lambda run: (run["sequence"], run["elevation_correction"]))
    OFFICIAL_PATH.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"recorded official KISS-ICP seq {entry['sequence']} "
          f"correction={corrected}: RTE {entry['metrics']['kitti_rte_trans_pct']:.3f} %")


def paper_values() -> dict[tuple[str, str], float]:
    methods = json.loads(PAPER_NUMBERS.read_text())["methods"]
    values = {}
    for selector, info in methods.items():
        for key, value in info.get("reported_values", {}).items():
            match = re.fullmatch(r"kitti_(\d{2})", key)
            if match:
                values[(selector, match.group(1))] = float(value)
    return values


def collect_pairs() -> list[dict]:
    papers = paper_values()
    rows = []
    for path in sorted(RESULTS_DIR.glob("*_elevation_matrix.json")):
        match = AGGREGATE.fullmatch(path.name)
        if not match:
            continue
        aggregate = json.loads(path.read_text())
        selector = aggregate["stable_interface"]["methods"]
        sequence = match.group("seq")
        variants = {v["id"]: v for v in aggregate["variants"] if v.get("status") == "ok"}
        for vid, raw in variants.items():
            corrected = variants.get(vid + SUFFIX)
            if vid.endswith(SUFFIX) or corrected is None:
                continue
            raw_rte = raw.get("kitti_rte_trans_pct")
            cor_rte = corrected.get("kitti_rte_trans_pct")
            rows.append({
                "method": aggregate["stable_interface"]["primary_method"],
                "selector": selector,
                "sequence": sequence,
                "variant": vid,
                "corrected_design_style": corrected.get("design_style"),
                "paper_rte_pct": papers.get((selector, sequence)),
                "raw": {k: raw.get(k) for k in ("kitti_rte_trans_pct", "kitti_rte_rot_deg_per_100m",
                                                 "rpe_trans_pct", "ate_m")},
                "corrected": {k: corrected.get(k) for k in ("kitti_rte_trans_pct",
                                                             "kitti_rte_rot_deg_per_100m",
                                                             "rpe_trans_pct", "ate_m")},
                "rte_change_pct": (100.0 * (cor_rte - raw_rte) / raw_rte
                                   if raw_rte and cor_rte is not None else None),
                "aggregate": f"experiments/results/{path.name}",
                "host_cpu": (raw.get("host") or {}).get("cpu"),
            })
    rows.sort(key=lambda r: (r["sequence"], r["selector"], r["variant"]))
    return rows


def fmt(value: float | None, digits: int = 3) -> str:
    return "-" if value is None else f"{value:.{digits}f}"


def render(rows: list[dict], official: list[dict]) -> str:
    lines = [
        "# KITTI elevation correction",
        "",
        "_Generated by `evaluation/scripts/summarize_kitti_elevation_correction.py`._",
        "",
        "The Velodyne HDL-64E used by KITTI has an intrinsic calibration error: points "
        "appear about 0.2 deg too low. Upstream KISS-ICP (`_correct_kitti_scan`) and CT-ICP "
        "(`KITTI_GLOBAL_VERTICAL_ANGLE_OFFSET` in `dataset.cpp`) rotate every point up by "
        "0.205 deg in their KITTI loaders, so their published KITTI pipelines run on corrected "
        "scans (the CT-ICP paper's Table I row is labelled KITTI-corrected). "
        "The repository's KITTI Odometry runs used raw scans. "
        "`pcd_dogfooding --input-vertical-angle-correction-deg 0.205` now applies the same "
        "correction to every method before downsampling; it is off by default, so stored "
        "results do not change.",
        "",
        "All errors are the official KITTI translational RTE (100-800 m segments) in percent. "
        "Each variant is the Table 6 variant for that sequence, tuned on raw scans; its "
        "corrected twin changes nothing else.",
        "",
        "## Official KISS-ICP",
        "",
        "| Sequence | Raw scans | Corrected (upstream default) | Change | Rotation raw / corrected [deg/100 m] | Paper |",
        "|---|---:|---:|---:|---|---:|",
    ]
    papers = paper_values()
    by_seq: dict[str, dict[bool, dict]] = {}
    for run in official:
        by_seq.setdefault(run["sequence"], {})[run["elevation_correction"]] = run
    for seq, pair in sorted(by_seq.items()):
        raw, cor = pair.get(False), pair.get(True)
        r = raw["metrics"]["kitti_rte_trans_pct"] if raw else None
        c = cor["metrics"]["kitti_rte_trans_pct"] if cor else None
        change = f"{100 * (c - r) / r:+.0f} %" if r and c is not None else "-"
        rot = (f"{fmt(raw['metrics']['kitti_rte_rot_deg_per_100m'])} / "
               f"{fmt(cor['metrics']['kitti_rte_rot_deg_per_100m'])}" if raw and cor else "-")
        lines.append(f"| {seq} | {fmt(r)} | {fmt(c)} | {change} | {rot} | "
                     f"{fmt(papers.get(('kiss_icp', seq)), 2)} |")
    version = official[0]["kiss_icp_version"] if official else "?"
    lines += [
        "",
        f"kiss-icp {version} from PyPI with its default configuration, run by "
        "`run_official_kiss_baseline.py` (`--no-kitti-elevation-correction` for the raw "
        "column). Ground truth is opened only after the odometry process exits.",
        "",
        "## Repository methods",
        "",
        "| Seq | Method | Variant | Raw | Corrected | Change | Paper | Corrected variant in Table 6 |",
        "|---|---|---|---:|---:|---:|---:|---|",
    ]
    for row in rows:
        change = row["rte_change_pct"]
        in_table = "yes" if row["corrected_design_style"] == "paper_input" else "no (ablation)"
        lines.append(
            f"| {row['sequence']} | {row['method']} | `{row['variant']}` | "
            f"{fmt(row['raw']['kitti_rte_trans_pct'])} | "
            f"{fmt(row['corrected']['kitti_rte_trans_pct'])} | "
            f"{'-' if change is None else f'{change:+.0f} %'} | "
            f"{fmt(row['paper_rte_pct'], 2)} | {in_table} |"
        )
    hosts = sorted({row["host_cpu"] for row in rows if row.get("host_cpu")})
    lines += [
        "",
        "Raw and corrected runs of a pair ran on the same host (" + "; ".join(hosts) + "), so "
        "each change is a paired comparison. Absolute values can differ from the stored Table 6 "
        "runs on other hosts: CT-ICP's `arch_tuned_all_combined` on seq 00 reproduces bit for bit "
        "on one host but gives 3.446 % here against the stored 1.664 % from an i5-1145G7 with the "
        "same source and command (seq 07 also moves, 1.067 % to 1.060 %). The KISS-ICP, LiTAMIN2, "
        "SuMa, and LF-GICP raw runs reproduce their stored values bit for bit.",
        "",
        "Corrected variants enter Table 6 only where the published pipeline is known to use "
        "corrected scans: KISS-ICP (upstream KITTI loader) and CT-ICP (loader and paper). For the "
        "others they are ablations: LF-GICP's paper states raw scans, and the LiTAMIN2 and SuMa "
        "preprocessing has not been checked against their papers. The README leaderboard "
        "and the KITTI 07 Pareto figure rank raw-scan runs only, so every method there sees the "
        "same input.",
        "",
        "Reproduce (needs the KITTI scans under `dogfooding_results/kitti_seq_NN_full`):",
        "",
        "```bash",
        "python3 evaluation/scripts/run_experiment_matrix.py --merge-existing-index \\",
        "  --manifest experiments/kiss_icp_kitti_seq_07_full_elevation_matrix.json  # etc.",
        "python3 evaluation/scripts/summarize_kitti_elevation_correction.py report",
        "```",
        "",
    ]
    return "\n".join(lines)


def report() -> None:
    rows = collect_pairs()
    official = json.loads(OFFICIAL_PATH.read_text())["runs"] if OFFICIAL_PATH.is_file() else []
    REPORT_JSON.write_text(json.dumps({
        "schema_version": 1,
        "generated_by": "evaluation/scripts/summarize_kitti_elevation_correction.py",
        "correction_deg": 0.205,
        "metric": "official KITTI translational RTE [%] (100-800 m)",
        "official_kiss_icp": official,
        "pairs": rows,
    }, indent=2) + "\n")
    REPORT_MD.write_text(render(rows, official))
    print(REPORT_MD.read_text())


def main() -> int:
    args = parse_args()
    if args.command == "ingest-official":
        ingest_official(args.manifest, args.sequence)
    else:
        report()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
