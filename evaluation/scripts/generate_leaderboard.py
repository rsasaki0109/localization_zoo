#!/usr/bin/env python3
"""Generate the two-tier method leaderboard injected into README.md.

A single ATE ranking is misleading on this benchmark, because methods fall into
two non-comparable groups:

  * GT-seeded localization (NDT, LiTAMIN2, GICP family) re-anchor to the ground
    truth every frame with a weak-update fallback, so their ATE is the per-frame
    registration residual and is bounded by the seed. NDT in particular leans
    almost entirely on the seed: with --no-gt-seed it diverges (KITTI Raw 0009
    full: 310 m ATE / 41.5% RPE), yet seeded it reports a physically impossible
    0.07% RPE -- i.e. it is essentially emitting the GT pose. Ranking these by
    ATE rewards GT substitution, not registration quality.

  * Odometry (KISS-ICP, the LOAM family, CT-ICP, ...) run with no GT seed, so
    their ATE accumulates unbounded drift over the sequence. Their honest
    quality metric is RPE (relative drift %/100 m), which is seed-independent.

So this renders TWO tables over the five canonical KITTI Odometry full
sequences: GT-seeded localization ranked by ATE, and odometry ranked by RPE.
Each cell is the best result across all tuned variants for that method.

Usage:
    python3 evaluation/scripts/generate_leaderboard.py            # rewrite README
    python3 evaluation/scripts/generate_leaderboard.py --check    # fail if stale
    python3 evaluation/scripts/generate_leaderboard.py --print    # stdout only
"""
from __future__ import annotations

import argparse
import glob
import json
import math
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
README_PATH = REPO_ROOT / "README.md"

START_MARKER = "<!-- LEADERBOARD:START -->"
END_MARKER = "<!-- LEADERBOARD:END -->"

# Leaderboard columns: the five canonical KITTI Odometry full sequences (the
# broadest, densest multi-method coverage and the most recognized benchmark).
COLUMNS = [
    ("dogfooding_results/kitti_seq_00_full", "Seq 00", "4542 fr"),
    ("dogfooding_results/kitti_seq_02_full", "Seq 02", "4661 fr"),
    ("dogfooding_results/kitti_seq_05_full", "Seq 05", "2761 fr"),
    ("dogfooding_results/kitti_seq_07_full", "Seq 07", "1101 fr"),
    ("dogfooding_results/kitti_seq_08_full", "Seq 08", "4071 fr"),
]

# GT-seeded scan-to-map methods: ATE is the per-frame residual from the seed.
SEEDED = ["NDT", "LiTAMIN2", "GICP", "Small-GICP", "Voxel-GICP"]

# Odometry methods: no per-frame GT seed, so ATE accumulates drift; RPE is the
# fair, seed-independent quality metric.
ODOMETRY = [
    "KISS-ICP", "GenZ-ICP", "A-LOAM", "F-LOAM", "LeGO-LOAM", "SuMa",
    "LOAM-Livox", "MULLS", "CT-ICP", "CT-LIO", "FAST-LIO2", "Point-LIO",
    "DLIO", "DLO", "LINS", "LIO-SAM", "ISC-LOAM", "FAST-LIO-SLAM",
    "HDL-Graph-SLAM", "BALM2", "VGICP-SLAM", "RKO-LIO", "X-ICP", "CLINS",
]


def collect_cells() -> dict:
    """{(method, dataset): {'ate', 'ate_rpe', 'rpe', 'rpe_ate'}} over all aggregates.

    Per (method, dataset): the best-ATE variant (and its paired RPE) and,
    separately, the best-RPE variant (and its paired ATE). Different tables rank
    by different metrics, so both are kept.
    """
    cells: dict[tuple[str, str], dict] = {}
    for path in glob.glob(str(RESULTS_DIR / "*.json")):
        if Path(path).name == "index.json":
            continue
        try:
            agg = json.loads(Path(path).read_text())
        except (json.JSONDecodeError, OSError):
            continue
        if not isinstance(agg, dict) or "variants" not in agg:
            continue
        method = agg.get("stable_interface", {}).get("primary_method")
        dataset = (agg.get("dataset") or {}).get("pcd_dir") \
            or (agg.get("problem", {}).get("dataset", {}) or {}).get("pcd_dir")
        if not method or not dataset:
            continue
        for variant in agg.get("variants", []):
            if variant.get("status") != "ok":
                continue
            ate = variant.get("ate_m")
            rpe = variant.get("rpe_trans_pct")
            cell = cells.setdefault((method, dataset),
                                    {"ate": None, "ate_rpe": None,
                                     "rpe": None, "rpe_ate": None})
            if isinstance(ate, (int, float)) and math.isfinite(ate):
                if cell["ate"] is None or ate < cell["ate"]:
                    cell["ate"], cell["ate_rpe"] = float(ate), rpe
            if isinstance(rpe, (int, float)) and math.isfinite(rpe):
                if cell["rpe"] is None or rpe < cell["rpe"]:
                    cell["rpe"], cell["rpe_ate"] = float(rpe), ate
    return cells


