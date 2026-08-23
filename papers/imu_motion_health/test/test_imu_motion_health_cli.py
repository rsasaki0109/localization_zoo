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


def write_event_fixture(path: pathlib.Path) -> None:
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(["timestamp", "gx", "gy", "gz", "ax", "ay", "az"])
        for index in range(31):
            writer.writerow([index * 0.01, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY])
        writer.writerow([0.31, 0.0, 0.0, 0.0, 0.0, 0.0, 30.0])
        writer.writerow([0.32, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY])
        writer.writerow([0.35, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY])


def write_bias_jump_fixture(path: pathlib.Path) -> None:
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(["timestamp", "gx", "gy", "gz", "ax", "ay", "az"])
        for index in range(31):
            writer.writerow([index * 0.01, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY])
        for timestamp in (0.31, 0.32, 0.33):
            writer.writerow([timestamp, 0.30, 0.0, 0.0, 0.0, 0.0, GRAVITY])
        writer.writerow([0.34, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY])
        writer.writerow([0.38, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY])


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

        # Event JSONL contains a stable started/ended pair and the summary
        # reports completed counts.  Explicit CLI values win over a preset in
        # either argument order (the cargo preset threshold is 15 m/s^2).
        event_fixture = directory / "events.csv"
        write_event_fixture(event_fixture)
        event_path = directory / "events.jsonl"
        event_summary = directory / "events-summary.json"
        common = [
            "--input",
            str(event_fixture),
            "--startup-duration",
            "0.05",
            "--startup-min-samples",
            "5",
            "--vibration-rms",
            "100",
            "--events-output",
            str(event_path),
            "--summary-output",
            str(event_summary),
        ]
        result = run(args.cli, "--profile", "cargo", "--impact-accel", "11", *common)
        if result.returncode != 0:
            raise AssertionError(result.stderr)
        event_records = [json.loads(line) for line in event_path.read_text().splitlines()]
        if len(event_records) != 2:
            raise AssertionError(f"expected started/ended event pair, got {event_records}")
        if event_records[0]["phase"] != "started" or event_records[1]["phase"] != "ended":
            raise AssertionError("event lifecycle phases were not emitted")
        if event_records[0]["id"] != event_records[1]["id"]:
            raise AssertionError("event id was not stable across lifecycle")
        summary = json.loads(event_summary.read_text())
        if summary["event_counts"]["impact"] != 1 or summary["events_ended"] != 1:
            raise AssertionError(f"event counts missing from summary: {summary}")

        reverse_event_path = directory / "events-reverse.jsonl"
        reverse_summary_path = directory / "events-reverse-summary.json"
        reverse = [
            "--impact-accel",
            "11",
            "--profile=cargo",
            "--events-output",
            str(reverse_event_path),
            "--summary-output",
            str(reverse_summary_path),
        ] + common[:8]
        result = run(args.cli, *reverse)
        if result.returncode != 0:
            raise AssertionError(result.stderr)
        reverse_summary = json.loads(reverse_summary_path.read_text())
        if reverse_summary["event_counts"]["impact"] != 1:
            raise AssertionError("profile/CLI precedence depended on argument order")

        # The parser is intentionally strict: a typo is a usage/configuration
        # error instead of an silently ignored detector setting.
        bad_profile = directory / "bad-profile.yaml"
        bad_profile.write_text("impct_accel_threshold: 1\n", encoding="utf-8")
        result = run(
            args.cli,
            "--input",
            str(event_fixture),
            "--profile-file",
            str(bad_profile),
        )
        if result.returncode != 2 or "unknown profile key" not in result.stderr:
            raise AssertionError("strict YAML profile errors regressed")

        # A persistent gyro residual while gravity remains quiet is diagnosed
        # as a bias jump and has its own lifecycle event/counter.
        bias_fixture = directory / "bias-jump.csv"
        write_bias_jump_fixture(bias_fixture)
        bias_events = directory / "bias-jump.events.jsonl"
        bias_summary = directory / "bias-jump.summary.json"
        result = run(
            args.cli,
            "--input",
            str(bias_fixture),
            "--startup-duration",
            "0.05",
            "--startup-min-samples",
            "5",
            "--gyro-bias-jump",
            "0.15",
            "--bias-jump-min-duration",
            "0.02",
            "--events-output",
            str(bias_events),
            "--summary-output",
            str(bias_summary),
        )
        if result.returncode != 0:
            raise AssertionError(result.stderr)
        bias_records = [json.loads(line) for line in bias_events.read_text().splitlines()]
        bias_records = [record for record in bias_records if record["type"] == "bias_jump"]
        if [record["phase"] for record in bias_records] != ["started", "ended"]:
            raise AssertionError(f"bias-jump lifecycle missing: {bias_records}")
        if bias_records[0]["id"] != bias_records[1]["id"]:
            raise AssertionError("bias-jump event id was not stable")
        bias_summary_json = json.loads(bias_summary.read_text())
        if bias_summary_json["event_counts"]["bias_jump"] != 1:
            raise AssertionError("bias-jump event count missing from summary")
        if bias_summary_json["counters"]["gyro_bias_jump_detections"] != 1:
            raise AssertionError("bias-jump detection counter missing from summary")

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as error:
        print(f"test_imu_motion_health_cli: {error}", file=sys.stderr)
        sys.exit(1)
