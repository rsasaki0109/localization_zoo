#!/usr/bin/env python3
"""Posture confirmation from a VQF attitude estimate instead of averaged gravity.

The committed policy confirms an impact at t from the mean accelerometer
direction over [t-2.0, t-1.0] s versus a settled window after the impact, so it
waits for the body to come to rest (median latency 2.95 s). Here a 6D VQF
estimate (papers/attitude_estimation, causal) gives the body's up direction at
every sample, so a posture change can be seen while the body is still moving:

    u   = mean VQF up direction over [t - 2.0, t - 1.0] s  (same as now)
    v(s) = VQF up direction at s, for s in [t + start, t + 1.5]
    confirm at the first s where angle(u, v) >= threshold has held for `dwell`
    seconds; otherwise fall back to the committed t + 1.5 s window rule.

The dwell is what keeps jumps out: a landing tilts the body only briefly.
With the committed impact thresholds almost no ADL produces an impact event
(one test UpwardJump does), so the jump margin is measured as if every jump
had triggered one: at the manifest's impact_timestamp (peak acceleration).

Selection follows the committed protocol (public_datasets.md): candidates are
scored on CGU-BES train + validation subjects (01-12) only; a margin rule is
fixed before the test subjects are opened; the test subjects (13-15) and the
UCI / Parkinson false-positive sets are then evaluated once.

  python3 analyze_vqf_confirmation.py quats   # VQF attitude for every recording
  python3 analyze_vqf_confirmation.py select  # train+val grid and the chosen candidate
  python3 analyze_vqf_confirmation.py test    # one evaluation of the chosen candidate
"""

from __future__ import annotations

import concurrent.futures
import csv
import json
import math
import pathlib
import statistics
import subprocess
import sys
import tempfile

ROOT = pathlib.Path("build/imu_public_data")
RES = ROOT / "results"
QUAT_DIR = ROOT / "vqf_quats"
CLI = pathlib.Path("build/papers/attitude_estimation/attitude_benchmark_cli")
SELECTION = pathlib.Path(__file__).resolve().parent / "vqf_confirmation_selection.json"

PRE = (2.0, 1.0)           # committed reference window before the impact
FALLBACK_POST = (0.5, 1.5)  # committed settled window
THRESHOLD_DEG = 50.0        # committed fallback threshold
GRID = {
    "threshold_deg": [40.0, 45.0, 50.0, 55.0, 60.0],
    "dwell_s": [0.1, 0.2, 0.3, 0.5],
    "start_s": [0.0, 0.25],
    "tau_acc": [1.0, 3.0],
}
MARGIN_DEG = 5.0             # largest held jump angle must stay this far below the threshold
JUMPS = ("ForwardJump", "UpwardJump")
MIN_TRAIN_SENSITIVITY = 0.95  # same floor as the committed threshold choice


def manifest() -> list[dict]:
    return json.loads((ROOT / "normalized/manifest.json").read_text())["recordings"]


def impacts(i: int) -> list[float]:
    events = [json.loads(l) for l in open(RES / "runs" / f"{i:05d}.events.jsonl") if l.strip()]
    return [e["timestamp"] for e in events if e["phase"] == "started" and e["type"] == "impact"]


def read_rows(path: pathlib.Path) -> list[dict]:
    return [{k: float(v) for k, v in r.items()} for r in csv.DictReader(open(path))]


def quats_for(i: int, entry: dict, tau_acc: float) -> pathlib.Path:
    out = QUAT_DIR / f"tau{tau_acc:g}" / f"{i:05d}.csv"
    if out.exists():
        return out
    out.parent.mkdir(parents=True, exist_ok=True)
    rows = read_rows(ROOT / "normalized" / entry["path"])
    dt = statistics.median(b["timestamp"] - a["timestamp"] for a, b in zip(rows, rows[1:]))
    with tempfile.NamedTemporaryFile("w", suffix=".csv", delete=False) as tmp:
        tmp.write("gx,gy,gz,ax,ay,az,mx,my,mz,qw,qx,qy,qz,movement\n")
        for r in rows:
            tmp.write(f"{r['gx']},{r['gy']},{r['gz']},{r['ax']},{r['ay']},{r['az']},0,0,0,nan,nan,nan,nan,0\n")
    try:
        subprocess.run([str(CLI), tmp.name, "--rate", repr(1.0 / dt), "--method", "vqf", "--mode", "6d",
                        "--tau-acc", repr(tau_acc), "--output-quat", str(out)],
                       check=True, capture_output=True)
    finally:
        pathlib.Path(tmp.name).unlink()
    return out


