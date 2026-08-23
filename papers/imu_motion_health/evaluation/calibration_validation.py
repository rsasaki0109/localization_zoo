#!/usr/bin/env python3
"""LiDAR-free IMU recording, calibration, validation, and reporting tools.

Only the Python standard library is required.  Raw CSV may contain the seven
columns accepted by imu_motion_health_cli plus an optional temperature_c.
"""

from __future__ import annotations

import argparse
import csv
import html
import json
import math
import pathlib
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from typing import Iterable, Sequence


BASE_COLUMNS = ("timestamp", "gx", "gy", "gz", "ax", "ay", "az")
TEMP_COLUMN = "temperature_c"
AXES = ("x", "y", "z")


@dataclass(frozen=True)
class Row:
    timestamp: float
    gyro: tuple[float, float, float]
    accel: tuple[float, float, float]
    temperature_c: float | None = None


def _finite(values: Iterable[float]) -> bool:
    return all(math.isfinite(value) for value in values)


def _read_stream(stream: Iterable[str], label: str) -> list[Row]:
    rows: list[Row] = []
    reader = csv.DictReader(line for line in stream if line.strip() and not line.lstrip().startswith("#"))
    if reader.fieldnames not in [list(BASE_COLUMNS), list(BASE_COLUMNS) + [TEMP_COLUMN]]:
        raise ValueError(f"{label}: expected columns {','.join(BASE_COLUMNS)}[,temperature_c]")
    for line_number, item in enumerate(reader, 2):
        try:
            values = [float(item[name]) for name in BASE_COLUMNS]
            temperature = float(item[TEMP_COLUMN]) if TEMP_COLUMN in item and item[TEMP_COLUMN] != "" else None
        except (TypeError, ValueError) as exc:
            raise ValueError(f"{label}:{line_number}: invalid numeric value") from exc
        if not _finite(values) or (temperature is not None and not math.isfinite(temperature)):
            raise ValueError(f"{label}:{line_number}: calibration input must be finite")
        rows.append(Row(values[0], tuple(values[1:4]), tuple(values[4:7]), temperature))
    if len(rows) < 2:
        raise ValueError(f"{label}: at least two samples are required")
    return rows


def read_rows(path: pathlib.Path) -> list[Row]:
    with path.open("r", encoding="utf-8", newline="") as stream:
        return _read_stream(stream, str(path))


def write_rows(path: pathlib.Path, rows: Sequence[Row], include_temperature: bool = False) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        columns = list(BASE_COLUMNS) + ([TEMP_COLUMN] if include_temperature else [])
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(columns)
        for row in rows:
            values = [row.timestamp, *row.gyro, *row.accel]
            if include_temperature:
                values.append("" if row.temperature_c is None else row.temperature_c)
            writer.writerow([f"{value:.12g}" if isinstance(value, float) else value for value in values])


def _mean(values: Sequence[float]) -> float:
    return sum(values) / len(values)


def _rms(values: Sequence[float]) -> float:
    return math.sqrt(_mean([value * value for value in values]))


def _linear_fit(xs: Sequence[float], ys: Sequence[float], reference: float) -> tuple[float, float]:
    centered = [x - reference for x in xs]
    denominator = sum(x * x for x in centered)
    slope = sum(x * y for x, y in zip(centered, ys)) / denominator if denominator > 1e-12 else 0.0
    intercept = _mean([y - slope * x for x, y in zip(centered, ys)])
    return intercept, slope


