#!/usr/bin/env python3
"""Watch a ROS 2 IMU-only launch command for a required duration."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
import time


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--duration-s", required=True, type=float)
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("command", nargs=argparse.REMAINDER, help="command after --")
    args = parser.parse_args()
    command = args.command[1:] if args.command[:1] == ["--"] else args.command
    if args.duration_s <= 0 or not command:
        parser.error("positive --duration-s and a command after -- are required")
    started = time.monotonic()
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    early_exit = None
    try:
        while time.monotonic() - started < args.duration_s:
            early_exit = process.poll()
            if early_exit is not None:
                break
            time.sleep(min(0.1, args.duration_s / 10.0))
    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                process.kill()
        stdout, stderr = process.communicate()
    elapsed = time.monotonic() - started
    passed = early_exit is None and elapsed >= args.duration_s
    report = {"schema": "imu_ros2_endurance_report_v1", "command": command,
              "required_duration_s": args.duration_s, "elapsed_s": elapsed,
              "passed": passed, "early_exit_code": early_exit,
              "stdout_tail": stdout[-4000:], "stderr_tail": stderr[-4000:]}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
