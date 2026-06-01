#!/usr/bin/env python3
"""Diagnose pair-prior selector signals against GT pair motion."""

from __future__ import annotations

import argparse
import json
import math
from collections import Counter, defaultdict
from pathlib import Path

import numpy as np

from intensity_bev_odometry_demo import Candidate, compute_metrics, load_gt_xyyaw, se2_inverse, se2_matrix
from select_pair_prior import candidate_from_pair


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--gt-csv", required=True, type=Path)
    p.add_argument("--output-json", required=True, type=Path)
    p.add_argument("--output-md", type=Path)
    p.add_argument(
        "--candidate",
        action="append",
        required=True,
        help="Candidate prior as name:path. Repeat for KISS, SmallGICP, etc.",
    )
    p.add_argument("--selector-json", type=Path)
    p.add_argument("--shared-residual-json", type=Path)
    p.add_argument("--candidate-source", choices=("used", "raw"), default="used")
    return p.parse_args()


def load_candidate_sets(specs: list[str], source: str) -> dict[str, dict[int, Candidate]]:
    loaded: dict[str, dict[int, Candidate]] = {}
    for spec in specs:
        if ":" not in spec:
            raise ValueError(f"--candidate must be name:path, got {spec}")
        name, path_str = spec.split(":", 1)
        payload = json.loads(Path(path_str).read_text())
        pairs: dict[int, Candidate] = {}
        for pair in payload.get("pairs", []):
            pairs[int(pair["index"])] = candidate_from_pair(pair, source)
        loaded[name] = pairs
    return loaded


def load_selector(path: Path | None) -> dict[int, dict]:
    if path is None:
        return {}
    payload = json.loads(path.read_text())
    return {int(pair["index"]): pair for pair in payload.get("pairs", [])}


def load_shared_residuals(path: Path | None) -> dict[int, dict]:
    if path is None:
        return {}
    payload = json.loads(path.read_text())
    return {int(pair["index"]): pair for pair in payload.get("pairs", [])}


def gt_pair_motions(gt: np.ndarray, n: int) -> dict[int, Candidate]:
    mats = [se2_matrix(float(x), float(y), float(yaw)) for x, y, yaw in gt[:n]]
    pairs: dict[int, Candidate] = {}
    for i in range(1, len(mats)):
        rel = se2_inverse(mats[i - 1]) @ mats[i]
        pairs[i] = Candidate(
            float(rel[0, 2]),
            float(rel[1, 2]),
            float(math.atan2(rel[1, 0], rel[0, 0])),
            0.0,
            0,
        )
    return pairs


def candidate_matrix(cand: Candidate) -> np.ndarray:
    return se2_matrix(cand.dx, cand.dy, cand.yaw)


def pair_error(cand: Candidate, gt: Candidate) -> dict[str, float]:
    err = se2_inverse(candidate_matrix(gt)) @ candidate_matrix(cand)
    trans = float(np.linalg.norm(err[:2, 2]))
    yaw = abs(math.degrees(float(math.atan2(err[1, 0], err[0, 0]))))
    step_err = abs(math.hypot(cand.dx, cand.dy) - math.hypot(gt.dx, gt.dy))
    return {
        "translation_error_m": trans,
        "yaw_error_deg": yaw,
        "step_length_error_m": float(step_err),
    }


def integrate_pairs(pairs: list[Candidate]) -> np.ndarray:
    pose = np.eye(3)
    poses = [[0.0, 0.0, 0.0]]
    for cand in pairs:
        pose = pose @ candidate_matrix(cand)
        poses.append([float(pose[0, 2]), float(pose[1, 2]), float(math.atan2(pose[1, 0], pose[0, 0]))])
    return np.asarray(poses, dtype=np.float64)


def summarize_errors(errors: list[float]) -> dict[str, float]:
    arr = np.asarray(errors, dtype=np.float64)
    if arr.size == 0:
        return {"mean": float("nan"), "median": float("nan"), "p95": float("nan")}
    return {
        "mean": float(arr.mean()),
        "median": float(np.median(arr)),
        "p95": float(np.percentile(arr, 95.0)),
    }


