#!/usr/bin/env python3
"""Replay and score the deterministic IMU fault matrix.

The evaluator understands the current ``imu_motion_health_cli`` contract
(JSONL snapshots plus a summary JSON) and is intentionally tolerant of a
future first-class event output.  If ``<scenario>.events.json`` or
``<scenario>.events.jsonl`` exists next to the run artifacts, its events are
included in the report and used for event observations.

Typical use::

    python generate_fault_matrix.py --output-dir build/imu_faults
    python evaluate_fault_matrix.py --manifest build/imu_faults/manifest.json \
        --cli build/imu_motion_health/imu_motion_health_cli \
        --output build/imu_faults/report.json

The output is deterministic for a fixed manifest and CLI result.  It contains
the per-scenario evidence (counts, state spans, event records, and relative
motion) rather than hiding failures behind one aggregate number.
"""

from __future__ import annotations

import argparse
import json
import math
import pathlib
import subprocess
import sys
from typing import Any, Dict, Iterable, List, Mapping, MutableMapping, Optional, Sequence, Tuple


SCHEMA = "imu_motion_health_evaluation_v1"


def load_json(path: pathlib.Path) -> Any:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def load_jsonl(path: pathlib.Path) -> List[Dict[str, Any]]:
    records: List[Dict[str, Any]] = []
    if not path.is_file():
        return records
    with path.open("r", encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            text = line.strip()
            if not text:
                continue
            try:
                value = json.loads(text)
            except json.JSONDecodeError as error:
                raise ValueError(f"{path}:{line_number}: invalid JSON: {error}")
            if isinstance(value, dict):
                records.append(value)
    return records


def _is_summary(value: Mapping[str, Any]) -> bool:
    schema = value.get("schema")
    return isinstance(schema, str) and schema.endswith("_summary_v1")


def split_records(records: Iterable[Mapping[str, Any]]) -> Tuple[List[Dict[str, Any]], Optional[Dict[str, Any]]]:
    snapshots: List[Dict[str, Any]] = []
    summary: Optional[Dict[str, Any]] = None
    for record in records:
        copied = dict(record)
        if _is_summary(copied):
            summary = copied
        else:
            snapshots.append(copied)
    return snapshots, summary


def _finite(value: Any) -> Optional[float]:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def _timestamp(record: Mapping[str, Any]) -> Optional[float]:
    return _finite(record.get("timestamp", record.get("time_s")))


def _count_true(snapshots: Iterable[Mapping[str, Any]], field: str) -> int:
    return sum(1 for item in snapshots if bool(item.get(field, False)))


def _counter(summary: Optional[Mapping[str, Any]], snapshots: Sequence[Mapping[str, Any]], field: str) -> int:
    if summary:
        counters = summary.get("counters")
        if isinstance(counters, Mapping):
            value = counters.get(field)
            if isinstance(value, (int, float)):
                return int(value)
    # A future producer might expose only a per-snapshot cumulative counter.
    maximum = 0
    for snapshot in snapshots:
        value = snapshot.get(field)
        if isinstance(value, (int, float)):
            maximum = max(maximum, int(value))
    return maximum


def state_intervals(snapshots: Sequence[Mapping[str, Any]]) -> List[Dict[str, Any]]:
    """Collapse per-sample states into inclusive time intervals."""

    intervals: List[Dict[str, Any]] = []
    current: Optional[str] = None
    start: Optional[float] = None
    last: Optional[float] = None
    for snapshot in snapshots:
        state = snapshot.get("motion_state")
        timestamp = _timestamp(snapshot)
        if not isinstance(state, str):
            continue
        if current is None:
            current, start = state, timestamp
        elif state != current:
            intervals.append({"state": current, "start_s": start,
                              "end_s": last, "duration_s": _duration(start, last)})
            current, start = state, timestamp
        last = timestamp
    if current is not None:
        intervals.append({"state": current, "start_s": start,
                          "end_s": last, "duration_s": _duration(start, last)})
    return intervals


def _duration(start: Optional[float], end: Optional[float]) -> Optional[float]:
    if start is None or end is None:
        return None
    return max(0.0, end - start)


def normalize_events(value: Any) -> List[Dict[str, Any]]:
    """Read common event JSON shapes without imposing a new wire contract."""

    if isinstance(value, Mapping):
        if isinstance(value.get("events"), list):
            value = value["events"]
        elif isinstance(value.get("event"), Mapping):
            value = [value["event"]]
        else:
            value = []
    if not isinstance(value, list):
        return []
    events: List[Dict[str, Any]] = []
    for item in value:
        if not isinstance(item, Mapping):
            continue
        event = dict(item)
        # Normalize aliases while preserving producer-specific fields.
        if "type" not in event:
            for alias in ("event_type", "name", "label", "kind"):
                if isinstance(event.get(alias), str):
                    event["type"] = event[alias]
                    break
        if "start_s" not in event:
            for alias in ("start_timestamp", "start_time", "timestamp"):
                if _finite(event.get(alias)) is not None:
                    event["start_s"] = _finite(event[alias])
                    break
        if "end_s" not in event:
            for alias in ("end_timestamp", "end_time"):
                if _finite(event.get(alias)) is not None:
                    event["end_s"] = _finite(event[alias])
                    break
        events.append(event)
    return events


def events_from_snapshots(snapshots: Sequence[Mapping[str, Any]]) -> List[Dict[str, Any]]:
    extracted: List[Dict[str, Any]] = []
    for snapshot in snapshots:
        if isinstance(snapshot.get("events"), list):
            extracted.extend(normalize_events(snapshot["events"]))
        elif isinstance(snapshot.get("event"), Mapping):
            extracted.extend(normalize_events([snapshot["event"]]))
    return extracted


def load_events(path: Optional[pathlib.Path], snapshots: Sequence[Mapping[str, Any]],
                summary: Optional[Mapping[str, Any]]) -> List[Dict[str, Any]]:
    if path is not None and path.is_file():
        if path.suffix.lower() == ".jsonl":
            records = load_jsonl(path)
            return normalize_events(records)
        return normalize_events(load_json(path))
    if summary is not None:
        events = normalize_events(summary.get("events"))
        if events:
            return events
    return events_from_snapshots(snapshots)


def _event_type(event: Mapping[str, Any]) -> str:
    value = event.get("type", event.get("event_type", event.get("name", "")))
    return str(value).lower()


def _flag_count(snapshots: Sequence[Mapping[str, Any]], field: str) -> int:
    return _count_true(snapshots, field)


def observation_result(observation: Mapping[str, Any], snapshots: Sequence[Mapping[str, Any]],
                       summary: Optional[Mapping[str, Any]], events: Sequence[Mapping[str, Any]]) -> Dict[str, Any]:
    kind = str(observation.get("kind", ""))
    minimum = int(observation.get("minimum", observation.get("minimum_samples", 1)))
    if kind == "flag":
        field = str(observation.get("field", ""))
        observed = _flag_count(snapshots, field)
        return {"kind": kind, "field": field, "observed": observed,
                "minimum": minimum, "passed": observed >= minimum}
    if kind == "counter":
        field = str(observation.get("field", ""))
        observed = _counter(summary, snapshots, field)
        return {"kind": kind, "field": field, "observed": observed,
                "minimum": minimum, "passed": observed >= minimum}
    if kind == "state":
        value = str(observation.get("value", ""))
        observed = sum(1 for item in snapshots
                       if str(item.get("motion_state", "")).lower() == value.lower())
        return {"kind": kind, "value": value, "observed": observed,
                "minimum": minimum, "passed": observed >= minimum}
    if kind == "state_any":
        values = [str(item).lower() for item in observation.get("values", [])]
        observed = sum(1 for item in snapshots
                       if str(item.get("motion_state", "")).lower() in values)
        return {"kind": kind, "values": values, "observed": observed,
                "minimum": minimum, "passed": observed >= minimum}
    if kind == "event":
        value = str(observation.get("value", "")).lower()
        observed = sum(1 for event in events if _event_type(event) == value)
        return {"kind": kind, "value": value, "observed": observed,
                "minimum": minimum, "passed": observed >= minimum}
    if kind == "forbid_flags":
        fields = [str(item) for item in observation.get("fields", [])]
        counts = {field: _flag_count(snapshots, field) for field in fields}
        passed = all(value == 0 for value in counts.values())
        return {"kind": kind, "fields": fields, "observed": counts,
                "minimum": 0, "passed": passed}
    return {"kind": kind, "passed": False,
            "error": "unsupported truth observation"}


def _find_existing_run(run_dir: pathlib.Path, scenario: str, suffix: str) -> Optional[pathlib.Path]:
    candidates = [
        run_dir / f"{scenario}.{suffix}",
        run_dir / f"{scenario}.run.{suffix}",
        run_dir / scenario / suffix,
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def _find_events(run_dir: Optional[pathlib.Path], scenario: str) -> Optional[pathlib.Path]:
    if run_dir is None:
        return None
    for candidate in (
        run_dir / f"{scenario}.events.json",
        run_dir / f"{scenario}.events.jsonl",
        run_dir / scenario / "events.json",
        run_dir / scenario / "events.jsonl",
    ):
        if candidate.is_file():
            return candidate
    return None


def _relative_motion(summary: Optional[Mapping[str, Any]], snapshots: Sequence[Mapping[str, Any]]) -> Dict[str, Any]:
    state: Mapping[str, Any] = {}
    if summary and isinstance(summary.get("final_state"), Mapping):
        state = summary["final_state"]
    elif snapshots:
        state = snapshots[-1]
    result: Dict[str, Any] = {}
    for key in ("relative_position", "relative_velocity", "position", "velocity",
                "orientation_wxyz"):
        value = state.get(key)
        if isinstance(value, list):
            result[key] = value
    return result


def evaluate_scenario(truth: Mapping[str, Any], snapshots: Sequence[Mapping[str, Any]],
                      summary: Optional[Mapping[str, Any]], events: Sequence[Mapping[str, Any]],
                      command: Optional[Sequence[str]] = None,
                      returncode: Optional[int] = None,
                      stderr: str = "") -> Dict[str, Any]:
    observations = truth.get("observations", [])
    checks = [observation_result(item, snapshots, summary, events)
              for item in observations if isinstance(item, Mapping)]
    passed = all(bool(item.get("passed", False)) for item in checks)
    if returncode is not None and returncode != 0:
        passed = False
    counters = summary.get("counters", {}) if isinstance(summary, Mapping) else {}
    if not isinstance(counters, Mapping):
        counters = {}
    return {
        "scenario": truth.get("scenario"),
        "passed": passed,
        "truth": dict(truth),
        "checks": checks,
        "snapshot_count": len(snapshots),
        "summary_schema": summary.get("schema") if summary else None,
        "summary": dict(summary) if summary else None,
        "counters": dict(counters),
        "state_intervals": state_intervals(snapshots),
        "events": [dict(item) for item in events],
        "relative_motion": _relative_motion(summary, snapshots),
        "command": list(command) if command is not None else None,
        "returncode": returncode,
        "stderr": stderr,
    }


def _run_cli(cli: pathlib.Path, csv_path: pathlib.Path, output_dir: pathlib.Path,
             scenario: str) -> Tuple[pathlib.Path, pathlib.Path, pathlib.Path,
                                      Sequence[str], int, str]:
    output_dir.mkdir(parents=True, exist_ok=True)
    jsonl = output_dir / f"{scenario}.jsonl"
    summary = output_dir / f"{scenario}.summary.json"
    events = output_dir / f"{scenario}.events.jsonl"
    command = [str(cli), "--input", str(csv_path), "--jsonl-output", str(jsonl),
               "--summary-output", str(summary), "--events-output", str(events)]
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            text=True, check=False)
    # Keep replay compatible with an older CLI while preferring the event
    # stream whenever the executable supports it.  A real data/configuration
    # failure is never hidden; only an explicit unknown-option response gets a
    # legacy retry.
    lower_error = result.stderr.lower()
    unknown_events_option = result.returncode != 0 and not events.is_file() and \
        ("event" in lower_error and
         ("unknown option" in lower_error or "unrecognized" in lower_error or
          "unexpected argument" in lower_error or "invalid option" in lower_error))
    if unknown_events_option:
        command = [str(cli), "--input", str(csv_path), "--jsonl-output", str(jsonl),
                   "--summary-output", str(summary)]
        result = subprocess.run(command, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True, check=False)
    return jsonl, summary, events, command, result.returncode, result.stderr


def evaluate_manifest(manifest_path: pathlib.Path, cli: Optional[pathlib.Path] = None,
                      output_dir: Optional[pathlib.Path] = None,
                      runs_dir: Optional[pathlib.Path] = None,
                      events_dir: Optional[pathlib.Path] = None) -> Dict[str, Any]:
    manifest = load_json(manifest_path)
    if not isinstance(manifest, Mapping):
        raise ValueError("manifest must be a JSON object")
    base_dir = manifest_path.parent
    scenario_reports: List[Dict[str, Any]] = []
    if cli is None and runs_dir is None:
        raise ValueError("provide --cli to replay or --runs-dir for existing artifacts")
    if output_dir is None:
        output_dir = base_dir / "runs"
    for entry in manifest.get("scenarios", []):
        if not isinstance(entry, Mapping):
            continue
        scenario = str(entry.get("scenario"))
        truth_path = base_dir / str(entry.get("truth", f"{scenario}.truth.json"))
        csv_path = base_dir / str(entry.get("csv", f"{scenario}.csv"))
        truth = load_json(truth_path)
        command: Optional[Sequence[str]] = None
        returncode: Optional[int] = None
        stderr = ""
        generated_events_path: Optional[pathlib.Path] = None
        if cli is not None:
            jsonl_path, summary_path, generated_events_path, command, returncode, stderr = _run_cli(
                cli, csv_path, output_dir, scenario)
        else:
            assert runs_dir is not None
            jsonl_path = _find_existing_run(runs_dir, scenario, "jsonl")
            summary_path = _find_existing_run(runs_dir, scenario, "summary.json")
            if jsonl_path is None:
                raise ValueError(f"missing JSONL artifact for {scenario} in {runs_dir}")
            if summary_path is None:
                summary_path = _find_existing_run(runs_dir, scenario, "summary")
        records = load_jsonl(jsonl_path)
        snapshots, embedded_summary = split_records(records)
        summary: Optional[Dict[str, Any]] = None
        if summary_path is not None and summary_path.is_file():
            value = load_json(summary_path)
            if isinstance(value, Mapping):
                summary = dict(value)
        if summary is None:
            summary = embedded_summary
        event_path = generated_events_path if generated_events_path is not None and \
            generated_events_path.is_file() else _find_events(
                events_dir or (output_dir if cli is not None else runs_dir), scenario)
        events = load_events(event_path, snapshots, summary)
        report = evaluate_scenario(truth, snapshots, summary, events,
                                   command, returncode, stderr)
        report["csv"] = str(csv_path)
        report["jsonl"] = str(jsonl_path)
        report["summary_path"] = str(summary_path) if summary_path else None
        report["events_path"] = str(event_path) if event_path else None
        scenario_reports.append(report)
    passed = sum(1 for item in scenario_reports if item["passed"])
    return {
        "schema": SCHEMA,
        "manifest": str(manifest_path),
        "cli": str(cli) if cli is not None else None,
        "scenarios": scenario_reports,
        "aggregate": {
            "total": len(scenario_reports),
            "passed": passed,
            "failed": len(scenario_reports) - passed,
            "pass": passed == len(scenario_reports),
        },
    }


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", required=True, type=pathlib.Path)
    parser.add_argument("--cli", type=pathlib.Path,
                        help="imu_motion_health_cli executable; replay each CSV")
    parser.add_argument("--runs-dir", type=pathlib.Path,
                        help="existing artifacts (<scenario>.jsonl/.summary.json)")
    parser.add_argument("--events-dir", type=pathlib.Path,
                        help="optional directory with <scenario>.events.json[l]")
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--artifacts-dir", type=pathlib.Path,
                        help="where CLI replay JSONL/summary files are written")
    args = parser.parse_args(argv)
    if args.cli is not None and not args.cli.is_file():
        print(f"evaluate_fault_matrix: CLI not found: {args.cli}", file=sys.stderr)
        return 2
    try:
        report = evaluate_manifest(args.manifest, args.cli, args.artifacts_dir,
                                   args.runs_dir, args.events_dir)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("w", encoding="utf-8", newline="\n") as stream:
            json.dump(report, stream, indent=2, sort_keys=True, allow_nan=False)
            stream.write("\n")
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"evaluate_fault_matrix: {error}", file=sys.stderr)
        return 2
    aggregate = report["aggregate"]
    print(json.dumps(aggregate, sort_keys=True))
    return 0 if aggregate["pass"] else 1


if __name__ == "__main__":
    sys.exit(main())
