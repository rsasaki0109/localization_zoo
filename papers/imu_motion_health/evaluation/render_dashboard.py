#!/usr/bin/env python3
"""Render a self-contained, LiDAR-free IMU replay dashboard.

The result is one HTML file with no CDN, web server, or JavaScript package
dependency.  It contains a time slider and play/pause replay controls for
the JSONL snapshots, cards for health and relative motion, inline SVG plots,
and event/truth tables.  This makes it useful in CI artifacts as well as on a
sensor engineer's laptop.

Example::

    python render_dashboard.py --jsonl build/imu_faults/runs/impact.jsonl \
      --summary build/imu_faults/runs/impact.summary.json \
      --truth build/imu_faults/impact.truth.json \
      --output build/imu_faults/impact.html

``--png-output`` is optional.  It is attempted only when matplotlib is
installed; the HTML remains standard-library-only.
"""

from __future__ import annotations

import argparse
import html
import json
import math
import pathlib
import sys
from typing import Any, Dict, Iterable, List, Mapping, Optional, Sequence, Tuple

try:
    from .evaluate_fault_matrix import (load_events, load_json, load_jsonl,
                                        split_records)
except ImportError:  # direct ``python render_dashboard.py`` execution
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
    from evaluate_fault_matrix import (load_events, load_json, load_jsonl,
                                       split_records)


SCHEMA = "imu_motion_health_dashboard_v1"


def _finite(value: Any, default: Optional[float] = None) -> Optional[float]:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return default
    return number if math.isfinite(number) else default


def _vector(value: Any) -> List[Optional[float]]:
    if not isinstance(value, list):
        return [None, None, None]
    values: List[Optional[float]] = []
    for item in value[:3]:
        values.append(_finite(item))
    while len(values) < 3:
        values.append(None)
    return values


def _series(snapshots: Iterable[Mapping[str, Any]]) -> List[Dict[str, Any]]:
    output: List[Dict[str, Any]] = []
    for snapshot in snapshots:
        output.append({
            "t": _finite(snapshot.get("timestamp")),
            "dt": _finite(snapshot.get("dt")),
            "state": str(snapshot.get("motion_state", "unknown")),
            "health": str(snapshot.get("health_state", "unknown")),
            "sample_accepted": bool(snapshot.get("sample_accepted", False)),
            "diagnostic": str(snapshot.get("diagnostic", "")),
            "confidence": _finite(snapshot.get("confidence"), 0.0),
            "gyro_norm": _finite(snapshot.get("gyro_norm"), 0.0),
            "accel_norm": _finite(snapshot.get("accel_norm"), 0.0),
            "linear_accel_norm": _finite(snapshot.get("linear_accel_norm"), 0.0),
            "vibration_rms": _finite(snapshot.get("vibration_rms"), 0.0),
            "tilt_angle_deg": _finite(snapshot.get("tilt_angle_deg"), 0.0),
            "relative_position": _vector(snapshot.get("relative_position")),
            "relative_velocity": _vector(snapshot.get("relative_velocity")),
            "flags": {
                key: bool(snapshot.get(key, False))
                for key in ("nonfinite", "nonmonotonic", "gap",
                            "gyro_saturated", "accel_saturated", "integrated")
            },
        })
    return output


def _safe_json(value: Any) -> str:
    text = json.dumps(value, ensure_ascii=False, separators=(",", ":"),
                      allow_nan=False)
    # A JSON string inside <script> must not be allowed to close the tag.
    return text.replace("</", "<\\/")


def _event_label(event: Mapping[str, Any]) -> str:
    value = event.get("type", event.get("event_type", event.get("name", "event")))
    return str(value)


def _duration(summary: Optional[Mapping[str, Any]], series: Sequence[Mapping[str, Any]]) -> Optional[float]:
    if summary is not None:
        value = _finite(summary.get("duration_s"))
        if value is not None:
            return value
    values = [_finite(item.get("t")) for item in series]
    finite = [item for item in values if item is not None]
    return finite[-1] - finite[0] if len(finite) >= 2 else None


