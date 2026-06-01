#!/usr/bin/env python3
"""Run pair-prior selector calibration across multiple KITTI windows."""

from __future__ import annotations

import argparse
import csv
import json
import statistics
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Window:
    sequence: str
    start: int
    size: int

    @property
    def key(self) -> str:
        return f"kitti_seq_{self.sequence}_start{self.start:04d}_{self.size}"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--sequence", action="append")
    p.add_argument("--start", action="append", type=int)
    p.add_argument("--window-size", type=int, default=30)
    p.add_argument("--pcd-root", type=Path, default=Path("dogfooding_results"))
    p.add_argument("--gt-root", type=Path, default=Path("experiments/reference_data"))
    p.add_argument(
        "--output-dir",
        type=Path,
        default=Path("experiments/results/intensity_odometry/window_benchmark"),
    )
    p.add_argument("--kiss-bin", type=Path, default=Path("build/evaluation/kiss_pair_odometry"))
    p.add_argument(
        "--small-gicp-bin",
        type=Path,
        default=Path("build/evaluation/small_gicp_pair_odometry"),
    )
    p.add_argument("--feature-set", default="shared_no_method")
    p.add_argument("--reuse", action="store_true", help="Reuse existing JSON outputs when present.")
    p.add_argument(
        "--skip-selectors",
        action="store_true",
        help="Only generate candidates, residuals, diagnostics, and calibration.",
    )
    return p.parse_args()


def run(cmd: list[str | Path], *, reuse_output: Path | None = None, reuse: bool = False) -> None:
    if reuse and reuse_output is not None and reuse_output.is_file():
        print(f"[reuse] {reuse_output}")
        return
    printable = " ".join(str(part) for part in cmd)
    print(f"[run] {printable}")
    subprocess.run([str(part) for part in cmd], check=True)


def ensure_window_inputs(args: argparse.Namespace, window: Window) -> tuple[Path, Path]:
    source_pcd = args.pcd_root / f"kitti_seq_{window.sequence}_full"
    source_gt = args.gt_root / f"kitti_seq_{window.sequence}_full_gt.csv"
    if not source_pcd.is_dir():
        raise FileNotFoundError(source_pcd)
    if not source_gt.is_file():
        raise FileNotFoundError(source_gt)

    window_dir = args.output_dir / "windows" / window.key
    pcd_dir = window_dir / "pcd"
    gt_csv = window_dir / "gt.csv"
    pcd_dir.mkdir(parents=True, exist_ok=True)

    for local_idx in range(window.size):
        source_idx = window.start + local_idx
        src = source_pcd / f"{source_idx:08d}" / "cloud.pcd"
        if not src.is_file():
            raise FileNotFoundError(src)
        dst_dir = pcd_dir / f"{local_idx:08d}"
        dst_dir.mkdir(parents=True, exist_ok=True)
        dst = dst_dir / "cloud.pcd"
        if dst.exists() or dst.is_symlink():
            if dst.resolve() == src.resolve():
                continue
            dst.unlink()
        dst.symlink_to(src.resolve())

    with source_gt.open(newline="") as f:
        rows = list(csv.reader(f))
    header = rows[0]
    body = rows[1 + window.start : 1 + window.start + window.size]
    if len(body) != window.size:
        raise RuntimeError(f"{source_gt} has only {len(body)} rows for {window.key}")
    with gt_csv.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        for i, row in enumerate(body):
            out = list(row)
            if out:
                out[0] = str(i)
            writer.writerow(out)

    return pcd_dir, gt_csv


def window_paths(args: argparse.Namespace, window: Window) -> dict[str, Path]:
    out = args.output_dir / "results"
    return {
        "kiss": out / f"{window.key}_kiss_pair_gate2.json",
        "small_gicp": out / f"{window.key}_small_gicp_v0.75_c512.json",
        "shared": out / f"{window.key}_shared_residuals.json",
        "shared_md": out / f"{window.key}_shared_residuals.md",
        "diagnostic": out / f"selector_diagnostics_{window.key}.json",
        "diagnostic_md": out / f"selector_diagnostics_{window.key}.md",
        "selector_margin": out / f"selected_prior_margin_{window.key}.json",
        "selector_stability": out / f"selected_prior_stability_delta_{window.key}.json",
        "selector_stability_margin_safe": out / f"selected_prior_stability_margin_0p0010_{window.key}.json",
        "selector_stability_margin_mean": out / f"selected_prior_stability_margin_0p0025_{window.key}.json",
        "selector_stability_motion": out / f"selected_prior_stability_motion_0p0025_{window.key}.json",
        "selector_calibrated_full": out / f"selected_prior_calibrated_full_{window.key}.json",
        "selector_calibrated_holdout": out / f"selected_prior_calibrated_holdout_{window.key}.json",
        "holdout_calibration": out / f"pair_prior_quality_calibration_{args.feature_set}_{window.key}_holdout.json",
    }


