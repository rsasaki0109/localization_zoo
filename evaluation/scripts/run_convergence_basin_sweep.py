#!/usr/bin/env python3
"""Run GT-seed perturbation sweeps against pcd_dogfooding."""

from __future__ import annotations

import argparse
import csv
import itertools
import json
import shlex
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]

PROFILE_ARGS = {
    "none": [],
    "ndt_t1": ["--ndt-resolution", "0.5", "--ndt-max-iterations", "12"],
    "ndt_fast": ["--ndt-fast-profile"],
    "ndt_dense": ["--ndt-dense-profile"],
    "litamin2_paper": ["--litamin2-paper-profile"],
    "small_gicp_fast": ["--small-gicp-fast-profile"],
    "small_gicp_dense": ["--small-gicp-dense-profile"],
    "voxel_gicp_fast": ["--voxel-gicp-fast-profile"],
    "voxel_gicp_dense": ["--voxel-gicp-dense-profile"],
}


def parse_float_list(value: str) -> list[float]:
    items = [item.strip() for item in value.split(",") if item.strip()]
    if not items:
        raise argparse.ArgumentTypeError("expected a comma-separated float list")
    return [float(item) for item in items]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Sweep local-frame GT-seed perturbations and aggregate "
            "pcd_dogfooding summary JSON metrics."
        )
    )
    parser.add_argument(
        "--binary",
        default="build/evaluation/pcd_dogfooding",
        help="pcd_dogfooding binary path relative to repo root unless absolute.",
    )
    parser.add_argument(
        "--pcd-dir",
        default="dogfooding_results/kitti_seq_07_108",
        help="PCD frame directory relative to repo root unless absolute.",
    )
    parser.add_argument(
        "--gt-csv",
        default="experiments/reference_data/kitti_seq_07_108_gt.csv",
        help="GT CSV relative to repo root unless absolute.",
    )
    parser.add_argument("--methods", default="ndt", help="pcd_dogfooding method selector.")
    parser.add_argument(
        "--profile",
        choices=sorted(PROFILE_ARGS),
        default="ndt_t1",
        help="Convenience argument bundle for the selected method.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=108,
        help="Maximum frames passed to pcd_dogfooding; use <=0 for all frames.",
    )
    parser.add_argument(
        "--x-values",
        type=parse_float_list,
        default=parse_float_list("0,0.5,1,2,5"),
        help="Comma-separated local-frame x perturbations in meters.",
    )
    parser.add_argument(
        "--y-values",
        type=parse_float_list,
        default=parse_float_list("0"),
        help="Comma-separated local-frame y perturbations in meters.",
    )
    parser.add_argument(
        "--z-values",
        type=parse_float_list,
        default=parse_float_list("0"),
        help="Comma-separated local-frame z perturbations in meters.",
    )
    parser.add_argument(
        "--yaw-values",
        type=parse_float_list,
        default=parse_float_list("0,2,5,10,20"),
        help="Comma-separated local-frame yaw perturbations in degrees.",
    )
    parser.add_argument(
        "--success-ate-threshold",
        type=float,
        default=0.5,
        help="ATE threshold in meters used for the aggregate success flag.",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=float,
        default=None,
        help="Optional wall-clock timeout per perturbation run.",
    )
    parser.add_argument(
        "--output-dir",
        default="experiments/results/convergence_basin",
        help="Directory for aggregate and per-run artifacts.",
    )
    parser.add_argument(
        "--run-id",
        default=None,
        help="Run id directory name. Defaults to UTC timestamp.",
    )
    parser.add_argument(
        "--extra-args",
        action="append",
        default=[],
        help="Extra pcd_dogfooding args, shell-split. Can be passed multiple times.",
    )
    parser.add_argument(
        "--reuse-existing",
        action="store_true",
        help="Reuse existing summary.json files instead of rerunning variants.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print commands and write no outputs.",
    )
    return parser.parse_args()


def resolve_path(path_text: str) -> Path:
    path = Path(path_text)
    if path.is_absolute():
        return path
    return REPO_ROOT / path


