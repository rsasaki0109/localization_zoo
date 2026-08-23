#!/usr/bin/env python3
"""Fetch, normalize, tune, and benchmark public LiDAR-free IMU datasets."""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import hashlib
import html
import json
import math
import pathlib
import shutil
import subprocess
import sys
import urllib.request
import zipfile

HERE = pathlib.Path(__file__).resolve().parent
REGISTRY = HERE / "public_datasets.json"
G = 9.80665
FALL_WORDS = ("forwardfall", "backwardfall", "leftfall", "rightfall")


def registry() -> dict:
    return json.loads(REGISTRY.read_text(encoding="utf-8"))["datasets"]


def _download(url: str, target: pathlib.Path) -> None:
    target.parent.mkdir(parents=True, exist_ok=True)
    with urllib.request.urlopen(url, timeout=120) as response, target.open("wb") as stream:
        shutil.copyfileobj(response, stream)


def _md5(path: pathlib.Path) -> str:
    digest = hashlib.md5()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def fetch(names: list[str], output: pathlib.Path) -> dict:
    records = []
    for name in names:
        spec = registry()[name]
        target = output / "raw" / spec["filename"]
        if "article_api" in spec:
            metadata = json.load(urllib.request.urlopen(spec["article_api"], timeout=60))
            remote = next(item for item in metadata["files"] if item["id"] == spec["file_id"])
            url = remote["download_url"]
            license_name = metadata["license"]["name"]
        else:
            url, license_name = spec["download_url"], spec["license"]
        if not target.exists() or (spec.get("md5") and _md5(target) != spec["md5"]):
            _download(url, target)
        digest = _md5(target)
        if spec.get("md5") and digest != spec["md5"]:
            raise ValueError(f"{name}: checksum mismatch: {digest}")
        if license_name != spec["license"]:
            raise ValueError(f"{name}: license changed from {spec['license']} to {license_name}")
        extracted = output / name; extracted.mkdir(parents=True, exist_ok=True)
        if target.suffix.lower() == ".zip":
            with zipfile.ZipFile(target) as archive: archive.extractall(extracted)
            for nested in extracted.glob("*.zip"):
                with zipfile.ZipFile(nested) as archive: archive.extractall(extracted)
        elif target.suffix.lower() == ".rar":
            extractor = next((shutil.which(item) for item in ("7zz", "7z", "bsdtar", "tar") if shutil.which(item)), None)
            if not extractor: raise ValueError("RAR extraction requires 7z, 7zz, bsdtar, or libarchive tar")
            command = [extractor, "x", "-y", f"-o{extracted}", str(target)] if pathlib.Path(extractor).stem.startswith("7") else [extractor, "-xf", str(target), "-C", str(extracted)]
            try:
                subprocess.run(command, check=True, capture_output=True)
            except subprocess.CalledProcessError as exc:
                raise ValueError(f"RAR extractor failed ({pathlib.Path(extractor).name}); install 7z or libarchive bsdtar") from exc
        records.append({"name": name, "path": str(target), "extracted": str(extracted), "bytes": target.stat().st_size,
                        "md5": digest, "license": license_name, "doi": spec["doi"]})
    receipt = {"schema": "imu_public_dataset_fetch_v1", "datasets": records}
    output.mkdir(parents=True, exist_ok=True)
    (output / "fetch_receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    return receipt


def _write_cli(path: pathlib.Path, rows, sample_rate_hz: float = 200.0) -> dict:
    path.parent.mkdir(parents=True, exist_ok=True)
    count = 0
    max_accel = max_gyro = 0.0
    accel_series = []; gyro_series = []; accel_vectors = []
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(("timestamp", "gx", "gy", "gz", "ax", "ay", "az"))
        for timestamp, gyro, accel in rows:
            writer.writerow([f"{timestamp:.9f}", *[f"{v:.9f}" for v in (*gyro, *accel)]])
            an = math.sqrt(sum(v * v for v in accel)); gn = math.sqrt(sum(v * v for v in gyro))
            accel_series.append(an); gyro_series.append(gn); accel_vectors.append(accel); max_accel = max(max_accel, an); max_gyro = max(max_gyro, gn); count += 1
    peak_index = max(range(len(accel_series)), key=accel_series.__getitem__) if accel_series else 0
    search_start = max(0, peak_index - int(2.0 * sample_rate_hz)); onset_index = peak_index
    for index in range(search_start, peak_index + 1):
        if abs(accel_series[index] - G) > 3.0 or gyro_series[index] > 0.5:
            onset_index = index; break
    window = max(1, min(len(accel_vectors), int(sample_rate_hz)))
    means = [[sum(v[axis] for v in segment) / len(segment) for axis in range(3)]
             for segment in (accel_vectors[:window], accel_vectors[-window:])]
    denominator = math.sqrt(sum(v*v for v in means[0])) * math.sqrt(sum(v*v for v in means[1]))
    cosine = max(-1.0, min(1.0, sum(a*b for a,b in zip(*means)) / denominator)) if denominator else 1.0
    posture_change = math.degrees(math.acos(cosine))
    return {"samples": count, "max_accel_norm": max_accel, "max_gyro_norm": max_gyro,
            "posture_change_deg": posture_change,
            "impact_timestamp": peak_index / sample_rate_hz, "motion_onset_s": onset_index / sample_rate_hz}


def convert_cgu(source: pathlib.Path, output: pathlib.Path) -> list[dict]:
    entries = []
    for path in sorted(source.glob("Subject*.txt")):
        stem = path.stem; subject, activity = stem.split("_", 1)
        with path.open(encoding="utf-8") as stream:
            raw = list(csv.reader(stream))[4:]
        def rows():
            for index, item in enumerate(raw):
                if len(item) >= 6:
                    values = [float(v.strip()) for v in item[:6]]
                    yield index / 200.0, tuple(values[3:6]), tuple(v * G for v in values[0:3])
        destination = output / "cgu_bes" / subject / f"{activity}.csv"
        stats = _write_cli(destination, rows())
        is_fall = activity.lower() in FALL_WORDS
        proxy_onset = stats["motion_onset_s"] if is_fall else None
        split = "train" if int(subject[-2:]) <= 9 else "validation" if int(subject[-2:]) <= 12 else "test"
        entries.append({"dataset": "cgu_bes", "subject": subject, "activity": activity,
                        "label": "fall" if is_fall else "adl", "split": split,
                        "path": str(destination.relative_to(output)), "truth_source": "kinematic_proxy" if is_fall else "activity_label",
                        "fall_onset_s": proxy_onset, **stats})
    return entries


def convert_uci(source: pathlib.Path, output: pathlib.Path, limit: int = 0) -> list[dict]:
    root = source / "UCI HAR Dataset"
    labels = dict(line.split(maxsplit=1) for line in (root / "activity_labels.txt").read_text().splitlines())
    entries = []
    for partition in ("train", "test"):
        subjects = (root / partition / f"subject_{partition}.txt").read_text().split()
        activities = (root / partition / f"y_{partition}.txt").read_text().split()
        signal_root = root / partition / "Inertial Signals"
        names = [f"body_gyro_{a}_{partition}.txt" for a in "xyz"] + [f"total_acc_{a}_{partition}.txt" for a in "xyz"]
        streams = [[list(map(float, line.split())) for line in (signal_root / name).read_text().splitlines()] for name in names]
        partition_count = 0
        partition_limit = math.ceil(limit / 2) if limit else 0
        for index, (subject, activity_id) in enumerate(zip(subjects, activities)):
            if partition_limit and partition_count >= partition_limit: break
            rows = ((sample / 50.0, tuple(streams[a][index][sample] for a in range(3)),
                     tuple(streams[a][index][sample] * G for a in range(3, 6))) for sample in range(128))
            activity = labels[activity_id].lower()
            destination = output / "uci_har" / f"Subject{int(subject):02d}" / f"{partition}_{index:05d}_{activity}.csv"
            stats = _write_cli(destination, rows, 50.0)
            entries.append({"dataset": "uci_har", "subject": f"Subject{int(subject):02d}", "activity": activity,
                            "label": "adl", "split": partition, "path": str(destination.relative_to(output)),
                            "truth_source": "video_activity_label", "fall_onset_s": None, **stats})
            partition_count += 1
    return entries


def convert_parkinson(source: pathlib.Path, output: pathlib.Path, limit: int = 0) -> list[dict]:
    entries = []
    for index, path in enumerate(sorted(source.glob("PD_*Ankle.csv"))):
        if limit and index >= limit: break
        data = list(csv.DictReader(path.open(encoding="utf-8")))
        first = dt.datetime.fromisoformat(data[0]["loggingTime"])
        def rows():
            last = -1.0
            for item in data:
                required = [item.get("loggingTime", "")] + [item.get(f"accelerometerAcceleration{a}", "") for a in "XYZ"] + [item.get(f"gyroRotation{a}", "") for a in "XYZ"]
                if any(not value.strip() for value in required): continue
                timestamp = (dt.datetime.fromisoformat(item["loggingTime"]) - first).total_seconds()
                if timestamp <= last: continue
                last = timestamp
                accel = tuple(float(item[f"accelerometerAcceleration{a}"]) for a in "XYZ")
                gyro = tuple(math.radians(float(item[f"gyroRotation{a}"])) for a in "XYZ")
                yield timestamp, gyro, accel
        subject = path.stem.split("_")[1]; side = "left" if "Left" in path.stem else "right"
        destination = output / "parkinson" / f"Subject{int(subject):02d}" / f"{side}_ankle.csv"
        stats = _write_cli(destination, rows())
        entries.append({"dataset": "parkinson", "subject": f"Subject{int(subject):02d}", "activity": f"gait_{side}",
                        "label": "adl", "split": "external", "path": str(destination.relative_to(output)),
                        "truth_source": "dataset_gait_context", "fall_onset_s": None, **stats})
    return entries


def tune(entries: list[dict], output: pathlib.Path) -> dict:
    candidates = [(a, g, p) for a in (35.0, 40.0, 45.0, 46.0, 47.0, 50.0)
                  for g in (1.0, 1.5, 2.0, 2.5, 3.0) for p in (15.0, 30.0, 45.0)]
    train = [e for e in entries if e["dataset"] == "cgu_bes" and e["split"] == "train"]
    scored = []
    for accel, gyro, posture in candidates:
        predicts = lambda e: (e["max_accel_norm"] >= accel and e["max_gyro_norm"] >= gyro
                              and e["posture_change_deg"] >= posture)
        tp = sum(e["label"] == "fall" and predicts(e) for e in train)
        fn = sum(e["label"] == "fall" and not predicts(e) for e in train)
        fp = sum(e["label"] == "adl" and predicts(e) for e in train)
        tn = sum(e["label"] == "adl" and not predicts(e) for e in train)
        sensitivity = tp / (tp + fn); fpr = fp / (fp + tn)
        scored.append({"impact_accel_threshold": accel, "impact_gyro_threshold": gyro,
                       "minimum_posture_change_deg": posture,
                       "sensitivity": sensitivity, "false_positive_rate": fpr})
    best = min(scored, key=lambda x: (x["false_positive_rate"] > .02, x["sensitivity"] < .95,
                                      -x["sensitivity"], x["false_positive_rate"], x["impact_accel_threshold"]))
    profile = output / "wearable-public-v1.yaml"; profile.parent.mkdir(parents=True, exist_ok=True)
    profile.write_text("imu_motion_health:\n  profile: wearable\n" +
                       f"  impact_accel_threshold: {best['impact_accel_threshold']}\n" +
                       f"  impact_gyro_threshold: {best['impact_gyro_threshold']}\n" +
                       "  impact_requires_accel_and_gyro: true\n" +
                       "  fall_freefall_threshold: 0.1\n  tilt_angle_threshold_deg: 179.0\n", encoding="utf-8")
    policy = output / "wearable-public-v1-policy.json"
    policy.write_text(json.dumps({"schema":"imu_fall_candidate_policy_v1",
                                  "minimum_posture_change_deg":best["minimum_posture_change_deg"],
                                  "event_type":"impact"}, indent=2)+"\n", encoding="utf-8")
    return {"schema": "imu_public_profile_tuning_v1", "training_split": "CGU-BES Subject01-09",
            "objective": {"minimum_sensitivity": .95, "maximum_false_positive_rate": .02},
            "selected": best, "all_candidates": scored, "profile": str(profile)}


def _read_events(path: pathlib.Path) -> list[dict]:
    return [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines() if line.strip()] if path.exists() else []


def benchmark(manifest: dict, root: pathlib.Path, cli: pathlib.Path, profile: pathlib.Path,
              output: pathlib.Path, policy_path: pathlib.Path | None = None) -> dict:
    policy = (json.loads(policy_path.read_text(encoding="utf-8")) if policy_path else
              {"event_type": "impact", "minimum_posture_change_deg": 0.0})
    results = []
    for index, entry in enumerate(manifest["recordings"]):
        events = output / "runs" / f"{index:05d}.events.jsonl"; summary = output / "runs" / f"{index:05d}.summary.json"
        events.parent.mkdir(parents=True, exist_ok=True)
        process = subprocess.run([str(cli), "--input", str(root / entry["path"]), "--profile-file", str(profile),
                                  "--events-output", str(events), "--summary-output", str(summary)], capture_output=True, text=True)
        records = _read_events(events); detections = [e for e in records if e["phase"] == "started" and e["type"] == "impact"]
        detected = bool(detections) and entry["posture_change_deg"] >= policy["minimum_posture_change_deg"]; latency = None
        if entry["label"] == "fall" and detected:
            latency = detections[0]["timestamp"] - entry["fall_onset_s"]
        results.append({**entry, "detected": detected, "latency_s": latency, "cli_exit": process.returncode})
    groups = {}
    for dataset in sorted({e["dataset"] for e in results}):
        rows = [e for e in results if e["dataset"] == dataset]; falls = [e for e in rows if e["label"] == "fall"]; adl = [e for e in rows if e["label"] == "adl"]
        latencies = [e["latency_s"] for e in falls if e["latency_s"] is not None]
        groups[dataset] = {"recordings": len(rows), "fall_sensitivity": sum(e["detected"] for e in falls) / len(falls) if falls else None,
                           "adl_false_positive_rate": sum(e["detected"] for e in adl) / len(adl) if adl else None,
                           "median_latency_s": sorted(latencies)[len(latencies)//2] if latencies else None,
                           "cli_failures": sum(e["cli_exit"] != 0 for e in rows)}
    cgu_test = [e for e in results if e["dataset"] == "cgu_bes" and e["split"] == "test"]
    test_falls = [e for e in cgu_test if e["label"] == "fall"]; test_adl = [e for e in cgu_test if e["label"] == "adl"]
    acceptance = {"fall_sensitivity_at_least_0_95": sum(e["detected"] for e in test_falls) / len(test_falls) >= .95,
                  "adl_false_positive_rate_at_most_0_02": sum(e["detected"] for e in test_adl) / len(test_adl) <= .02,
                  "all_cli_replays_succeeded": all(e["cli_exit"] == 0 for e in results)}
    return {"schema": "imu_public_dataset_benchmark_v1", "groups": groups, "acceptance": acceptance,
            "passed": all(acceptance.values()), "results": results}


def render(report: dict, output: pathlib.Path, baseline: dict | None = None) -> None:
    rows = []
    for name, group in report["groups"].items():
        sensitivity = "—" if group["fall_sensitivity"] is None else f"{group['fall_sensitivity']:.1%}"
        false_positive = "—" if group["adl_false_positive_rate"] is None else f"{group['adl_false_positive_rate']:.1%}"
        latency = "—" if group["median_latency_s"] is None else f"{group['median_latency_s']:.3f}"
        rows.append(f"<tr><td>{html.escape(name)}</td><td>{group['recordings']}</td><td>{sensitivity}</td><td>{false_positive}</td><td>{latency}</td></tr>")
    comparison = ""
    if baseline:
        base = baseline["groups"]["cgu_bes"]; tuned = report["groups"]["cgu_bes"]
        comparison = ("<h2>Baseline → tuned</h2><table><tr><th>Metric</th><th>Baseline wearable</th><th>wearable-public-v1</th></tr>"
                      f"<tr><td>Fall sensitivity</td><td>{base['fall_sensitivity']:.1%}</td><td>{tuned['fall_sensitivity']:.1%}</td></tr>"
                      f"<tr><td>ADL false positives</td><td>{base['adl_false_positive_rate']:.1%}</td><td>{tuned['adl_false_positive_rate']:.1%}</td></tr></table>")
    document = f"""<!doctype html><meta charset=utf-8><title>Public IMU benchmark</title><style>body{{font:15px system-ui;margin:2rem;max-width:1100px}}table{{border-collapse:collapse;width:100%}}th,td{{padding:.6rem;border-bottom:1px solid #ddd;text-align:left}}.ok{{color:#087830}}.bad{{color:#b42318}}pre{{background:#f6f8fa;padding:1rem;overflow:auto}}</style><h1>Public IMU benchmark</h1><h2 class={'ok' if report['passed'] else 'bad'}>{'PASS' if report['passed'] else 'NEEDS TUNING'}</h2><table><tr><th>Dataset</th><th>Recordings</th><th>Fall sensitivity</th><th>ADL false positives</th><th>Median latency (s)</th></tr>{''.join(rows)}</table>{comparison}<p>CGU-BES latency uses an explicitly labeled kinematic proxy because the dataset has activity-level, not frame-level, fall onset labels.</p><pre>{html.escape(json.dumps({'baseline': baseline, 'tuned': report} if baseline else report, indent=2))}</pre>"""
    output.parent.mkdir(parents=True, exist_ok=True); output.write_text(document, encoding="utf-8")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__); sub = parser.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("fetch"); p.add_argument("--output", type=pathlib.Path, required=True); p.add_argument("--dataset", action="append", choices=registry(), required=True)
    p = sub.add_parser("convert"); p.add_argument("--cgu", type=pathlib.Path, required=True); p.add_argument("--uci", type=pathlib.Path, required=True); p.add_argument("--parkinson", type=pathlib.Path, required=True); p.add_argument("--output", type=pathlib.Path, required=True); p.add_argument("--uci-limit", type=int, default=300); p.add_argument("--parkinson-limit", type=int, default=100)
    p = sub.add_parser("tune"); p.add_argument("--manifest", type=pathlib.Path, required=True); p.add_argument("--output", type=pathlib.Path, required=True)
    p = sub.add_parser("benchmark"); p.add_argument("--manifest", type=pathlib.Path, required=True); p.add_argument("--root", type=pathlib.Path, required=True); p.add_argument("--cli", type=pathlib.Path, required=True); p.add_argument("--profile", type=pathlib.Path, required=True); p.add_argument("--policy", type=pathlib.Path); p.add_argument("--output", type=pathlib.Path, required=True); p.add_argument("--allow-fail", action="store_true")
    p = sub.add_parser("report"); p.add_argument("--input", type=pathlib.Path, required=True); p.add_argument("--baseline", type=pathlib.Path); p.add_argument("--output", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    try:
        if args.cmd == "fetch": fetch(args.dataset, args.output)
        elif args.cmd == "convert":
            recordings = convert_cgu(args.cgu, args.output) + convert_uci(args.uci, args.output, args.uci_limit) + convert_parkinson(args.parkinson, args.output, args.parkinson_limit)
            (args.output / "manifest.json").write_text(json.dumps({"schema":"imu_public_recording_manifest_v1","recordings":recordings}, indent=2)+"\n", encoding="utf-8")
        elif args.cmd == "tune":
            result = tune(json.loads(args.manifest.read_text())["recordings"], args.output); (args.output / "tuning.json").write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
        elif args.cmd == "benchmark":
            result = benchmark(json.loads(args.manifest.read_text()),args.root,args.cli,args.profile,args.output,args.policy); (args.output/"benchmark.json").write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8"); return 0 if result["passed"] or args.allow_fail else 1
        else: render(json.loads(args.input.read_text()), args.output,
                     json.loads(args.baseline.read_text()) if args.baseline else None)
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc: print(f"error: {exc}",file=sys.stderr); return 2
    return 0

if __name__ == "__main__": raise SystemExit(main())