def run_window_prereqs(args: argparse.Namespace, window: Window) -> dict[str, Path]:
    pcd_dir, gt_csv = ensure_window_inputs(args, window)
    paths = window_paths(args, window)
    paths["kiss"].parent.mkdir(parents=True, exist_ok=True)

    run([
        args.kiss_bin,
        pcd_dir,
        gt_csv,
        paths["kiss"],
        window.size,
        "--target-voxel-size",
        "0.75",
        "--source-voxel-size",
        "0.75",
        "--max-correspondence-distance",
        "1.5",
        "--max-step-translation",
        "2",
        "--max-step-yaw-deg",
        "6",
        "--max-iterations",
        "30",
        "--min-correspondences",
        "80",
    ], reuse_output=paths["kiss"], reuse=args.reuse)
    run([
        args.small_gicp_bin,
        pcd_dir,
        gt_csv,
        paths["small_gicp"],
        window.size,
        "--voxel-resolution",
        "0.75",
        "--max-correspondence-distance",
        "4",
        "--max-correspondences",
        "512",
        "--max-iterations",
        "20",
        "--max-step-translation",
        "2",
        "--max-step-yaw-deg",
        "6",
        "--k-neighbors",
        "12",
    ], reuse_output=paths["small_gicp"], reuse=args.reuse)

    run([
        sys.executable,
        "evaluation/scripts/evaluate_pair_prior_residuals.py",
        "--sequence-pcd-dir",
        pcd_dir,
        "--max-frames",
        window.size,
        "--candidate",
        f"kiss:{paths['kiss']}",
        "--candidate",
        f"small_gicp:{paths['small_gicp']}",
        "--output-json",
        paths["shared"],
        "--output-md",
        paths["shared_md"],
        "--trim-fraction",
        "0.50",
        "--normal-k",
        "12",
        "--min-linearity",
        "0.35",
        "--stability-translation-step",
        "0.20",
        "--stability-yaw-step-deg",
        "1.0",
        "--progress-every",
        "0",
    ], reuse_output=paths["shared"], reuse=args.reuse)

    run([
        sys.executable,
        "evaluation/scripts/diagnose_pair_prior_selector.py",
        "--gt-csv",
        gt_csv,
        "--candidate",
        f"kiss:{paths['kiss']}",
        "--candidate",
        f"small_gicp:{paths['small_gicp']}",
        "--shared-residual-json",
        paths["shared"],
        "--output-json",
        paths["diagnostic"],
        "--output-md",
        paths["diagnostic_md"],
    ], reuse_output=paths["diagnostic"], reuse=args.reuse)

    paths["pcd_dir"] = pcd_dir
    paths["gt_csv"] = gt_csv
    return paths


def write_holdout_calibrations(calibration_json: Path, windows: list[Window], path_sets: dict[str, dict[str, Path]]) -> None:
    payload = json.loads(calibration_json.read_text())
    rows_by_seq = {row["seq"]: row for row in payload["leave_one_sequence_out"]}
    for window in windows:
        row = rows_by_seq.get(window.key)
        if row is None:
            row = rows_by_seq[window.key.replace(f"_{window.size}", "")]
        out = {
            "diagnostics": payload["diagnostics"],
            "method_names": payload["method_names"],
            "feature_names": payload["feature_names"],
            "ridge": payload["ridge"],
            "feature_set": payload["feature_set"],
            "held_out_seq": row["seq"],
            "full_model": {
                "weights": row["weights"],
                "feature_mean": row["feature_mean"],
                "feature_scale": row["feature_scale"],
            },
        }
        path_sets[window.key]["holdout_calibration"].write_text(json.dumps(out, indent=2) + "\n")


