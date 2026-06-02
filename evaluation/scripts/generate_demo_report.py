#!/usr/bin/env python3
"""Generate a self-contained HTML report from demo_localization_zoo outputs."""

from __future__ import annotations

import argparse
import html
import json
import math
import re
from datetime import datetime, timezone
from pathlib import Path


COLORS = [
    "#2563eb",
    "#0f766e",
    "#9f1239",
    "#a16207",
    "#7c3aed",
    "#475569",
]


def esc(value: object) -> str:
    return html.escape(str(value), quote=True)


def fmt(value: object, digits: int = 3) -> str:
    if value is None:
        return "-"
    try:
        number = float(value)
    except (TypeError, ValueError):
        return str(value)
    if not math.isfinite(number):
        return "-"
    return f"{number:.{digits}f}"


def load_json(path: Path) -> dict:
    if not path.exists():
        return {}
    with path.open() as handle:
        return json.load(handle)


def parse_synthetic_log(path: Path) -> list[dict]:
    if not path.exists():
        return []
    pattern = re.compile(
        r"^(?P<name>[A-Za-z0-9_+-]+)\s+"
        r"(?P<ate>[0-9.]+)\s+"
        r"(?P<frames>[0-9]+)\s+"
        r"(?P<time_ms>[0-9.]+)\s+"
        r"(?P<fps>[0-9.]+)\s*$"
    )
    rows = []
    for line in path.read_text().splitlines():
        match = pattern.match(line.strip())
        if not match:
            continue
        item = match.groupdict()
        rows.append(
            {
                "name": item["name"],
                "ate_m": float(item["ate"]),
                "frames": int(item["frames"]),
                "time_ms": float(item["time_ms"]),
                "fps": float(item["fps"]),
            }
        )
    return rows


def load_poses(path: Path) -> list[tuple[float, float]]:
    poses = []
    if not path.exists():
        return poses
    with path.open() as handle:
        for line in handle:
            values = line.strip().split()
            if len(values) != 12:
                continue
            try:
                numbers = [float(value) for value in values]
            except ValueError:
                continue
            poses.append((numbers[3], numbers[7]))
    return poses