def fit_calibration(paths: Sequence[pathlib.Path], gravity: float = 9.80665) -> dict:
    sessions = [(path, read_rows(path)) for path in paths]
    all_rows = [row for _, rows in sessions for row in rows]
    temperatures = [row.temperature_c for row in all_rows if row.temperature_c is not None]
    reference = _mean(temperatures) if temperatures else 25.0
    gyro_intercept: list[float] = []
    gyro_slope: list[float] = []
    for axis in range(3):
        usable = [row for row in all_rows if row.temperature_c is not None]
        if usable:
            intercept, slope = _linear_fit(
                [row.temperature_c for row in usable], [row.gyro[axis] for row in usable], reference
            )
        else:
            intercept, slope = _mean([row.gyro[axis] for row in all_rows]), 0.0
        gyro_intercept.append(intercept)
        gyro_slope.append(slope)

    accel_mean = [_mean([row.accel[axis] for row in all_rows]) for axis in range(3)]
    norm = math.sqrt(sum(value * value for value in accel_mean))
    expected = [gravity * value / norm for value in accel_mean] if norm > 1e-9 else [0.0, 0.0, gravity]
    accel_bias = [value - target for value, target in zip(accel_mean, expected)]
    corrected_gyro_norms = []
    for row in all_rows:
        temperature = reference if row.temperature_c is None else row.temperature_c
        corrected = [row.gyro[i] - gyro_intercept[i] - gyro_slope[i] * (temperature - reference) for i in range(3)]
        corrected_gyro_norms.append(math.sqrt(sum(value * value for value in corrected)))
    return {
        "schema": "imu_calibration_v1",
        "method": "stationary_linear_temperature",
        "units": {"gyro": "rad/s", "accel": "m/s^2", "temperature": "degC"},
        "reference_temperature_c": reference,
        "gyro_bias_at_reference": gyro_intercept,
        "gyro_temperature_coefficient": gyro_slope,
        "accel_bias": accel_bias,
        "gravity_magnitude": gravity,
        "quality": {
            "sessions": len(sessions),
            "samples": len(all_rows),
            "temperature_span_c": (max(temperatures) - min(temperatures)) if temperatures else 0.0,
            "corrected_stationary_gyro_rms_rad_s": _rms(corrected_gyro_norms),
        },
        "sources": [str(path) for path, _ in sessions],
    }


def apply_calibration(rows: Sequence[Row], calibration: dict) -> list[Row]:
    reference = float(calibration["reference_temperature_c"])
    gyro_bias = calibration["gyro_bias_at_reference"]
    coefficients = calibration["gyro_temperature_coefficient"]
    accel_bias = calibration["accel_bias"]
    output = []
    for row in rows:
        temperature = reference if row.temperature_c is None else row.temperature_c
        gyro = tuple(row.gyro[i] - gyro_bias[i] - coefficients[i] * (temperature - reference) for i in range(3))
        accel = tuple(row.accel[i] - accel_bias[i] for i in range(3))
        output.append(Row(row.timestamp, gyro, accel, row.temperature_c))
    return output


def session_metrics(rows: Sequence[Row]) -> dict:
    dts = [rows[i].timestamp - rows[i - 1].timestamp for i in range(1, len(rows))]
    positive = [dt for dt in dts if dt > 0.0]
    gyro_norms = [math.sqrt(sum(value * value for value in row.gyro)) for row in rows]
    accel_norms = [math.sqrt(sum(value * value for value in row.accel)) for row in rows]
    duration = rows[-1].timestamp - rows[0].timestamp
    return {
        "samples": len(rows),
        "duration_s": duration,
        "sample_rate_hz": (len(rows) - 1) / duration if duration > 0 else 0.0,
        "nonmonotonic_intervals": sum(dt <= 0.0 for dt in dts),
        "gaps_over_250ms": sum(dt > 0.25 for dt in dts),
        "max_gap_s": max(positive, default=0.0),
        "gyro_rms_rad_s": _rms(gyro_norms),
        "accel_norm_mean_mps2": _mean(accel_norms),
        "accel_norm_std_mps2": math.sqrt(_mean([(value - _mean(accel_norms)) ** 2 for value in accel_norms])),
    }