def run_selector(
    args: argparse.Namespace,
    window: Window,
    paths: dict[str, Path],
    policy: str,
    output: Path,
    calibration: Path | None = None,
    score_margin: float | None = None,
) -> None:
    cmd: list[str | Path] = [
        sys.executable,
        "evaluation/scripts/select_pair_prior.py",
        "--sequence-pcd-dir",
        paths["pcd_dir"],
        "--gt-csv",
        paths["gt_csv"],
        "--max-frames",
        window.size,
        "--candidate",
        f"kiss:{paths['kiss']}",
        "--candidate",
        f"small_gicp:{paths['small_gicp']}",
        "--shared-residual-json",
        paths["shared"],
        "--policy",
        policy,
        "--output-json",
        output,
        "--progress-every",
        "0",
    ]
    if policy in (
        "preferred_with_cubemap_margin",
        "preferred_with_stability_margin",
        "preferred_with_stability_motion_consistency",
    ):
        margin = 0.01 if score_margin is None else score_margin
        cmd.extend(["--preferred-name", "small_gicp", "--score-margin", str(margin)])
    if calibration is not None:
        cmd.extend(["--calibration-json", calibration])
    run(cmd, reuse_output=output, reuse=args.reuse)


def metrics_from(path: Path) -> tuple[float | None, float | None]:
    payload = json.loads(path.read_text())
    metrics = payload.get("metrics") or {}
    return metrics.get("ate_xy_m"), metrics.get("step_length_rmse_m")


def selected_counts(path: Path) -> dict[str, int]:
    payload = json.loads(path.read_text())
    counts: dict[str, int] = {}
    for pair in payload.get("pairs", []):
        name = pair.get("selected")
        if name:
            counts[name] = counts.get(name, 0) + 1
    return counts


POLICIES = [
    ("kiss", "kiss", "KISS"),
    ("small_gicp", "small_gicp", "SmallGICP"),
    ("margin", "selector_margin", "Cubemap margin"),
    ("stability_delta", "selector_stability", "Stability delta"),
    ("stability_margin_safe", "selector_stability_margin_safe", "Stability margin 0.001"),
    ("stability_margin_mean", "selector_stability_margin_mean", "Stability margin 0.0025"),
    ("stability_motion", "selector_stability_motion", "Stability motion 0.0025"),
    ("calibrated_full", "selector_calibrated_full", "Calibrated full"),
    ("calibrated_holdout", "selector_calibrated_holdout", "Calibrated holdout"),
]


def summarize_aggregate(rows: list[dict]) -> list[dict]:
    wins = {label: 0 for label, _, _ in POLICIES}
    for row in rows:
        present = [
            label for label, _, _ in POLICIES
            if row.get(f"{label}_ate_xy_m") is not None
        ]
        if not present:
            continue
        best = min(present, key=lambda label: row[f"{label}_ate_xy_m"])
        wins[best] += 1

    aggregate = []
    for label, _, title in POLICIES:
        ate = [
            float(row[f"{label}_ate_xy_m"])
            for row in rows
            if row.get(f"{label}_ate_xy_m") is not None
        ]
        step = [
            float(row[f"{label}_step_rmse_m"])
            for row in rows
            if row.get(f"{label}_step_rmse_m") is not None
        ]
        if not ate:
            continue
        aggregate.append({
            "policy": label,
            "title": title,
            "mean_ate_xy_m": statistics.mean(ate),
            "median_ate_xy_m": statistics.median(ate),
            "mean_step_rmse_m": statistics.mean(step),
            "max_ate_xy_m": max(ate),
            "window_wins": wins[label],
        })
    return aggregate


