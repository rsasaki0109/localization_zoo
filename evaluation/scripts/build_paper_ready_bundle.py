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
FROZEN_DATE = "2026-07-03"
BUNDLE_ID = "paper_ready_core_2026_07_03"


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
        "id": "degen_sense",
        "selector": "degen_sense",
        "method": "DegenSense",
        "tier": "T1 evidence candidate",
        "paper": "A Real-time Degeneracy Sensing and Compensation Method for Enhanced LiDAR SLAM",
        "claim": "Condition-number degeneracy sensing is stable on KITTI; compensation is gated to real IMU packets.",
        "mechanism": "Degeneracy-factor sensing with IMU/LiDAR compensation disabled on no-IMU KITTI so the constant-velocity fallback is diagnostic only.",
        "paper_result": {
            "variant": "no_imu_diagnostics_only",
            "role": "competitive no-IMU KITTI fallback",
            "flags": [],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_degen_sense.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_degen_sense.json",
            },
        },
        "remaining_work": "Synchronized LiDAR-IMU validation committed on HDL-400 open; add to manuscript table with KITTI no-IMU fallback + IMU mechanism rows locked.",
        "no_ablation_claim_limit": "Competitive KITTI fallback only; full LIO mechanism requires synchronized IMU validation.",
    },
    {
        "id": "d2lio",
        "selector": "d2lio",
        "method": "D2-LIO",
        "tier": "T1 evidence candidate",
        "paper": "D²-LIO: Enhanced Optimization for LiDAR-IMU Odometry Considering Directional Degeneracy",
        "claim": "Directional degeneracy diagnostics are stable on KITTI; the directional prior is gated to real IMU packets.",
        "mechanism": "Adaptive outlier gating plus Hessian eigen-direction diagnostics, with IMU prior regularization disabled on no-IMU KITTI.",
        "paper_result": {
            "variant": "no_imu_diagnostics_only",
            "role": "competitive no-IMU KITTI fallback",
            "flags": [],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_d2lio.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_d2lio.json",
            },
        },
        "remaining_work": "Synchronized LiDAR-IMU validation committed on HDL-400 open; add to manuscript table with KITTI no-IMU fallback + IMU mechanism rows locked.",
        "no_ablation_claim_limit": "Competitive KITTI fallback only; full LIO mechanism requires synchronized IMU validation.",
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
        "remaining_work": "Synthetic non-flat stress, KITTI seq08, and MulRan ParkingLot public validation are committed; MulRan no-gt-seed odometry diverges so ground on/off is inconclusive — GT-seeded or IMU-backed MulRan protocol remains open before promoting to T0.",
    },
    {
        "id": "quadric_lo",
        "selector": "quadric_lo",
        "method": "Quadric-LO",
        "tier": "T1+ evidence candidate",
        "mechanism": "Point-to-quadric Taubin-distance residuals with rare point-to-plane fallback.",
        "paper_result": {
            "variant": "plane_fallback_on",
            "role": "paper mechanism enabled",
            "flags": ["--quadric-lo-dense-profile"],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_quadric_lo.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_quadric_lo.json",
            },
        },
        "ablation": {
            "summary": "docs/benchmarks/kitti_full_new_methods/quadric_lo_plane_fallback_ablation.json",
            "delta_key": "delta_no_fallback_vs_on",
            "variants": {
                "plane_fallback_on": {
                    "role": "paper mechanism enabled",
                    "flags": ["--quadric-lo-dense-profile"],
                },
                "plane_fallback_off": {
                    "role": "plane fallback disabled",
                    "flags": [
                        "--quadric-lo-dense-profile",
                        "--quadric-lo-no-plane-fallback",
                    ],
                },
            },
        },
        "remaining_work": "Synthetic curved-object stress and KITTI seq02 public validation are committed; add dedicated orchard or non-urban multi-beam benchmarks before promoting to T0.",
    },
    {
        "id": "lidar_iba",
        "selector": "lidar_iba",
        "method": "LiDAR-IBA",
        "tier": "T1 evidence candidate",
        "paper": "Li et al., A Consistency-Improved LiDAR(-Inertial) Bundle Adjustment, arXiv:2602.06380",
        "claim": "Stereographic plane-normal front-end is competitive on KITTI; sliding-window BA is implemented but the committed no-BA profile is better for translational RPE on this IMU-free protocol.",
        "mechanism": "Stereographic plane-normal front-end with optional sliding-window plane BA and FEJ gauge fix.",
        "paper_result": {
            "variant": "no_ba",
            "role": "main KITTI odometry row",
            "flags": ["--lidar-iba-no-ba"],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_lidar_iba.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_lidar_iba.json",
            },
        },
        "ablation": {
            "summary": "docs/benchmarks/kitti_full_new_methods/lidar_iba_ba_ablation.json",
            "delta_key": "delta_ba_on_vs_no_ba",
            "variants": {
                "no_ba": {
                    "role": "committed KITTI main row",
                    "flags": ["--lidar-iba-no-ba"],
                },
                "ba_on": {
                    "role": "sliding-window plane BA enabled",
                    "flags": [],
                },
            },
        },
        "remaining_work": "Validate the BA + IMU path on a synchronized LiDAR-IMU benchmark before making full LIO claims.",
    },
    {
        "id": "tricp_lo",
        "selector": "tricp_lo",
        "method": "TrICP-LO",
        "tier": "T1 evidence candidate",
        "paper": "Chetverikov et al., The Trimmed Iterative Closest Point Algorithm, ICPR 2002 / Image Vis. Comput. 2005",
        "claim": "Least-trimmed-squares point-to-plane odometry is competitive with KISS-ICP on KITTI; FRMSD auto-overlap sticks to the min_overlap floor and fixed overlap 0.900 is only a marginal RPE change.",
        "mechanism": "Rank-based trimmed point-to-plane ICP with FRMSD automatic overlap estimation.",
        "paper_result": {
            "variant": "dense_auto_overlap",
            "role": "main KITTI odometry row",
            "flags": ["--tricp-lo-dense-profile"],
            "artifacts": {
                "00": "docs/benchmarks/kitti_full_new_methods/seq00_tricp_lo.json",
                "07": "docs/benchmarks/kitti_full_new_methods/seq07_tricp_lo.json",
            },
        },
        "ablation": {
            "summary": "docs/benchmarks/kitti_full_new_methods/tricp_lo_overlap_ablation.json",
            "delta_key": "delta_fixed_vs_auto_overlap",
            "variants": {
                "auto_overlap": {
                    "role": "committed KITTI main row",
                    "flags": ["--tricp-lo-dense-profile"],
                },
                "fixed_overlap": {
                    "role": "fixed overlap_ratio=0.900",
                    "flags": ["--tricp-lo-dense-profile", "--tricp-lo-fixed-overlap"],
                },
            },
        },
        "remaining_work": "Add a high-outlier or non-overlap stress set before promoting FRMSD overlap claims beyond clean KITTI odometry.",
    },
]


