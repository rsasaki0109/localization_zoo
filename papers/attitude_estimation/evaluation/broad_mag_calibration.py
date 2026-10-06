#!/usr/bin/env python3
"""Check the magnetometer calibration on BROAD with a known distortion.

BROAD's magnetometer is already calibrated (the field norm varies by ~3 %).
This script applies a fixed, known hard/soft-iron distortion
m' = A m + b to every trial, fits the calibration from ONE recording (trial 01,
undisturbed slow rotation, i.e. a calibration rotation), applies it to all 39
trials, and compares the VQF 9D total error (BROAD metric) for the original,
distorted, and calibrated magnetometer. It then repeats the calibrated case
end to end through imu_motion_health_cli with the profile keys the
calibration tool writes.

Prerequisite: broad_benchmark.py fetch + convert, and the built CLIs.
"""

from __future__ import annotations

import concurrent.futures
import json
import pathlib
import subprocess
import sys
import tempfile

import numpy as np

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / "papers/imu_motion_health/evaluation"))
import broad_attitude as SDK  # noqa: E402

CSV = pathlib.Path("dogfooding_results/broad/csv")
WORK = pathlib.Path("dogfooding_results/broad/mag_calibration")
ATTITUDE_CLI = "build/papers/attitude_estimation/attitude_benchmark_cli"
CAL_CLI = "build/papers/attitude_estimation/magnetometer_calibration_cli"
SDK_CLI = "build/papers/imu_motion_health/imu_motion_health_cli"
CALIBRATION_TRIAL = "01_undisturbed_slow_rotation_A"
# Known distortion: symmetric soft iron (scale 0.9-1.15, cross terms up to
# 0.08) and a 31 uT hard-iron offset, typical of a sensor near a battery/PCB.
A = np.array([[1.15, 0.08, -0.05], [0.08, 0.90, 0.03], [-0.05, 0.03, 1.05]])
B = np.array([25.0, -15.0, 10.0])


def write_variant(name: str, kind: str, calibration: dict | None) -> pathlib.Path:
    data = np.loadtxt(CSV / f"{name}.csv", delimiter=",", skiprows=1)
    mag = data[:, 6:9]
    if kind in ("distorted", "calibrated"):
        mag = mag @ A.T + B
    if kind == "calibrated":
        W, b = np.array(calibration["matrix"]), np.array(calibration["offset"])
        mag = (mag - b) @ W.T
    out = WORK / kind / f"{name}.csv"
    out.parent.mkdir(parents=True, exist_ok=True)
    data = data.copy()
    data[:, 6:9] = mag
    np.savetxt(out, data, delimiter=",", header="gx,gy,gz,ax,ay,az,mx,my,mz,qw,qx,qy,qz,movement",
               comments="", fmt="%.17g")
    return out


def vqf_total(csv_path: pathlib.Path, rate: float) -> float:
    out = subprocess.run([ATTITUDE_CLI, str(csv_path), "--rate", repr(rate), "--method", "vqf", "--mode", "9d"],
                         capture_output=True, text=True, check=True).stdout
    return json.loads(out)["total_rmse_deg"]


def main() -> int:
    trials = json.loads((CSV / "trials.json").read_text())["trials"]
    # 1. Fit the calibration from the distorted calibration rotation only.
    calib_csv = write_variant(CALIBRATION_TRIAL, "distorted", None)
    with tempfile.TemporaryDirectory() as tmp:
        profile = pathlib.Path(tmp) / "mag.yaml"
        fit = json.loads(subprocess.run([CAL_CLI, str(calib_csv), "--columns", "6,7,8", "--profile-yaml",
                                         str(profile)], capture_output=True, text=True, check=True).stdout)
        profile_text = profile.read_text()
    W = np.array(fit["matrix"])
    # The fit recovers A^-1 up to the field-norm scale and b exactly.
    scale = np.linalg.det(np.linalg.inv(W)) ** (1 / 3) / np.linalg.det(A) ** (1 / 3)
    result = {
        "distortion": {"A": A.tolist(), "b": B.tolist()},
        "calibration_trial": CALIBRATION_TRIAL,
        "fit": fit,
        "offset_error_uT": float(np.linalg.norm(np.array(fit["offset"]) - B)),
        "soft_iron_error": float(np.abs(np.linalg.inv(W) / scale - A).max()),
        "vqf_9d_total_rmse_deg": {},
    }
    # 2. VQF 9D on all trials for each magnetometer variant.
    for kind in ("original", "distorted", "calibrated"):
        with concurrent.futures.ThreadPoolExecutor(8) as pool:
            futures = {n: pool.submit(lambda n=n: vqf_total(write_variant(n, kind, fit), trials[n]["sampling_rate"]))
                       for n in trials}
            values = {n: f.result() for n, f in futures.items()}
        result["vqf_9d_total_rmse_deg"][kind] = {"mean": round(float(np.mean(list(values.values()))), 3),
                                                 "max": round(float(np.max(list(values.values()))), 3)}
        print(f"VQF 9D {kind:10s} mean {np.mean(list(values.values())):7.3f}  max {np.max(list(values.values())):7.2f}")
    # 3. End to end through the SDK: distorted magnetometer + calibration profile.
    keys = {line.split(":")[0].strip(): float(line.split(":")[1])
            for line in profile_text.splitlines() if line.startswith("  mag_")}
    with concurrent.futures.ThreadPoolExecutor(8) as pool:
        futures = {n: pool.submit(SDK.run_trial, SDK_CLI, WORK / "distorted" / f"{n}.csv", trials[n]["sampling_rate"],
                                  ["--with-mag"], keys, True) for n in trials}
        sdk = [f.result() for f in futures.values()]
    result["sdk_9d_total_rmse_deg_calibrated"] = round(float(np.mean(sdk)), 3)
    print(f"SDK 9D distorted + calibration profile: mean {np.mean(sdk):.3f}")
    print(f"fit: offset error {result['offset_error_uT']:.3f} uT, soft-iron error {result['soft_iron_error']:.4f}, "
          f"coverage {fit['direction_bins']}/26, residual {fit['rms_residual']:.3f} uT")
    out = ROOT / "experiments/results/broad_mag_calibration.json"
    out.write_text(json.dumps(result, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
