#!/usr/bin/env python3
"""Export dogfooding results as a GitHub Pages-friendly benchmark bundle."""

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def load_poses(path: Path):
    poses = []
    with path.open() as handle:
        for line in handle:
            values = [float(v) for v in line.strip().split()]
            if len(values) != 12:
                continue
            pose = np.eye(4)
            pose[:3, :] = np.array(values).reshape(3, 4)
            poses.append(pose)
    return poses


def compute_ate(est_poses, gt_poses):
    count = min(len(est_poses), len(gt_poses))
    if count == 0:
        return None
    errors = []
    for idx in range(count):
        delta = est_poses[idx][:3, 3] - gt_poses[idx][:3, 3]
        errors.append(float(np.dot(delta, delta)))
    return float(np.sqrt(np.mean(errors)))


def compute_trajectory_length(poses):
    if len(poses) < 2:
        return 0.0
    total = 0.0
    for idx in range(1, len(poses)):
        total += float(
            np.linalg.norm(poses[idx][:3, 3] - poses[idx - 1][:3, 3])
        )
    return total


def display_name_from_stem(stem: str) -> str:
    stem = stem.replace("_", "-")
    if stem.lower() == "gt":
        return "Ground Truth"
    return stem


def parse_key_value(items, separator="="):
    parsed = {}
    for item in items:
        if separator not in item:
            raise ValueError(f"Expected '{separator}' in '{item}'")
        key, value = item.split(separator, 1)
        parsed[key] = value
    return parsed


def parse_metrics(items):
    parsed = {}
    for item in items:
        parts = item.split(":")
        if len(parts) != 3:
            raise ValueError(
                f"Expected 'METHOD:time_ms:fps' in metric entry '{item}'"
            )
        parsed[parts[0]] = {
            "time_ms": float(parts[1]),
            "fps": float(parts[2]),
        }
    return parsed


def parse_extra_methods(items):
    parsed = []
    for item in items:
        parts = item.split(":", 2)
        if len(parts) != 3:
            raise ValueError(
                f"Expected 'METHOD:status:note' in extra method entry '{item}'"
            )
        parsed.append(
            {
                "name": parts[0],
                "status": parts[1],
                "note": parts[2],
            }
        )
    return parsed


def render_plot(gt_poses, methods, output_path: Path):
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    gt_xyz = np.array([pose[:3, 3] for pose in gt_poses])
    axes[0].plot(gt_xyz[:, 0], gt_xyz[:, 1], color="#111827", linewidth=2.5, label="Ground Truth")

    ranked_methods = [method for method in methods if method["status"] == "ok"]
    ranked_methods.sort(
        key=lambda method: method["ate_m"] if method["ate_m"] is not None else float("inf")
    )
    palette = [
        "#b91c1c",
        "#1d4ed8",
        "#15803d",
        "#a16207",
        "#7c3aed",
        "#0f766e",
        "#475569",
    ]

    for index, method in enumerate(ranked_methods):
        poses = load_poses(Path(method["artifact_abs_path"]))
        xyz = np.array([pose[:3, 3] for pose in poses])
        if xyz.size == 0:
            continue
        axes[0].plot(
            xyz[:, 0],
            xyz[:, 1],
            linewidth=1.7,
            linestyle="--",
            color=palette[index % len(palette)],
            label=method["name"],
        )

    axes[0].set_title("Trajectory Comparison")
    axes[0].set_xlabel("X [m]")
    axes[0].set_ylabel("Y [m]")
    axes[0].grid(alpha=0.25)
    axes[0].set_aspect("equal")
    axes[0].legend(loc="best")

    chart_methods = ranked_methods[:8]
    names = [method["name"] for method in chart_methods]
    ates = [method["ate_m"] for method in chart_methods]
    bars = axes[1].bar(names, ates, color=palette[: len(chart_methods)])
    axes[1].set_title("Absolute Trajectory Error")
    axes[1].set_ylabel("ATE RMSE [m]")
    axes[1].tick_params(axis="x", rotation=20)
    for bar, ate in zip(bars, ates):
        axes[1].text(
            bar.get_x() + bar.get_width() / 2.0,
            bar.get_height(),
            f"{ate:.2f}",
            ha="center",
            va="bottom",
            fontsize=9,
        )

    plt.tight_layout()
    fig.savefig(output_path, dpi=160)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results-dir", default="dogfooding_results")
    parser.add_argument("--output-dir", default="docs/benchmarks/latest")
    parser.add_argument("--title", default="Localization Zoo Benchmark")
    parser.add_argument("--dataset-name", required=True)
    parser.add_argument("--dataset-path", default="")
    parser.add_argument("--timestamp-source", default="")
    parser.add_argument("--command", default="")
    parser.add_argument("--generated-at", default="")
    parser.add_argument("--note", action="append", default=[])
    parser.add_argument("--skip", action="append", default=[])
    parser.add_argument("--extra-method", action="append", default=[])
    parser.add_argument("--metric", action="append", default=[])
    args = parser.parse_args()

    results_dir = Path(args.results_dir)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    gt_path = results_dir / "gt.txt"
    gt_poses = load_poses(gt_path)
    if not gt_poses:
        raise RuntimeError(f"Ground-truth pose file not found or empty: {gt_path}")

    notes = parse_key_value(args.note)
    skipped = parse_key_value(args.skip)
    extra_methods = parse_extra_methods(args.extra_method)
    metrics = parse_metrics(args.metric)

    methods = []
    for pose_path in sorted(results_dir.glob("*.txt")):
        if pose_path.name == "gt.txt":
            continue
        name = display_name_from_stem(pose_path.stem)
        poses = load_poses(pose_path)
        ate = compute_ate(poses, gt_poses)
        method_entry = {
            "name": name,
            "status": "ok",
            "ate_m": ate,
            "frames": len(poses),
            "artifact": pose_path.name,
            "artifact_abs_path": str(pose_path.resolve()),
            "note": notes.get(name, ""),
        }
        if name in metrics:
            method_entry.update(metrics[name])
        methods.append(method_entry)

    for name, note in skipped.items():
        methods.append(
            {
                "name": name,
                "status": "skipped",
                "ate_m": None,
                "frames": 0,
                "note": note,
                "artifact": "",
                "artifact_abs_path": "",
            }
        )

    for method in extra_methods:
        methods.append(
            {
                "name": method["name"],
                "status": method["status"],
                "ate_m": None,
                "frames": 0,
                "note": method["note"],
                "artifact": "",
                "artifact_abs_path": "",
            }
        )

    methods.sort(
        key=lambda method: (
            method["status"] != "ok",
            method["ate_m"] if method["ate_m"] is not None else float("inf"),
            method["name"],
        )
    )

    plot_path = output_dir / "trajectory.png"
    render_plot(gt_poses, methods, plot_path)

    payload = {
        "title": args.title,
        "generated_at": args.generated_at
        or datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "dataset": {
            "name": args.dataset_name,
            "path": args.dataset_path,
            "frames": len(gt_poses),
            "trajectory_length_m": compute_trajectory_length(gt_poses),
            "timestamp_source": args.timestamp_source,
        },
        "command": args.command,
        "artifacts": {
            "trajectory_plot": "trajectory.png",
        },
        "methods": [
            {
                key: value
                for key, value in method.items()
                if key != "artifact_abs_path"
            }
            for method in methods
        ],
    }

    with (output_dir / "results.json").open("w") as handle:
        json.dump(payload, handle, indent=2)
        handle.write("\n")


if __name__ == "__main__":
    main()
