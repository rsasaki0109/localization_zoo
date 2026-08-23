#!/usr/bin/env python3
"""Generate deterministic stationary/walking/vehicle IMU validation sessions."""

from __future__ import annotations

import argparse
import csv
import json
import math
import pathlib
import subprocess
import sys


def write_session(path: pathlib.Path, activity: str, temperature: float, seconds: float = 3.0) -> None:
    rate = 100.0
    count = int(seconds * rate) + 1
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(("timestamp", "gx", "gy", "gz", "ax", "ay", "az", "temperature_c"))
        for index in range(count):
            t = index / rate
            temp = temperature + 0.2 * t / seconds
            bias = (0.012 + 0.0008 * (temp - 25.0), -0.008 - 0.0005 * (temp - 25.0), 0.018 + 0.001 * (temp - 25.0))
            if activity == "stationary":
                gyro = tuple(bias[i] + 0.0005 * math.sin(2.0 * math.pi * (i + 1) * t) for i in range(3))
                accel = (0.03, -0.02, 9.84665 + 0.002 * math.sin(2.0 * math.pi * t))
            elif activity == "walking":
                gyro = (bias[0] + 0.15 * math.sin(2 * math.pi * 1.8 * t), bias[1], bias[2] + 0.08 * math.cos(2 * math.pi * 1.8 * t))
                accel = (0.4 * math.sin(2 * math.pi * 1.8 * t), -0.02, 9.84665 + 1.0 * math.sin(2 * math.pi * 3.6 * t))
            else:
                gyro = (bias[0], bias[1] + 0.04 * math.sin(2 * math.pi * 0.7 * t), bias[2] + 0.06 * math.sin(2 * math.pi * 0.3 * t))
                accel = (0.8 * math.sin(2 * math.pi * 0.4 * t), 0.1 * math.sin(2 * math.pi * 7 * t), 9.84665)
            writer.writerow([f"{value:.9f}" for value in (t, *gyro, *accel, temp)])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", required=True, type=pathlib.Path)
    parser.add_argument("--cli", type=pathlib.Path)
    args = parser.parse_args()
    root = args.output_dir.resolve()
    sessions = [("stationary_cold", "stationary", 10.0), ("stationary_hot", "stationary", 45.0),
                ("walking", "walking", 25.0), ("vehicle", "vehicle", 32.0)]
    for name, activity, temperature in sessions:
        write_session(root / f"{name}.csv", activity, temperature)
    manifest = {"schema": "imu_validation_manifest_v1", "sessions": [
        {"name": name, "activity": activity, "path": f"{name}.csv"} for name, activity, _ in sessions]}
    (root / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    tool = pathlib.Path(__file__).parents[1] / "evaluation" / "calibration_validation.py"
    subprocess.run([sys.executable, str(tool), "calibrate", "--input", str(root / "stationary_cold.csv"),
                    "--input", str(root / "stationary_hot.csv"), "--output", str(root / "calibration.json")], check=True)
    command = [sys.executable, str(tool), "validate", "--manifest", str(root / "manifest.json"),
               "--calibration", str(root / "calibration.json"), "--artifacts-dir", str(root / "runs"),
               "--output", str(root / "validation.json")]
    if args.cli:
        command.extend(("--cli", str(args.cli)))
    subprocess.run(command, check=True)
    subprocess.run([sys.executable, str(tool), "report", "--validation", str(root / "validation.json"),
                    "--calibration", str(root / "calibration.json"), "--output", str(root / "report.html")], check=True)
    print(root / "report.html")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
