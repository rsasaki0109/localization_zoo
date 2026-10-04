#!/usr/bin/env python3
"""Causal posture-confirmation analysis for the public IMU fall benchmark.

The committed benchmark gates impacts with posture_change_deg computed from the
first and last second of the whole recording (non-causal). Here each CLI impact
event at time t is confirmed only from data a device would have by t + post_end:
mean gravity direction over [t - pre_start, t - pre_end] versus
[t + post_start, t + post_end]. Latency is measured to the confirmation time.
"""
import csv, json, math, pathlib, statistics, sys

ROOT = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else pathlib.Path("build/imu_public_data")
RES = ROOT / "results"


def load_accel(path):
    rows = list(csv.DictReader(open(path)))
    return [(float(r["timestamp"]), (float(r["ax"]), float(r["ay"]), float(r["az"]))) for r in rows]


def mean_dir(samples, a, b):
    sel = [v for t, v in samples if a <= t <= b]
    if not sel:
        return None
    m = [sum(v[i] for v in sel) / len(sel) for i in range(3)]
    n = math.sqrt(sum(x * x for x in m))
    return [x / n for x in m] if n > 0 else None


def angle(u, v):
    return math.degrees(math.acos(max(-1.0, min(1.0, sum(a * b for a, b in zip(u, v))))))


def evaluate(entries, impacts, accel, pre, post, thr):
    out = []
    for i, e in enumerate(entries):
        s = accel[i]; t_end = s[-1][0]
        confirm = None
        for t in impacts[i]:
            if t + post[1] > t_end:
                continue  # recording ends before confirmation is possible
            u = mean_dir(s, t - pre[0], t - pre[1]); v = mean_dir(s, t + post[0], t + post[1])
            if u and v and angle(u, v) >= thr:
                confirm = t + post[1]; break
        lat = confirm - e["fall_onset_s"] if (confirm is not None and e["label"] == "fall") else None
        out.append((e, confirm is not None, lat))
    return out


def summarize(rows, pred):
    sel = [r for r in rows if pred(r[0])]
    falls = [r for r in sel if r[0]["label"] == "fall"]; adls = [r for r in sel if r[0]["label"] != "fall"]
    sens = sum(r[1] for r in falls) / len(falls) if falls else None
    fp = sum(r[1] for r in adls) / len(adls) if adls else None
    lats = [r[2] for r in falls if r[2] is not None]
    return {"n": len(sel), "falls": len(falls), "sensitivity": sens, "adl_fp": fp,
            "median_latency_s": statistics.median(lats) if lats else None}


def main():
    manifest = json.load(open(ROOT / "normalized/manifest.json"))["recordings"]
    impacts = []
    for i in range(len(manifest)):
        ev = [json.loads(l) for l in open(RES / "runs" / f"{i:05d}.events.jsonl") if l.strip()]
        impacts.append([e["timestamp"] for e in ev if e["phase"] == "started" and e["type"] == "impact"])
    accel = [load_accel(ROOT / "normalized" / e["path"]) for e in manifest]

    groups = {
        "cgu_train": lambda e: e["dataset"] == "cgu_bes" and e["split"] == "train",
        "cgu_val": lambda e: e["dataset"] == "cgu_bes" and e["split"] == "validation",
        "cgu_test": lambda e: e["dataset"] == "cgu_bes" and e["split"] == "test",
        "uci_har": lambda e: e["dataset"] == "uci_har",
        "parkinson": lambda e: e["dataset"] == "parkinson",
    }
    # Oracle (committed) gate for reference.
    oracle = []
    for i, e in enumerate(manifest):
        det = bool(impacts[i]) and e["posture_change_deg"] >= 30.0
        lat = impacts[i][0] - e["fall_onset_s"] if det and e["label"] == "fall" else None
        oracle.append((e, det, lat))
    print("oracle (committed, non-causal):", {k: summarize(oracle, g) for k, g in groups.items()})

    best = None
    for pre in [(1.0, 0.0), (1.5, 0.5), (2.0, 1.0)]:
        for post in [(0.0, 0.5), (0.25, 0.75), (0.5, 1.0), (0.5, 1.5)]:
            for thr in [20.0, 30.0, 45.0]:
                rows = evaluate(manifest, impacts, accel, pre, post, thr)
                tr = summarize(rows, groups["cgu_train"])
                ok = tr["sensitivity"] is not None and tr["sensitivity"] >= 0.95 and tr["adl_fp"] <= 0.02
                key = (ok, -(tr["median_latency_s"] or 99), tr["sensitivity"], -tr["adl_fp"])
                if best is None or key > best[0]:
                    best = (key, pre, post, thr, rows)
                print(f"pre={pre} post={post} thr={thr}: train sens={tr['sensitivity']:.3f} fp={tr['adl_fp']:.3f} lat={tr['median_latency_s']}")
    _, pre, post, thr, rows = best
    print("\nselected on cgu_train:", pre, post, thr)
    print(json.dumps({k: summarize(rows, g) for k, g in groups.items()}, indent=1))


if __name__ == "__main__":
    main()