METRIC_KEYS = [
    "ate_m",
    "rpe_trans_pct",
    "rpe_rot_deg_per_m",
    "time_ms",
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


def dataset_metadata(artifact_rel: str) -> dict[str, Any]:
    data = load_json(artifact_rel)
    required = ("num_frames", "trajectory_length_m", "timestamp_source")
    missing = [key for key in required if data.get(key) is None]
    if missing:
        raise SystemExit(f"artifact missing dataset metadata: {artifact_rel} {missing}")
    return {
        "frames": data["num_frames"],
        "trajectory_length_m": data["trajectory_length_m"],
        "timestamp_source": data["timestamp_source"],
    }


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
    ablation_config = config.get("ablation")
    summary_rel = ablation_config.get("summary") if ablation_config else None
    summary: dict[str, Any] = {}
    if summary_rel:
        summary = load_json(summary_rel)
        if summary.get("method") != config["method"]:
            raise SystemExit(f"method mismatch in {summary_rel}: {summary.get('method')}")

    paper_sequences: list[dict[str, Any]] = []
    table_rows: list[dict[str, Any]] = []
    paper_result = config["paper_result"]
    for sequence, artifact_rel in sorted(paper_result["artifacts"].items()):
        method = method_payload(artifact_rel)
        metrics = metrics_from_method(method)
        dataset = dataset_metadata(artifact_rel)
        command = command_for(
            config["selector"], sequence, paper_result["flags"], artifact_rel
        )
        row = {
            "method": config["method"],
            "tier": config["tier"],
            "sequence": sequence,
            "variant": paper_result["variant"],
            "artifact": artifact_rel,
            "benchmark_id": "kitti_odometry",
            "benchmark_label": f"KITTI {sequence} full",
            "comparison_group_id": f"kitti_{sequence}_full",
            "sequence_or_window": sequence,
            "scope": "full_sequence",
            "result_role": "paper_ready_full",
            "evaluation_policy": "odometry_only",
            "initialization_policy": "first_pose_anchored",
            "rankable": True,
            "rank_metric": "rpe_trans_pct",
            "trajectory_length_m": dataset["trajectory_length_m"],
            "timestamp_source": dataset["timestamp_source"],
            "generated_at": FROZEN_DATE,
            "runtime_profile": paper_result["variant"],
            "command": command,
            **metrics,
        }
        table_rows.append(row)
        paper_sequences.append({
            "sequence": sequence,
            "artifact": artifact_rel,
            "trajectory_length_m": dataset["trajectory_length_m"],
            "metrics": metrics,
            "command": command,
        })

    ablation_pairs: list[dict[str, Any]] = []
    if ablation_config:
        variant_configs = ablation_config["variants"]
        delta_key = ablation_config["delta_key"]
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
        "paper": summary.get("paper", config.get("paper")),
        "claim": summary.get("claim", config.get("claim")),
        "mechanism": config["mechanism"],
        "paper_result": {
            "variant": paper_result["variant"],
            "role": paper_result["role"],
            "sequences": paper_sequences,
        },
        "remaining_work": config["remaining_work"],
    }
    if ablation_config:
        method_entry["ablation"] = {
            "summary_artifact": summary_rel,
            "protocol": summary.get("protocol", {}),
            "pairs": ablation_pairs,
            "conclusion": summary.get("conclusion"),
        }
    else:
        method_entry["ablation"] = {
            "summary_artifact": None,
            "protocol": {
                "type": "no paired ablation in frozen bundle",
                "claim_limit": config.get(
                    "no_ablation_claim_limit",
                    "Competitive KITTI fallback only; full LIO mechanism requires synchronized IMU validation.",
                ),
            },
            "pairs": [],
            "conclusion": config.get("claim"),
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
        "bundle_id": BUNDLE_ID,
        "frozen_date": FROZEN_DATE,
        "generated_by": "evaluation/scripts/build_paper_ready_bundle.py",
        "scope": {
            "dataset": "KITTI Odometry full sequences",
            "sequences": ["00", "07"],
            "seed": "first-pose anchor, --no-gt-seed velocity prediction after frame 0",
            "artifact_root": "docs/benchmarks/kitti_full_new_methods",
            "claim_policy": "Use only tiered T0/T1 evidence for manuscript claims; no-IMU fallback rows are KITTI odometry evidence, not full LIO claims.",
        },
        "status": {
            "frozen_methods": len(methods),
            "paper_table_rows": len(table_rows),
            "paired_ablation_summaries": sum(
                1 for config in METHOD_CONFIGS if config.get("ablation")
            ),
            "t0_evidence_candidates": ["I-LOAM", "KC-LO"],
            "t1_evidence_candidates": ["LiDAR-IBA", "TrICP-LO"],
            "t1_plus_evidence_candidates": ["M-GCLO", "Quadric-LO"],
            "competitive_no_imu_fallbacks": ["DegenSense", "D2-LIO"],
            "supporting_synthetic_stress_checks": [
                {
                    "name": "RF-LIO/ID-LIO synthetic dynamic-object stress",
                    "artifact": "docs/benchmarks/dynamic_object_stress/rf_id_lio_dynamic_object_stress_summary.json",
                    "claim_limit": "Mechanism stress only; dedicated high-dynamic multi-beam benchmarks are still required before manuscript-level dynamic-scene claims.",
                },
                {
                    "name": "M-GCLO synthetic non-flat ground stress",
                    "artifact": "docs/benchmarks/nonflat_ground_stress/m_gclo_nonflat_ground_stress_summary.json",
                    "claim_limit": "Mechanism stress only; dedicated off-road / multi-beam non-flat benchmarks such as MulRan are still required before promoting M-GCLO to T0.",
                },
                {
                    "name": "Quadric-LO synthetic curved-object stress",
                    "artifact": "docs/benchmarks/quadric_curved_stress/quadric_curved_stress_summary.json",
                    "claim_limit": "Mechanism stress only; dedicated orchard or non-urban multi-beam benchmarks are still required before promoting Quadric-LO to T0.",
                }
            ],
            "supporting_public_validation_checks": [
                {
                    "name": "M-GCLO KITTI seq08 public ground on/off validation",
                    "artifact": "docs/benchmarks/kitti_seq08_public/m_gclo_kitti_seq08_validation_summary.json",
                    "claim_limit": "Public hilly KITTI seq08 check only; MulRan ParkingLot is committed separately but no-gt-seed odometry diverges there.",
                },
                {
                    "name": "M-GCLO MulRan ParkingLot public ground on/off validation",
                    "artifact": "docs/benchmarks/mulran_parkinglot_public/m_gclo_mulran_parkinglot_validation_summary.json",
                    "claim_limit": "Dedicated off-road / multi-beam check committed, but shared no-gt-seed odometry diverges (~103% RPE); ground on/off delta is inconclusive and does not support T0 promotion.",
                },
                {
                    "name": "Quadric-LO KITTI seq02 public plane-fallback on/off validation",
                    "artifact": "docs/benchmarks/kitti_seq02_public/quadric_lo_kitti_seq02_validation_summary.json",
                    "claim_limit": "Public residential KITTI seq02 check only; dedicated orchard or non-urban multi-beam benchmarks remain open before T0 promotion.",
                },
                {
                    "name": "RF-LIO/ID-LIO KITTI seq05 public dynamic validation",
                    "artifact": "docs/benchmarks/kitti_seq05_public/rf_id_lio_kitti_seq05_validation_summary.json",
                    "claim_limit": "Public urban KITTI seq05 check only; dedicated high-dynamic multi-beam benchmarks remain open before manuscript-level dynamic-scene claims.",
                },
                {
                    "name": "LIO synchronized LiDAR-IMU public validation (HDL-400 open)",
                    "artifact": "docs/benchmarks/lio_imu_public/hdl_400_lio_imu_validation_summary.json",
                    "claim_limit": "IMU-gated compensation/prior paths activate on 120-frame HDL-400 open; metric deltas vs no-imu.csv fallback are small on this window and LiDAR-IBA IMU is not wired — mechanism evidence only, not full LIO T0 promotion.",
                },
                {
                    "name": "LIO synchronized LiDAR-IMU public validation (NCLT 2013-01-10)",
                    "artifact": "docs/benchmarks/lio_imu_public/nclt_2013_01_10_120_lio_imu_validation_summary.json",
                    "claim_limit": "Public NCLT velodyne_sync + MS25 IMU check; DegenSense IMU compensation improves ATE vs no-imu.csv fallback but KISS sanity is poor on this window — mechanism evidence only, not full LIO T0 promotion.",
                },
            ],
            "remaining_before_full_manuscript_table": [
                "RF-LIO/ID-LIO dedicated high-dynamic multi-beam validation (partial KITTI seq05 urban dense-profile check committed)",
                "M-GCLO dedicated off-road / multi-beam non-flat validation (KITTI seq08 + MulRan ParkingLot committed; MulRan no-gt-seed tracking failure leaves ground ablation inconclusive)",
                "Quadric-LO dedicated orchard or non-urban multi-beam validation (partial KITTI seq02 residential check committed)",
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