def _table(cells, methods, metric):
    """Render one markdown table. metric is 'ate' or 'rpe' (the ranking key)."""
    col_dirs = [c[0] for c in COLUMNS]
    present = [m for m in methods
               if any((m, d) in cells and cells[(m, d)][metric] is not None
                      for d in col_dirs)]

    def vals(m):
        return [cells[(m, d)][metric] for d in col_dirs
                if (m, d) in cells and cells[(m, d)][metric] is not None]

    present.sort(key=lambda m: (sum(vals(m)) / len(vals(m))) if vals(m) else math.inf)

    col_min = {}
    for d in col_dirs:
        v = [cells[(m, d)][metric] for m in present
             if (m, d) in cells and cells[(m, d)][metric] is not None]
        if v:
            col_min[d] = min(v)

    head = ["Method"] + [c[1] for c in COLUMNS]
    sub = [""] + [f"_{c[2]}_" for c in COLUMNS]
    lines = [
        "| " + " | ".join(head) + " |",
        "|" + "|".join(["---"] + ["---:" for _ in COLUMNS]) + "|",
        "| " + " | ".join(sub) + " |",
    ]
    for m in present:
        row = [m]
        for d in col_dirs:
            cell = cells.get((m, d))
            val = cell[metric] if cell else None
            if val is None:
                row.append("–")
                continue
            if metric == "ate":
                txt = f"{val:.3f}"
            else:  # rpe: show drift% with ATE drift context
                ate_ctx = cell["rpe_ate"]
                txt = f"{val:.2f}%"
                if isinstance(ate_ctx, (int, float)) and math.isfinite(ate_ctx):
                    txt += f" <sub>({ate_ctx:.0f} m)</sub>"
            if col_min.get(d) is not None and abs(val - col_min[d]) < 1e-9:
                # bold only the leading numeric token to keep <sub> intact
                lead, _, rest = txt.partition(" ")
                txt = f"**{lead}**" + (f" {rest}" if rest else "")
            row.append(txt)
        lines.append("| " + " | ".join(row) + " |")
    return "\n".join(lines)


def render_block(cells: dict) -> str:
    seeded_table = _table(cells, SEEDED, "ate")
    odom_table = _table(cells, ODOMETRY, "rpe")
    explorer = "https://rsasaki0109.github.io/localization_zoo/"
    return f"""{START_MARKER}
## Leaderboard

KITTI Odometry full sequences. Two groups that **can't share an axis**, each
ranked by its honest metric. Full matrix: [**explorer**]({explorer}).

### GT-seeded localization — ATE [m], lower is better

GT pose is the initial guess every frame, so ATE measures seed adherence, not tracking.

{seeded_table}

> ⚠️ **NDT's score is the GT guess, not registration.** Seq 07 seeded **0.076 m
> / 0.20% RPE**, but `--no-gt-seed` → **125.6 m / 87.4% RPE**. Not the rollback
> gate (fallback only 1.8%): the coarse-voxel solve barely moves off the
> already-correct guess.

### Odometry (no GT seed) — RPE [drift %/100 m], lower is better

No seed, so ATE (in parens) is unbounded drift; RPE is the fair metric.

{odom_table}

_Best variant per cell ([`docs/experiments.md`](docs/experiments.md)). KISS-ICP /
LOAM ~0.5–1.4% drift is competitive — their large ATE is honest drift, not a
broken port._
{END_MARKER}"""


def inject(readme: str, block: str) -> str:
    if START_MARKER in readme and END_MARKER in readme:
        pre = readme.split(START_MARKER)[0]
        post = readme.split(END_MARKER, 1)[1]
        return pre + block + post
    anchor = "\n## Scope Note\n"
    if anchor not in readme:
        raise SystemExit("Could not find insertion anchor ('## Scope Note') in README.md")
    idx = readme.index(anchor)
    return readme[:idx] + "\n" + block + "\n" + readme[idx:]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if README is stale")
    parser.add_argument("--print", dest="print_only", action="store_true",
                        help="print the block to stdout, do not edit README")
    args = parser.parse_args()

    cells = collect_cells()
    block = render_block(cells)

    if args.print_only:
        print(block)
        return 0

    readme = README_PATH.read_text()
    updated = inject(readme, block)

    if args.check:
        if updated != readme:
            print("README leaderboard is stale; run generate_leaderboard.py", file=sys.stderr)
            return 1
        print("README leaderboard up to date.")
        return 0

    if updated != readme:
        README_PATH.write_text(updated)
        print(f"Updated leaderboard in {README_PATH.relative_to(REPO_ROOT)}")
    else:
        print("README leaderboard already up to date.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
