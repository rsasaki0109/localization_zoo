#!/usr/bin/env python3
"""Generate the fixed-configuration KITTI leaderboard from pinned aggregates."""
from __future__ import annotations
import argparse
import hashlib
import html
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = "evaluation/data/fixed_configuration_leaderboard.json"
START = "<!-- FIXED-LEADERBOARD:START -->"
END = "<!-- FIXED-LEADERBOARD:END -->"


def number(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value) and value >= 0


def collect(root=ROOT):
    manifest = json.loads((root / MANIFEST).read_text())
    sequences = dict(sorted(manifest["sequences"].items()))
    rows = []
    for spec in manifest["methods"]:
        cells = {}
        for sequence, frames in sequences.items():
            source = spec["results"].get(sequence)
            cell = {"status": "missing", "source": source, "value": None}
            if source:
                path = root / source
                if path.is_file():
                    raw = path.read_bytes()
                    aggregate = json.loads(raw)
                    if aggregate["stable_interface"]["methods"] != spec["method"]:
                        raise ValueError(f"Method mismatch: {source}")
                    if Path(aggregate["dataset"]["pcd_dir"]).name != f"kitti_seq_{sequence}_full":
                        raise ValueError(f"Dataset mismatch: {source}")
                    variants = [v for v in aggregate["variants"] if v["id"] == spec["variant"]]
                    if len(variants) != 1:
                        raise ValueError(f"Expected exactly one pinned variant: {source}")
                    variant = variants[0]
                    if variant["args"] != spec["args"] or aggregate["dataset"].get("extra_args", []) != spec["dataset_extra_args"]:
                        raise ValueError(f"Configuration mismatch: {source}")
                    note = variant.get("note", "").lower()
                    audit = spec.get("odometry_audit", {})
                    audited = (audit.get("policy") == "first_pose_anchor_only"
                               and audit.get("source") == "evaluation/src/pcd_dogfooding.cpp"
                               and bool(audit.get("function")) and bool(audit.get("reason")))
                    arguments = variant["args"] + aggregate["dataset"].get("extra_args", [])
                    forbidden = any("gt-seed" in arg and arg != "--no-gt-seed" for arg in arguments)
                    if (("no gt seed" not in note and not audited) or forbidden
                            or "gt-seeded" in note or variant.get("design_style") == "ablation"):
                        raise ValueError(f"Odometry evidence missing or ablation: {source}")
                    cell.update(source_sha256=hashlib.sha256(raw).hexdigest(),
                                frames=variant.get("frames"), command=variant.get("command"),
                                recorded_status=variant.get("status"), recorded_note=variant.get("note"))
                    if variant.get("status") != "ok":
                        cell["status"] = "failed"
                    elif variant.get("frames") != frames:
                        cell["status"] = "incomplete"
                    elif not number(variant.get(manifest["metric"])):
                        cell["status"] = "metric_missing"
                    else:
                        cell.update(status="ok", value=variant[manifest["metric"]])
            cells[sequence] = cell
        complete = all(c["status"] == "ok" for c in cells.values())
        rows.append({**spec, "cells": cells, "rank": None,
                     "coverage": sum(c["status"] == "ok" for c in cells.values()),
                     # Built-in float sum changed in Python 3.12; use the same
                     # compensated summation on every supported interpreter.
                     "mean": math.fsum(c["value"] for c in cells.values()) / len(sequences) if complete else None})
    rows.sort(key=lambda r: (r["mean"] is None, r["mean"] if r["mean"] is not None else math.inf, r["name"]))
    for index, row in enumerate(rows):
        if row["mean"] is not None:
            row["rank"] = index + 1
    return {"schema_version": 1, "evidence_ref": manifest["evidence_ref"], "metric": manifest["metric"], "aggregation": manifest["aggregation"],
            "sequences": sequences, "methods": rows}


