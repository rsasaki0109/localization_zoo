#!/usr/bin/env python3
"""Reproduce the TUM VI IMU noise parameters with allan_variance_cli.

TUM VI (Schubert et al., IROS 2018; CC BY 4.0) provides 111 h of a resting
BMI160 IMU (dataset-calib-imu-static2, 4.8 GB .npy) and reports, in the
caption of Fig. 5:

- slope -1/2 fitted over 0.02 <= tau <= 1 s on the Allan deviation averaged
  over all three axes; sigma_w is the line's value at tau = 1 s;
- slope +1/2 fitted over 1000 <= tau <= 6000 s, averaged over all three
  accelerometer axes but only gyro y and z; sigma_b is the value at tau = 3 s.

Subcommands: fetch (md5-verified download), run (Allan deviation of the six
channels), report (TUM protocol + comparison, results JSON, Markdown, plot,
and a Kalibr-style imu.yaml).
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import pathlib
import shutil
import subprocess
import sys
import urllib.request

HERE = pathlib.Path(__file__).resolve().parent
URL = "https://cdn3.vision.in.tum.de/tumvi/imu_static/dataset-calib-imu-static2.npy"
MD5 = "a1c62269c620a77b7c4b902887a26f54"
CHANNELS = ["gx", "gy", "gz", "ax", "ay", "az"]
# Published values (TUM VI paper, Fig. 5 caption) and the BMI160 datasheet
# values the paper quotes for comparison.
PUBLISHED = {
    "gyro_white_noise_density": 8.0e-5,       # rad/s/sqrt(Hz)
    "gyro_bias_random_walk": 2.2e-6,          # rad/s^2/sqrt(Hz)
    "accel_white_noise_density": 1.4e-3,      # m/s^2/sqrt(Hz)
    "accel_bias_random_walk": 8.6e-5,         # m/s^3/sqrt(Hz)
}
DATASHEET = {"gyro_white_noise_density": 1.2e-4, "accel_white_noise_density": 1.8e-3}
# Which channels each TUM fit averages, its range, and where it is read off.
PROTOCOL = {
    "gyro_white_noise_density": (["gx", "gy", "gz"], -0.5, 0.02, 1.0, 1.0),
    "gyro_bias_random_walk": (["gy", "gz"], 0.5, 1000.0, 6000.0, 3.0),
    "accel_white_noise_density": (["ax", "ay", "az"], -0.5, 0.02, 1.0, 1.0),
    "accel_bias_random_walk": (["ax", "ay", "az"], 0.5, 1000.0, 6000.0, 3.0),
}


def md5sum(path: pathlib.Path) -> str:
    digest = hashlib.md5()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1 << 22), b""):
            digest.update(block)
    return digest.hexdigest()


def fetch(data_dir: pathlib.Path) -> None:
    target = data_dir / "dataset-calib-imu-static2.npy"
    data_dir.mkdir(parents=True, exist_ok=True)
    if not target.exists() or md5sum(target) != MD5:
        with urllib.request.urlopen(URL, timeout=120) as response, target.open("wb") as out:
            shutil.copyfileobj(response, out)
    digest = md5sum(target)
    if digest != MD5:
        raise SystemExit(f"checksum mismatch: {digest}")
    print(f"[fetch] {target} verified (CC BY 4.0)")


def run(data_dir: pathlib.Path, binary: str, out: pathlib.Path) -> None:
    cmd = [binary, str(data_dir / "dataset-calib-imu-static2.npy"), "--columns", "1,2,3,4,5,6",
           "--names", ",".join(CHANNELS), "--time-column", "0", "--points-per-decade", "20"]
    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    out.write_text(result.stdout)
    print(f"[run] {out}")


def fixed_slope_fit(tau, adev, slope, tau_min, tau_max, tau_eval):
    # Same estimator as allan_variance::fixedSlopeFit: mean log10 offset.
    offsets = [math.log10(a) - slope * math.log10(t)
               for t, a in zip(tau, adev) if tau_min <= t <= tau_max and a > 0]
    if not offsets:
        raise ValueError("no points in fit range")
    return 10 ** (sum(offsets) / len(offsets) + slope * math.log10(tau_eval)), len(offsets)


def report(run_json: pathlib.Path, out_json: pathlib.Path, out_md: pathlib.Path,
           out_png: pathlib.Path | None, out_yaml: pathlib.Path) -> None:
    data = json.loads(run_json.read_text())
    curves = {c["name"]: c for c in data["columns"]}
    tau = curves["gx"]["tau"]
    results = {"samples": data["samples"], "sampling_rate": data["sampling_rate"],
               "hours": data["samples"] / data["sampling_rate"] / 3600, "tum_protocol": {},
               "per_axis_automatic": {}}
    for key, (channels, slope, lo, hi, at) in PROTOCOL.items():
        mean = [sum(curves[c]["adev"][i] for c in channels) / len(channels) for i in range(len(tau))]
        value, points = fixed_slope_fit(tau, mean, slope, lo, hi, at)
        results["tum_protocol"][key] = {
            "value": value, "published": PUBLISHED[key], "ratio": round(value / PUBLISHED[key], 3),
            "channels": channels, "fit_range_s": [lo, hi], "fit_points": points,
            "datasheet": DATASHEET.get(key)}
    for name, c in curves.items():
        results["per_axis_automatic"][name] = {k: c[k] for k in (
            "white_noise_density", "bias_random_walk", "bias_instability", "bias_instability_tau")}
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(results, indent=2) + "\n")

    p = results["tum_protocol"]
    out_yaml.write_text(
        "# Kalibr-style IMU noise model from the TUM VI static recording (BMI160),\n"
        "# reproduced with papers/allan_variance; values as measured, not inflated.\n"
        f"accelerometer_noise_density: {p['accel_white_noise_density']['value']:.4e}  # m/s^2/sqrt(Hz)\n"
        f"accelerometer_random_walk: {p['accel_bias_random_walk']['value']:.4e}  # m/s^3/sqrt(Hz)\n"
        f"gyroscope_noise_density: {p['gyro_white_noise_density']['value']:.4e}  # rad/s/sqrt(Hz)\n"
        f"gyroscope_random_walk: {p['gyro_bias_random_walk']['value']:.4e}  # rad/s^2/sqrt(Hz)\n"
        f"update_rate: {results['sampling_rate']:.1f}  # Hz\n")

    units = {"gyro_white_noise_density": "rad/s/√Hz", "gyro_bias_random_walk": "rad/s²/√Hz",
             "accel_white_noise_density": "m/s²/√Hz", "accel_bias_random_walk": "m/s³/√Hz"}
    lines = [
        "# Allan variance: TUM VI static IMU",
        "",
        "_Generated by `papers/allan_variance/evaluation/tumvi_allan.py report`._",
        "",
        f"TUM VI `dataset-calib-imu-static2` (BMI160, CC BY 4.0): {results['hours']:.1f} h, "
        f"{results['samples']:,} samples at {results['sampling_rate']:.2f} Hz. "
        "Overlapping Allan deviation with 20 cluster sizes per decade; the fits follow the "
        "paper's Fig. 5 caption (channels averaged, fixed slope, read off at tau = 1 s or 3 s).",
        "",
        "| Parameter | Channels | Fit range [s] | Repo | TUM VI paper | Repo / paper |",
        "|---|---|---|---:|---:|---:|",
    ]
    for key, r in p.items():
        lines.append(f"| {key.replace('_', ' ')} [{units[key]}] | {', '.join(r['channels'])} | "
                     f"{r['fit_range_s'][0]:g}-{r['fit_range_s'][1]:g} | {r['value']:.3g} | "
                     f"{r['published']:.2g} | {r['ratio']:.2f}x |")
    lines += ["", "Per-axis automatic extraction (tangent fits; bias instability = Allan-deviation "
              "minimum / 0.664):", "",
              "| Axis | White noise density | Bias random walk | Bias instability | at tau [s] |",
              "|---|---:|---:|---:|---:|"]
    for name, r in results["per_axis_automatic"].items():
        lines.append(f"| {name} | {r['white_noise_density']:.3g} | {r['bias_random_walk']:.3g} | "
                     f"{r['bias_instability']:.3g} | {r['bias_instability_tau']:.0f} |")
    if out_png is not None:
        lines += ["", f"![Allan deviation](assets/{out_png.name})"]
    out_md.write_text("\n".join(lines) + "\n")

    if out_png is not None:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))
        for ax, group, unit in ((axes[0], ["ax", "ay", "az"], "m/s²"), (axes[1], ["gx", "gy", "gz"], "rad/s")):
            for name in group:
                ax.loglog(tau, curves[name]["adev"], label=name)
            kind = "accel" if group[0] == "ax" else "gyro"
            wn = p[f"{kind}_white_noise_density"]["value"]
            rw = p[f"{kind}_bias_random_walk"]["value"]
            ax.loglog([0.005, 10], [wn / math.sqrt(t) for t in (0.005, 10)], "k--", lw=1, label="σw/√τ")
            ax.loglog([100, 4e4], [rw * math.sqrt(t / 3) for t in (100, 4e4)], "k:", lw=1, label="σb√(τ/3)")
            ax.set_xlabel("τ [s]")
            ax.set_ylabel(f"Allan deviation [{unit}]")
            ax.grid(True, which="both", alpha=0.3)
            ax.legend(fontsize=8)
        fig.suptitle("TUM VI dataset-calib-imu-static2 (111 h, BMI160)")
        fig.tight_layout()
        out_png.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(out_png, dpi=110)
    print(f"[report] {out_json} {out_md} {out_yaml}")


def check(results_path: pathlib.Path, expected_path: pathlib.Path, tolerance: float) -> int:
    results = json.loads(results_path.read_text())["tum_protocol"]
    expected = json.loads(expected_path.read_text())["tum_protocol"]
    drift = [k for k, v in expected.items()
             if not math.isclose(results[k]["value"], v, rel_tol=tolerance)]
    for k in drift:
        print(f"drift: {k}: expected {expected[k]}, got {results[k]['value']}")
    print(f"[check] {len(expected)} parameters, {len(drift)} drift(s)")
    return 1 if drift else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    data_default = pathlib.Path("dogfooding_results/tumvi_imu_static")
    p = sub.add_parser("fetch")
    p.add_argument("--data-dir", type=pathlib.Path, default=data_default)
    p = sub.add_parser("run")
    p.add_argument("--data-dir", type=pathlib.Path, default=data_default)
    p.add_argument("--binary", default="build/papers/allan_variance/allan_variance_cli")
    p.add_argument("--out", type=pathlib.Path, default=data_default / "allan_curves.json")
    p = sub.add_parser("report")
    p.add_argument("--run-json", type=pathlib.Path, default=data_default / "allan_curves.json")
    p.add_argument("--out-json", type=pathlib.Path,
                   default=pathlib.Path("experiments/results/tumvi_allan_variance.json"))
    p.add_argument("--out-md", type=pathlib.Path, default=pathlib.Path("docs/allan_variance.md"))
    p.add_argument("--out-png", type=pathlib.Path,
                   default=pathlib.Path("docs/assets/tumvi_allan_deviation.png"))
    p.add_argument("--out-yaml", type=pathlib.Path, default=HERE / "tumvi_bmi160_imu.yaml")
    p = sub.add_parser("check")
    p.add_argument("--results", type=pathlib.Path,
                   default=pathlib.Path("experiments/results/tumvi_allan_variance.json"))
    p.add_argument("--expected", type=pathlib.Path, default=HERE / "tumvi_expected.json")
    p.add_argument("--tolerance", type=float, default=1e-6)
    args = parser.parse_args()
    if args.command == "fetch":
        fetch(args.data_dir)
    elif args.command == "run":
        run(args.data_dir, args.binary, args.out)
    elif args.command == "report":
        report(args.run_json, args.out_json, args.out_md, args.out_png, args.out_yaml)
    else:
        return check(args.results, args.expected, args.tolerance)
    return 0


if __name__ == "__main__":
    sys.exit(main())
