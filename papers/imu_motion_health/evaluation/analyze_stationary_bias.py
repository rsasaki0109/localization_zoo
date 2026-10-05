#!/usr/bin/env python3
"""Choose a fix for the stationary gyro-bias update on held-in BROAD trials.

broad_attitude.py found the default orientation_wxyz at 26.8 deg inclination
RMSE on BROAD, 3.4 deg with stationary bias learning off. The candidates are
three safeguards (and their combinations), each motivated independently:

  A  stationary_bias_reference_rate_hz: 100 - the per-sample gain is defined
     at 100 Hz and scaled with dt (0.01/sample is a 0.35 s time constant at
     BROAD's 286 Hz but 1 s at 100 Hz).
  B  stationary_bias_max_change: 0.0175 rad/s (1 deg/s) - the learned bias may
     not leave the startup calibration by more than typical MEMS in-run drift.
  C  stationary_bias_min_duration_s: 1.5 - learn only after 1.5 s of
     stationarity (VQF's rest time).

Protocol, fixed before running: candidates are scored on the odd-numbered
BROAD trials only; the lowest mean inclination RMSE wins, ties (within
0.05 deg) going to fewer safeguards. The even-numbered trials are evaluated
once, after the choice.

  python3 analyze_stationary_bias.py select
  python3 analyze_stationary_bias.py test
"""

from __future__ import annotations

import concurrent.futures
import itertools
import json
import pathlib
import sys
import tempfile

import numpy as np

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import broad_attitude as B  # noqa: E402

CSV = pathlib.Path("dogfooding_results/broad/csv")
CLI = "build/papers/imu_motion_health/imu_motion_health_cli"
RECORD = pathlib.Path(__file__).resolve().parent / "stationary_bias_selection.json"
SAFEGUARDS = {
    "A": ("stationary_bias_reference_rate_hz", 100.0),
    "B": ("stationary_bias_max_change", 0.0175),
    "C": ("stationary_bias_min_duration_s", 1.5),
}


def candidates():
    # Every safeguard is written explicitly (0 = off), so the candidates do not
    # depend on the SDK defaults, which now are the chosen candidate.
    for r in range(len(SAFEGUARDS) + 1):
        for combo in itertools.combinations(SAFEGUARDS, r):
            params = {key: (value if name in combo else 0) for name, (key, value) in SAFEGUARDS.items()}
            yield "".join(combo) or "legacy", params


def score(params: dict, trials: dict) -> dict:
    with tempfile.TemporaryDirectory() as tmp:
        profile = pathlib.Path(tmp) / "profile.yaml"
        # The candidates concern the integrating attitude, so pin it.
        profile.write_text("imu_motion_health:\n  vqf_attitude: false\n" +
                           "".join(f"  {k}: {v}\n" for k, v in params.items()))
        args = ["--profile-file", str(profile)]
        with concurrent.futures.ThreadPoolExecutor(8) as pool:
            futures = {t: pool.submit(B.run_trial, CLI, CSV / f"{t}.csv", info["sampling_rate"], args)
                       for t, info in trials.items()}
            return {t: f.result() for t, f in futures.items()}


def split(trials: dict, parity: int) -> dict:
    return {t: v for t, v in trials.items() if int(t[:2]) % 2 == parity}


def main() -> int:
    command = sys.argv[1] if len(sys.argv) > 1 else "select"
    trials = json.loads((CSV / "trials.json").read_text())["trials"]
    if command == "select":
        dev = split(trials, 1)
        scored = []
        for name, params in candidates():
            values = list(score(params, dev).values())
            scored.append({"candidate": name, "params": params, "dev_mean": round(float(np.mean(values)), 3),
                           "dev_median": round(float(np.median(values)), 3)})
            print(f"{name:7s} dev mean {np.mean(values):7.3f}  median {np.median(values):6.3f}")
        best = min(s["dev_mean"] for s in scored)
        chosen = min((s for s in scored if s["dev_mean"] <= best + 0.05),
                     key=lambda s: (len(s["params"]), s["dev_mean"]))
        RECORD.write_text(json.dumps({
            "protocol": "odd-numbered BROAD trials only; lowest mean inclination RMSE, ties within 0.05 deg "
                        "to fewer safeguards; even-numbered trials evaluated once afterwards",
            "dev_trials": sorted(dev), "candidates": scored, "chosen": chosen}, indent=2) + "\n")
        print("chosen:", chosen["candidate"], chosen["params"])
        return 0
    if command == "test":
        record = json.loads(RECORD.read_text())
        chosen = record["chosen"]["params"]
        held_out = split(trials, 0)
        out = {}
        legacy = {key: 0 for key, _ in SAFEGUARDS.values()}
        for name, params in (("legacy", legacy), ("chosen", {**legacy, **chosen})):
            values = list(score(params, held_out).values())
            out[name] = {"mean": round(float(np.mean(values)), 3), "median": round(float(np.median(values)), 3),
                         "max": round(float(np.max(values)), 3)}
        record["held_out"] = out
        RECORD.write_text(json.dumps(record, indent=2) + "\n")
        print(json.dumps(out, indent=2))
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