def validate_manifest(manifest_path: pathlib.Path, calibration_path: pathlib.Path, cli: pathlib.Path | None,
                      artifacts_dir: pathlib.Path, min_rate_hz: float, max_stationary_gyro_rms: float) -> dict:
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    calibration = json.loads(calibration_path.read_text(encoding="utf-8"))
    base = manifest_path.parent
    artifacts_dir.mkdir(parents=True, exist_ok=True)
    results = []
    for spec in manifest["sessions"]:
        source = pathlib.Path(spec["path"])
        if not source.is_absolute():
            source = base / source
        corrected = apply_calibration(read_rows(source), calibration)
        corrected_path = artifacts_dir / f"{spec['name']}.corrected.csv"
        write_rows(corrected_path, corrected)
        metrics = session_metrics(corrected)
        checks = {
            "monotonic": metrics["nonmonotonic_intervals"] == 0,
            "sample_rate": metrics["sample_rate_hz"] >= min_rate_hz,
            "no_large_gaps": metrics["gaps_over_250ms"] == 0,
        }
        if spec["activity"] == "stationary":
            checks["stationary_gyro_rms"] = metrics["gyro_rms_rad_s"] <= max_stationary_gyro_rms
        cli_summary = None
        if cli is not None:
            summary_path = artifacts_dir / f"{spec['name']}.summary.json"
            process = subprocess.run(
                [str(cli), "--input", str(corrected_path), "--summary-output", str(summary_path)],
                capture_output=True, text=True, check=False,
            )
            checks["cli_exit"] = process.returncode == 0
            if summary_path.exists():
                cli_summary = json.loads(summary_path.read_text(encoding="utf-8"))
                checks["cli_all_rows_accepted"] = cli_summary["counters"]["accepted_samples"] == len(corrected)
        results.append({"name": spec["name"], "activity": spec["activity"], "source": str(source),
                        "metrics": metrics, "checks": checks, "passed": all(checks.values()),
                        "cli_summary": cli_summary})
    return {"schema": "imu_validation_report_v1", "manifest": str(manifest_path),
            "calibration": str(calibration_path), "sessions": results,
            "passed": bool(results) and all(result["passed"] for result in results)}


def endurance_replay(source: pathlib.Path, calibration_path: pathlib.Path, cli: pathlib.Path,
                     duration_s: float, output: pathlib.Path) -> dict:
    calibration = json.loads(calibration_path.read_text(encoding="utf-8"))
    corrected = apply_calibration(read_rows(source), calibration)
    source_duration = corrected[-1].timestamp - corrected[0].timestamp
    if source_duration <= 0.0:
        raise ValueError("endurance source duration must be positive")
    repeats = max(1, math.ceil(duration_s / source_duration))
    dt = source_duration / (len(corrected) - 1)
    with tempfile.TemporaryDirectory(prefix="imu_endurance_") as directory:
        replay_path = pathlib.Path(directory) / "replay.csv"
        with replay_path.open("w", encoding="utf-8", newline="") as stream:
            writer = csv.writer(stream, lineterminator="\n")
            writer.writerow(BASE_COLUMNS)
            index = 0
            for _ in range(repeats):
                for row in corrected:
                    writer.writerow([f"{index * dt:.12g}", *[f"{v:.12g}" for v in (*row.gyro, *row.accel)]])
                    index += 1
        summary_path = pathlib.Path(directory) / "summary.json"
        process = subprocess.run([str(cli), "--input", str(replay_path), "--summary-output", str(summary_path)],
                                 capture_output=True, text=True, check=False)
        summary = json.loads(summary_path.read_text(encoding="utf-8")) if summary_path.exists() else None
    report = {"schema": "imu_endurance_report_v1", "requested_duration_s": duration_s,
              "replayed_samples": repeats * len(corrected), "cli_exit_code": process.returncode,
              "passed": process.returncode == 0 and summary is not None and
                        summary["counters"]["accepted_samples"] == repeats * len(corrected),
              "summary": summary, "stderr": process.stderr[-2000:]}
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report


