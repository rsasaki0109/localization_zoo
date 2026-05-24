#!/usr/bin/env python3
"""Summarize fixed-map NDT seed experiments with failure-aware decisions."""

from __future__ import annotations

import argparse
import json
import math
import re
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_INPUT_DIR = REPO_ROOT / "experiments" / "results" / "fixed_map_ndt"
DEFAULT_OUTPUT_JSON = DEFAULT_INPUT_DIR / "fixed_map_ndt_failure_audit.json"
DEFAULT_OUTPUT_MD = DEFAULT_INPUT_DIR / "fixed_map_ndt_failure_audit.md"

PRODUCT_ATE_M = 1.0
PRODUCT_RPE_TRANS_PCT = 5.0
PRODUCT_MIN_FPS = 10.0
SILENT_ACCEPT_RATE = 0.5
BAD_ATE_M = 5.0
BAD_RPE_TRANS_PCT = 10.0

DECISION_ORDER = ("pass", "watch", "investigate", "fail")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, default=DEFAULT_INPUT_DIR)
    parser.add_argument("--output-json", type=Path, default=DEFAULT_OUTPUT_JSON)
    parser.add_argument("--output-md", type=Path, default=DEFAULT_OUTPUT_MD)
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def as_float(value: Any) -> float | None:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def parse_fraction(note: str, pattern: str) -> tuple[int | None, int | None]:
    match = re.search(pattern, note)
    if not match:
        return (None, None)
    return (int(match.group(1)), int(match.group(2)))


def parse_int(note: str, pattern: str) -> int | None:
    match = re.search(pattern, note)
    return int(match.group(1)) if match else None


def parse_float(note: str, pattern: str) -> float | None:
    match = re.search(pattern, note)
    if not match:
        return None
    return as_float(match.group(1).rstrip("."))


def accepted_rate(row: dict[str, Any]) -> float | None:
    accepted = row.get("accepted")
    attempted = row.get("attempted")
    if accepted is None or attempted in (None, 0):
        return None
    return float(accepted) / float(attempted)


def source_from_filename(path: Path) -> tuple[str, str]:
    name = path.stem
    if "_seed_ct_icp" in name:
        return ("ct_icp", "ct_icp_prior")
    if "_seed_velocity" in name:
        return ("velocity", "dead_reckoning_prior")
    if "_seed_scan_context" in name:
        return ("scan_context", "global_initializer")
    if "_seed_gt" in name:
        return ("gt", "oracle_seed")
    if re.search(r"_gt_(x|y|z|yaw)", name):
        return ("gt_perturbed", "basin_probe")
    return ("unknown", "unknown")


def variant_from_filename(path: Path) -> dict[str, Any]:
    match = re.match(r"kitti_seq_(\d+)_108_(.+)\.json$", path.name)
    if not match:
        raise ValueError(f"unsupported fixed-map NDT filename: {path.name}")
    sequence = match.group(1)
    variant = match.group(2)
    seed_source, source_class = source_from_filename(path)
    top_k = parse_int(variant, r"topk_(\d+)")
    return {
        "sequence": sequence,
        "variant": variant,
        "seed_source": seed_source,
        "source_class": source_class,
        "top_k": top_k,
        "scan_context_unfiltered": variant.endswith("_unfiltered"),
    }


def metadata_from_note(note: str) -> dict[str, Any]:
    accepted, attempted = parse_fraction(note, r"accepted=(\d+)/(\d+)")
    hits, hit_attempts = parse_fraction(note, r"ScanContext hits=(\d+)/(\d+)")
    metadata = {
        "map_stride": parse_int(note, r"stride=(\d+)"),
        "map_points": parse_int(note, r"map_points=(\d+)"),
        "accepted": accepted,
        "attempted": attempted,
        "rejected": parse_int(note, r"rejected=(\d+)"),
        "seed_fallbacks": parse_int(note, r"seed fallbacks=(\d+)"),
        "scan_context_hits": hits,
        "scan_context_attempts": hit_attempts,
        "scan_context_mean_distance": parse_float(note, r"mean distance=([0-9.+-]+)"),
        "ndt_candidate_evals": parse_int(note, r"NDT candidates evaluated=(\d+)"),
        "seed_perturbation_dx_m": parse_float(note, r"dx=([0-9.+-]+)m"),
        "seed_perturbation_dy_m": parse_float(note, r"dy=([0-9.+-]+)m"),
        "seed_perturbation_yaw_deg": parse_float(note, r"yaw=([0-9.+-]+)deg"),
    }
    metadata["accepted_rate"] = accepted_rate(metadata)
    if hits is not None and hit_attempts:
        metadata["scan_context_hit_rate"] = float(hits) / float(hit_attempts)
    else:
        metadata["scan_context_hit_rate"] = None
    return metadata


