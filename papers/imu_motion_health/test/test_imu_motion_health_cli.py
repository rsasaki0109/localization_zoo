#!/usr/bin/env python3
"""Small black-box test for the LiDAR-free IMU replay executable."""

import argparse
import csv
import json
import math
import pathlib
import subprocess
import sys
import tempfile


GRAVITY = 9.80665


def write_fixture(path: pathlib.Path, include_bad_row: bool = False) -> None:
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(["timestamp", "gx", "gy", "gz", "ax", "ay", "az"])
        for index in range(10):
            writer.writerow([index * 0.01, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY])
        for index in range(10, 20):
            writer.writerow([index * 0.01, 0.0, 0.0, 0.0, 1.5, 0.0, GRAVITY])
        if include_bad_row:
            writer.writerow(["this", "row", "is", "not", "numeric"])


def run(cli: pathlib.Path, *args: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        [str(cli), *args],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cli", required=True, type=pathlib.Path)
    args = parser.parse_args()

    with tempfile.TemporaryDirectory(prefix="imu_motion_health_cli_test_") as raw:
        directory = pathlib.Path(raw)
        fixture = directory / "fixture.csv"
        write_fixture(fixture)

        # With JSONL on stdout the final summary is the last JSON line.  This
        # also proves that a valid header and finite snapshots survive a full
        # parse/process/serialize round trip.
        result = run(
            args.cli,
            "--input",
            str(fixture),
            "--startup-duration",
            "0.05",
            "--startup-min-samples",
            "5",
            "--emit-jsonl",
        )
        if result.returncode != 0:
            raise AssertionError(result.stderr)
        records = [json.loads(line) for line in result.stdout.splitlines()]
        if len(records) != 21:
            raise AssertionError(f"expected 21 JSON records, got {len(records)}")
        summary = records[-1]
        if summary.get("schema") != "imu_motion_health_summary_v1":
            raise AssertionError("summary schema missing from stdout")
        if summary["rows_read"] != 20 or summary["parse_errors"] != 0:
            raise AssertionError(f"unexpected row counts: {summary}")
        if summary["final_state"]["motion_state"] != "moving":
            raise AssertionError("moving phase was not classified")
        if not math.isfinite(summary["final_state"]["confidence"]):
            raise AssertionError("confidence is not finite")

        # Separate output files are useful for machines that need a pure
        # summary on stdout or a durable per-sample trace.
        jsonl_path = directory / "trace.jsonl"
        summary_path = directory / "summary.json"
        result = run(
            args.cli,
            "--input",
            str(fixture),
            "--startup-duration=0.05",
            "--startup-min-samples=5",
            "--jsonl-output",
            str(jsonl_path),
            "--summary-output",
            str(summary_path),
        )
        if result.returncode != 0 or result.stdout.strip():
            raise AssertionError("file-output mode wrote an unexpected stdout")
        trace = [json.loads(line) for line in jsonl_path.read_text().splitlines()]
        file_summary = json.loads(summary_path.read_text())
        if len(trace) != 20 or file_summary["rows_read"] != 20:
            raise AssertionError("file-output row count mismatch")

        # Non-finite numeric tokens are passed to the core and must still
        # produce standards-compliant JSON (null, not NaN/Infinity).
        nonfinite_fixture = directory / "nonfinite.csv"
        nonfinite_fixture.write_text(
            "timestamp,gx,gy,gz,ax,ay,az\n"
            "nan,0,0,0,0,0,9.80665\n",
            encoding="utf-8",
        )
        result = run(args.cli, "--input", str(nonfinite_fixture), "--emit-jsonl")
        if result.returncode != 0:
            raise AssertionError(result.stderr)
        nonfinite_records = [
            json.loads(line) for line in result.stdout.splitlines()
        ]
        if not nonfinite_records[0]["nonfinite"]:
            raise AssertionError("non-finite sample was not diagnosed")
        if nonfinite_records[0]["timestamp"] is not None:
            raise AssertionError("non-finite timestamp was not encoded as null")

        # Strict mode rejects malformed rows.  --skip-invalid keeps replaying,
        # emits a complete summary, and returns the documented input-error
        # status so a batch job cannot silently lose data.
        bad_fixture = directory / "bad.csv"
        write_fixture(bad_fixture, include_bad_row=True)
        result = run(args.cli, "--input", str(bad_fixture))
        if result.returncode != 4 or "line 22" not in result.stderr:
            raise AssertionError("strict malformed-row handling regressed")
        bad_summary = directory / "bad-summary.json"
        result = run(
            args.cli,
            "--input",
            str(bad_fixture),
            "--skip-invalid",
            "--summary-output",
            str(bad_summary),
        )
        if result.returncode != 4:
            raise AssertionError("skip-invalid must retain input-error exit code")
        if json.loads(bad_summary.read_text())["parse_errors"] != 1:
            raise AssertionError("parse error was not included in summary")

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as error:
        print(f"test_imu_motion_health_cli: {error}", file=sys.stderr)
        sys.exit(1)
