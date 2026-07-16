#!/usr/bin/env python3

"""Run the common Hard PCL window-odometry matrix and attach GT metrics."""

from __future__ import annotations

import argparse
import concurrent.futures
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Run:
    name: str
    binary: str
    output_name: str
    extra_args: tuple[str, ...] = ()


RUNS = (
    Run("kiss_keyframe", "kiss_keyframe_odometry", "kiss_keyframe.json"),
    Run("litamin2", "litamin2_window_odometry", "litamin2.json"),
    Run("ct_icp", "ct_icp_window_odometry", "ct_icp.json"),
    Run("xicp", "xicp_window_odometry", "xicp.json"),
    Run("degen_sense_imu", "degen_sense_window_odometry", "degen_sense_imu.json"),
    Run(
        "degen_sense_no_imu",
        "degen_sense_window_odometry",
        "degen_sense_no_imu.json",
        ("--no-imu",),
    ),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pcd-dir", type=Path, required=True)
    parser.add_argument("--gt-csv", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--build-dir", type=Path, default=Path("build/evaluation"))
    parser.add_argument("--max-frames", type=int, default=-1)
    parser.add_argument("--start-frame", type=int, default=0)
    parser.add_argument(
        "--methods",
        default=",".join(run.name for run in RUNS),
        help="Comma-separated run names.",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=1,
        help="Concurrent runners. Use 1 for stable timing and up to 6 for scouting.",
    )
    return parser.parse_args()


def run_one(
    run: Run,
    *,
    pcd_dir: Path,
    gt_csv: Path,
    output_dir: Path,
    build_dir: Path,
    max_frames: int,
    start_frame: int,
) -> tuple[str, float]:
    output_path = output_dir / run.output_name
    log_path = output_path.with_suffix(output_path.suffix + ".log")
    command = [
        str(build_dir / run.binary),
        str(pcd_dir),
        str(gt_csv),
        str(output_path),
        str(max_frames),
    ]
    if start_frame:
        command.extend(("--start-frame", str(start_frame)))
    command.extend(run.extra_args)

    started = time.monotonic()
    with log_path.open("w") as log:
        completed = subprocess.run(
            command,
            stdout=log,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
    elapsed = time.monotonic() - started
    if completed.returncode != 0:
        raise RuntimeError(
            f"{run.name} failed with exit code {completed.returncode}; see {log_path}"
        )

    evaluator = Path(__file__).with_name("evaluate_window_odometry_gt.py")
    eval_command = [
        sys.executable,
        str(evaluator),
        "--result",
        str(output_path),
        "--gt-csv",
        str(gt_csv),
        "--start-frame",
        str(start_frame),
    ]
    subprocess.run(eval_command, check=True)
    return run.name, elapsed


def main() -> int:
    args = parse_args()
    if args.jobs <= 0:
        raise ValueError("--jobs must be positive")
    selected_names = [name.strip() for name in args.methods.split(",") if name.strip()]
    known = {run.name: run for run in RUNS}
    unknown = sorted(set(selected_names).difference(known))
    if unknown:
        raise ValueError(
            f"Unknown methods: {', '.join(unknown)}; choose from {', '.join(known)}"
        )
    if not selected_names:
        raise ValueError("--methods selected no runs")
    for path, label in (
        (args.pcd_dir, "PCD directory"),
        (args.gt_csv, "GT CSV"),
        (args.build_dir, "build directory"),
    ):
        if not path.exists():
            raise FileNotFoundError(f"{label} not found: {path}")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    selected = [known[name] for name in selected_names]
    kwargs = {
        "pcd_dir": args.pcd_dir,
        "gt_csv": args.gt_csv,
        "output_dir": args.output_dir,
        "build_dir": args.build_dir,
        "max_frames": args.max_frames,
        "start_frame": args.start_frame,
    }
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
        futures = {executor.submit(run_one, run, **kwargs): run for run in selected}
        for future in concurrent.futures.as_completed(futures):
            run = futures[future]
            try:
                name, elapsed = future.result()
            except Exception:
                for pending in futures:
                    pending.cancel()
                raise
            print(f"[done] {name} wall_runtime_s={elapsed:.3f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