def metric_is_bad(row: dict[str, Any]) -> bool:
    ate = as_float(row.get("ate_m"))
    rpe = as_float(row.get("rpe_trans_pct"))
    return (
        ate is None
        or rpe is None
        or ate > BAD_ATE_M
        or rpe > BAD_RPE_TRANS_PCT
    )


def metric_is_product_candidate(row: dict[str, Any]) -> bool:
    ate = as_float(row.get("ate_m"))
    rpe = as_float(row.get("rpe_trans_pct"))
    fps = as_float(row.get("fps"))
    rate = as_float(row.get("accepted_rate"))
    return (
        ate is not None
        and rpe is not None
        and fps is not None
        and ate <= PRODUCT_ATE_M
        and rpe <= PRODUCT_RPE_TRANS_PCT
        and fps >= PRODUCT_MIN_FPS
        and (rate is None or rate >= SILENT_ACCEPT_RATE)
    )


def classify_row(row: dict[str, Any]) -> dict[str, Any]:
    status = str(row.get("status") or "missing")
    ate = as_float(row.get("ate_m"))
    rpe = as_float(row.get("rpe_trans_pct"))
    fps = as_float(row.get("fps"))
    rate = as_float(row.get("accepted_rate"))
    seed_source = str(row.get("seed_source"))
    source_class = str(row.get("source_class"))
    reasons: list[str] = []

    if status != "ok" or ate is None or rpe is None:
        return {
            "policy_decision": "fail",
            "policy_reasons": ["invalid_metric_or_status"],
        }

    if source_class == "oracle_seed":
        return {
            "policy_decision": "pass",
            "policy_reasons": ["oracle_localizer_smoke_pass"],
        }

    if source_class == "dense_db_smoke":
        return {
            "policy_decision": "pass",
            "policy_reasons": ["dense_db_place_recognition_smoke_pass"],
        }

    if source_class == "basin_probe":
        if rpe > PRODUCT_RPE_TRANS_PCT:
            return {
                "policy_decision": "watch",
                "policy_reasons": ["basin_probe_rotation_rpe_high"],
            }
        return {
            "policy_decision": "pass",
            "policy_reasons": ["basin_probe_translation_recovered"],
        }

    if metric_is_product_candidate(row):
        return {
            "policy_decision": "pass",
            "policy_reasons": ["non_oracle_product_candidate"],
        }

    if fps is not None and fps < PRODUCT_MIN_FPS and ate <= PRODUCT_ATE_M and rpe <= PRODUCT_RPE_TRANS_PCT:
        return {
            "policy_decision": "watch",
            "policy_reasons": ["runtime_below_budget"],
        }

    bad = metric_is_bad(row)
    if rate is not None and rate >= SILENT_ACCEPT_RATE and bad:
        reasons.append("accepted_bad_localization")
        if seed_source == "scan_context" and row.get("scan_context_unfiltered"):
            reasons.append("unfiltered_ndt_score_trap")
        return {"policy_decision": "fail", "policy_reasons": reasons}

    if rate is not None and rate < 0.1 and bad:
        return {
            "policy_decision": "investigate",
            "policy_reasons": ["rejected_bad_seed_detectable"],
        }

    if seed_source == "scan_context" and ate <= 3.0 and rpe <= BAD_RPE_TRANS_PCT:
        return {
            "policy_decision": "watch",
            "policy_reasons": ["global_initializer_near_basin"],
        }

    if bad:
        return {
            "policy_decision": "investigate",
            "policy_reasons": ["bad_localization"],
        }

    return {
        "policy_decision": "watch",
        "policy_reasons": ["near_candidate_but_threshold_miss"],
    }


def row_from_file(path: Path) -> dict[str, Any]:
    payload = load_json(path)
    method = (payload.get("methods") or [{}])[0]
    note = str(method.get("note") or "")
    variant = variant_from_filename(path)
    metadata = metadata_from_note(note)
    if (
        variant["seed_source"] == "scan_context"
        and metadata.get("map_stride") == 1
        and variant.get("top_k") is None
    ):
        variant["source_class"] = "dense_db_smoke"
    row = {
        **variant,
        **metadata,
        "result_json": display_path(path),
        "method": method.get("name"),
        "status": method.get("status"),
        "ate_m": as_float(method.get("ate_m")),
        "rpe_trans_pct": as_float(method.get("rpe_trans_pct")),
        "rpe_rot_deg_per_m": as_float(method.get("rpe_rot_deg_per_m")),
        "fps": as_float(method.get("fps")),
        "frames": method.get("frames"),
        "note": note,
    }
    row.update(classify_row(row))
    return row


