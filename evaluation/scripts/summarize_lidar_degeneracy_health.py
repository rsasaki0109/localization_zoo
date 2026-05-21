#!/usr/bin/env python3
"""Compare GT-free LiDAR degeneracy health summaries across methods."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any


DEFAULT_INPUTS = (
    (
        "fog_200",
        (
            ("geometry_icp", "experiments/results/lidar_degeneracy/fog_200/odometry_health/odometry_health_summary.json"),
            ("intensity_bev", "experiments/results/lidar_degeneracy/fog_200/intensity_bev_health/odometry_health_summary.json"),
            ("kiss_keyframe", "experiments/results/lidar_degeneracy/fog_200/kiss_keyframe_health/odometry_health_summary.json"),
            ("ct_icp", "experiments/results/lidar_degeneracy/fog_200/ct_icp_health/odometry_health_summary.json"),
        ),
    ),
    (
        "tunnel_geom_2700_200",
        (
            ("geometry_icp", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/odometry_health/odometry_health_summary.json"),
            ("intensity_bev", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/intensity_bev_health/odometry_health_summary.json"),
            ("kiss_keyframe", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/kiss_keyframe_health/odometry_health_summary.json"),
            ("ct_icp", "experiments/results/lidar_degeneracy/tunnel_geom_2700_200/ct_icp_health/odometry_health_summary.json"),
        ),
    ),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("experiments/results/lidar_degeneracy/method_health_comparison"),
    )
    return parser.parse_args()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def fmt(value: Any, digits: int = 3) -> str:
    if value is None:
        return "n/a"
    try:
        number = float(value)
    except (TypeError, ValueError):
        return "n/a"
    if not math.isfinite(number):
        return "n/a"
    return f"{number:.{digits}f}"


def health_label(row: dict[str, Any]) -> str:
    flags = row.get("health_flags", {})
    active = [name for name, value in flags.items() if value]
    return ", ".join(active) if active else "ok"


def keyframe_text(row: dict[str, Any]) -> str:
    attempted = row.get("keyframe_attempted")
    accepted = row.get("keyframe_accepted")
    if attempted is None or accepted is None:
        return "n/a"
    if int(attempted) == 0:
        return "n/a"
    return f"{int(accepted)}/{int(attempted)}"


def method_sort_key(method: str) -> tuple[int, str]:
    order = {"geometry_icp": 0, "intensity_bev": 1, "kiss_keyframe": 2, "ct_icp": 3}
    return (order.get(method, 99), method)


def flatten_sequence(sequence: str, inputs: tuple[tuple[str, str], ...]) -> dict[str, Any]:
    methods: dict[str, dict[str, Any]] = {}
    windows: dict[str, dict[str, Any]] = {}
    for method, summary_path_str in inputs:
        summary_path = Path(summary_path_str)
        payload = load_json(summary_path)
        methods[method] = {
            "summary_json": str(summary_path),
            "method": payload.get("method", method),
            "sequence_pcd_dir": payload.get("sequence_pcd_dir"),
        }
        for row in payload["windows"]:
            window_key = f"{row['name']}:{row['window']['start']}-{row['window']['end']}"
            entry = windows.setdefault(
                window_key,
                {
                    "sequence": sequence,
                    "name": row["name"],
                    "start": int(row["window"]["start"]),
                    "end": int(row["window"]["end"]),
                    "frames": int(row["frames"]),
                    "obscurant_score": float(row["window"].get("obscurant_score", 0.0)),
                    "methods": {},
                },
            )
            entry["methods"][method] = {
                "accepted_rate": row.get("accepted_rate"),
                "converged_rate": row.get("converged_rate"),
                "score_mean": row.get("score_mean"),
                "overlap_mean": row.get("overlap_mean"),
                "used_path_length_m": row.get("used_path_length_m"),
                "used_step_max_m": row.get("used_step_max_m"),
                "keyframes": keyframe_text(row),
                "flags": health_label(row),
                "result_json": row.get("result_json"),
            }
    return {
        "sequence": sequence,
        "methods": methods,
        "windows": sorted(
            windows.values(),
            key=lambda row: (row["start"], row["end"], row["name"]),
        ),
    }


def summarize_sequence(sequence_payload: dict[str, Any]) -> dict[str, Any]:
    method_stats: dict[str, dict[str, Any]] = {}
    for window in sequence_payload["windows"]:
        for method, row in window["methods"].items():
            stats = method_stats.setdefault(
                method,
                {
                    "windows": 0,
                    "mean_acceptance": 0.0,
                    "mean_convergence": 0.0,
                    "failed_windows": 0,
                    "risk_windows": 0,
                    "min_acceptance": 1.0,
                    "max_used_path_length_m": 0.0,
                },
            )
            accepted = float(row["accepted_rate"] or 0.0)
            converged = row.get("converged_rate")
            converged_value = float(converged) if converged is not None else accepted
            stats["windows"] += 1
            stats["mean_acceptance"] += accepted
            stats["mean_convergence"] += converged_value
            stats["failed_windows"] += 1 if accepted < 0.5 else 0
            stats["risk_windows"] += 1 if row.get("flags") != "ok" else 0
            stats["min_acceptance"] = min(stats["min_acceptance"], accepted)
            stats["max_used_path_length_m"] = max(
                stats["max_used_path_length_m"],
                float(row.get("used_path_length_m") or 0.0),
            )
    for stats in method_stats.values():
        n = max(int(stats["windows"]), 1)
        stats["mean_acceptance"] /= n
        stats["mean_convergence"] /= n
    return method_stats


def write_markdown(path: Path, payload: dict[str, Any]) -> None:
    lines = [
        "# LiDAR Degeneracy Method Health Comparison",
        "",
        "## Method Aggregate",
        "",
        "| Sequence | Method | Windows | Mean accepted | Min accepted | Mean converged | Failed windows | Risk windows | Max used path m |",
        "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for sequence in payload["sequences"]:
        aggregates = payload["aggregates"][sequence["sequence"]]
        for method in sorted(aggregates, key=method_sort_key):
            row = aggregates[method]
            lines.append(
                f"| `{sequence['sequence']}` | `{method}` | {row['windows']} | "
                f"{fmt(row['mean_acceptance'])} | {fmt(row['min_acceptance'])} | "
                f"{fmt(row['mean_convergence'])} | {row['failed_windows']} | "
                f"{row['risk_windows']} | {fmt(row['max_used_path_length_m'])} |"
            )

    lines.extend(
        [
            "",
            "## Window Detail",
            "",
            "| Sequence | Window | Frames | Obscurant | Method | Accepted | Converged | Score | Overlap | Used path m | Max step m | Keyframes | Flags |",
            "| --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |",
        ]
    )
    for sequence in payload["sequences"]:
        for window in sequence["windows"]:
            for method in sorted(window["methods"], key=method_sort_key):
                row = window["methods"][method]
                lines.append(
                    f"| `{sequence['sequence']}` | `{window['name']}` "
                    f"{window['start']}-{window['end']} | {window['frames']} | "
                    f"{fmt(window['obscurant_score'])} | `{method}` | "
                    f"{fmt(row['accepted_rate'])} | {fmt(row['converged_rate'])} | "
                    f"{fmt(row['score_mean'])} | {fmt(row['overlap_mean'], 1)} | "
                    f"{fmt(row['used_path_length_m'])} | {fmt(row['used_step_max_m'])} | "
                    f"{row['keyframes']} | {row['flags']} |"
                )

    lines.extend(
        [
            "",
            "## Readout",
            "",
            "- `fog_200`: intensity BEV keeps 100% acceptance on all selected windows, including the strongest fog slice, but the clear baseline is flagged as low-used-path and should be treated as a possible zero-motion false lock.",
            "- `fog_200`: KISS keyframe rejects every selected window, geometry ICP collapses on the strongest fog window, and CT-ICP keeps baseline/tail healthy but drops on strongest fog.",
            "- `tunnel_geom_2700_200`: the short-window checks stay accepted, so this slice is not yet a local-odometry failure case.",
            "- CT-ICP convergence is reported separately from acceptance because this repo's CT-ICP dogfooding path uses gate-accepted refinements even when the internal stopping bit is low.",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    sequences = [flatten_sequence(name, inputs) for name, inputs in DEFAULT_INPUTS]
    aggregates = {
        sequence["sequence"]: summarize_sequence(sequence) for sequence in sequences
    }
    payload = {
        "inputs": {
            name: {method: path for method, path in inputs}
            for name, inputs in DEFAULT_INPUTS
        },
        "sequences": sequences,
        "aggregates": aggregates,
    }
    output_json = args.output_dir / "method_health_comparison.json"
    output_md = args.output_dir / "method_health_comparison.md"
    output_json.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_markdown(output_md, payload)
    print(f"[done] wrote {output_json} and {output_md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