def main() -> int:
    args = parse_args()
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    if args.output_md is not None:
        args.output_md.parent.mkdir(parents=True, exist_ok=True)

    candidates = load_candidate_sets(args.candidate, args.candidate_source)
    names = list(candidates)
    selector = load_selector(args.selector_json)
    shared_residuals = load_shared_residuals(args.shared_residual_json)
    gt = load_gt_xyyaw(args.gt_csv)
    n = min([gt.shape[0], *[max(pairs.keys()) + 1 for pairs in candidates.values()]])
    gt_pairs = gt_pair_motions(gt, n)

    pair_rows = []
    selected_pairs: list[Candidate] = []
    oracle_pairs: list[Candidate] = []
    method_pairs: dict[str, list[Candidate]] = {name: [] for name in names}
    selected_counts: Counter[str] = Counter()
    oracle_counts: Counter[str] = Counter()
    selector_hits = 0
    selector_total = 0
    method_error_lists: dict[str, list[float]] = defaultdict(list)
    signal_hits: Counter[str] = Counter()
    signal_total = 0

    for i in range(1, n):
        gt_pair = gt_pairs[i]
        method_rows = {}
        best_name = ""
        best_error = float("inf")
        for name in names:
            cand = candidates[name].get(i, Candidate(0.0, 0.0, 0.0, -1.0, 0))
            err = pair_error(cand, gt_pair)
            method_pairs[name].append(cand)
            method_error_lists[name].append(err["translation_error_m"])
            if err["translation_error_m"] < best_error:
                best_error = err["translation_error_m"]
                best_name = name

            selector_pair = selector.get(i, {})
            selector_candidate = selector_pair.get("candidates", {}).get(name, {})
            shared_pair = shared_residuals.get(i, {})
            shared_candidate = shared_pair.get("methods", {}).get(name, {})
            method_rows[name] = {
                "dx_curr_to_prev_m": cand.dx,
                "dy_curr_to_prev_m": cand.dy,
                "yaw_curr_to_prev_deg": math.degrees(cand.yaw),
                "geometry_score": cand.score,
                "overlap": cand.overlap,
                "cubemap_score": selector_candidate.get("cubemap_score"),
                "shared_residual_score": shared_candidate.get("shared_residual_score"),
                "shared_residual_rmse_m": shared_candidate.get(
                    "symmetric_trimmed_rmse_m", shared_candidate.get("rmse_m")
                ),
                "shared_residual_inlier_ratio": shared_candidate.get(
                    "symmetric_inlier_ratio", shared_candidate.get("inlier_ratio")
                ),
                "shared_stability_score": shared_candidate.get("stability_score"),
                "shared_stability_mean_delta_m": shared_candidate.get("stability_mean_delta_m"),
                "shared_stability_min_delta_m": shared_candidate.get("stability_min_delta_m"),
                **err,
            }

        selected_name = selector.get(i, {}).get("selected")
        signal_total += 1
        cubemap_scores = {
            name: row["cubemap_score"]
            for name, row in method_rows.items()
            if row["cubemap_score"] is not None
        }
        if cubemap_scores:
            signal_hits["cubemap_score"] += int(
                max(cubemap_scores, key=cubemap_scores.get) == best_name
            )
        geometry_scores = {name: row["geometry_score"] for name, row in method_rows.items()}
        signal_hits["geometry_score"] += int(
            max(geometry_scores, key=geometry_scores.get) == best_name
        )
        overlaps = {name: row["overlap"] for name, row in method_rows.items()}
        signal_hits["overlap"] += int(max(overlaps, key=overlaps.get) == best_name)
        shared_scores = {
            name: row["shared_residual_score"]
            for name, row in method_rows.items()
            if row["shared_residual_score"] is not None
        }
        if shared_scores:
            signal_hits["shared_residual_score"] += int(
                max(shared_scores, key=shared_scores.get) == best_name
            )
        shared_rmse = {
            name: row["shared_residual_rmse_m"]
            for name, row in method_rows.items()
            if row["shared_residual_rmse_m"] is not None
        }
        if shared_rmse:
            signal_hits["shared_residual_rmse"] += int(
                min(shared_rmse, key=shared_rmse.get) == best_name
            )
        shared_stability_scores = {
            name: row["shared_stability_score"]
            for name, row in method_rows.items()
            if row["shared_stability_score"] is not None
        }
        if shared_stability_scores:
            signal_hits["shared_stability_score"] += int(
                max(shared_stability_scores, key=shared_stability_scores.get) == best_name
            )
        shared_stability_deltas = {
            name: row["shared_stability_mean_delta_m"]
            for name, row in method_rows.items()
            if row["shared_stability_mean_delta_m"] is not None
        }
        if shared_stability_deltas:
            signal_hits["shared_stability_mean_delta"] += int(
                max(shared_stability_deltas, key=shared_stability_deltas.get) == best_name
            )

        if selected_name in candidates:
            selected_counts[selected_name] += 1
            selected_pairs.append(candidates[selected_name][i])
            selector_total += 1
            selector_hits += int(selected_name == best_name)
        oracle_counts[best_name] += 1
        oracle_pairs.append(candidates[best_name][i])

        pair_rows.append({
            "index": i,
            "oracle_best": best_name,
            "selector_selected": selected_name,
            "selector_hit": selected_name == best_name if selected_name else None,
            "signal_best": {
                "cubemap_score": max(cubemap_scores, key=cubemap_scores.get)
                if cubemap_scores
                else None,
                "geometry_score": max(geometry_scores, key=geometry_scores.get),
                "overlap": max(overlaps, key=overlaps.get),
                "shared_residual_score": max(shared_scores, key=shared_scores.get)
                if shared_scores
                else None,
                "shared_residual_rmse": min(shared_rmse, key=shared_rmse.get)
                if shared_rmse
                else None,
                "shared_stability_score": max(
                    shared_stability_scores, key=shared_stability_scores.get
                )
                if shared_stability_scores
                else None,
                "shared_stability_mean_delta": max(
                    shared_stability_deltas, key=shared_stability_deltas.get
                )
                if shared_stability_deltas
                else None,
            },
            "gt_dx_m": gt_pair.dx,
            "gt_dy_m": gt_pair.dy,
            "gt_yaw_deg": math.degrees(gt_pair.yaw),
            "methods": method_rows,
        })

    trajectory_metrics = {}
    for name, pairs in method_pairs.items():
        trajectory_metrics[name] = compute_metrics(integrate_pairs(pairs), gt)
    trajectory_metrics["oracle_pair_error"] = compute_metrics(integrate_pairs(oracle_pairs), gt)
    if selected_pairs:
        trajectory_metrics["selector"] = compute_metrics(integrate_pairs(selected_pairs), gt)

    method_pair_error_summary = {
        name: summarize_errors(errors) for name, errors in method_error_lists.items()
    }
    payload = {
        "gt_csv": str(args.gt_csv),
        "candidate_source": args.candidate_source,
        "candidates": args.candidate,
        "selector_json": str(args.selector_json) if args.selector_json else None,
        "shared_residual_json": str(args.shared_residual_json) if args.shared_residual_json else None,
        "frames": n,
        "selector_accuracy": selector_hits / selector_total if selector_total else None,
        "selector_hits": selector_hits,
        "selector_total": selector_total,
        "signal_accuracy": {
            name: hit / signal_total for name, hit in signal_hits.items()
        },
        "signal_hits": dict(signal_hits),
        "signal_total": signal_total,
        "selected_counts": dict(selected_counts),
        "oracle_counts": dict(oracle_counts),
        "trajectory_metrics": trajectory_metrics,
        "method_pair_translation_error_m": method_pair_error_summary,
        "pairs": pair_rows,
    }
    args.output_json.write_text(json.dumps(payload, indent=2) + "\n")

    if args.output_md is not None:
        lines = [
            "# Pair Prior Selector Diagnostics",
            "",
            f"- GT: `{args.gt_csv}`",
            f"- Selector: `{args.selector_json}`",
            f"- Selector pair accuracy: {selector_hits}/{selector_total}"
            if selector_total
            else "- Selector pair accuracy: n/a",
            f"- Selected counts: {dict(selected_counts)}",
            f"- Oracle counts: {dict(oracle_counts)}",
            f"- Signal hits: {dict(signal_hits)} / {signal_total}",
            "",
            "## Trajectory Metrics",
            "",
            "| Method | ATE xy [m] | Step RMSE [m] |",
            "| --- | ---: | ---: |",
        ]
        for name, metrics in trajectory_metrics.items():
            lines.append(
                f"| {name} | {metrics['ate_xy_m']:.3f} | {metrics['step_length_rmse_m']:.3f} |"
            )
        lines.extend([
            "",
            "## Pair Translation Error",
            "",
            "| Method | Mean [m] | Median [m] | P95 [m] |",
            "| --- | ---: | ---: | ---: |",
        ])
        for name, stats in method_pair_error_summary.items():
            lines.append(
                f"| {name} | {stats['mean']:.3f} | {stats['median']:.3f} | {stats['p95']:.3f} |"
            )
        lines.extend([
            "",
            "## Selector Misses",
            "",
            "| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |",
            "| ---: | --- | --- | ---: | ---: |",
        ])
        for row in pair_rows:
            selected = row["selector_selected"]
            oracle = row["oracle_best"]
            if selected is None or selected == oracle:
                continue
            selected_err = row["methods"][selected]["translation_error_m"]
            oracle_err = row["methods"][oracle]["translation_error_m"]
            lines.append(
                f"| {row['index']} | {selected} | {oracle} | {selected_err:.3f} | {oracle_err:.3f} |"
            )
        args.output_md.write_text("\n".join(lines) + "\n")

    print(
        f"[done] selector_accuracy={selector_hits}/{selector_total} "
        f"oracle_counts={dict(oracle_counts)} -> {args.output_json}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
