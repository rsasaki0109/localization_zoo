#!/usr/bin/env python3

"""Generate docs/variant_analysis.md: why do variants differ across datasets?

Analyzes experiment results to produce:
1. GT-seeded vs no-GT-seed comparison table
2. Per-method cross-dataset default stability analysis
3. Variant profile impact analysis (what CLI flags correlate with ATE/FPS changes)
"""

from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
DOCS_DIR = REPO_ROOT / "docs"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def fmt(val: float | None, digits: int = 3) -> str:
    if val is None:
        return "-"
    return f"{val:.{digits}f}"


def classify_dataset(pcd_dir: str) -> str:
    if "istanbul" in pcd_dir:
        return "Istanbul"
    if "hdl_400" in pcd_dir:
        return "HDL-400"
    if "mcd_tuhh" in pcd_dir:
        return "MCD-TUHH"
    if "mcd_ntu" in pcd_dir:
        return "MCD-NTU"
    if "mcd_kth" in pcd_dir:
        return "MCD-KTH"
    if "kitti_raw" in pcd_dir:
        parts = pcd_dir.split("/")[-1]
        return f"KITTI-{parts}"
    return pcd_dir


def main() -> int:
    index_path = RESULTS_DIR / "index.json"
    if not index_path.exists():
        print("[skip] No index.json")
        return 1

    index = load_json(index_path)
    lines: list[str] = []
    ts = datetime.now(timezone.utc).isoformat(timespec="seconds")
    lines.append("# Variant Analysis\n")
    lines.append(f"> Generated: {ts}\n")
    lines.append(
        "This document analyzes **why** variant performance differs across datasets "
        "and initialization modes. It complements `decisions.md` (which records **what** "
        "was chosen) with **why** the choices diverge.\n"
    )

    # --- Section 1: GT-seeded vs No-GT-seed ---
    lines.append("## 1. GT-Seeded vs Odometry-Chain Initialization\n")
    lines.append(
        "Scan-to-map methods (LiTAMIN2, GICP, NDT) can use GT poses as per-frame "
        "initialization or rely on their own odometry chain. KISS-ICP and CT-ICP are "
        "always odometry-based.\n"
    )

    # Collect GT vs nogt pairs
    gt_results: dict[str, dict] = {}  # selector -> {gt: {...}, nogt: {...}}
    for entry in index.get("problems", []):
        if entry.get("status") != "ready":
            continue
        agg_path = REPO_ROOT / entry["aggregate_path"]
        if not agg_path.exists():
            continue
        agg = load_json(agg_path)
        pid = agg.get("problem", {}).get("id", "")
        selector = agg.get("stable_interface", {}).get("methods", "")

        is_nogt = "nogt" in pid
        is_same_data = "0009" in pid  # compare on same dataset

        if not is_same_data:
            continue

        for v in agg.get("variants", []):
            if not v.get("decision", "").startswith("Adopt"):
                continue
            key = selector
            mode = "nogt" if is_nogt else "gt_200" if "200" in pid and "full" not in pid else "gt_full"
            gt_results.setdefault(key, {})[mode] = {
                "ate": v.get("ate_m"),
                "fps": v.get("fps"),
                "variant": v.get("id", ""),
                "note": v.get("note", ""),
            }

    if gt_results:
        lines.append("### Drive 0009 Comparison\n")
        lines.append("| Method | Mode | Default Variant | ATE [m] | FPS |")
        lines.append("|---|---|---|---:|---:|")
        for selector in sorted(gt_results.keys()):
            modes = gt_results[selector]
            for mode in ["gt_200", "nogt", "gt_full"]:
                if mode in modes:
                    d = modes[mode]
                    label = {"gt_200": "GT-seeded 200f", "nogt": "No GT seed 200f", "gt_full": "GT-seeded full"}[mode]
                    lines.append(
                        f"| {selector} | {label} | {d['variant']} | {fmt(d['ate'])} | {fmt(d['fps'], 1)} |"
                    )
        lines.append("")
        lines.append(
            "**Key finding**: Scan-to-map methods with GT seeding maintain sub-meter accuracy, "
            "but LiTAMIN2 and NDT diverge catastrophically without GT seeds. "
            "GICP is more robust to initialization. KISS-ICP and CT-ICP are unaffected "
            "(already pure odometry).\n"
        )

    # --- Section 2: Cross-dataset default stability ---
    lines.append("## 2. Cross-Dataset Default Stability\n")
    lines.append(
        "Does the same variant win across all datasets? Instability here is the "
        "core evidence for the variant-first thesis.\n"
    )

    # Collect defaults per selector per dataset
    defaults_by_selector: dict[str, list[dict]] = {}
    for entry in index.get("problems", []):
        if entry.get("status") != "ready":
            continue
        agg_path = REPO_ROOT / entry["aggregate_path"]
        if not agg_path.exists():
            continue
        agg = load_json(agg_path)
        selector = agg.get("stable_interface", {}).get("methods", "")
        dataset = classify_dataset(agg.get("dataset", {}).get("pcd_dir", ""))
        pid = agg.get("problem", {}).get("id", "")
        if "nogt" in pid:
            continue  # skip nogt for stability analysis

        for v in agg.get("variants", []):
            if v.get("decision", "").startswith("Adopt"):
                defaults_by_selector.setdefault(selector, []).append({
                    "dataset": dataset,
                    "variant": v.get("id", ""),
                    "ate": v.get("ate_m"),
                    "fps": v.get("fps"),
                })

    for selector in sorted(defaults_by_selector.keys()):
        entries = defaults_by_selector[selector]
        primary = selector.upper().replace("_", "-")
        if selector == "kiss_icp": primary = "KISS-ICP"
        elif selector == "ct_icp": primary = "CT-ICP"
        elif selector == "litamin2": primary = "LiTAMIN2"

        lines.append(f"### {primary}\n")
        lines.append("| Dataset | Default Variant | ATE [m] | FPS |")
        lines.append("|---|---|---:|---:|")
        for e in sorted(entries, key=lambda x: x["dataset"]):
            lines.append(f"| {e['dataset']} | {e['variant']} | {fmt(e['ate'])} | {fmt(e['fps'], 1)} |")

        # Count unique defaults
        unique_defaults = set(e["variant"] for e in entries)
        n_datasets = len(entries)
        lines.append(f"\n**Stability**: {len(unique_defaults)} unique default(s) across {n_datasets} windows.\n")

    # --- Section 3: Profile impact ---
    lines.append("## 3. Profile Impact Summary\n")
    lines.append(
        "How do profile flags (fast/balanced/dense) affect ATE and FPS? "
        "Values averaged across all datasets where the variant ran.\n"
    )

    # Collect all variant results by (selector, variant_id)
    all_variants: dict[str, dict[str, list[dict]]] = {}  # selector -> variant_id -> [results]
    for entry in index.get("problems", []):
        if entry.get("status") != "ready":
            continue
        agg_path = REPO_ROOT / entry["aggregate_path"]
        if not agg_path.exists():
            continue
        agg = load_json(agg_path)
        selector = agg.get("stable_interface", {}).get("methods", "")
        pid = agg.get("problem", {}).get("id", "")
        if "nogt" in pid:
            continue

        for v in agg.get("variants", []):
            if v.get("status") != "OK":
                continue
            vid = v.get("id", "")
            all_variants.setdefault(selector, {}).setdefault(vid, []).append({
                "ate": v.get("ate_m"),
                "fps": v.get("fps"),
            })

    for selector in sorted(all_variants.keys()):
        primary = selector.upper().replace("_", "-")
        if selector == "kiss_icp": primary = "KISS-ICP"
        elif selector == "ct_icp": primary = "CT-ICP"
        elif selector == "litamin2": primary = "LiTAMIN2"

        lines.append(f"### {primary}\n")
        lines.append("| Variant | Avg ATE [m] | Avg FPS | N |")
        lines.append("|---|---:|---:|---:|")

        for vid in sorted(all_variants[selector].keys()):
            results = all_variants[selector][vid]
            ates = [r["ate"] for r in results if r["ate"] is not None]
            fpss = [r["fps"] for r in results if r["fps"] is not None]
            avg_ate = sum(ates) / len(ates) if ates else None
            avg_fps = sum(fpss) / len(fpss) if fpss else None
            lines.append(f"| {vid} | {fmt(avg_ate)} | {fmt(avg_fps, 1)} | {len(results)} |")
        lines.append("")

    output_path = DOCS_DIR / "variant_analysis.md"
    output_path.write_text("\n".join(lines))
    print(f"[done] wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
