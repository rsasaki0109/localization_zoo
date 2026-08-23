#!/usr/bin/env python3
"""Generate a deterministic IMU fault-injection matrix.

The generated streams are deliberately small, SI-unit CSV files that can be
replayed by ``imu_motion_health_cli``.  Every mutation is described in a
machine-readable truth file, so a regression test can distinguish an
injected condition from an ordinary moving/quiet state.  No random numbers
are used; ``--seed`` is recorded for reproducibility and reserves a stable
place in the manifest for future scenarios.

Example::

    python generate_fault_matrix.py --output-dir build/imu_faults

This writes one ``*.csv`` and ``*.truth.json`` per scenario plus a
``manifest.json``.  The output directory is safe to archive as an evaluation
fixture and is independent of the current working directory.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import pathlib
import sys
from typing import Dict, Iterable, List, MutableMapping, Sequence, Tuple


GRAVITY = 9.80665
DEFAULT_RATE_HZ = 100.0
DEFAULT_DURATION_S = 3.0
SCHEMA = "imu_motion_health_fault_truth_v1"
MANIFEST_SCHEMA = "imu_motion_health_fault_matrix_v1"
CSV_HEADER = ("timestamp", "gx", "gy", "gz", "ax", "ay", "az")
Sample = Tuple[float, float, float, float, float, float, float]


SCENARIOS = (
    "nominal",
    "impact",
    "freefall_fall",
    "vibration",
    "timestamp_gap",
    "timestamp_backward",
    "nonfinite",
    "gyro_saturation",
    "accel_saturation",
    "row_dropout",
    "gyro_bias_jump",
)


def _base_rows(rate_hz: float = DEFAULT_RATE_HZ,
               duration_s: float = DEFAULT_DURATION_S) -> List[Sample]:
    """Return a quiet, gravity-aligned stream with deterministic timestamps."""

    count = int(round(duration_s * rate_hz)) + 1
    dt = 1.0 / rate_hz
    return [(i * dt, 0.0, 0.0, 0.0, 0.0, 0.0, GRAVITY)
            for i in range(count)]


def _replace(rows: List[Sample], index: int, **updates: float) -> None:
    names = ["timestamp", "gx", "gy", "gz", "ax", "ay", "az"]
    row = list(rows[index])
    for name, value in updates.items():
        row[names.index(name)] = value
    rows[index] = tuple(row)  # type: ignore[assignment]


def _interval(start: float, end: float, label: str, kind: str = "injected",
              **extra: object) -> Dict[str, object]:
    item: Dict[str, object] = {
        "label": label,
        "kind": kind,
        "start_s": round(start, 9),
        "end_s": round(end, 9),
    }
    item.update(extra)
    return item


def _observation(kind: str, **kwargs: object) -> Dict[str, object]:
    item: Dict[str, object] = {"kind": kind}
    item.update(kwargs)
    return item


def _scenario(name: str, rate_hz: float) -> Tuple[List[Sample], Dict[str, object]]:
    """Construct one stream and its expected observations.

    Truth is expressed in terms of the public JSON snapshot contract.  The
    evaluator deliberately accepts either per-sample flags or aggregate
    counters, which keeps these fixtures useful when a future CLI adds a
    first-class event stream.
    """

    rows = _base_rows(rate_hz)
    dt = 1.0 / rate_hz
    fault_intervals: List[Dict[str, object]] = []
    observations: List[Dict[str, object]] = []
    notes: List[str] = []

    def index_time(index: int) -> float:
        return round(index * dt, 9)

    if name == "nominal":
        # Keep this fixture completely quiet: it is the false-positive guard.
        observations.append(_observation("forbid_flags", fields=[
            "nonfinite", "nonmonotonic", "gap", "gyro_saturated",
            "accel_saturated",
        ]))
        observations.append(_observation(
            "state", value="stationary", minimum_samples=1,
        ))
        notes.append("quiet gravity-aligned stream; false-positive baseline")

    elif name == "impact":
        index = 150
        _replace(rows, index, ax=40.0)
        fault_intervals.append(_interval(index_time(index), index_time(index),
                                         "impact", peak_accel_mps2=40.0))
        observations.append(_observation("state", value="impact",
                                         minimum_samples=1))
        notes.append("single 40 m/s^2 specific-force spike")

    elif name == "freefall_fall":
        start, stop = 150, 160
        for index in range(start, stop + 1):
            _replace(rows, index, ax=0.0, ay=0.0, az=0.0)
        impact_index = stop + 1
        _replace(rows, impact_index, ax=0.0, ay=0.0, az=35.0)
        fault_intervals.append(_interval(index_time(start), index_time(stop),
                                         "freefall", end_inclusive=True))
        fault_intervals.append(_interval(index_time(impact_index),
                                         index_time(impact_index), "impact",
                                         peak_accel_mps2=35.0))
        observations.append(_observation("state", value="fall",
                                         minimum_samples=1))
        notes.append("110 ms free-fall followed by a 35 m/s^2 landing spike")

    elif name == "vibration":
        start, stop = 150, 220
        frequency_hz = 15.0
        amplitude_mps2 = 2.5
        for index in range(start, stop + 1):
            phase = 2.0 * math.pi * frequency_hz * (index_time(index) -
                                                       index_time(start))
            _replace(rows, index, ax=amplitude_mps2 * math.sin(phase))
        fault_intervals.append(_interval(index_time(start), index_time(stop),
                                         "vibration", amplitude_mps2=amplitude_mps2,
                                         frequency_hz=frequency_hz))
        observations.append(_observation("state", value="vibration",
                                         minimum_samples=1))
        notes.append("2.5 m/s^2, 15 Hz lateral vibration for 710 ms")

    elif name == "timestamp_gap":
        index = 150
        gap_s = 0.40
        # Shift this sample and every later timestamp together.  There is one
        # monotonic gap and no accidental cascade of backward timestamps.
        for row_index in range(index, len(rows)):
            row = rows[row_index]
            _replace(rows, row_index, timestamp=row[0] + gap_s)
        fault_intervals.append(_interval(index_time(index - 1),
                                         index_time(index) + gap_s,
                                         "timestamp_gap", gap_s=gap_s))
        observations.append(_observation("flag", field="gap", minimum=1))
        observations.append(_observation("counter", field="timestamp_gaps",
                                         minimum=1))
        notes.append("single 400 ms timestamp gap; later samples remain monotonic")

    elif name == "timestamp_backward":
        index = 150
        backwards_s = 0.04
        _replace(rows, index, timestamp=rows[index - 1][0] - backwards_s)
        fault_intervals.append(_interval(index_time(index), index_time(index),
                                         "timestamp_backward",
                                         backwards_s=backwards_s))
        observations.append(_observation("flag", field="nonmonotonic",
                                         minimum=1))
        observations.append(_observation(
            "counter", field="nonmonotonic_timestamps", minimum=1,
        ))
        notes.append("one timestamp moves 40 ms backwards and is rejected")

    elif name == "nonfinite":
        index = 150
        _replace(rows, index, gx=math.nan)
        fault_intervals.append(_interval(index_time(index), index_time(index),
                                         "nonfinite", component="gx"))
        observations.append(_observation("flag", field="nonfinite", minimum=1))
        observations.append(_observation("counter", field="nonfinite_samples",
                                         minimum=1))
        notes.append("one NaN gyroscope component")

    elif name == "gyro_saturation":
        index = 150
        _replace(rows, index, gx=40.0)
        fault_intervals.append(_interval(index_time(index), index_time(index),
                                         "gyro_saturation", gyro_rad_s=40.0))
        observations.append(_observation("flag", field="gyro_saturated",
                                         minimum=1))
        observations.append(_observation(
            "counter", field="gyro_saturated_samples", minimum=1,
        ))
        notes.append("40 rad/s exceeds the documented default 34 rad/s limit")

    elif name == "accel_saturation":
        index = 150
        _replace(rows, index, ax=160.0)
        fault_intervals.append(_interval(index_time(index), index_time(index),
                                         "accel_saturation", accel_mps2=160.0))
        observations.append(_observation("flag", field="accel_saturated",
                                         minimum=1))
        observations.append(_observation(
            "counter", field="accel_saturated_samples", minimum=1,
        ))
        notes.append("160 m/s^2 exceeds the documented default 16 g limit")

    elif name == "row_dropout":
        start, count = 150, 35
        end = start + count - 1
        removed = rows[start:start + count]
        del rows[start:start + count]
        fault_intervals.append(_interval(index_time(start - 1),
                                         index_time(end + 1), "row_dropout",
                                         dropped_rows=count,
                                         dropped_start_s=removed[0][0],
                                         dropped_end_s=removed[-1][0]))
        observations.append(_observation("flag", field="gap", minimum=1))
        observations.append(_observation("counter", field="timestamp_gaps",
                                         minimum=1))
        notes.append("35 consecutive rows removed, yielding a 360 ms stream gap")

    elif name == "gyro_bias_jump":
        start, stop = 150, 195
        bias_rad_s = 0.8
        for index in range(start, stop + 1):
            _replace(rows, index, gx=bias_rad_s)
        fault_intervals.append(_interval(index_time(start), index_time(stop),
                                         "gyro_bias_jump", gyro_bias_rad_s=bias_rad_s))
        observations.append(_observation("flag", field="gyro_bias_jump",
                                         minimum=1))
        observations.append(_observation(
            "counter", field="gyro_bias_jump_detections", minimum=1,
        ))
        observations.append(_observation("event", value="bias_jump",
                                         minimum=1))
        notes.append(
            "constant 0.8 rad/s stationary gyro step after trusted startup"
        )

    else:
        raise ValueError("unknown scenario: " + name)

    truth: Dict[str, object] = {
        "schema": SCHEMA,
        "scenario": name,
        "seed": 0,
        "sample_rate_hz": rate_hz,
        "gravity_mps2": GRAVITY,
        "source_rows": len(_base_rows(rate_hz)),
        "rows_written": len(rows),
        "fault_intervals": fault_intervals,
        "observations": observations,
        "notes": notes,
    }
    return rows, truth


def _json_number(value: float) -> object:
    # JSON truth is finite by design; sensor NaNs only live in CSV.  This
    # helper makes the invariant explicit if a future scenario adds a value.
    return value if math.isfinite(value) else None


def _format_csv_value(value: float) -> str:
    if math.isnan(value):
        return "nan"
    if math.isinf(value):
        return "inf" if value > 0 else "-inf"
    return f"{value:.9f}"


def write_csv(path: pathlib.Path, rows: Iterable[Sample]) -> int:
    path.parent.mkdir(parents=True, exist_ok=True)
    count = 0
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(CSV_HEADER)
        for row in rows:
            writer.writerow([_format_csv_value(float(value)) for value in row])
            count += 1
    return count


def write_json(path: pathlib.Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(value, stream, indent=2, sort_keys=True, allow_nan=False)
        stream.write("\n")


def generate(output_dir: pathlib.Path, names: Sequence[str], seed: int = 0,
             rate_hz: float = DEFAULT_RATE_HZ) -> Dict[str, object]:
    if rate_hz <= 0.0 or not math.isfinite(rate_hz):
        raise ValueError("rate_hz must be a finite positive number")
    output_dir.mkdir(parents=True, exist_ok=True)
    entries: List[Dict[str, object]] = []
    for name in names:
        rows, truth = _scenario(name, rate_hz)
        truth["seed"] = seed
        csv_path = output_dir / f"{name}.csv"
        truth_path = output_dir / f"{name}.truth.json"
        written = write_csv(csv_path, rows)
        truth["rows_written"] = written
        write_json(truth_path, truth)
        entries.append({
            "scenario": name,
            "csv": csv_path.name,
            "truth": truth_path.name,
            "rows_written": written,
            "sample_rate_hz": rate_hz,
        })
    manifest: Dict[str, object] = {
        "schema": MANIFEST_SCHEMA,
        "generator": "generate_fault_matrix.py",
        "generator_version": 1,
        "seed": seed,
        "sample_rate_hz": rate_hz,
        "gravity_mps2": GRAVITY,
        "output_dir": str(output_dir),
        "scenarios": entries,
    }
    write_json(output_dir / "manifest.json", manifest)
    return manifest


def _parse_names(values: Sequence[str]) -> List[str]:
    names: List[str] = []
    for value in values:
        for name in value.split(","):
            name = name.strip().lower()
            if not name:
                continue
            if name == "all":
                for candidate in SCENARIOS:
                    if candidate not in names:
                        names.append(candidate)
                continue
            if name not in SCENARIOS:
                raise ValueError(f"unknown scenario '{name}' (choose from {', '.join(SCENARIOS)})")
            if name not in names:
                names.append(name)
    return names or list(SCENARIOS)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", required=True, type=pathlib.Path)
    parser.add_argument("--scenario", action="append", default=[],
                        help="scenario name, comma-separated names, or all (default: all)")
    parser.add_argument("--seed", type=int, default=0,
                        help="recorded reproducibility seed (generation is deterministic)")
    parser.add_argument("--sample-rate-hz", type=float, default=DEFAULT_RATE_HZ)
    args = parser.parse_args(argv)
    try:
        names = _parse_names(args.scenario)
        manifest = generate(args.output_dir, names, args.seed,
                            args.sample_rate_hz)
    except (OSError, ValueError) as error:
        print(f"generate_fault_matrix: {error}", file=sys.stderr)
        return 2
    print(json.dumps({"manifest": str(args.output_dir / "manifest.json"),
                      "scenarios": len(manifest["scenarios"])},
                     sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())