def _png(path: pathlib.Path, series: Sequence[Mapping[str, Any]]) -> Optional[str]:
    """Write an optional diagnostic PNG if matplotlib is available."""

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        return "matplotlib is not installed; skipped PNG output"
    times = [_finite(item.get("t"), 0.0) or 0.0 for item in series]
    confidence = [_finite(item.get("confidence"), 0.0) or 0.0 for item in series]
    accel = [_finite(item.get("accel_norm"), 0.0) or 0.0 for item in series]
    figure, axis = plt.subplots(2, 1, figsize=(10, 5), sharex=True)
    axis[0].plot(times, confidence, color="#0f766e", linewidth=1.5)
    axis[0].set_ylabel("confidence")
    axis[0].set_ylim(0.0, 1.0)
    axis[0].grid(alpha=0.25)
    axis[1].plot(times, accel, color="#b45309", linewidth=1.2)
    axis[1].set_ylabel("accel norm (m/s²)")
    axis[1].set_xlabel("time (s)")
    axis[1].grid(alpha=0.25)
    figure.tight_layout()
    path.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(path, dpi=140)
    plt.close(figure)
    return None


def build_html(series: Sequence[Mapping[str, Any]], summary: Optional[Mapping[str, Any]],
               events: Sequence[Mapping[str, Any]], truth: Optional[Mapping[str, Any]],
               title: str) -> str:
    dashboard_data = {
        "schema": SCHEMA,
        "title": title,
        "series": list(series),
        "summary": dict(summary) if summary is not None else {},
        "events": [dict(item) for item in events],
        "truth": dict(truth) if truth is not None else {},
    }
    embedded = _safe_json(dashboard_data)
    # The JavaScript is deliberately small and has no external dependencies.
    # It uses SVG paths rather than canvas so screenshots and browser zoom stay
    # crisp in a CI artifact.
    return f'''<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(title)}</title>
<style>
:root {{ color-scheme: dark; --bg:#0b1220; --panel:#111c2e; --line:#293b55;
  --text:#e7eef7; --muted:#9fb0c5; --teal:#2dd4bf; --amber:#fbbf24;
  --red:#fb7185; --blue:#60a5fa; }}
* {{ box-sizing:border-box; }}
body {{ margin:0; padding:24px; background:var(--bg); color:var(--text);
  font:14px/1.45 system-ui,-apple-system,"Segoe UI",sans-serif; }}
h1 {{ margin:0 0 4px; font-size:24px; }} h2 {{ margin:0 0 10px; font-size:16px; }}
.subtitle {{ color:var(--muted); margin-bottom:18px; }}
.grid {{ display:grid; grid-template-columns:repeat(auto-fit,minmax(150px,1fr));
  gap:10px; margin-bottom:14px; }}
.card,.panel {{ border:1px solid var(--line); border-radius:10px; background:var(--panel);
  padding:13px; box-shadow:0 2px 10px #0002; }}
.label {{ color:var(--muted); font-size:11px; text-transform:uppercase; letter-spacing:.08em; }}
.value {{ font-size:21px; font-weight:650; margin-top:3px; word-break:break-word; }}
.ok {{ color:var(--teal); }} .warn {{ color:var(--amber); }} .bad {{ color:var(--red); }}
.controls {{ display:flex; align-items:center; gap:9px; flex-wrap:wrap; margin-bottom:12px; }}
button {{ background:#19304b; border:1px solid #376084; border-radius:6px; color:var(--text);
  padding:6px 11px; cursor:pointer; }} button:hover {{ background:#244663; }}
input[type=range] {{ flex:1; min-width:200px; accent-color:var(--teal); }}
.time {{ min-width:120px; color:var(--muted); font-variant-numeric:tabular-nums; }}
.chart {{ width:100%; height:205px; display:block; background:#0d1727; border-radius:6px; }}
.chart text {{ fill:var(--muted); font-size:10px; }} .chart .gridline {{ stroke:#26384e; stroke-width:1; }}
.chart .confidence {{ stroke:var(--teal); }} .chart .accel {{ stroke:var(--amber); }}
.chart .vibration {{ stroke:var(--red); }} .chart .cursor {{ stroke:#fff; stroke-dasharray:4 4; }}
.chart path {{ fill:none; stroke-width:1.6; vector-effect:non-scaling-stroke; }}
.two {{ display:grid; grid-template-columns:repeat(auto-fit,minmax(320px,1fr)); gap:14px;
  margin-bottom:14px; }}
table {{ border-collapse:collapse; width:100%; font-size:12px; }} th,td {{ text-align:left;
  padding:7px 6px; border-bottom:1px solid var(--line); vertical-align:top; }} th {{ color:var(--muted); }}
code {{ color:#b9d8ff; }} .empty {{ color:var(--muted); padding:10px 0; }}
ul {{ margin:5px 0 0 18px; padding:0; }}
</style>
</head>
<body>
<h1 id="title"></h1><div class="subtitle">IMU-only replay · no LiDAR input · self-contained artifact</div>
<div class="grid">
  <div class="card"><div class="label">motion state</div><div id="state" class="value">—</div></div>
  <div class="card"><div class="label">health</div><div id="health" class="value">—</div></div>
  <div class="card"><div class="label">confidence</div><div id="confidence" class="value">—</div></div>
  <div class="card"><div class="label">relative position</div><div id="position" class="value">—</div></div>
  <div class="card"><div class="label">relative velocity</div><div id="velocity" class="value">—</div></div>
  <div class="card"><div class="label">sample / duration</div><div id="stream" class="value">—</div></div>
</div>
<div class="panel controls">
  <button id="play">▶ Play</button><button id="reset">↺ Reset</button>
  <input id="slider" type="range" min="0" max="0" value="0" step="1" aria-label="replay sample">
  <span id="time" class="time">t = —</span>
</div>
<div class="two">
  <section class="panel"><h2>Confidence and acceleration</h2><svg id="metrics" class="chart" viewBox="0 0 800 205" role="img" aria-label="confidence and acceleration plot"></svg></section>
  <section class="panel"><h2>Relative motion</h2><svg id="motion" class="chart" viewBox="0 0 800 205" role="img" aria-label="relative position and velocity plot"></svg></section>
</div>
<div class="two">
  <section class="panel"><h2>Detected events</h2><div id="events"></div></section>
  <section class="panel"><h2>Truth / injected intervals</h2><div id="truth"></div></section>
</div>
<section class="panel"><h2>Diagnostics and counters</h2><div id="diagnostics"></div></section>
<script>
const DATA = {embedded};
const $ = (id) => document.getElementById(id);
const series = DATA.series || [];
const finite = (x, fallback=0) => Number.isFinite(Number(x)) ? Number(x) : fallback;
const fmt = (x, digits=3) => Number.isFinite(Number(x)) ? Number(x).toFixed(digits) : '—';
const vec = (v) => Array.isArray(v) ? '[' + v.map(x => fmt(x,3)).join(', ') + ']' : '—';
const esc = (v) => String(v ?? '').replace(/[&<>"']/g, c => ({{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}})[c]);
$('title').textContent = DATA.title || 'IMU Motion & Health Dashboard';
const slider = $('slider'); slider.max = Math.max(0, series.length - 1);
let cursor = 0; let playing = false; let timer = null;
function pathFor(key, width, height, minValue, maxValue) {{
  if (!series.length) return '';
  const values = series.map(s => finite(s[key], 0));
  let lo = minValue ?? Math.min(...values), hi = maxValue ?? Math.max(...values);
  if (!Number.isFinite(lo) || !Number.isFinite(hi)) {{ lo=0; hi=1; }}
  if (hi - lo < 1e-9) {{ hi=lo+1; }}
  return values.map((v,i) => {{ const x = i * width / Math.max(1,series.length-1);
    const y = height - (v-lo) * height / (hi-lo); return (i?'L':'M')+x.toFixed(2)+','+y.toFixed(2); }}).join(' ');
}}
function motionPath(component, key, width, height) {{
  const values = series.map(s => Array.isArray(s[key]) ? finite(s[key][component],0) : 0);
  let lo=Math.min(...values), hi=Math.max(...values); if (hi-lo<1e-9) {{ hi=lo+1; }}
  return values.map((v,i) => {{ const x=i*width/Math.max(1,series.length-1), y=height-(v-lo)*height/(hi-lo); return (i?'L':'M')+x.toFixed(2)+','+y.toFixed(2); }}).join(' ');
}}
function chartSvg(kind) {{
  const W=800,H=175, out=[]; out.push('<g transform="translate(0,12)">');
  [0,.25,.5,.75,1].forEach(q => {{ const y=H*q; out.push('<line class="gridline" x1="0" x2="800" y1="'+y+'" y2="'+y+'"/>'); }});
  if (kind==='metrics') {{
    out.push('<path class="confidence" d="'+pathFor('confidence',W,H,0,1)+'"/>');
    out.push('<path class="accel" d="'+pathFor('accel_norm',W,H)+'"/>');
    out.push('<path class="vibration" d="'+pathFor('vibration_rms',W,H)+'"/>');
  }} else {{
    out.push('<path class="confidence" d="'+motionPath(0,'relative_position',W,H)+'"/>');
    out.push('<path class="accel" d="'+motionPath(1,'relative_position',W,H)+'"/>');
    out.push('<path class="vibration" d="'+motionPath(2,'relative_position',W,H)+'"/>');
  }}
  const x=cursor*W/Math.max(1,series.length-1); out.push('<line class="cursor" x1="'+x+'" x2="'+x+'" y1="0" y2="'+H+'"/>');
  out.push('</g><text x="8" y="200">'+(kind==='metrics'?'teal confidence · amber accel · red vibration':'x/y/z relative position')+'</text>');
  return out.join('');
}}
function render() {{
  if (!series.length) return;
  const item=series[cursor] || series[series.length-1]; slider.value=cursor;
  $('state').textContent=item.state; $('health').textContent=item.health;
  $('state').className='value '+(item.state==='stationary'?'ok':(item.state==='impact'||item.state==='fall'?'bad':'warn'));
  $('health').className='value '+(item.health==='ready'?'ok':(item.health==='invalid'?'bad':'warn'));
  $('confidence').textContent=fmt(item.confidence,3);
  $('position').textContent=vec(item.relative_position); $('velocity').textContent=vec(item.relative_velocity);
  $('stream').textContent=(cursor+1)+' / '+series.length+' · '+fmt(item.t,3)+' s';
  $('time').textContent='t = '+fmt(item.t,3)+' s · Δt '+fmt(item.dt,4)+' s';
  $('metrics').innerHTML=chartSvg('metrics'); $('motion').innerHTML=chartSvg('motion');
}}
function tick() {{ if (!playing) return; cursor += 1; if (cursor >= series.length) {{ cursor=series.length-1; playing=false; $('play').textContent='▶ Play'; }} render(); if (playing) timer=setTimeout(tick,40); }}
$('play').onclick=() => {{ playing=!playing; $('play').textContent=playing?'Ⅱ Pause':'▶ Play'; if (playing) tick(); }};
$('reset').onclick=() => {{ playing=false; $('play').textContent='▶ Play'; cursor=0; render(); }};
slider.oninput=() => {{ playing=false; $('play').textContent='▶ Play'; cursor=Number(slider.value); render(); }};
function table(headers, rows) {{ if (!rows.length) return '<div class="empty">No records in this artifact.</div>'; return '<table><thead><tr>'+headers.map(h=>'<th>'+esc(h)+'</th>').join('')+'</tr></thead><tbody>'+rows.map(r=>'<tr>'+r.map(c=>'<td>'+esc(c)+'</td>').join('')+'</tr>').join('')+'</tbody></table>'; }}
const events=DATA.events || [];
$('events').innerHTML=table(['ID','Type','Start','End','Peak'], events.map((e,i)=>[e.id ?? e.event_id ?? i+1,_eventType(e),e.start_s ?? e.start_timestamp ?? '—',e.end_s ?? e.end_timestamp ?? '—',e.peak ?? e.peak_accel_mps2 ?? e.peak_value ?? '—']));
function _eventType(e) {{ return e.type ?? e.event_type ?? e.name ?? e.label ?? 'event'; }}
const intervals=(DATA.truth||{{}}).fault_intervals || [];
$('truth').innerHTML=table(['Label','Start','End','Details'], intervals.map(i=>[i.label ?? 'fault',i.start_s ?? '—',i.end_s ?? '—',Object.entries(i).filter(([k])=>!['label','kind','start_s','end_s'].includes(k)).map(([k,v])=>k+'='+v).join(', ')]));
const summary=DATA.summary || {{}}; const counters=summary.counters || {{}};
$('diagnostics').innerHTML=table(['Field','Value'], Object.entries({{schema:summary.schema ?? '—',rows_read:summary.rows_read ?? series.length,parse_errors:summary.parse_errors ?? 0,duration_s:summary.duration_s ?? '—',final_motion_state:summary.motion_state ?? (series.at(-1)||{{}}).state,final_health_state:summary.health_state ?? (series.at(-1)||{{}}).health,diagnostic:(summary.final_state||{{}}).diagnostic ?? '—',...counters}}));
render();
</script>
</body></html>'''