def relpath(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def flatten_extra_args(extra_args: list[str]) -> list[str]:
    flattened: list[str] = []
    for item in extra_args:
        flattened.extend(shlex.split(item))
    return flattened


def variant_id(x: float, y: float, z: float, yaw_deg: float) -> str:
    def fmt(value: float, suffix: str) -> str:
        text = f"{value:+.3f}".replace("+", "p").replace("-", "m").replace(".", "p")
        return f"{suffix}{text}"

    return "_".join([fmt(x, "x"), fmt(y, "y"), fmt(z, "z"), fmt(yaw_deg, "yaw")])


def load_summary(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def method_rows(summary: dict[str, Any], threshold: float) -> list[dict[str, Any]]:
    rows = []
    for method in summary.get("methods", []):
        status = str(method.get("status", "ok"))
        ate_m = method.get("ate_m")
        success = status == "ok" and ate_m is not None and float(ate_m) <= threshold
        rows.append(
            {
                "name": method.get("name"),
                "status": status,
                "ate_m": ate_m,
                "rpe_trans_pct": method.get("rpe_trans_pct"),
                "rpe_rot_deg_per_m": method.get("rpe_rot_deg_per_m"),
                "frames": method.get("frames"),
                "time_ms": method.get("time_ms"),
                "fps": method.get("fps"),
                "success": success,
                "note": method.get("note", ""),
            }
        )
    return rows


def summarize_methods(runs: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    by_method: dict[str, dict[str, Any]] = {}
    for run in runs:
        for method in run["methods"]:
            name = str(method["name"])
            bucket = by_method.setdefault(
                name,
                {
                    "runs": 0,
                    "successes": 0,
                    "best_ate_m": None,
                    "worst_success_ate_m": None,
                    "max_success_abs_x_m": None,
                    "max_success_abs_yaw_deg": None,
                },
            )
            bucket["runs"] += 1
            ate_m = method.get("ate_m")
            if ate_m is not None:
                ate_f = float(ate_m)
                if bucket["best_ate_m"] is None or ate_f < bucket["best_ate_m"]:
                    bucket["best_ate_m"] = ate_f
            if method.get("success"):
                bucket["successes"] += 1
                x_abs = abs(float(run["perturbation"]["x_m"]))
                yaw_abs = abs(float(run["perturbation"]["yaw_deg"]))
                if (
                    bucket["max_success_abs_x_m"] is None
                    or x_abs > bucket["max_success_abs_x_m"]
                ):
                    bucket["max_success_abs_x_m"] = x_abs
                if (
                    bucket["max_success_abs_yaw_deg"] is None
                    or yaw_abs > bucket["max_success_abs_yaw_deg"]
                ):
                    bucket["max_success_abs_yaw_deg"] = yaw_abs
                if (
                    ate_m is not None
                    and (
                        bucket["worst_success_ate_m"] is None
                        or float(ate_m) > bucket["worst_success_ate_m"]
                    )
                ):
                    bucket["worst_success_ate_m"] = float(ate_m)
    return by_method


def write_csv(path: Path, runs: list[dict[str, Any]]) -> None:
    fieldnames = [
        "variant_id",
        "x_m",
        "y_m",
        "z_m",
        "yaw_deg",
        "method",
        "status",
        "success",
        "ate_m",
        "rpe_trans_pct",
        "rpe_rot_deg_per_m",
        "frames",
        "time_ms",
        "fps",
        "summary_path",
        "log_path",
    ]
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for run in runs:
            perturb = run["perturbation"]
            for method in run["methods"]:
                writer.writerow(
                    {
                        "variant_id": run["id"],
                        "x_m": perturb["x_m"],
                        "y_m": perturb["y_m"],
                        "z_m": perturb["z_m"],
                        "yaw_deg": perturb["yaw_deg"],
                        "method": method.get("name"),
                        "status": method.get("status"),
                        "success": method.get("success"),
                        "ate_m": method.get("ate_m"),
                        "rpe_trans_pct": method.get("rpe_trans_pct"),
                        "rpe_rot_deg_per_m": method.get("rpe_rot_deg_per_m"),
                        "frames": method.get("frames"),
                        "time_ms": method.get("time_ms"),
                        "fps": method.get("fps"),
                        "summary_path": run["summary_path"],
                        "log_path": run["log_path"],
                    }
                )


def main() -> int:
    args = parse_args()
    binary = resolve_path(args.binary)
    pcd_dir = resolve_path(args.pcd_dir)
    gt_csv = resolve_path(args.gt_csv)
    run_id = args.run_id or datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    output_dir = resolve_path(args.output_dir) / run_id
    extra_args = flatten_extra_args(args.extra_args)

    base_cmd = [
        str(binary),
        str(pcd_dir),
        str(gt_csv),
    ]
    if args.max_frames > 0:
        base_cmd.append(str(args.max_frames))
    base_cmd.extend(["--methods", args.methods])
    base_cmd.extend(PROFILE_ARGS[args.profile])
    base_cmd.extend(extra_args)

    planned = list(
        itertools.product(args.x_values, args.y_values, args.z_values, args.yaw_values)
    )
    if args.dry_run:
        for x, y, z, yaw_deg in planned:
            cmd = base_cmd + [
                "--seed-perturb-x",
                str(x),
                "--seed-perturb-y",
                str(y),
                "--seed-perturb-z",
                str(z),
                "--seed-perturb-yaw-deg",
                str(yaw_deg),
                "--summary-json",
                "<variant>/summary.json",
            ]
            print(shlex.join(cmd))
        return 0

    output_dir.mkdir(parents=True, exist_ok=True)
    runs: list[dict[str, Any]] = []
    for index, (x, y, z, yaw_deg) in enumerate(planned, start=1):
        vid = variant_id(x, y, z, yaw_deg)
        variant_dir = output_dir / vid
        variant_dir.mkdir(parents=True, exist_ok=True)
        summary_path = variant_dir / "summary.json"
        log_path = variant_dir / "run.log"
        cmd = base_cmd + [
            "--seed-perturb-x",
            str(x),
            "--seed-perturb-y",
            str(y),
            "--seed-perturb-z",
            str(z),
            "--seed-perturb-yaw-deg",
            str(yaw_deg),
            "--summary-json",
            str(summary_path),
        ]
        print(f"[{index}/{len(planned)}] {vid}")
        if not (args.reuse_existing and summary_path.exists()):
            timed_out = False
            try:
                completed = subprocess.run(
                    cmd,
                    cwd=REPO_ROOT,
                    text=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    timeout=args.timeout_seconds,
                    check=False,
                )
                output = completed.stdout
                returncode = completed.returncode
            except subprocess.TimeoutExpired as exc:
                timed_out = True
                output = exc.stdout or ""
                returncode = -1
                if isinstance(output, bytes):
                    output = output.decode(errors="replace")
                output += f"\nTIMEOUT after {args.timeout_seconds} seconds\n"
            log_path.write_text(output)
            if returncode != 0:
                status = "timeout_budget" if timed_out else f"runner_exit_{returncode}"
                runs.append(
                    {
                        "id": vid,
                        "perturbation": {
                            "x_m": x,
                            "y_m": y,
                            "z_m": z,
                            "yaw_deg": yaw_deg,
                        },
                        "command": shlex.join(cmd),
                        "summary_path": relpath(summary_path),
                        "log_path": relpath(log_path),
                        "methods": [
                            {
                                "name": args.methods,
                                "status": status,
                                "ate_m": None,
                                "success": False,
                                "note": "pcd_dogfooding exited non-zero; see log.",
                            }
                        ],
                    }
                )
                continue
        summary = load_summary(summary_path)
        runs.append(
            {
                "id": vid,
                "perturbation": {
                    "x_m": x,
                    "y_m": y,
                    "z_m": z,
                    "yaw_deg": yaw_deg,
                },
                "command": shlex.join(cmd),
                "summary_path": relpath(summary_path),
                "log_path": relpath(log_path),
                "methods": method_rows(summary, args.success_ate_threshold),
            }
        )

    aggregate = {
        "schema_version": 1,
        "created_at": datetime.now(timezone.utc).isoformat(),
        "run_id": run_id,
        "dataset": {
            "pcd_dir": relpath(pcd_dir),
            "gt_csv": relpath(gt_csv),
            "max_frames": args.max_frames,
        },
        "settings": {
            "methods": args.methods,
            "profile": args.profile,
            "success_ate_threshold_m": args.success_ate_threshold,
            "x_values_m": args.x_values,
            "y_values_m": args.y_values,
            "z_values_m": args.z_values,
            "yaw_values_deg": args.yaw_values,
            "extra_args": extra_args,
        },
        "method_summaries": summarize_methods(runs),
        "runs": runs,
    }
    aggregate_path = output_dir / "aggregate.json"
    csv_path = output_dir / "runs.csv"
    aggregate_path.write_text(json.dumps(aggregate, indent=2) + "\n")
    write_csv(csv_path, runs)

    print(f"Wrote {relpath(aggregate_path)}")
    print(f"Wrote {relpath(csv_path)}")
    for name, summary in aggregate["method_summaries"].items():
        print(
            f"{name}: {summary['successes']}/{summary['runs']} success, "
            f"best ATE={summary['best_ate_m']}, "
            f"max |x| success={summary['max_success_abs_x_m']}, "
            f"max |yaw| success={summary['max_success_abs_yaw_deg']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