def summarize(args: argparse.Namespace, windows: list[Window], path_sets: dict[str, dict[str, Path]], calibration_json: Path) -> None:
    rows = []
    for window in windows:
        paths = path_sets[window.key]
        row = {"window": window.key}
        for label, path_key, _title in POLICIES:
            if paths[path_key].is_file():
                ate, step = metrics_from(paths[path_key])
                row[f"{label}_ate_xy_m"] = ate
                row[f"{label}_step_rmse_m"] = step
                if path_key.startswith("selector_"):
                    row[f"{label}_counts"] = selected_counts(paths[path_key])
        rows.append(row)
    aggregate = summarize_aggregate(rows)

    summary_json = args.output_dir / "window_benchmark_summary.json"
    summary_md = args.output_dir / "window_benchmark_summary.md"
    summary_json.write_text(json.dumps({
        "feature_set": args.feature_set,
        "calibration_json": str(calibration_json),
        "aggregate": aggregate,
        "windows": rows,
    }, indent=2) + "\n")

    lines = [
        "# Pair Prior Window Benchmark",
        "",
        f"- Feature set: `{args.feature_set}`",
        f"- Calibration: `{calibration_json}`",
        "",
        "## Aggregate",
        "",
        "| Policy | Mean ATE [m] | Median ATE [m] | Mean step RMSE [m] | Max ATE [m] | Window wins |",
        "| --- | ---: | ---: | ---: | ---: | ---: |",
    ]
    for row in aggregate:
        lines.append(
            f"| {row['title']} | {row['mean_ate_xy_m']:.3f} | "
            f"{row['median_ate_xy_m']:.3f} | {row['mean_step_rmse_m']:.3f} | "
            f"{row['max_ate_xy_m']:.3f} | {row['window_wins']} |"
        )
    lines.extend([
        "",
        "## Windows",
        "",
        "| Window | KISS | SmallGICP | Cubemap margin | Stability delta | Stability margin 0.001 | Stability margin 0.0025 | Stability motion 0.0025 | Calibrated full | Calibrated holdout |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ])
    for row in rows:
        def cell(prefix: str) -> str:
            ate = row.get(f"{prefix}_ate_xy_m")
            step = row.get(f"{prefix}_step_rmse_m")
            if ate is None or step is None:
                return "-"
            return f"{ate:.3f} / {step:.3f}"

        lines.append(
            f"| {row['window']} | {cell('kiss')} | {cell('small_gicp')} | "
            f"{cell('margin')} | {cell('stability_delta')} | "
            f"{cell('stability_margin_safe')} | {cell('stability_margin_mean')} | "
            f"{cell('stability_motion')} | "
            f"{cell('calibrated_full')} | {cell('calibrated_holdout')} |"
        )
    summary_md.write_text("\n".join(lines) + "\n")
    print(f"[done] summary -> {summary_md}")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    sequences = args.sequence or ["02", "05", "08"]
    starts = args.start or [0, 30, 60, 90]
    windows = [
        Window(sequence=sequence, start=start, size=args.window_size)
        for sequence in sequences
        for start in starts
    ]
    windows = list(dict.fromkeys(windows))

    path_sets: dict[str, dict[str, Path]] = {}
    for window in windows:
        path_sets[window.key] = run_window_prereqs(args, window)

    calibration_json = args.output_dir / "results" / f"pair_prior_quality_calibration_{args.feature_set}_loo.json"
    calibration_md = args.output_dir / "results" / f"pair_prior_quality_calibration_{args.feature_set}_loo.md"
    cmd: list[str | Path] = [
        sys.executable,
        "evaluation/scripts/calibrate_pair_prior_quality.py",
        "--feature-set",
        args.feature_set,
        "--output-json",
        calibration_json,
        "--output-md",
        calibration_md,
    ]
    for window in windows:
        cmd.extend(["--diagnostic", path_sets[window.key]["diagnostic"]])
    run(cmd, reuse_output=calibration_json, reuse=args.reuse)

    if not args.skip_selectors:
        write_holdout_calibrations(calibration_json, windows, path_sets)
        for window in windows:
            paths = path_sets[window.key]
            run_selector(
                args,
                window,
                paths,
                "preferred_with_cubemap_margin",
                paths["selector_margin"],
            )
            run_selector(
                args,
                window,
                paths,
                "shared_stability_delta",
                paths["selector_stability"],
            )
            run_selector(
                args,
                window,
                paths,
                "preferred_with_stability_margin",
                paths["selector_stability_margin_safe"],
                score_margin=0.001,
            )
            run_selector(
                args,
                window,
                paths,
                "preferred_with_stability_margin",
                paths["selector_stability_margin_mean"],
                score_margin=0.0025,
            )
            run_selector(
                args,
                window,
                paths,
                "preferred_with_stability_motion_consistency",
                paths["selector_stability_motion"],
                score_margin=0.0025,
            )
            run_selector(
                args,
                window,
                paths,
                "calibrated_quality",
                paths["selector_calibrated_full"],
                calibration_json,
            )
            run_selector(
                args,
                window,
                paths,
                "calibrated_quality",
                paths["selector_calibrated_holdout"],
                paths["holdout_calibration"],
            )

    summarize(args, windows, path_sets, calibration_json)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