def render(jsonl_path: pathlib.Path, output_path: pathlib.Path,
           summary_path: Optional[pathlib.Path] = None,
           events_path: Optional[pathlib.Path] = None,
           truth_path: Optional[pathlib.Path] = None,
           title: Optional[str] = None,
           png_path: Optional[pathlib.Path] = None) -> List[str]:
    records = load_jsonl(jsonl_path)
    snapshots, embedded_summary = split_records(records)
    summary: Optional[Mapping[str, Any]] = embedded_summary
    if summary_path is not None and summary_path.is_file():
        loaded = load_json(summary_path)
        if isinstance(loaded, Mapping):
            summary = loaded
    truth: Optional[Mapping[str, Any]] = None
    if truth_path is not None and truth_path.is_file():
        loaded_truth = load_json(truth_path)
        if isinstance(loaded_truth, Mapping):
            truth = loaded_truth
    events = load_events(events_path, snapshots, summary)
    series = _series(snapshots)
    if title is None:
        title = str((truth or {}).get("scenario", jsonl_path.stem)) + " · IMU replay"
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(build_html(series, summary, events, truth, title),
                           encoding="utf-8", newline="\n")
    messages: List[str] = []
    if png_path is not None:
        warning = _png(png_path, series)
        if warning:
            messages.append(warning)
    return messages


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jsonl", required=True, type=pathlib.Path)
    parser.add_argument("--summary", type=pathlib.Path)
    parser.add_argument("--events", type=pathlib.Path)
    parser.add_argument("--truth", type=pathlib.Path)
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--png-output", type=pathlib.Path)
    parser.add_argument("--title")
    args = parser.parse_args(argv)
    if not args.jsonl.is_file():
        print(f"render_dashboard: JSONL not found: {args.jsonl}", file=sys.stderr)
        return 2
    try:
        warnings = render(args.jsonl, args.output, args.summary, args.events,
                          args.truth, args.title, args.png_output)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"render_dashboard: {error}", file=sys.stderr)
        return 2
    print(f"wrote {args.output}")
    for warning in warnings:
        print(f"warning: {warning}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