def discover_rows(input_dir: Path) -> list[dict[str, Any]]:
    rows = []
    for path in sorted(input_dir.glob("kitti_seq_*_108_*.json")):
        rows.append(row_from_file(path))
    rows.sort(
        key=lambda row: (
            str(row["sequence"]),
            str(row["seed_source"]),
            int(row.get("map_stride") or 0),
            str(row["variant"]),
        )
    )
    return rows


def finite_values(rows: list[dict[str, Any]], key: str) -> list[float]:
    out = []
    for row in rows:
        value = as_float(row.get(key))
        if value is not None:
            out.append(value)
    return out


def source_summary(rows: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in rows:
        grouped[str(row["seed_source"])].append(row)
    summary = {}
    for source, source_rows in sorted(grouped.items()):
        decisions = Counter(str(row["policy_decision"]) for row in source_rows)
        ate_values = finite_values(source_rows, "ate_m")
        rpe_values = finite_values(source_rows, "rpe_trans_pct")
        summary[source] = {
            "rows": len(source_rows),
            "decisions": {decision: decisions.get(decision, 0) for decision in DECISION_ORDER},
            "best_ate_m": min(ate_values) if ate_values else None,
            "best_rpe_trans_pct": min(rpe_values) if rpe_values else None,
        }
    return summary


def sequence_best_non_oracle(rows: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in rows:
        if row.get("source_class") in {"oracle_seed", "basin_probe", "dense_db_smoke"}:
            continue
        grouped[str(row["sequence"])].append(row)
    out = {}
    for sequence, seq_rows in sorted(grouped.items()):
        valid = [row for row in seq_rows if as_float(row.get("ate_m")) is not None]
        if not valid:
            continue
        best = min(valid, key=lambda row: float(row["ate_m"]))
        out[sequence] = {
            "variant": best["variant"],
            "seed_source": best["seed_source"],
            "ate_m": best["ate_m"],
            "rpe_trans_pct": best["rpe_trans_pct"],
            "fps": best["fps"],
            "accepted": best.get("accepted"),
            "attempted": best.get("attempted"),
            "policy_decision": best["policy_decision"],
            "policy_reasons": best["policy_reasons"],
            "result_json": best["result_json"],
        }
    return out


def aggregate(rows: list[dict[str, Any]]) -> dict[str, Any]:
    decisions = Counter(str(row["policy_decision"]) for row in rows)
    silent_bad = [
        row
        for row in rows
        if "accepted_bad_localization" in (row.get("policy_reasons") or [])
    ]
    non_oracle_pass = [
        row
        for row in rows
        if row.get("policy_decision") == "pass"
        and row.get("source_class") not in {"oracle_seed", "basin_probe", "dense_db_smoke"}
    ]
    return {
        "rows": len(rows),
        "decision_counts": {decision: decisions.get(decision, 0) for decision in DECISION_ORDER},
        "silent_bad_accept_rows": len(silent_bad),
        "non_oracle_product_candidates": len(non_oracle_pass),
        "source_summary": source_summary(rows),
        "best_non_oracle_by_sequence": sequence_best_non_oracle(rows),
    }


def fmt(value: Any, digits: int = 3) -> str:
    number = as_float(value)
    if number is None:
        return "n/a"
    return f"{number:.{digits}f}"


def fraction_text(row: dict[str, Any]) -> str:
    accepted = row.get("accepted")
    attempted = row.get("attempted")
    if accepted is None or attempted is None:
        return "n/a"
    return f"{accepted}/{attempted}"


def reasons_text(row: dict[str, Any]) -> str:
    return ", ".join(str(reason) for reason in row.get("policy_reasons") or []) or "n/a"


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    aggregate_row = payload["aggregate"]
    counts = aggregate_row["decision_counts"]
    lines = [
        "# Fixed-Map NDT Failure Audit",
        "",
        f"Input dir: `{payload['input_dir']}`",
        "",
        "## Gate",
        "",
        f"- Product candidate: ATE <= {PRODUCT_ATE_M:.1f} m, RPE <= {PRODUCT_RPE_TRANS_PCT:.1f}%, FPS >= {PRODUCT_MIN_FPS:.1f}",
        f"- Silent bad accept: accepted rate >= {SILENT_ACCEPT_RATE:.1f} and ATE > {BAD_ATE_M:.1f} m or RPE > {BAD_RPE_TRANS_PCT:.1f}%",
        "",
        "## Aggregate",
        "",
        "| Rows | Pass | Watch | Investigate | Fail | Silent bad accepts | Non-oracle product candidates |",
        "| ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        f"| {aggregate_row['rows']} | {counts['pass']} | {counts['watch']} | "
        f"{counts['investigate']} | {counts['fail']} | "
        f"{aggregate_row['silent_bad_accept_rows']} | "
        f"{aggregate_row['non_oracle_product_candidates']} |",
        "",
        "## Source Summary",
        "",
        "| Seed source | Rows | Pass | Watch | Investigate | Fail | Best ATE [m] | Best RPE [%] |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for source, row in aggregate_row["source_summary"].items():
        decisions = row["decisions"]
        lines.append(
            f"| `{source}` | {row['rows']} | {decisions['pass']} | "
            f"{decisions['watch']} | {decisions['investigate']} | "
            f"{decisions['fail']} | {fmt(row['best_ate_m'])} | "
            f"{fmt(row['best_rpe_trans_pct'])} |"
        )

    lines.extend(
        [
            "",
            "## Best Non-Oracle Per Sequence",
            "",
            "| Sequence | Source | Variant | ATE [m] | RPE [%] | FPS | Accepted | Decision | Reasons |",
            "| --- | --- | --- | ---: | ---: | ---: | ---: | --- | --- |",
        ]
    )
    for sequence, row in aggregate_row["best_non_oracle_by_sequence"].items():
        lines.append(
            f"| `{sequence}` | `{row['seed_source']}` | `{row['variant']}` | "
            f"{fmt(row['ate_m'])} | {fmt(row['rpe_trans_pct'])} | "
            f"{fmt(row['fps'])} | {fraction_text(row)} | "
            f"`{row['policy_decision']}` | {reasons_text(row)} |"
        )

    risk_rows = [
        row
        for row in payload["rows"]
        if row["policy_decision"] in {"fail", "investigate"}
    ]
    lines.extend(
        [
            "",
            "## Risk Rows",
            "",
            "| Sequence | Source | Variant | ATE [m] | RPE [%] | FPS | Accepted | Decision | Reasons |",
            "| --- | --- | --- | ---: | ---: | ---: | ---: | --- | --- |",
        ]
    )
    for row in risk_rows:
        lines.append(
            f"| `{row['sequence']}` | `{row['seed_source']}` | `{row['variant']}` | "
            f"{fmt(row['ate_m'])} | {fmt(row['rpe_trans_pct'])} | "
            f"{fmt(row['fps'])} | {fraction_text(row)} | "
            f"`{row['policy_decision']}` | {reasons_text(row)} |"
        )
    if not risk_rows:
        lines.append("| n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a | n/a |")

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- GT-seeded rows validate the fixed-map NDT localizer and map path; they are oracle smoke tests, not deployable initialization.",
            "- Scan Context stride=1 rows use every reference frame in the retrieval DB, so they are dense-DB smoke checks and are excluded from non-oracle product-candidate counts.",
            "- High acceptance with bad GT error is treated as the dangerous case because it can silently publish a wrong pose.",
            "- Low acceptance with bad GT error is still bad localization, but it is easier to guard because the localizer is already rejecting most corrections.",
            "- Sparse Scan Context rows that miss the product threshold remain useful as initializer-basin probes, while unfiltered top-K rows expose NDT-score-only selection risk.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def build_payload(input_dir: Path) -> dict[str, Any]:
    rows = discover_rows(input_dir)
    return {
        "input_dir": display_path(input_dir),
        "thresholds": {
            "product_ate_m": PRODUCT_ATE_M,
            "product_rpe_trans_pct": PRODUCT_RPE_TRANS_PCT,
            "product_min_fps": PRODUCT_MIN_FPS,
            "silent_accept_rate": SILENT_ACCEPT_RATE,
            "bad_ate_m": BAD_ATE_M,
            "bad_rpe_trans_pct": BAD_RPE_TRANS_PCT,
        },
        "aggregate": aggregate(rows),
        "rows": rows,
    }


def main() -> int:
    args = parse_args()
    payload = build_payload(args.input_dir)
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(args.output_md, payload)
    print(f"[done] wrote {display_path(args.output_json)} and {display_path(args.output_md)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