def render(data):
    esc = html.escape
    base = "https://github.com/rsasaki0109/localization_zoo/blob/" + data["evidence_ref"] + "/"
    lines = [START, '<h2 id="fixed-leaderboard">Fixed-configuration KITTI leaderboard</h2>',
        f'<p>{len(data["methods"])} explicitly pinned method configurations across sequences 00–10. Official KITTI translational RTE (100–800 m), in %. Lower is better.</p>',
        '<p class="note">Ranked by the unweighted arithmetic mean of all 11 sequence errors, not pooled segment error. Incomplete coverage is unranked. All KITTI sequences have been used during project development; this is a retrospective comparison, not an unseen-test claim. Input preprocessing differs between methods and is part of each fixed configuration.</p>',
        '<div class="table-wrap"><table><thead><tr><th>Rank</th><th>Method / fixed variant</th><th>Mean %</th><th>Coverage</th>']
    lines += [f'<th>{s}</th>' for s in data["sequences"]]
    lines.append('</tr></thead><tbody>')
    for row in data["methods"]:
        mean = f'{row["mean"]:.3f}' if row["mean"] is not None else "—"
        lines.append(f'<tr><td>{row["rank"] or "—"}</td><th scope="row"><a href="#fixed-{esc(row["method"])}">{esc(row["name"])}</a><br><small>{esc(row["variant"])}</small></th><td>{mean}</td><td>{row["coverage"]}/11</td>')
        for cell in row["cells"].values():
            value = f'{cell["value"]:.3f}' if cell["status"] == "ok" else esc(cell["status"].replace("_", " "))
            if cell["source"]:
                value = f'<a href="{base}{esc(cell["source"])}">{value}</a>'
            lines.append(f'<td>{value}</td>')
        lines.append('</tr>')
    lines.append('</tbody></table></div>')
    lines.append('<p class="note">Missing = no recorded result; incomplete = wrong frame count; failed = unsuccessful run; metric missing = official RTE unavailable. Large errors remain in the mean. Historical results are not a fresh rerun of current code.</p>')
    for row in data["methods"]:
        audit = row.get("odometry_audit")
        audit_html = (f'<p><a href="{base}{esc(audit["source"])}#L{int(audit["line"])}">'
                      f'{esc(audit["function"])}: initialization audit</a>. {esc(audit["reason"])}</p>') if audit else ""
        lines.extend([f'<details id="fixed-{esc(row["method"])}"><summary>{esc(row["name"])} — configuration and selection history</summary>',
            f'<p>Selection / calibration sequences: {esc(", ".join(row["selection_sequences"])) or "Not specified in source evidence"}.</p>',
            f'<p>{esc(row["selection_note"])}</p>',
            f'<p><a href="{base}{esc(row["selection_source"])}">Method evidence and limitations</a></p>',
            audit_html,
            f'<pre><code>{esc(" ".join(row["dataset_extra_args"] + row["args"]) or "(recorded native defaults)")}</code></pre></details>'])
    lines.extend(['<p class="note"><a href="./fixed_configuration_leaderboard.json">Download results and provenance (JSON)</a> · <a href="' + 'https://github.com/rsasaki0109/localization_zoo/blob/main/' + MANIFEST + '">Pinned configurations and sources</a></p>', END])
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    data = collect()
    page = ROOT / "docs/index.html"
    current = page.read_text()
    block = render(data)
    if START in current and END in current:
        updated = current.split(START, 1)[0] + block + current.split(END, 1)[1]
    else:
        anchor = '      <h2 id="leaderboard">'
        updated = current.replace(anchor, block + "\n\n" + anchor, 1)
        if updated == current:
            raise ValueError("Leaderboard insertion anchor missing")
    outputs = {page: updated, ROOT / "docs/fixed_configuration_leaderboard.json": json.dumps(data, indent=2, ensure_ascii=False) + "\n"}
    stale = [path for path, value in outputs.items() if not path.exists() or path.read_text() != value]
    if args.check:
        if stale:
            raise SystemExit("Fixed leaderboard stale: " + ", ".join(str(p.relative_to(ROOT)) for p in stale))
        print("Fixed leaderboard up to date.")
    else:
        for path in stale:
            path.write_text(outputs[path])
        print(f"Generated fixed leaderboard: {len(data['methods'])} methods.")

if __name__ == "__main__":
    main()
