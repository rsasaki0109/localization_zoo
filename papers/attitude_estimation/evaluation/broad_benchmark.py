#!/usr/bin/env python3
"""Fetch, convert, run, and report the BROAD inertial orientation benchmark.

BROAD (Laidig et al., Data 2021, CC BY 4.0) has 39 trials of IMU data with
optical motion capture (OMC) ground truth. Errors follow the dataset's own
definition (example_code/broad_utils.py): the error quaternion is expressed
in the earth frame and its total / heading / inclination angles are RMS-averaged
over the movement phases only.

Subcommands:
  fetch    download the pinned data_mat files and verify their git blob SHA-1
  convert  write one CSV per trial plus trials.json (sampling rates, groups)
  run      run attitude_benchmark_cli for every configured variant and trial
  report   aggregate per-trial errors, compare with published values, write
           the results JSON and a Markdown summary
  check    fail if a results JSON drifts from the committed expected values
"""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import math
import pathlib
import shutil
import subprocess
import sys
import urllib.request

HERE = pathlib.Path(__file__).resolve().parent
REGISTRY = HERE / "broad_dataset.json"
VARIANTS = HERE / "broad_variants.json"
REFERENCE = HERE / "broad_reference.json"
EXPECTED = HERE / "broad_expected.json"
METRICS = ("total_rmse_deg", "heading_rmse_deg", "inclination_rmse_deg")


def git_blob_sha1(path: pathlib.Path) -> str:
    data = path.read_bytes()
    digest = hashlib.sha1(f"blob {len(data)}\0".encode())
    digest.update(data)
    return digest.hexdigest()


def fetch(data_dir: pathlib.Path) -> None:
    registry = json.loads(REGISTRY.read_text())
    target_dir = data_dir / "data_mat"
    target_dir.mkdir(parents=True, exist_ok=True)
    for spec in registry["files"]:
        target = target_dir / spec["name"]
        if not target.exists() or git_blob_sha1(target) != spec["git_blob_sha1"]:
            url = registry["raw_url"].format(commit=registry["commit"], name=spec["name"])
            with urllib.request.urlopen(url, timeout=120) as response, target.open("wb") as out:
                shutil.copyfileobj(response, out)
        digest = git_blob_sha1(target)
        if digest != spec["git_blob_sha1"]:
            raise SystemExit(f"{spec['name']}: checksum mismatch ({digest})")
    print(f"[fetch] {len(registry['files'])} files verified in {target_dir} ({registry['license']})")


def convert(data_dir: pathlib.Path) -> None:
    import numpy as np
    import scipy.io as spio

    trial_info = json.loads((data_dir / "data_mat" / "trials.json").read_text())
    csv_dir = data_dir / "csv"
    csv_dir.mkdir(parents=True, exist_ok=True)
    trials = {}
    for name, info in trial_info["trials"].items():
        data = spio.loadmat(data_dir / "data_mat" / f"{name}.mat")
        gyr, acc, mag = data["imu_gyr"], data["imu_acc"], data["imu_mag"]
        quat = data["opt_quat"]
        movement = data["movement"].squeeze().astype(bool)
        table = np.column_stack([gyr, acc, mag, quat, movement.astype(float)])
        header = "gx,gy,gz,ax,ay,az,mx,my,mz,qw,qx,qy,qz,movement"
        np.savetxt(csv_dir / f"{name}.csv", table, delimiter=",", header=header,
                   comments="", fmt="%.17g")
        trials[name] = {
            "sampling_rate": float(np.squeeze(data["sampling_rate"])),
            "samples": int(gyr.shape[0]),
            "movement_samples": int(movement.sum()),
            "groups": info["groups"],
        }
    out = {"groups": [g["name"] for g in trial_info["groups"]], "trials": trials}
    (csv_dir / "trials.json").write_text(json.dumps(out, indent=2) + "\n")
    print(f"[convert] {len(trials)} trials -> {csv_dir}")


def run_one(binary: str, csv_path: pathlib.Path, rate: float, args: list[str]) -> dict:
    cmd = [binary, str(csv_path), "--rate", repr(rate), *args]
    proc = subprocess.run(cmd, capture_output=True, text=True, check=True)
    return json.loads(proc.stdout)


