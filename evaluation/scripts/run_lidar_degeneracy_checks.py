#!/usr/bin/env python3
"""Run lightweight LiDAR degeneracy report checks."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = REPO_ROOT / "evaluation" / "scripts"
COMPILE_TARGETS = [
    SCRIPTS / "summarize_lidar_degeneracy_health.py",
    SCRIPTS / "calibrate_lidar_degeneracy_risk.py",
    SCRIPTS / "test_lidar_degeneracy_triage_policy.py",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--regenerate-reports",
        action="store_true",
        help="Also rerun the method-health and risk-calibration report generators.",
    )
    return parser.parse_args()


def run(cmd: list[str]) -> None:
    print("[run] " + " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def main() -> int:
    args = parse_args()
    run([sys.executable, "-m", "py_compile", *[str(path) for path in COMPILE_TARGETS]])
    run([sys.executable, str(SCRIPTS / "test_lidar_degeneracy_triage_policy.py")])
    if args.regenerate_reports:
        run([sys.executable, str(SCRIPTS / "summarize_lidar_degeneracy_health.py")])
        run([sys.executable, str(SCRIPTS / "calibrate_lidar_degeneracy_risk.py")])
    print("[ok] lidar degeneracy checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
