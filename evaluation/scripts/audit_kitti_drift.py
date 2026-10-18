#!/usr/bin/env python3
"""Re-run stored full-sequence KITTI cells and report result drift.

For each (method, KITTI Odometry full sequence) cell, the leaderboard shows the
variant with the lowest stored 100 m RPE. This script re-runs exactly that
variant with the current code and checks stored vs re-run RPE, ATE and frame
count. The official KITTI RTE is also recorded. A sequence filter supports
fetch/run/delete workflows without keeping every sequence on disk.

Exit codes: 0 = every selected cell matches, 1 = drift, 2 = invalid selection,
inputs or replay. Failed replays stay in the report alongside completed ones.
Filtered audits require an explicit output path and never silently replace the
full historical audit. Each replay writes its exact command, summary and log.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
OUTPUT = RESULTS_DIR / "kitti_drift_audit.json"
FULL_DATASET = re.compile(r"kitti_seq_(\d\d)_full")

sys.path.insert(0, str(Path(__file__).resolve().parent))
from capture_benchmark_environment import capture_run_host  # noqa: E402
from rescore_kitti_rte import find_variant, primary_result, replay_command  # noqa: E402

METRIC_ABS_TOLERANCE = 1e-6  # Existing RPE tolerance, also used for rounded ATE.


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=REPO_ROOT / "build/evaluation/pcd_dogfooding")
    parser.add_argument(
        "--data-root",
        type=Path,
        default=Path(os.environ.get("LOCALIZATION_ZOO_DATA_ROOT", REPO_ROOT)),
        help="Directory containing dogfooding_results/kitti_seq_<NN>_full",
    )
    parser.add_argument("--work-dir", type=Path, help="New or empty directory for replay summaries and logs")
    parser.add_argument("--method", action="append", default=[], help="Primary method names to audit")
    parser.add_argument("--sequence", action="append", default=[], choices=[f"{i:02d}" for i in range(11)],
                        help="Full KITTI sequence to audit; repeat to select several")
    parser.add_argument("--list", action="store_true", help="List selected cells as JSON without running them")
    parser.add_argument("--jobs", type=int, default=1, help="Concurrent re-runs (FPS is then depressed)")
    parser.add_argument("--output", type=Path, help="Report path; required for filtered audits")
    parser.add_argument("--extra-arg", action="append", default=[], help="Recorded with each row")
    args = parser.parse_args(argv)
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if not args.list:
        if args.work_dir is None:
            parser.error("--work-dir is required for a replay")
        if (args.method or args.sequence) and args.output is None:
            parser.error("filtered audits require --output to preserve the full historical audit")
        if args.work_dir.exists() and (not args.work_dir.is_dir() or any(args.work_dir.iterdir())):
            parser.error("--work-dir must be new or empty; preserve prior replay evidence")
    args.output = args.output or OUTPUT
    for key in ("binary", "data_root", "work_dir", "output"):
        value = getattr(args, key)
        if value is not None:
            setattr(args, key, value.resolve())
    return args


def leaderboard_cells(results_dir: Path, methods: set[str]) -> list[dict[str, str]]:
    """Best stored-RPE variant per (method, full KITTI sequence), as the leaderboard picks it."""
    best: dict[tuple[str, str], tuple[float, str, str]] = {}
    for path in sorted(results_dir.glob("*_matrix.json")):
        aggregate = json.loads(path.read_text())
        method = str(aggregate.get("stable_interface", {}).get("primary_method", ""))
        match = FULL_DATASET.fullmatch(Path(str(aggregate.get("dataset", {}).get("pcd_dir", ""))).name)
        if not match or (methods and method not in methods):
            continue
        for variant in aggregate.get("variants", []):
            rpe = variant.get("rpe_trans_pct")
            if variant.get("status") != "ok" or not isinstance(rpe, (int, float)) or not math.isfinite(rpe):
                continue
            key = (method, match.group(1))
            if key not in best or rpe < best[key][0]:
                best[key] = (float(rpe), str(variant["id"]), f"experiments/results/{path.name}")
    return [
        {"method": method, "sequence": sequence, "repo_best_variant": variant, "repo_best_aggregate": aggregate}
        for (method, sequence), (_, variant, aggregate) in sorted(best.items())
    ]


def audit(row: dict[str, str], args: argparse.Namespace) -> dict[str, Any]:
    summary_path = args.work_dir / f"{Path(row['repo_best_aggregate']).stem}__{row['repo_best_variant']}.json"
    log_path = summary_path.with_suffix(".log")
    item: dict[str, Any] = {
        "method": row["method"], "sequence": row["sequence"],
        "aggregate_path": row["repo_best_aggregate"], "variant_id": row["repo_best_variant"],
        "summary_path": str(summary_path), "log_path": str(log_path),
        "command": None, "cwd": str(REPO_ROOT), "process_exit_code": None,
        "rescore_extra_args": list(args.extra_arg),
        "audit_status": "error", "drifted": None,
    }
    try:
        aggregate = json.loads((REPO_ROOT / row["repo_best_aggregate"]).read_text())
        stored = find_variant(aggregate, row["repo_best_variant"])
        item["command"] = replay_command(row, args)
        dataset = aggregate["dataset"]
        if not (args.data_root / dataset["pcd_dir"]).is_dir():
            raise ValueError(f"missing scan directory: {args.data_root / dataset['pcd_dir']}")
        if not (REPO_ROOT / dataset["gt_csv"]).is_file():
            raise ValueError(f"missing ground truth: {dataset['gt_csv']}")
        with log_path.open("w") as log:
            process = subprocess.run(item["command"], cwd=REPO_ROOT, stdout=log, stderr=subprocess.STDOUT)
        item["process_exit_code"] = process.returncode
        if process.returncode != 0:
            raise subprocess.CalledProcessError(process.returncode, item["command"])
        rerun = primary_result(json.loads(summary_path.read_text()),
                               str(aggregate["stable_interface"]["primary_method"]))
        comparison = compare_result(stored, rerun)
        metrics = {key: rerun.get(key) for key in ("kitti_rte_trans_pct", "kitti_rte_rot_deg_per_100m", "fps")}
        for key, value in metrics.items():
            if value is not None and (isinstance(value, bool) or not isinstance(value, (int, float))
                                      or not math.isfinite(value) or value < 0):
                raise ValueError(f"invalid {key}: {value}")
        item.update(args=stored["args"], **metrics, **comparison)
    except (OSError, ValueError, KeyError, TypeError, subprocess.CalledProcessError) as error:
        item["error"] = f"{type(error).__name__}: {error}"
        if isinstance(error, subprocess.CalledProcessError):
            item["process_exit_code"] = error.returncode
    print(f"[audit] {row['method']} {row['sequence']} {row['repo_best_variant']}: "
          f"{item['audit_status']}" + (f" ({item['error']})" if "error" in item else ""), flush=True)
    return item


def compare_result(stored: dict[str, Any], rerun: dict[str, Any]) -> dict[str, Any]:
    """Reject missing/non-finite metrics, then compare both errors and coverage."""
    item: dict[str, Any] = {}
    differences = []
    if rerun.get("status") != "ok":
        raise ValueError(f"replay result status is not ok: {rerun.get('status')}")
    for metric in ("rpe_trans_pct", "ate_m"):
        values = [stored.get(metric), rerun.get(metric)]
        if any(isinstance(v, bool) or not isinstance(v, (int, float)) or not math.isfinite(v) or v < 0
               for v in values):
            raise ValueError(f"missing or invalid {metric}: {values}")
        delta = abs(values[1] - values[0])
        item[f"stored_{metric}"] = values[0]
        item[f"rerun_{metric}"] = values[1]
        item["rerun_rpe_abs_delta" if metric == "rpe_trans_pct" else "rerun_ate_abs_delta"] = delta
        if delta > METRIC_ABS_TOLERANCE:
            differences.append(metric)
    frames = [stored.get("frames"), rerun.get("frames")]
    if any(type(v) is not int or v <= 0 for v in frames):
        raise ValueError(f"missing or invalid frame count: {frames}")
    item["stored_frames"], item["frames"] = frames
    if frames[0] != frames[1]:
        differences.append("frames")
    item.update(audit_status="drift" if differences else "ok", drifted=bool(differences),
                drift_metrics=differences)
    return item


def select_cells(rows: list[dict[str, str]], methods: set[str], sequences: set[str]) -> list[dict[str, str]]:
    selected = [row for row in rows if (not methods or row["method"] in methods)
                and (not sequences or row["sequence"] in sequences)]
    missing_methods = methods - {row["method"] for row in selected}
    missing_sequences = sequences - {row["sequence"] for row in selected}
    if not selected or missing_methods or missing_sequences:
        raise ValueError(f"no cells for selection: methods={sorted(missing_methods)}, "
                         f"sequences={sorted(missing_sequences)}")
    if methods and sequences:
        missing_pairs = {(m, s) for m in methods for s in sequences} - {
            (r["method"], r["sequence"]) for r in selected}
        if missing_pairs:
            raise ValueError(f"no stored cells for method/sequence pairs: {sorted(missing_pairs)}")
    return selected


def report_exit_code(items: list[dict[str, Any]]) -> int:
    if not items or any(item["audit_status"] == "error" for item in items):
        return 2
    return 1 if any(item["drifted"] for item in items) else 0


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        rows = select_cells(leaderboard_cells(RESULTS_DIR, set(args.method)),
                            set(args.method), set(args.sequence))
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"[error] {error}", file=sys.stderr)
        return 2
    scope = {"methods": sorted(set(args.method)), "sequences": sorted(set(args.sequence)),
             "selected_cell_count": len(rows), "filtered": bool(args.method or args.sequence)}
    if args.list:
        print(json.dumps({"scope": scope, "rows": rows}, indent=2))
        return 0
    try:
        args.work_dir.mkdir(parents=True, exist_ok=True)
        binary_hash = None
        if args.binary.is_file():
            digest = hashlib.sha256()
            with args.binary.open("rb") as binary:
                for chunk in iter(lambda: binary.read(1024 * 1024), b""):
                    digest.update(chunk)
            binary_hash = digest.hexdigest()
    except OSError as error:
        print(f"[error] {error}", file=sys.stderr)
        return 2
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        items = list(pool.map(lambda row: audit(row, args), rows))
    exit_code = report_exit_code(items)
    payload = {
        "schema_version": 2,
        "generated_at": datetime.now(timezone.utc).replace(microsecond=0).isoformat(),
        "purpose": "Selected stored full-sequence KITTI cells re-run with the current code",
        "concurrent_jobs": args.jobs,
        "scope": scope,
        "metric_absolute_tolerance": METRIC_ABS_TOLERANCE,
        "binary_sha256": binary_hash,
        "exit_code": exit_code,
        "host": capture_run_host(),
        "rows": items,
    }
    try:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.output.with_name(args.output.name + ".tmp")
        temporary.write_text(json.dumps(payload, indent=2, allow_nan=False) + "\n")
        temporary.replace(args.output)
    except (OSError, ValueError) as error:
        print(f"[error] cannot write audit report: {error}", file=sys.stderr)
        return 2
    drifted = sum(item["audit_status"] == "drift" for item in items)
    failed = sum(item["audit_status"] == "error" for item in items)
    print(f"[done] {drifted} drifted, {failed} failed of {len(items)} cells; "
          f"exit {exit_code}; wrote {args.output}")
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