def expand(variant: dict) -> list[tuple[str, list[str]]]:
    """(run id, args) pairs; a variant with a "grid" runs once per grid value."""
    grid = variant.get("grid")
    if not grid:
        return [(variant["id"], variant["args"])]
    return [(f"{variant['id']}@{value:g}", [*variant["args"], grid["flag"], repr(value)])
            for value in grid["values"]]


def tuned_metric(variant: dict) -> str:
    # 6D filters cannot observe heading, so they are judged on inclination.
    return "inclination_rmse_deg" if variant["mode"] == "6d" else "total_rmse_deg"


def run(data_dir: pathlib.Path, binary: str, out_path: pathlib.Path, jobs: int,
        only: list[str]) -> None:
    csv_dir = data_dir / "csv"
    trials = json.loads((csv_dir / "trials.json").read_text())["trials"]
    variants = json.loads(VARIANTS.read_text())["variants"]
    if only:
        variants = [v for v in variants if v["id"] in only]
    results = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        for variant in variants:
            for run_id, args in expand(variant):
                futures = {
                    name: pool.submit(run_one, binary, csv_dir / f"{name}.csv",
                                      info["sampling_rate"], args)
                    for name, info in trials.items()
                }
                results[run_id] = {name: f.result() for name, f in futures.items()}
            print(f"[run] {variant['id']}: {len(expand(variant))} setting(s) x {len(trials)} trials")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    previous = json.loads(out_path.read_text()) if out_path.exists() and only else {}
    previous.update(results)
    out_path.write_text(json.dumps(previous, indent=2) + "\n")


def mean(values: list[float]) -> float:
    return sum(values) / len(values)


def report(data_dir: pathlib.Path, runs_path: pathlib.Path, out_json: pathlib.Path,
           out_md: pathlib.Path) -> None:
    trials = json.loads((data_dir / "csv" / "trials.json").read_text())
    runs = json.loads(runs_path.read_text())
    variants = json.loads(VARIANTS.read_text())["variants"]
    reference = json.loads(REFERENCE.read_text())
    summary = {"dataset": json.loads(REGISTRY.read_text())["citation"], "variants": []}
    for variant in variants:
        row = {"id": variant["id"], "method": variant["method"], "mode": variant["mode"],
               "args": variant["args"], "intent": variant["intent"], "groups": {}}
        if variant.get("grid"):
            # TAGP (BROAD's definition): the grid value with the lowest error
            # averaged over all 39 trials. It is selected on the evaluated data.
            metric = tuned_metric(variant)
            candidates = [(mean([r[metric] for r in runs[run_id].values()]), run_id, args)
                          for run_id, args in expand(variant) if run_id in runs]
            if not candidates:
                continue
            _, best_id, best_args = min(candidates)
            row["args"] = best_args
            row["tagp"] = {"flag": variant["grid"]["flag"], "value": float(best_args[-1]),
                           "grid_size": len(candidates), "metric": metric}
            per_trial = runs[best_id]
        else:
            per_trial = runs.get(variant["id"])
            if not per_trial:
                continue
        for group in trials["groups"]:
            names = [n for n, info in trials["trials"].items() if group in info["groups"]]
            row["groups"][group] = {m: round(mean([per_trial[n][m] for n in names]), 4)
                                    for m in METRICS}
        ref = reference["values"].get(variant.get("reference", ""))
        if ref:
            metric = ref["metric"]
            row["reference"] = {k: ref[k] for k in ("label", "source", "metric", "all_trials")}
            if "trials" in ref:
                diffs = [abs(per_trial[n][metric] - ref["trials"][n]) for n in ref["trials"]]
                row["reference"]["max_abs_trial_diff_deg"] = round(max(diffs), 4)
            row["reference"]["ratio"] = round(row["groups"]["all_trials"][metric] / ref["all_trials"], 3)
        summary["variants"].append(row)
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(summary, indent=2) + "\n")
    out_md.write_text(render_markdown(summary) + "\n")
    print(f"[report] {out_json} {out_md}")