def up_directions(i: int, entry: dict, tau_acc: float) -> list[tuple[float, tuple[float, float, float]]]:
    """(timestamp, body-frame up direction R(q)^T e_z) per sample."""
    times = [r["timestamp"] for r in read_rows(ROOT / "normalized" / entry["path"])]
    out = []
    with open(quats_for(i, entry, tau_acc)) as f:
        next(f)
        for t, line in zip(times, f):
            w, x, y, z = map(float, line.split(","))
            out.append((t, (2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y))))
    return out


def mean_dir(samples, a, b):
    sel = [v for t, v in samples if a <= t <= b]
    if not sel:
        return None
    m = [sum(v[k] for v in sel) / len(sel) for k in range(3)]
    n = math.sqrt(sum(c * c for c in m))
    return [c / n for c in m] if n > 0 else None


def angle(u, v) -> float:
    n = math.sqrt(sum(c * c for c in v))
    return math.degrees(math.acos(max(-1.0, min(1.0, sum(a * b for a, b in zip(u, v)) / n))))


def accel_samples(entry: dict):
    return [(r["timestamp"], (r["ax"], r["ay"], r["az"])) for r in read_rows(ROOT / "normalized" / entry["path"])]


def confirm(up, accel, impact_times, threshold, dwell, start):
    """(confirmation time or None, largest angle held for `dwell` after any impact)."""
    end = up[-1][0]
    held_max = 0.0
    for t in impact_times:
        u = mean_dir(up, t - PRE[0], t - PRE[1])
        if u is None:
            continue
        window = [(s, angle(u, v)) for s, v in up if t + start <= s <= t + FALLBACK_POST[1]]
        # Largest angle that held for `dwell`: min over each dwell-long run.
        for k, (s0, _) in enumerate(window):
            run = [a for s, a in window[k:] if s <= s0 + dwell]
            if run and window[k:] and window[-1][0] >= s0 + dwell:
                held_max = max(held_max, min(run))
        above_since = None
        for s, a in window:
            if a >= threshold:
                above_since = s if above_since is None else above_since
                if s - above_since >= dwell:
                    return s, held_max
            else:
                above_since = None
        # Committed fallback: settled accelerometer windows, 50 deg, at t + 1.5 s.
        if t + FALLBACK_POST[1] <= end:
            ua = mean_dir(accel, t - PRE[0], t - PRE[1])
            va = mean_dir(accel, t + FALLBACK_POST[0], t + FALLBACK_POST[1])
            if ua and va and angle(ua, va) >= THRESHOLD_DEG:
                return t + FALLBACK_POST[1], held_max
    return None, held_max


