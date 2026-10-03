#!/usr/bin/env python3
"""Re-run the README leaderboard's KITTI cells and report result drift.

For each (method, KITTI Odometry full sequence) cell, the leaderboard shows the
variant with the lowest stored 100 m RPE. This script re-runs exactly that
variant with the current code and records stored vs re-run RPE / ATE plus the
official KITTI RTE, so stale cells can be identified before anything is
re-published. It reuses rescore_kitti_rte.rescore_row for the replay.

Writes experiments/results/kitti_drift_audit.json.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import re
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
from rescore_kitti_rte import primary_result, rescore_row  # noqa: E402


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=REPO_ROOT / "build/evaluation/pcd_dogfooding")
    parser.add_argument(
        "--data-root",
        type=Path,
        default=Path(os.environ.get("LOCALIZATION_ZOO_DATA_ROOT", REPO_ROOT)),
        help="Directory containing dogfooding_results/kitti_seq_<NN>_full",
    )
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--method", action="append", default=[], help="Primary method names to audit")
    parser.add_argument("--jobs", type=int, default=1, help="Concurrent re-runs (FPS is then depressed)")
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--extra-arg", action="append", default=[], help="Recorded with each row")
    return parser.parse_args()


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
    item = rescore_row(row, args)
    aggregate = json.loads((REPO_ROOT / row["repo_best_aggregate"]).read_text())
    item["stored_ate_m"] = next(v.get("ate_m") for v in aggregate["variants"] if v["id"] == row["repo_best_variant"])
    summary_path = args.work_dir / f"{Path(row['repo_best_aggregate']).stem}__{row['repo_best_variant']}.json"
    rerun = primary_result(json.loads(summary_path.read_text()), str(aggregate["stable_interface"]["primary_method"]))
    item["rerun_ate_m"] = rerun.get("ate_m")
    delta = item["rerun_rpe_abs_delta"]
    item["drifted"] = bool(delta is None or delta > 1e-6)
    print(f"[audit] {row['method']} {row['sequence']} {row['repo_best_variant']}: "
          f"stored {item['stored_rpe_trans_pct']:.6f} rerun {item['rerun_rpe_trans_pct']} "
          f"({'DRIFT' if item['drifted'] else 'same'})", flush=True)
    return item


def main() -> int:
    args = parse_args()
    args.work_dir.mkdir(parents=True, exist_ok=True)
    rows = leaderboard_cells(RESULTS_DIR, set(args.method))
    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as pool:
        items = list(pool.map(lambda row: audit(row, args), rows))
    payload = {
        "schema_version": 1,
        "generated_at": datetime.now(timezone.utc).replace(microsecond=0).isoformat(),
        "purpose": "README leaderboard KITTI cells re-run with the current code",
        "concurrent_jobs": args.jobs,
        "host": capture_run_host(),
        "rows": items,
    }
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    drifted = sum(item["drifted"] for item in items)
    print(f"[done] {drifted} of {len(items)} cells drifted; wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
