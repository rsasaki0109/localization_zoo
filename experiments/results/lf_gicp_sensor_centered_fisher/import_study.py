#!/usr/bin/env python3
"""Import staged study runs into experiments/results and re-aggregate them.

Copies <stage-root>/runs/lf_gicp_kitti_seq_NN_full_sensor_fisher_matrix/<arm>/
into experiments/results/runs/, then re-aggregates each manifest with
run_experiment_matrix.py --merge-existing-index --reuse-existing, which reuses
the copied summaries and never re-runs a variant. The runner requires the
dataset directory to exist; an empty directory is created where the scans
were deleted and removed again afterwards.
"""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
ARMS = ("paper_default", "sensor_centered_fisher", "no_mitigation")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--stage-root", type=Path, required=True)
    parser.add_argument("--record", type=Path, required=True)
    args = parser.parse_args()
    record = []
    for i in range(11):
        sequence = f"{i:02d}"
        stem = f"lf_gicp_kitti_seq_{sequence}_full_sensor_fisher_matrix"
        for arm in ARMS:
            source = args.stage_root / "runs" / stem / arm
            if not (source / "summary.json").is_file():
                raise SystemExit(f"missing staged summary: {source}")
            target = REPO_ROOT / "experiments/results/runs" / stem / arm
            if target.exists():
                shutil.rmtree(target)
            shutil.copytree(source, target)
        dataset = REPO_ROOT / "dogfooding_results" / f"kitti_seq_{sequence}_full"
        created = False
        if not dataset.exists():
            dataset.mkdir(parents=True)
            created = True
        command = [sys.executable, "evaluation/scripts/run_experiment_matrix.py",
                   "--manifest", f"experiments/{stem}.json", "--merge-existing-index", "--reuse-existing"]
        started = time.time()
        completed = subprocess.run(command, cwd=REPO_ROOT, capture_output=True, text=True)
        record.append({"command": command, "exit_code": completed.returncode,
                       "started_unix": started, "finished_unix": time.time(),
                       "stdout_tail": completed.stdout[-2000:], "stderr_tail": completed.stderr[-2000:]})
        if created:
            dataset.rmdir()
        if completed.returncode != 0:
            args.record.write_text(json.dumps(record, indent=2) + "\n")
            raise SystemExit(f"aggregation failed for {stem}")
    args.record.write_text(json.dumps(record, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