def evaluate(indices, m, params, cache):
    rows = []
    for i in indices:
        e = m[i]
        up = cache[(i, params["tau_acc"])]
        c, held = confirm(up, cache[(i, "acc")], impacts(i), params["threshold_deg"], params["dwell_s"], params["start_s"])
        latency = c - e["fall_onset_s"] if (c is not None and e["label"] == "fall") else None
        rows.append({"index": i, "label": e["label"], "activity": e["activity"], "subject": e.get("subject"),
                     "confirmed": c is not None, "latency": latency, "held_max_deg": held})
    falls = [r for r in rows if r["label"] == "fall"]
    adls = [r for r in rows if r["label"] != "fall"]
    lat = [r["latency"] for r in falls if r["latency"] is not None]
    return {
        "sensitivity": sum(r["confirmed"] for r in falls) / len(falls) if falls else None,
        "false_positive_rate": sum(r["confirmed"] for r in adls) / len(adls) if adls else None,
        "median_latency_s": statistics.median(lat) if lat else None,
        "largest_adl_held_deg": max((r["held_max_deg"] for r in adls), default=0.0),
        "largest_jump_held_deg": max((confirm(cache[(r["index"], params["tau_acc"])], cache[(r["index"], "acc")],
                                              [m[r["index"]]["impact_timestamp"]], 999.0, params["dwell_s"],
                                              params["start_s"])[1]
                                      for r in adls if r["activity"] in JUMPS), default=0.0),
        "false_positives": [f"{r['subject']} {r['activity']}" for r in adls if r["confirmed"]],
    }


def load_cache(indices, m):
    cache = {}
    with concurrent.futures.ThreadPoolExecutor(8) as pool:
        futures = {}
        for i in indices:
            for tau in GRID["tau_acc"]:
                futures[(i, tau)] = pool.submit(up_directions, i, m[i], tau)
            futures[(i, "acc")] = pool.submit(accel_samples, m[i])
        for key, f in futures.items():
            cache[key] = f.result()
    return cache


def candidates():
    for th in GRID["threshold_deg"]:
        for dw in GRID["dwell_s"]:
            for st in GRID["start_s"]:
                for tau in GRID["tau_acc"]:
                    yield {"threshold_deg": th, "dwell_s": dw, "start_s": st, "tau_acc": tau}


def main() -> int:
    command = sys.argv[1] if len(sys.argv) > 1 else "select"
    m = manifest()
    cgu = lambda splits: [i for i, e in enumerate(m) if e["dataset"] == "cgu_bes" and e["split"] in splits]
    if command == "quats":
        load_cache(range(len(m)), m)
        print(f"[quats] {len(m)} recordings x {len(GRID['tau_acc'])} tau_acc")
        return 0
    if command == "select":
        idx = cgu({"train", "validation"})
        cache = load_cache(idx, m)
        scored = []
        for p in candidates():
            r = evaluate(idx, m, p, cache)
            scored.append({**p, **{k: r[k] for k in ("sensitivity", "false_positive_rate",
                                                     "median_latency_s", "largest_jump_held_deg")}})
        eligible = [s for s in scored if s["false_positive_rate"] == 0.0
                    and s["sensitivity"] >= MIN_TRAIN_SENSITIVITY
                    and s["largest_jump_held_deg"] <= s["threshold_deg"] - MARGIN_DEG]
        chosen = min(eligible, key=lambda s: (s["median_latency_s"], -s["sensitivity"])) if eligible else None
        SELECTION.write_text(json.dumps({
            "rule": (f"train+validation CGU subjects only; eligible = 0 false positives, sensitivity >= "
                     f"{MIN_TRAIN_SENSITIVITY}, largest dwell-held jump angle (impact assumed at peak "
                     f"acceleration) <= threshold - {MARGIN_DEG} deg; choose the lowest median latency"),
            "chosen": chosen, "eligible": len(eligible), "grid": scored}, indent=2) + "\n")
        print(json.dumps(chosen, indent=2), f"\n[select] {len(eligible)} of {len(scored)} eligible")
        return 0
    if command == "test":
        chosen = json.loads(SELECTION.read_text())["chosen"]
        out = {}
        groups = {"cgu_test": cgu({"test"}), "cgu_all": cgu({"train", "validation", "test"}),
                  "uci_har": [i for i, e in enumerate(m) if e["dataset"] == "uci_har"],
                  "parkinson": [i for i, e in enumerate(m) if e["dataset"] == "parkinson"]}
        cache = load_cache(sorted(set().union(*groups.values())), m)
        for name, idx in groups.items():
            out[name] = evaluate(idx, m, chosen, cache)
        print(json.dumps({"chosen": chosen, "results": out}, indent=2))
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