def render_report(report: dict, calibration: dict, output: pathlib.Path) -> None:
    rows = []
    for session in report["sessions"]:
        metrics = session["metrics"]
        rows.append(f"<tr><td>{html.escape(session['name'])}</td><td>{html.escape(session['activity'])}</td>"
                    f"<td>{metrics['duration_s']:.2f}</td><td>{metrics['sample_rate_hz']:.1f}</td>"
                    f"<td>{metrics['gyro_rms_rad_s']:.5f}</td><td class={'ok' if session['passed'] else 'bad'}>"
                    f"{'PASS' if session['passed'] else 'FAIL'}</td></tr>")
    status = "PASS" if report["passed"] else "FAIL"
    document = f"""<!doctype html><meta charset=utf-8><title>IMU calibration validation</title>
<style>body{{font:15px system-ui;margin:2rem;max-width:1100px}}table{{border-collapse:collapse;width:100%}}
th,td{{padding:.55rem;border-bottom:1px solid #ddd;text-align:left}}.ok{{color:#087830}}.bad{{color:#b42318}}
pre{{background:#f6f8fa;padding:1rem;overflow:auto}}.card{{border:1px solid #ddd;border-radius:10px;padding:1rem;margin:1rem 0}}</style>
<h1>IMU calibration & validation</h1><div class=card><h2 class={'ok' if report['passed'] else 'bad'}>{status}</h2>
<p>Temperature reference: {calibration['reference_temperature_c']:.2f} °C · Samples: {calibration['quality']['samples']} ·
Temperature span: {calibration['quality']['temperature_span_c']:.2f} °C</p></div>
<table><thead><tr><th>Session</th><th>Activity</th><th>Duration (s)</th><th>Rate (Hz)</th>
<th>Gyro RMS (rad/s)</th><th>Result</th></tr></thead><tbody>{''.join(rows)}</tbody></table>
<h2>Calibration artifact</h2><pre>{html.escape(json.dumps(calibration, indent=2))}</pre>
<h2>Machine report</h2><pre>{html.escape(json.dumps(report, indent=2))}</pre>"""
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(document, encoding="utf-8")


def record_stream(input_name: str, output: pathlib.Path, metadata_path: pathlib.Path,
                  device_id: str, activity: str) -> dict:
    started = time.time()
    if input_name == "-":
        rows = _read_stream(sys.stdin, "stdin")
    else:
        rows = read_rows(pathlib.Path(input_name))
    include_temperature = any(row.temperature_c is not None for row in rows)
    write_rows(output, rows, include_temperature)
    metrics = session_metrics(rows)
    metadata = {"schema": "imu_recording_metadata_v1", "device_id": device_id,
                "activity": activity, "csv": str(output), "host_recorded_unix_s": started,
                "has_temperature": include_temperature, "metrics": metrics}
    metadata_path.parent.mkdir(parents=True, exist_ok=True)
    metadata_path.write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    return metadata


