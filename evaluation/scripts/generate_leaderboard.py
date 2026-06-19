#!/usr/bin/env python3
"""Generate the odometry leaderboard injected into README.md.

The leaderboard ranks odometry methods (KISS-ICP, the LOAM family, CT-ICP, ...)
by RPE (relative drift %/100 m) over the five canonical KITTI Odometry full
sequences. RPE is seed-independent and is the honest local-accuracy metric;
their ATE accumulates unbounded drift over the run and is shown only as context.

GT-seeded methods (NDT, LiTAMIN2, GICP family) are deliberately NOT ranked: they
take the ground-truth pose as the per-frame initial guess, so their ATE measures
seed adherence rather than tracking -- and that ranking inverts (NDT's tiny ATE
comes from barely registering; with --no-gt-seed it diverges to ~87% RPE, the
worst tracker, while GICP's larger ATE reflects real registration). A short note
in the rendered block explains this; we do not put them in the ranked table.

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
    ("dogfooding_results/kitti_seq_00_full", "Seq 00", "4541 fr"),
    ("dogfooding_results/kitti_seq_02_full", "Seq 02", "4661 fr"),
    ("dogfooding_results/kitti_seq_05_full", "Seq 05", "2761 fr"),
    ("dogfooding_results/kitti_seq_07_full", "Seq 07", "1101 fr"),
    ("dogfooding_results/kitti_seq_08_full", "Seq 08", "4071 fr"),
]

# Odometry methods: no per-frame GT seed, so ATE accumulates drift; RPE is the
# fair, seed-independent quality metric. GT-seeded methods (NDT, LiTAMIN2, GICP
# family) are intentionally excluded from the ranking -- see the module docstring.
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


PRESERVED_SECTION_MARKER = "\n### From-paper reimplementations"


def extract_preserved_section(readme: str) -> str:
    """Keep manually curated sections that live below the generated odometry table."""
    if START_MARKER not in readme or END_MARKER not in readme:
        return ""
    inner = readme.split(START_MARKER, 1)[1].split(END_MARKER, 1)[0]
    if PRESERVED_SECTION_MARKER not in inner:
        return ""
    return inner[inner.index(PRESERVED_SECTION_MARKER):].strip()


def render_block(cells: dict, preserved_section: str = "") -> str:
    odom_table = _table(cells, ODOMETRY, "rpe")
    explorer = "https://rsasaki0109.github.io/localization_zoo/"
    preserved = ""
    if preserved_section:
        preserved = "\n\n" + preserved_section.strip()
    return f"""{START_MARKER}
## Leaderboard — odometry, RPE [drift %/100 m], lower is better

KITTI Odometry full sequences, ranked by relative pose error (the
seed-independent local-accuracy metric). ATE in parens is unbounded drift over
the run. Full matrix: [**explorer**]({explorer}).

{odom_table}

_Best variant per cell ([`docs/experiments.md`](docs/experiments.md)). KISS-ICP /
LOAM ~0.5–1.4% drift is competitive — their large ATE is honest drift, not a
broken port._

> **No GT-seeded methods here.** NDT / LiTAMIN2 / GICP use the ground-truth pose
> as the per-frame initial guess, so their ATE is seed adherence, not tracking —
> and the ranking even inverts: NDT's 0.02 m comes from *not* registering
> (`--no-gt-seed` → 87% RPE, the worst tracker), while GICP's larger ATE is real
> registration. They need a GT prior and aren't standalone odometry, so they are
> not ranked.
{preserved}
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
    readme = README_PATH.read_text()
    block = render_block(cells, extract_preserved_section(readme))

    if args.print_only:
        print(block)
        return 0

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