def render_markdown(summary: dict) -> str:
    lines = [
        "# BROAD attitude benchmark",
        "",
        "_Generated by `papers/attitude_estimation/evaluation/broad_benchmark.py report`._",
        "",
        f"Dataset: {summary['dataset']} (CC BY 4.0). Errors are RMS over movement phases, "
        "averaged over all 39 trials; 9D uses the magnetometer, 6D does not (heading is "
        "then unobservable, so inclination is the metric).",
        "",
        "Sources: BROAD = per-trial TAGP results of the BROAD example code; BROAD issue #1 = "
        "dlaidig/broad#1; VQF Fig. 9 = Laidig and Seel, Inf. Fusion 2023 (arXiv:2203.17024), "
        "one decimal. See papers/attitude_estimation/README.md.",
        "",
        "Grid variants report the TAGP (BROAD's term): the grid value with the lowest error "
        "averaged over all 39 trials, i.e. selected on the evaluated data.",
        "",
        "| Variant | Mode | Parameters | Total [deg] | Heading [deg] | Inclination [deg] | Published | Repo / published | Max per-trial diff [deg] |",
        "|---|---|---|---:|---:|---:|---|---:|---:|",
    ]
    for row in summary["variants"]:
        g = row["groups"]["all_trials"]
        ref = row.get("reference")
        published = f"{ref['all_trials']:g} {ref['metric'].split('_')[0]} ({ref['label']})" if ref else "-"
        ratio = f"{ref['ratio']:.3f}x" if ref else "-"
        diff = f"{ref['max_abs_trial_diff_deg']:.4f}" if ref and "max_abs_trial_diff_deg" in ref else "-"
        params = " ".join(a for a in row["args"]
                          if a not in ("--method", row["method"], "--mode", row["mode"]))
        if "tagp" in row:
            params += f" (TAGP of {row['tagp']['grid_size']})"
        lines.append(
            f"| `{row['id']}` | {row['mode']} | `{params}` | {g['total_rmse_deg']:.2f} | {g['heading_rmse_deg']:.2f} | "
            f"{g['inclination_rmse_deg']:.2f} | {published} | {ratio} | {diff} |")
    return "\n".join(lines)


def check(results_path: pathlib.Path, tolerance: float) -> int:
    results = json.loads(results_path.read_text())
    expected = json.loads(EXPECTED.read_text())["variants"]
    got = {row["id"]: row["groups"]["all_trials"] for row in results["variants"]}
    failures = []
    for vid, metrics in expected.items():
        if vid not in got:
            failures.append(f"{vid}: missing")
            continue
        for metric, value in metrics.items():
            actual = got[vid][metric]
            if not math.isclose(actual, value, rel_tol=tolerance, abs_tol=1e-4):
                failures.append(f"{vid}.{metric}: expected {value}, got {actual}")
    for failure in failures:
        print("drift:", failure)
    print(f"[check] {len(expected)} variants, {len(failures)} drift(s)")
    return 1 if failures else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("fetch", "convert", "run", "report"):
        p = sub.add_parser(name)
        p.add_argument("--data-dir", type=pathlib.Path, default=pathlib.Path("dogfooding_results/broad"))
        if name == "run":
            p.add_argument("--binary", default="build/papers/attitude_estimation/attitude_benchmark_cli")
            p.add_argument("--out", type=pathlib.Path, default=pathlib.Path("dogfooding_results/broad/runs.json"))
            p.add_argument("--jobs", type=int, default=8)
            p.add_argument("--only", action="append", default=[], help="Run only this variant id (repeatable).")
        if name == "report":
            p.add_argument("--runs", type=pathlib.Path, default=pathlib.Path("dogfooding_results/broad/runs.json"))
            p.add_argument("--out-json", type=pathlib.Path,
                           default=pathlib.Path("experiments/results/broad_attitude_benchmark.json"))
            p.add_argument("--out-md", type=pathlib.Path, default=pathlib.Path("docs/attitude_benchmark.md"))
    p = sub.add_parser("check")
    p.add_argument("--results", type=pathlib.Path,
                   default=pathlib.Path("experiments/results/broad_attitude_benchmark.json"))
    p.add_argument("--tolerance", type=float, default=1e-3)
    args = parser.parse_args()

    if args.command == "fetch":
        fetch(args.data_dir)
    elif args.command == "convert":
        convert(args.data_dir)
    elif args.command == "run":
        run(args.data_dir, args.binary, args.out, args.jobs, args.only)
    elif args.command == "report":
        report(args.data_dir, args.runs, args.out_json, args.out_md)
    else:
        return check(args.results, args.tolerance)
    return 0


if __name__ == "__main__":
    sys.exit(main())