def normalize_name(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", value.lower())


def split_csv(value: str) -> list[str]:
    return [item.strip() for item in value.split(",") if item.strip()]


def trajectory_series(pose_dir: Path, preferred_names: list[str]) -> list[dict]:
    if not pose_dir.exists():
        return []

    series = []
    gt_path = pose_dir / "gt.txt"
    gt_poses = load_poses(gt_path)
    if gt_poses:
        series.append({"name": "Ground Truth", "points": gt_poses, "color": "#111827"})

    pose_files = {
        normalize_name(path.stem): path
        for path in sorted(pose_dir.glob("*.txt"))
        if path.name != "gt.txt"
    }
    used = set()
    for index, name in enumerate(preferred_names):
        key = normalize_name(name)
        path = pose_files.get(key)
        if path is None:
            continue
        poses = load_poses(path)
        if not poses:
            continue
        used.add(key)
        series.append(
            {
                "name": name,
                "points": poses,
                "color": COLORS[index % len(COLORS)],
            }
        )

    for key, path in pose_files.items():
        if key in used:
            continue
        poses = load_poses(path)
        if not poses:
            continue
        series.append(
            {
                "name": path.stem.replace("_", "-"),
                "points": poses,
                "color": COLORS[len(series) % len(COLORS)],
            }
        )

    return series


def trajectory_svg(series: list[dict], title: str) -> str:
    width = 720
    height = 360
    pad = 42
    if not series:
        return '<div class="empty">No trajectory poses found.</div>'

    points = [point for item in series for point in item["points"]]
    xs = [point[0] for point in points]
    ys = [point[1] for point in points]
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    if abs(max_x - min_x) < 1e-6:
        min_x -= 0.5
        max_x += 0.5
    if abs(max_y - min_y) < 1e-6:
        min_y -= 0.5
        max_y += 0.5

    def project(point: tuple[float, float]) -> tuple[float, float]:
        x, y = point
        sx = pad + (x - min_x) / (max_x - min_x) * (width - 2 * pad)
        sy = height - pad - (y - min_y) / (max_y - min_y) * (height - 2 * pad)
        return sx, sy

    grid = []
    for idx in range(5):
        gx = pad + idx * (width - 2 * pad) / 4
        gy = pad + idx * (height - 2 * pad) / 4
        grid.append(f'<line x1="{gx:.1f}" y1="{pad}" x2="{gx:.1f}" y2="{height - pad}" />')
        grid.append(f'<line x1="{pad}" y1="{gy:.1f}" x2="{width - pad}" y2="{gy:.1f}" />')

    paths = []
    legend = []
    for item in series:
        coords = [project(point) for point in item["points"]]
        if len(coords) == 1:
            x, y = coords[0]
            paths.append(
                f'<circle cx="{x:.1f}" cy="{y:.1f}" r="3.5" fill="{item["color"]}" />'
            )
        else:
            polyline = " ".join(f"{x:.1f},{y:.1f}" for x, y in coords)
            dash = "" if item["name"] == "Ground Truth" else ' stroke-dasharray="7 5"'
            stroke_width = "3" if item["name"] == "Ground Truth" else "2"
            paths.append(
                f'<polyline points="{polyline}" fill="none" stroke="{item["color"]}" '
                f'stroke-width="{stroke_width}" stroke-linejoin="round" '
                f'stroke-linecap="round"{dash} />'
            )
        legend.append(
            f'<span><i style="background:{item["color"]}"></i>{esc(item["name"])}</span>'
        )

    return f"""
      <figure class="chart">
        <figcaption>{esc(title)}</figcaption>
        <svg viewBox="0 0 {width} {height}" role="img" aria-label="{esc(title)}">
          <rect x="0" y="0" width="{width}" height="{height}" rx="10" fill="#f8fafc" />
          <g class="grid">{"".join(grid)}</g>
          <line x1="{pad}" y1="{height - pad}" x2="{width - pad}" y2="{height - pad}" class="axis" />
          <line x1="{pad}" y1="{pad}" x2="{pad}" y2="{height - pad}" class="axis" />
          {"".join(paths)}
        </svg>
        <div class="legend">{"".join(legend)}</div>
      </figure>
    """


def bar_svg(rows: list[dict], title: str, value_key: str, suffix: str) -> str:
    values = [
        (row["name"], float(row[value_key]))
        for row in rows
        if row.get(value_key) is not None and math.isfinite(float(row[value_key]))
    ]
    if not values:
        return '<div class="empty">No numeric values found.</div>'
    values.sort(key=lambda item: item[1])
    max_value = max(value for _, value in values) or 1.0
    width = 720
    row_h = 38
    pad_x = 132
    height = 52 + row_h * len(values)
    bars = []
    for index, (name, value) in enumerate(values):
        y = 34 + index * row_h
        bar_w = (width - pad_x - 70) * (value / max_value)
        color = COLORS[index % len(COLORS)]
        bars.append(
            f'<text x="16" y="{y + 17}" class="bar-label">{esc(name)}</text>'
            f'<rect x="{pad_x}" y="{y}" width="{bar_w:.1f}" height="22" rx="4" fill="{color}" />'
            f'<text x="{pad_x + bar_w + 8:.1f}" y="{y + 16}" class="bar-value">{fmt(value)} {esc(suffix)}</text>'
        )
    return f"""
      <figure class="chart">
        <figcaption>{esc(title)}</figcaption>
        <svg viewBox="0 0 {width} {height}" role="img" aria-label="{esc(title)}">
          <rect x="0" y="0" width="{width}" height="{height}" rx="10" fill="#f8fafc" />
          {"".join(bars)}
        </svg>
      </figure>
    """


def table(headers: list[tuple[str, str]], rows: list[dict]) -> str:
    if not rows:
        return '<p class="empty">No rows.</p>'
    head = "".join(f"<th>{esc(label)}</th>" for _, label in headers)
    body_rows = []
    for row in rows:
        cells = []
        for key, _ in headers:
            value = row.get(key)
            if key.endswith("_m") or key in {"fps", "time_ms", "ate_m"}:
                value = fmt(value, 3 if key != "fps" else 1)
            cells.append(f"<td>{esc(value)}</td>")
        body_rows.append(f"<tr>{''.join(cells)}</tr>")
    return f"""
      <div class="table-wrap">
        <table>
          <thead><tr>{head}</tr></thead>
          <tbody>{"".join(body_rows)}</tbody>
        </table>
      </div>
    """


def status_summary(rows: list[dict]) -> str:
    if not rows:
        return "missing"
    bad = [row for row in rows if str(row.get("status", "")).lower() != "ok"]
    if bad:
        return f"{len(bad)} non-OK"
    return "all OK"


def best_method(rows: list[dict]) -> str:
    valid = [
        row for row in rows
        if row.get("ate_m") is not None and str(row.get("status", "ok")).lower() == "ok"
    ]
    if not valid:
        return "-"
    best = min(valid, key=lambda row: float(row["ate_m"]))
    return f"{best['name']} {fmt(best['ate_m'])} m"


def fastest_method(rows: list[dict]) -> str:
    valid = [
        row for row in rows
        if row.get("fps") is not None and str(row.get("status", "ok")).lower() == "ok"
    ]
    if not valid:
        return "-"
    fastest = max(valid, key=lambda row: float(row["fps"]))
    return f"{fastest['name']} {fmt(fastest['fps'], 1)} FPS"


def render(args: argparse.Namespace, generated_at: str) -> str:
    demo_dir = Path(args.demo_dir)
    synthetic_rows = parse_synthetic_log(demo_dir / "synthetic_benchmark.log")
    lidar = load_json(demo_dir / "lidar_fixture_summary.json")
    multimodal = load_json(demo_dir / "multimodal_fixture_summary.json")
    lidar_rows = lidar.get("methods", [])
    multimodal_rows = multimodal.get("methods", [])
    synthetic_names = [row["name"] for row in synthetic_rows]
    lidar_names = [row.get("name", "") for row in lidar_rows]
    requested_lidar_methods = split_csv(args.lidar_methods)
    requested_multimodal_methods = split_csv(args.multimodal_methods)
    multimodal_status = "skipped" if args.skip_multimodal else status_summary(multimodal_rows)

    synthetic_plot = trajectory_svg(
        trajectory_series(demo_dir / "benchmark_results", synthetic_names),
        "Synthetic trajectory overlay",
    )
    lidar_plot = trajectory_svg(
        trajectory_series(demo_dir / "dogfooding_results", lidar_names),
        "Committed MCD fixture trajectory overlay",
    )

    synthetic_table = table(
        [
            ("name", "Method"),
            ("ate_m", "ATE [m]"),
            ("frames", "Frames"),
            ("time_ms", "Time [ms]"),
            ("fps", "FPS"),
        ],
        synthetic_rows,
    )
    lidar_table = table(
        [
            ("name", "Method"),
            ("status", "Status"),
            ("ate_m", "ATE [m]"),
            ("frames", "Frames"),
            ("time_ms", "Time [ms]"),
            ("fps", "FPS"),
            ("note", "Note"),
        ],
        lidar_rows,
    )
    multimodal_table = table(
        [
            ("name", "Method"),
            ("status", "Status"),
            ("ate_m", "ATE [m]"),
            ("frames", "Frames"),
            ("time_ms", "Time [ms]"),
            ("fps", "FPS"),
            ("note", "Note"),
        ],
        multimodal_rows,
    )

    return f"""<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Localization Zoo Demo Report</title>
    <style>
      :root {{
        --bg: #f5f7fb;
        --surface: #ffffff;
        --subtle: #f8fafc;
        --ink: #17202a;
        --muted: #667085;
        --line: #d8dee8;
        --blue: #2563eb;
        --teal: #0f766e;
      }}
      * {{ box-sizing: border-box; }}
      body {{
        margin: 0;
        font-family: Inter, "Segoe UI", Arial, sans-serif;
        background: var(--bg);
        color: var(--ink);
      }}
      main {{
        width: min(1120px, calc(100vw - 28px));
        margin: 0 auto;
        padding: 34px 0 64px;
      }}
      .hero, .panel {{
        border: 1px solid var(--line);
        border-radius: 8px;
        background: var(--surface);
        box-shadow: 0 18px 44px rgba(23, 32, 42, 0.08);
      }}
      .hero {{
        padding: 26px;
        margin-bottom: 18px;
      }}
      h1, h2, h3, p {{ overflow-wrap: anywhere; }}
      h1 {{
        max-width: 13ch;
        margin: 0;
        font-size: 4rem;
        line-height: 1.02;
      }}
      h2 {{ margin: 0; font-size: 1.65rem; }}
      h3 {{ margin: 0 0 10px; font-size: 1.08rem; }}
      .eyebrow {{
        margin: 0 0 8px;
        color: var(--teal);
        font-size: 0.76rem;
        font-weight: 800;
        text-transform: uppercase;
      }}
      .lede {{
        max-width: 72ch;
        color: var(--muted);
        line-height: 1.6;
      }}
      .metrics {{
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(190px, 1fr));
        gap: 10px;
        margin-top: 18px;
      }}
      .metric {{
        min-width: 0;
        padding: 14px;
        border: 1px solid var(--line);
        border-radius: 8px;
        background: var(--subtle);
      }}
      .metric span {{
        display: block;
        color: var(--muted);
        font-size: 0.75rem;
        font-weight: 800;
        text-transform: uppercase;
      }}
      .metric strong {{
        display: block;
        margin-top: 6px;
        font-size: 1.2rem;
        line-height: 1.22;
        overflow-wrap: anywhere;
      }}
      .panel {{
        padding: 20px;
        margin-top: 18px;
      }}
      .grid {{
        display: grid;
        grid-template-columns: minmax(0, 1fr) minmax(280px, 0.5fr);
        gap: 16px;
        align-items: start;
      }}
      .chart {{
        margin: 0;
      }}
      .chart figcaption {{
        margin-bottom: 8px;
        color: var(--muted);
        font-weight: 800;
      }}
      svg {{
        width: 100%;
        height: auto;
        border: 1px solid var(--line);
        border-radius: 8px;
      }}
      .grid line {{ stroke: #dbe3ee; stroke-width: 1; }}
      .axis {{ stroke: #94a3b8; stroke-width: 1.4; }}
      .legend {{
        display: flex;
        flex-wrap: wrap;
        gap: 8px 14px;
        margin-top: 10px;
        color: var(--muted);
        font-size: 0.88rem;
      }}
      .legend span {{
        display: inline-flex;
        align-items: center;
        gap: 6px;
      }}
      .legend i {{
        width: 12px;
        height: 12px;
        border-radius: 50%;
      }}
      .bar-label, .bar-value {{
        fill: var(--ink);
        font: 700 14px Inter, "Segoe UI", Arial, sans-serif;
      }}
      .bar-value {{ fill: var(--muted); }}
      .table-wrap {{
        overflow-x: auto;
        border: 1px solid var(--line);
        border-radius: 8px;
      }}
      table {{
        width: 100%;
        min-width: 760px;
        border-collapse: collapse;
      }}
      th, td {{
        padding: 10px;
        border-bottom: 1px solid var(--line);
        text-align: left;
        vertical-align: top;
      }}
      th {{
        background: var(--subtle);
        color: var(--muted);
        font-size: 0.74rem;
        font-weight: 850;
        text-transform: uppercase;
      }}
      td {{
        line-height: 1.45;
        font-size: 0.9rem;
        font-variant-numeric: tabular-nums;
      }}
      tr:last-child td {{ border-bottom: 0; }}
      pre {{
        overflow-x: auto;
        margin: 12px 0 0;
        padding: 14px;
        border-radius: 8px;
        background: #111827;
        color: #e5e7eb;
      }}
      .empty {{ color: var(--muted); }}
      @media (max-width: 840px) {{
        .grid {{ grid-template-columns: 1fr; }}
        h1 {{ font-size: 2.7rem; }}
      }}
    </style>
  </head>
  <body>
    <main>
      <section class="hero">
        <p class="eyebrow">Localization Zoo</p>
        <h1>Demo Report</h1>
        <p class="lede">
          Generated from the one-command demo. The real-data checks use the committed
          three-frame MCD fixture, so this report can be reproduced from the repository.
        </p>
        <div class="metrics">
          <div class="metric"><span>Profile</span><strong>{esc(args.profile)}</strong></div>
          <div class="metric"><span>Generated</span><strong>{esc(generated_at)}</strong></div>
          <div class="metric"><span>LiDAR Methods</span><strong>{len(lidar_rows)} / {len(requested_lidar_methods) or len(lidar_rows)}</strong></div>
          <div class="metric"><span>LiDAR</span><strong>{esc(status_summary(lidar_rows))}</strong></div>
          <div class="metric"><span>Best LiDAR ATE</span><strong>{esc(best_method(lidar_rows))}</strong></div>
          <div class="metric"><span>Fastest LiDAR</span><strong>{esc(fastest_method(lidar_rows))}</strong></div>
          <div class="metric"><span>Multimodal Methods</span><strong>{len(multimodal_rows)} / {len(requested_multimodal_methods) or len(multimodal_rows)}</strong></div>
          <div class="metric"><span>Multimodal</span><strong>{esc(multimodal_status)}</strong></div>
        </div>
      </section>

      <section class="panel">
        <p class="eyebrow">Reproduce</p>
        <h2>Run This Locally</h2>
        <pre><code>{esc(args.command)}</code></pre>
      </section>

      <section class="panel">
        <p class="eyebrow">Synthetic</p>
        <h2>No-Download Benchmark</h2>
        <div class="grid">
          <div>{synthetic_plot}</div>
          <div>{bar_svg(synthetic_rows, "Synthetic ATE ranking", "ate_m", "m")}</div>
        </div>
        {synthetic_table}
      </section>

      <section class="panel">
        <p class="eyebrow">Real Fixture</p>
        <h2>Committed MCD LiDAR Smoke</h2>
        <div class="metrics">
          <div class="metric"><span>Frames</span><strong>{esc(lidar.get("num_frames", "-"))}</strong></div>
          <div class="metric"><span>Trajectory</span><strong>{fmt(lidar.get("trajectory_length_m"), 4)} m</strong></div>
          <div class="metric"><span>Timestamp Source</span><strong>{esc(lidar.get("timestamp_source", "-"))}</strong></div>
        </div>
        <div class="grid">
          <div>{lidar_plot}</div>
          <div>{bar_svg(lidar_rows, "LiDAR fixture ATE ranking", "ate_m", "m")}</div>
        </div>
        {lidar_table}
      </section>

      <section class="panel">
        <p class="eyebrow">Multimodal</p>
        <h2>Camera-Aware Fixture Smoke</h2>
        <div class="metrics">
          <div class="metric"><span>Frames</span><strong>{esc(multimodal.get("num_frames", "-"))}</strong></div>
          <div class="metric"><span>Landmarks</span><strong>{esc(multimodal.get("num_landmarks", "-"))}</strong></div>
          <div class="metric"><span>Observations</span><strong>{esc(multimodal.get("num_observations", "-"))}</strong></div>
        </div>
        {multimodal_table}
      </section>
    </main>
  </body>
</html>
"""


def build_manifest(args: argparse.Namespace, generated_at: str) -> dict:
    demo_dir = Path(args.demo_dir)
    synthetic_rows = parse_synthetic_log(demo_dir / "synthetic_benchmark.log")
    lidar = load_json(demo_dir / "lidar_fixture_summary.json")
    multimodal = load_json(demo_dir / "multimodal_fixture_summary.json")
    lidar_rows = lidar.get("methods", [])
    multimodal_rows = multimodal.get("methods", [])
    requested_lidar_methods = split_csv(args.lidar_methods)
    requested_multimodal_methods = split_csv(args.multimodal_methods)

    lidar_ok = bool(lidar_rows) and all(
        str(row.get("status", "")).lower() == "ok" for row in lidar_rows
    )
    if args.skip_multimodal:
        multimodal_ok = True
    else:
        multimodal_ok = bool(multimodal_rows) and all(
            str(row.get("status", "")).lower() == "ok" for row in multimodal_rows
        )
    synthetic_ok = bool(synthetic_rows)

    return {
        "schema_version": 1,
        "generated_at": generated_at,
        "command": args.command,
        "status": "ok" if synthetic_ok and lidar_ok and multimodal_ok else "needs_attention",
        "run": {
            "profile": args.profile,
            "lidar_methods": requested_lidar_methods,
            "multimodal_methods": requested_multimodal_methods,
            "multimodal_enabled": not args.skip_multimodal,
        },
        "artifacts": {
            "report_html": "report.html",
            "synthetic_log": "synthetic_benchmark.log",
            "lidar_summary": "lidar_fixture_summary.json",
            "multimodal_summary": "multimodal_fixture_summary.json",
            "synthetic_trajectories": "benchmark_results",
            "lidar_trajectories": "dogfooding_results",
        },
        "synthetic": {
            "method_count": len(synthetic_rows),
            "methods": [
                {
                    "name": row["name"],
                    "ate_m": row["ate_m"],
                    "frames": row["frames"],
                    "fps": row["fps"],
                }
                for row in synthetic_rows
            ],
        },
        "lidar_fixture": {
            "num_frames": lidar.get("num_frames"),
            "trajectory_length_m": lidar.get("trajectory_length_m"),
            "timestamp_source": lidar.get("timestamp_source"),
            "method_count": len(lidar_rows),
            "methods": [
                {
                    "name": row.get("name"),
                    "status": row.get("status"),
                    "ate_m": row.get("ate_m"),
                    "frames": row.get("frames"),
                    "fps": row.get("fps"),
                }
                for row in lidar_rows
            ],
        },
        "multimodal_fixture": {
            "num_frames": multimodal.get("num_frames"),
            "num_landmarks": multimodal.get("num_landmarks"),
            "num_observations": multimodal.get("num_observations"),
            "method_count": len(multimodal_rows),
            "methods": [
                {
                    "name": row.get("name"),
                    "status": row.get("status"),
                    "ate_m": row.get("ate_m"),
                    "frames": row.get("frames"),
                    "fps": row.get("fps"),
                }
                for row in multimodal_rows
            ],
        },
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--demo-dir",
        default="experiments/results/runs/demo_localization_zoo",
        help="Directory produced by demo_localization_zoo.sh.",
    )
    parser.add_argument("--output", default="", help="Output HTML path.")
    parser.add_argument(
        "--command",
        default="bash evaluation/scripts/demo_localization_zoo.sh",
        help="Command shown in the report.",
    )
    parser.add_argument("--profile", default="quick", help="Demo method profile label.")
    parser.add_argument(
        "--lidar-methods",
        default="",
        help="Comma-separated LiDAR methods requested by the demo script.",
    )
    parser.add_argument(
        "--multimodal-methods",
        default="",
        help="Comma-separated multimodal methods requested by the demo script.",
    )
    parser.add_argument(
        "--skip-multimodal",
        action="store_true",
        help="Mark multimodal validation as intentionally skipped.",
    )
    parser.add_argument("--manifest-output", default="", help="Output manifest path.")
    args = parser.parse_args()

    demo_dir = Path(args.demo_dir)
    output = Path(args.output) if args.output else demo_dir / "report.html"
    manifest_output = (
        Path(args.manifest_output) if args.manifest_output else demo_dir / "manifest.json"
    )
    generated_at = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    output.parent.mkdir(parents=True, exist_ok=True)
    manifest_output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(render(args, generated_at))
    manifest_output.write_text(
        json.dumps(build_manifest(args, generated_at), indent=2) + "\n"
    )
    print(f"demo report: {output}")
    print(f"demo manifest: {manifest_output}")


if __name__ == "__main__":
    main()
