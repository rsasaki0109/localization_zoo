#!/usr/bin/env python3
"""Accuracy of the SDK's orientation_wxyz on the BROAD benchmark.

Replays every BROAD trial (39 trials, optical motion capture ground truth,
CC BY 4.0) through imu_motion_health_cli and scores the per-row
orientation_wxyz with BROAD's inclination error (RMS over the movement phases,
see papers/attitude_estimation). The SDK uses no magnetometer, so heading is
unobservable and only inclination is scored; it is invariant to the arbitrary
heading of the SDK's world frame.

Prerequisite (BROAD CSVs):
  python3 papers/attitude_estimation/evaluation/broad_benchmark.py fetch
  python3 papers/attitude_estimation/evaluation/broad_benchmark.py convert
Then:
  python3 papers/imu_motion_health/evaluation/broad_attitude.py
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import pathlib
import subprocess
import sys
import tempfile

import numpy as np

VARIANTS = {
    "default": [],
    "wearable": ["--profile", "wearable"],
    "default_no_stationary_bias_learning": ["--stationary-bias-gain", "0"],
    "vqf_attitude": ["--vqf-attitude"],
}


def inclination_rmse_deg(estimate: np.ndarray, reference: np.ndarray, movement: np.ndarray) -> float:
    est = estimate / np.linalg.norm(estimate, axis=1)[:, None]
    ref = reference / np.linalg.norm(reference, axis=1)[:, None]
    w1, x1, y1, z1 = est.T
    w2, x2, y2, z2 = (ref * np.array([1, -1, -1, -1])).T  # conjugate
    w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2
    z = w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2
    error = 2 * np.arccos(np.clip(np.sqrt(w ** 2 + z ** 2), 0, 1))
    valid = movement & np.isfinite(reference).all(axis=1)
    return float(np.degrees(np.sqrt(np.mean(error[valid] ** 2))))


def run_trial(cli: str, csv_path: pathlib.Path, rate: float, args: list[str]) -> float:
    data = np.loadtxt(csv_path, delimiter=",", skiprows=1)
    times = np.arange(len(data)) / rate
    with tempfile.TemporaryDirectory() as tmp:
        sdk_input = pathlib.Path(tmp) / "imu.csv"
        np.savetxt(sdk_input, np.column_stack([times, data[:, 0:3], data[:, 3:6]]), delimiter=",",
                   header="timestamp,gx,gy,gz,ax,ay,az", comments="", fmt="%.9f")
        out = subprocess.run([cli, "--input", str(sdk_input), "--jsonl-output", "-",
                              "--summary-output", str(pathlib.Path(tmp) / "summary.json"), *args],
                             capture_output=True, text=True, check=True).stdout
    quats = np.array([json.loads(line)["orientation_wxyz"] for line in out.splitlines() if line.strip()], float)
    return inclination_rmse_deg(quats, data[:, 9:13], data[:, 13].astype(bool))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--broad-csv", type=pathlib.Path, default=pathlib.Path("dogfooding_results/broad/csv"))
    parser.add_argument("--cli", default="build/papers/imu_motion_health/imu_motion_health_cli")
    parser.add_argument("--out", type=pathlib.Path,
                        default=pathlib.Path("experiments/results/imu_motion_health_broad_attitude.json"))
    parser.add_argument("--jobs", type=int, default=8)
    args = parser.parse_args()

    trials = json.loads((args.broad_csv / "trials.json").read_text())["trials"]
    results = {"dataset": "BROAD (Laidig et al., Data 2021, CC BY 4.0), 39 trials",
               "metric": "inclination RMSE over movement phases [deg], mean over trials",
               "variants": {}}
    for name, extra in VARIANTS.items():
        with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
            futures = {t: pool.submit(run_trial, args.cli, args.broad_csv / f"{t}.csv",
                                      info["sampling_rate"], extra) for t, info in trials.items()}
            per_trial = {t: round(f.result(), 4) for t, f in futures.items()}
        values = list(per_trial.values())
        results["variants"][name] = {"args": extra, "mean": round(float(np.mean(values)), 3),
                                     "median": round(float(np.median(values)), 3),
                                     "max": round(float(np.max(values)), 3), "trials": per_trial}
        print(f"{name:38s} mean {np.mean(values):7.3f}  median {np.median(values):7.3f}  max {np.max(values):7.2f}")
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(results, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