def export_ros_parameters(calibration: dict, output: pathlib.Path, raw_topic: str,
                          calibrated_topic: str, temperature_topic: str) -> None:
    def vector(values: Sequence[float]) -> str:
        return "[" + ", ".join(f"{float(value):.12g}" for value in values) + "]"
    content = f"""imu_calibration_node:
  ros__parameters:
    input_topic: {json.dumps(raw_topic)}
    output_topic: {json.dumps(calibrated_topic)}
    temperature_topic: {json.dumps(temperature_topic)}
    use_temperature: true
    max_temperature_age_s: 2.0
    reference_temperature_c: {float(calibration['reference_temperature_c']):.12g}
    gyro_bias_at_reference: {vector(calibration['gyro_bias_at_reference'])}
    gyro_temperature_coefficient: {vector(calibration['gyro_temperature_coefficient'])}
    accel_bias: {vector(calibration['accel_bias'])}
"""
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(content, encoding="utf-8")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    record = commands.add_parser("record", help="normalize a driver CSV stream and archive metadata")
    record.add_argument("--input", default="-", help="CSV path or - for stdin")
    record.add_argument("--output", required=True, type=pathlib.Path)
    record.add_argument("--metadata", required=True, type=pathlib.Path)
    record.add_argument("--device-id", required=True)
    record.add_argument("--activity", choices=("stationary", "walking", "vehicle"), required=True)
    calibrate = commands.add_parser("calibrate")
    calibrate.add_argument("--input", action="append", required=True, type=pathlib.Path)
    calibrate.add_argument("--output", required=True, type=pathlib.Path)
    calibrate.add_argument("--gravity", type=float, default=9.80665)
    apply_cmd = commands.add_parser("apply")
    apply_cmd.add_argument("--input", required=True, type=pathlib.Path)
    apply_cmd.add_argument("--calibration", required=True, type=pathlib.Path)
    apply_cmd.add_argument("--output", required=True, type=pathlib.Path)
    validate = commands.add_parser("validate")
    validate.add_argument("--manifest", required=True, type=pathlib.Path)
    validate.add_argument("--calibration", required=True, type=pathlib.Path)
    validate.add_argument("--cli", type=pathlib.Path)
    validate.add_argument("--artifacts-dir", required=True, type=pathlib.Path)
    validate.add_argument("--output", required=True, type=pathlib.Path)
    validate.add_argument("--min-rate-hz", type=float, default=50.0)
    validate.add_argument("--max-stationary-gyro-rms", type=float, default=0.03)
    endurance = commands.add_parser("endurance")
    endurance.add_argument("--input", required=True, type=pathlib.Path)
    endurance.add_argument("--calibration", required=True, type=pathlib.Path)
    endurance.add_argument("--cli", required=True, type=pathlib.Path)
    endurance.add_argument("--duration-s", required=True, type=float)
    endurance.add_argument("--output", required=True, type=pathlib.Path)
    report = commands.add_parser("report")
    report.add_argument("--validation", required=True, type=pathlib.Path)
    report.add_argument("--calibration", required=True, type=pathlib.Path)
    report.add_argument("--output", required=True, type=pathlib.Path)
    export_ros = commands.add_parser("export-ros", help="convert calibration JSON to ROS parameter YAML")
    export_ros.add_argument("--calibration", required=True, type=pathlib.Path)
    export_ros.add_argument("--output", required=True, type=pathlib.Path)
    export_ros.add_argument("--raw-topic", default="/imu_raw")
    export_ros.add_argument("--calibrated-topic", default="/imu/calibrated")
    export_ros.add_argument("--temperature-topic", default="/imu/temperature")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.command == "record":
            record_stream(args.input, args.output, args.metadata, args.device_id, args.activity)
        elif args.command == "calibrate":
            artifact = fit_calibration(args.input, args.gravity)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(json.dumps(artifact, indent=2) + "\n", encoding="utf-8")
        elif args.command == "apply":
            calibration = json.loads(args.calibration.read_text(encoding="utf-8"))
            write_rows(args.output, apply_calibration(read_rows(args.input), calibration))
        elif args.command == "validate":
            result = validate_manifest(args.manifest, args.calibration, args.cli, args.artifacts_dir,
                                       args.min_rate_hz, args.max_stationary_gyro_rms)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
            return 0 if result["passed"] else 1
        elif args.command == "endurance":
            return 0 if endurance_replay(args.input, args.calibration, args.cli, args.duration_s, args.output)["passed"] else 1
        elif args.command == "report":
            render_report(json.loads(args.validation.read_text(encoding="utf-8")),
                          json.loads(args.calibration.read_text(encoding="utf-8")), args.output)
        elif args.command == "export-ros":
            export_ros_parameters(json.loads(args.calibration.read_text(encoding="utf-8")), args.output,
                                  args.raw_topic, args.calibrated_topic, args.temperature_topic)
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
