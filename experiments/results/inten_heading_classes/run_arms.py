#!/usr/bin/env python3
"""Run the InTEn-LOAM feature-class arms of protocol.md on one KITTI sequence.

Each arm runs concurrently in its own working directory with one OpenMP
thread and writes summary, log, saved poses and the passive trace. Commands
and exit codes go to the record file.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
BASE = ["--methods", "inten_loam", "--no-gt-seed", "--inten-loam-paper-odometry",
        "--input-max-range-m", "120", "--input-vertical-angle-correction-deg", "0.17",
        "--inten-loam-paper-set", "s2m_plane_max_dist=0.05"]
ARMS = {
    "reference": [],
    "no_edge": ["--inten-loam-paper-set", "edges_per_sector=0"],
    "no_facade": ["--inten-loam-paper-set", "facades_per_sector=0"],
    "no_intensity": ["--inten-loam-paper-set", "use_intensity=0"],
    "no_ground": ["--inten-loam-paper-set", "max_ground=0"],
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--sequence", default="10")
    parser.add_argument("--velodyne", type=Path, required=True)
    parser.add_argument("--work-root", type=Path, required=True)
    parser.add_argument("--binary", type=Path, default=REPO_ROOT / "build/evaluation/pcd_dogfooding")
    parser.add_argument("--record", type=Path, required=True)
    parser.add_argument("--arm", action="append", help="Subset of arms (default: all).")
    args = parser.parse_args()
    binary = args.binary.resolve()
    gt = REPO_ROOT / f"experiments/reference_data/kitti_seq_{args.sequence}_full_gt.csv"
    record = {"binary": str(binary), "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
              "git_head": subprocess.run(["git", "rev-parse", "HEAD"], cwd=REPO_ROOT, text=True,
                                         capture_output=True).stdout.strip(),
              "sequence": args.sequence, "arms": {}}
    env = dict(os.environ, OMP_NUM_THREADS="1", OPENBLAS_NUM_THREADS="1")
    running = {}
    for arm in (args.arm or list(ARMS)):
        work = args.work_root / arm
        work.mkdir(parents=True, exist_ok=True)
        trace = work / "trace.jsonl"
        if trace.exists():
            trace.unlink()
        command = [str(binary), str(args.velodyne), str(gt), *BASE, *ARMS[arm],
                   "--summary-json", str(work / "summary.json"), "--inten-loam-paper-trace-jsonl", str(trace)]
        log = (work / "run.log").open("w")
        running[arm] = (subprocess.Popen(command, cwd=work, env=env, stdout=log, stderr=subprocess.STDOUT),
                        command, time.time(), log)
    for arm, (process, command, started, log) in running.items():
        code = process.wait()
        log.close()
        record["arms"][arm] = {"command": command, "cwd": str(args.work_root / arm), "exit_code": code,
                               "started_unix": started, "finished_unix": time.time()}
        args.record.write_text(json.dumps(record, indent=2) + "\n")
    return 0 if all(a["exit_code"] == 0 for a in record["arms"].values()) else 1


if __name__ == "__main__":
    raise SystemExit(main())
