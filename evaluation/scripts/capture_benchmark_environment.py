#!/usr/bin/env python3
"""Capture the benchmark host and toolchain for the paper environment table (Table 8).

Records the machine this script runs on (CPU, memory, OS, compiler, build tools,
and the C++ libraries the zoo links against), plus every host record already
committed under evaluation/data/. Historical experiment aggregates do not record
which host produced them, so the table states that boundary instead of implying
one machine ran everything.

Writes under docs/assets/paper/:
  - benchmark_environment.json
  - benchmark_environment.md
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[2]
ASSETS_DIR = REPO_ROOT / "docs" / "assets" / "paper"
EVIDENCE_DIR = REPO_ROOT / "evaluation" / "data"

# (label, pkg-config modules to try, Debian packages to try)
LIBRARIES = [
    ("Eigen", ["eigen3"], ["libeigen3-dev"]),
    ("PCL", ["pcl_common-1.14", "pcl_common-1.12", "pcl_common-1.10", "pcl_common"], ["libpcl-dev"]),
    ("Ceres Solver", [], ["libceres-dev"]),
    ("glog", ["libglog"], ["libgoogle-glog-dev"]),
    ("GoogleTest", ["gtest"], ["libgtest-dev"]),
    ("OpenCV", ["opencv4"], ["libopencv-dev"]),
]

TOOLS = [
    ("C++ compiler", ["c++", "--version"]),
    ("CMake", ["cmake", "--version"]),
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=ASSETS_DIR,
        help="Directory for JSON/Markdown (default: docs/assets/paper)",
    )
    return parser.parse_args()


def run(cmd: list[str]) -> str:
    if shutil.which(cmd[0]) is None:
        return ""
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=10, check=False)
    except (OSError, subprocess.TimeoutExpired):
        return ""
    return result.stdout.strip() if result.returncode == 0 else ""


def cpu_model() -> str:
    cpuinfo = Path("/proc/cpuinfo")
    if cpuinfo.is_file():
        match = re.search(r"^model name\s*:\s*(.+)$", cpuinfo.read_text(), re.MULTILINE)
        if match:
            return match.group(1).strip()
    return platform.processor() or platform.machine()


def memory_gib() -> float | None:
    meminfo = Path("/proc/meminfo")
    if meminfo.is_file():
        match = re.search(r"^MemTotal:\s*(\d+)\s*kB", meminfo.read_text(), re.MULTILINE)
        if match:
            return round(int(match.group(1)) / 1024 / 1024, 1)
    return None


def os_name() -> str:
    os_release = Path("/etc/os-release")
    if os_release.is_file():
        match = re.search(r'^PRETTY_NAME="?([^"\n]+)"?', os_release.read_text(), re.MULTILINE)
        if match:
            return match.group(1)
    return f"{platform.system()} {platform.release()}"


def library_version(pkg_modules: list[str], deb_packages: list[str]) -> str:
    for module in pkg_modules:
        version = run(["pkg-config", "--modversion", module])
        if version:
            return version
    for package in deb_packages:
        version = run(["dpkg-query", "-W", "-f=${Version}", package])
        if version:
            return version
    return ""


def capture_host() -> dict[str, Any]:
    tools = {label: (run(cmd).splitlines() or [""])[0] for label, cmd in TOOLS}
    tools["Python"] = platform.python_version()
    return {
        "cpu": cpu_model(),
        "logical_cores": os.cpu_count(),
        "memory_gib": memory_gib(),
        "gpu": (run(["nvidia-smi", "--query-gpu=name", "--format=csv,noheader"]).splitlines() or ["none detected"])[0],
        "os": os_name(),
        "kernel": platform.release(),
        "architecture": platform.machine(),
        "tools": tools,
        "libraries": {label: library_version(mods, debs) for label, mods, debs in LIBRARIES},
    }


def collect_recorded_hosts(evidence_dir: Path, repo_root: Path) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for path in sorted(evidence_dir.glob("*.json")):
        try:
            data = json.loads(path.read_text())
        except (OSError, json.JSONDecodeError):
            continue
        host = data.get("host") if isinstance(data, dict) else None
        if not isinstance(host, dict):
            continue
        records.append(
            {
                "evidence": str(path.relative_to(repo_root)),
                "host": {key: value for key, value in host.items() if not isinstance(value, (dict, list))},
                "runtime": data.get("dependencies", {}).get("runtime", ""),
            }
        )
    return records


def render_markdown(payload: dict[str, Any]) -> str:
    host = payload["capture_host"]
    rows = [
        ("CPU", host["cpu"]),
        ("Logical cores", host["logical_cores"]),
        ("Memory", f"{host['memory_gib']} GiB" if host["memory_gib"] is not None else ""),
        ("GPU", host["gpu"]),
        ("OS", host["os"]),
        ("Kernel", host["kernel"]),
        ("Architecture", host["architecture"]),
    ]
    rows += list(host["tools"].items())
    rows += list(host["libraries"].items())
    lines = [
        "# Benchmark Environment (Table 8)",
        "",
        f"Generated by `evaluation/scripts/capture_benchmark_environment.py` at {payload['generated_at']}.",
        "",
        "## Capture host",
        "",
        "| Component | Value |",
        "|---|---|",
    ]
    lines += [f"| {label} | {value if value not in ('', None) else 'not found'} |" for label, value in rows]
    lines += ["", "## Hosts recorded in committed evidence", ""]
    if payload["recorded_hosts"]:
        lines += ["| Evidence | Host | Runtime |", "|---|---|---|"]
        for record in payload["recorded_hosts"]:
            host_text = ", ".join(f"{key}: {value}" for key, value in record["host"].items())
            lines.append(f"| `{record['evidence']}` | {host_text} | {record['runtime']} |")
    else:
        lines.append("None.")
    lines += ["", "## Provenance boundary", "", payload["provenance_note"], ""]
    return "\n".join(lines)


def main() -> None:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    payload = {
        "schema_version": 1,
        "generated_at": datetime.now(timezone.utc).replace(microsecond=0).isoformat(),
        "capture_host": capture_host(),
        "recorded_hosts": collect_recorded_hosts(EVIDENCE_DIR, REPO_ROOT),
        "provenance_note": (
            "Experiment aggregates under experiments/results/ do not record the host that "
            "produced each run. The capture host is the current benchmark machine, not a "
            "claim that every historical row ran on it; FPS values from different hosts "
            "should not be compared directly."
        ),
    }
    (args.output_dir / "benchmark_environment.json").write_text(json.dumps(payload, indent=2) + "\n")
    (args.output_dir / "benchmark_environment.md").write_text(render_markdown(payload))
    print(f"[done] wrote {args.output_dir / 'benchmark_environment.json'}")
    print(f"[done] wrote {args.output_dir / 'benchmark_environment.md'}")


if __name__ == "__main__":
    main()
