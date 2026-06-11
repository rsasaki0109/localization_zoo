#!/usr/bin/env python3
"""Build the frozen paper-ready evidence bundle from committed JSON artifacts."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUTPUT = REPO_ROOT / "docs" / "benchmarks" / "paper_ready_bundle.json"
KITTI_ROOT = "data/kitti_pcd"


METHOD_CONFIGS: list[dict[str, Any]] = [
    {
        "id": "i_loam",
        "selector": "i_loam",
        "method": "I-LOAM",
        "tier": "T0 evidence candidate",
        "mechanism": "LiDAR reflectance in LOAM correspondence search and residual weighting.",
        "paper_result": {
            "variant": "dense_mapping_on",
            "role": "main KITTI odometry row",
            "flags": ["--i-loam-dense-profile"],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_i_loam.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_i_loam.json",
            },
        },
        "ablation": {
            "summary": "docs/benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json",
            "delta_key": "delta",
            "variants": {
                "intensity_off": {
                    "role": "geometric baseline",
                    "flags": ["--i-loam-no-mapping", "--i-loam-no-intensity"],
                },
                "intensity_on": {
                    "role": "paper mechanism enabled",
                    "flags": ["--i-loam-no-mapping"],
                },
            },
        },
        "remaining_work": "Add to manuscript table with the committed scan-to-scan intensity ablation locked.",
    },
    {
        "id": "kc_lo",
        "selector": "kc_lo",
        "method": "KC-LO",
        "tier": "T0 evidence candidate",
        "mechanism": "Correspondence-free kernel correlation with optional coarse-to-fine sigma schedule.",
        "paper_result": {
            "variant": "fixed_sigma",
            "role": "main KITTI odometry row",
            "flags": ["--kc-lo-dense-profile", "--kc-lo-no-anneal"],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_kc_lo_no_anneal.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_kc_lo_no_anneal.json",
            },
        },
        "ablation": {
            "summary": "docs/benchmarks/kitti_full_new_methods/kc_lo_sigma_schedule_ablation.json",
            "delta_key": "delta_fixed_vs_annealed",
            "variants": {
                "annealed": {
                    "role": "coarse-to-fine sigma schedule",
                    "flags": ["--kc-lo-dense-profile"],
                },
                "fixed_sigma": {
                    "role": "fixed fine sigma",
                    "flags": ["--kc-lo-dense-profile", "--kc-lo-no-anneal"],
                },
            },
        },
        "remaining_work": "Add to manuscript table with fixed-sigma and annealed artifacts locked.",
    },
    {
        "id": "m_gclo",
        "selector": "m_gclo",
        "method": "M-GCLO",
        "tier": "T1+ evidence candidate",
        "mechanism": "Multiple ground point-to-plane constraints fused with non-ground NDT.",
        "paper_result": {
            "variant": "ground_on",
            "role": "paper mechanism enabled",
            "flags": ["--m-gclo-dense-profile"],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_m_gclo.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_m_gclo.json",
            },
        },
        "ablation": {
            "summary": "docs/benchmarks/kitti_full_new_methods/m_gclo_ground_factor_ablation.json",
            "delta_key": "delta_ground_off_vs_on",
            "variants": {
                "ground_on": {
                    "role": "paper mechanism enabled",
                    "flags": ["--m-gclo-dense-profile"],
                },
                "ground_off": {
                    "role": "ground factor disabled",
                    "flags": ["--m-gclo-dense-profile", "--m-gclo-no-ground"],
                },
            },
        },
        "remaining_work": "Add one non-flat dataset check before promoting to T0.",
    },
]


METRIC_KEYS = [
    "ate_m",
    "rpe_trans_pct",
    "rpe_rot_deg_per_m",
    "fps",
    "frames",
]


def rel(path: Path | str) -> str:
    path = Path(path)
    if not path.is_absolute():
        return path.as_posix()
    return path.relative_to(REPO_ROOT).as_posix()


def load_json(rel_path: str) -> dict[str, Any]:
    path = REPO_ROOT / rel_path
    if not path.exists():
        raise SystemExit(f"missing artifact: {rel_path}")
    return json.loads(path.read_text())


def method_payload(artifact_rel: str) -> dict[str, Any]:
    data = load_json(artifact_rel)
    methods = data.get("methods", [])
    if len(methods) != 1:
        raise SystemExit(f"expected one method in {artifact_rel}, found {len(methods)}")
    method = methods[0]
    if method.get("status") != "ok":
        raise SystemExit(f"artifact is not ok: {artifact_rel}")
    return method


def dataset_paths(artifact_rel: str, sequence: str) -> tuple[str, str]:
    data = load_json(artifact_rel)
    pcd_dir = str(data.get("pcd_dir", ""))
    gt_csv = str(data.get("gt_csv", ""))
    if pcd_dir.startswith("/") or gt_csv.startswith("/"):
        return f"{KITTI_ROOT}/seq{sequence}_full", f"{KITTI_ROOT}/seq{sequence}_gt.csv"
    if not pcd_dir or not gt_csv:
        raise SystemExit(f"artifact missing dataset paths: {artifact_rel}")
    return pcd_dir, gt_csv


def metrics_from_method(method: dict[str, Any]) -> dict[str, Any]:
    return {key: method.get(key) for key in METRIC_KEYS if key in method}


def assert_close(label: str, expected: Any, actual: Any, tol: float = 1e-6) -> None:
    if expected is None or actual is None:
        return
    if abs(float(expected) - float(actual)) > tol:
        raise SystemExit(f"{label} mismatch: summary={expected} raw={actual}")


def command_for(selector: str, sequence: str, flags: list[str], artifact_rel: str) -> str:
    pcd_dir, gt_csv = dataset_paths(artifact_rel, sequence)
    parts = [
        "./build/evaluation/pcd_dogfooding",
        pcd_dir,
        gt_csv,
        "--methods",
        selector,
        "--no-gt-seed",
        *flags,
        "--summary-json",
        artifact_rel,
    ]
    return " ".join(parts)


def build_method_entry(config: dict[str, Any]) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    summary_rel = config["ablation"]["summary"]
    summary = load_json(summary_rel)
    if summary.get("method") != config["method"]:
        raise SystemExit(f"method mismatch in {summary_rel}: {summary.get('method')}")

    paper_sequences: list[dict[str, Any]] = []
    table_rows: list[dict[str, Any]] = []
    paper_result = config["paper_result"]
    for sequence, artifact_rel in sorted(paper_result["artifacts"].items()):
        method = method_payload(artifact_rel)
        metrics = metrics_from_method(method)
        row = {
            "method": config["method"],
            "tier": config["tier"],
            "sequence": sequence,
            "variant": paper_result["variant"],
            "artifact": artifact_rel,
            **metrics,
        }
        table_rows.append(row)
        paper_sequences.append({
            "sequence": sequence,
            "artifact": artifact_rel,
            "metrics": metrics,
            "command": command_for(
                config["selector"], sequence, paper_result["flags"], artifact_rel
            ),
        })

    ablation_pairs: list[dict[str, Any]] = []
    variant_configs = config["ablation"]["variants"]
    delta_key = config["ablation"]["delta_key"]
    for pair in summary.get("pairs", []):
        sequence = str(pair["sequence"])
        variants: dict[str, Any] = {}
        for variant_name, variant_config in variant_configs.items():
            variant = pair[variant_name]
            artifact_rel = variant["artifact"]
            raw = method_payload(artifact_rel)
            for key in ("ate_m", "rpe_trans_pct", "rpe_rot_deg_per_m", "fps"):
                if key in variant and key in raw:
                    assert_close(f"{artifact_rel}:{key}", variant[key], raw[key])
            if "frames" in pair and "frames" in raw:
                assert_close(f"{artifact_rel}:frames", pair["frames"], raw["frames"], tol=0.0)
            variants[variant_name] = {
                "role": variant_config["role"],
                "artifact": artifact_rel,
                "metrics": metrics_from_method(raw),
                "command": command_for(
                    config["selector"], sequence, variant_config["flags"], artifact_rel
                ),
            }
        ablation_pairs.append({
            "sequence": sequence,
            "frames": pair.get("frames"),
            "variants": variants,
            "delta": pair.get(delta_key, {}),
        })

    method_entry = {
        "id": config["id"],
        "method": config["method"],
        "selector": config["selector"],
        "tier": config["tier"],
        "paper": summary.get("paper"),
        "claim": summary.get("claim"),
        "mechanism": config["mechanism"],
        "paper_result": {
            "variant": paper_result["variant"],
            "role": paper_result["role"],
            "sequences": paper_sequences,
        },
        "ablation": {
            "summary_artifact": summary_rel,
            "protocol": summary.get("protocol", {}),
            "pairs": ablation_pairs,
            "conclusion": summary.get("conclusion"),
        },
        "remaining_work": config["remaining_work"],
    }
    return method_entry, table_rows


def build_bundle() -> dict[str, Any]:
    methods: list[dict[str, Any]] = []
    table_rows: list[dict[str, Any]] = []
    for config in METHOD_CONFIGS:
        method_entry, rows = build_method_entry(config)
        methods.append(method_entry)
        table_rows.extend(rows)

    return {
        "schema_version": 1,
        "bundle_id": "paper_ready_core_2026_06_12",
        "frozen_date": "2026-06-12",
        "generated_by": "evaluation/scripts/build_paper_ready_bundle.py",
        "scope": {
            "dataset": "KITTI Odometry full sequences",
            "sequences": ["00", "07"],
            "seed": "first-pose anchor, --no-gt-seed velocity prediction after frame 0",
            "artifact_root": "docs/benchmarks/kitti_full_new_methods",
            "claim_policy": "Use only tiered T0/T1 evidence for manuscript claims; keep adapters and compact baselines outside the main claim table.",
        },
        "status": {
            "frozen_methods": len(methods),
            "paper_table_rows": len(table_rows),
            "paired_ablation_summaries": len(methods),
            "t0_evidence_candidates": ["I-LOAM", "KC-LO"],
            "t1_plus_evidence_candidates": ["M-GCLO"],
            "remaining_before_full_manuscript_table": [
                "Quadric-LO plane-fallback ablation",
                "RF-LIO/ID-LIO dynamic-object stress",
                "M-GCLO non-flat dataset check",
            ],
        },
        "paper_table_rows": table_rows,
        "methods": methods,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT), help="Manifest output path")
    parser.add_argument("--check", action="store_true", help="Fail if the output file is stale")
    args = parser.parse_args()

    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = REPO_ROOT / output_path

    bundle = build_bundle()
    text = json.dumps(bundle, indent=2, sort_keys=False) + "\n"

    if args.check:
        if not output_path.exists():
            print(f"missing bundle: {rel(output_path)}", file=sys.stderr)
            return 1
        current = output_path.read_text()
        if current != text:
            print(f"stale bundle: {rel(output_path)}", file=sys.stderr)
            return 1
        print(f"paper-ready bundle valid: {rel(output_path)}")
        return 0

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(text)
    print(f"wrote {rel(output_path)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
