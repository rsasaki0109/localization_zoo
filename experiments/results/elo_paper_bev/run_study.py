#!/usr/bin/env python3
"""Run the fixed ELO paper-BEV comparison on full KITTI sequences.

For each sequence: fetch the official scans, record their hashes, run the two
arms of experiments/elo_kitti_seq_NN_full_paper_bev_matrix.json
concurrently (one OpenMP thread each, one working directory each so the
runner's dogfooding_results/ outputs never collide), stage every run artifact,
write saved-pose step diagnostics, re-verify the scan hashes and delete the
scans. Every command, exit code and timing is appended to the record.

The arms and their arguments come only from the committed manifests; this
script never chooses a setting. Import the staged runs with import_study.py.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
STUDY_DIR = Path(__file__).resolve().parent
ARMS = ("dense_elevation_paper_normals_unbiased", "dense_elevation_paper_normals_unbiased_paper_bev")


def manifest_path(sequence: str) -> Path:
    return REPO_ROOT / "experiments" / f"elo_kitti_seq_{sequence}_full_paper_bev_matrix.json"


def run_logged(record: list, command: list[str], log_path: Path, **kwargs) -> int:
    started = time.time()
    with log_path.open("w") as log:
        completed = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, **kwargs)
    record.append({"command": command, "log": str(log_path), "started_unix": started,
                   "finished_unix": time.time(), "exit_code": completed.returncode})
    return completed.returncode


def scan_hashes(velodyne: Path) -> dict:
    """SHA-256 of every scan in index order, plus one hash over that list."""
    files = sorted(velodyne.glob("*.bin"))
    per_file = []
    total_bytes = 0
    for path in files:
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        per_file.append(f"{path.name} {digest}")
        total_bytes += path.stat().st_size
    aggregate = hashlib.sha256("\n".join(per_file).encode()).hexdigest()
    return {"scan_count": len(files), "total_bytes": total_bytes, "aggregate_sha256": aggregate}


def gt_pose_count(sequence: str) -> int:
    csv = REPO_ROOT / "experiments" / "reference_data" / f"kitti_seq_{sequence}_full_gt.csv"
    with csv.open() as handle:
        return sum(1 for line in handle if line.strip()) - 1


def write_launcher(path: Path, workdir: Path, binary: Path) -> None:
    path.write_text("#!/bin/sh\nset -eu\n"
                    f"cd {workdir}\n"
                    f"exec {binary} \"$@\"\n")
    path.chmod(0o755)


def run_sequence(sequence: str, args: argparse.Namespace, record: dict) -> None:
    seq_record: dict = {"sequence": sequence, "commands": []}
    record["sequences"].append(seq_record)
    work = args.work_root / sequence
    work.mkdir(parents=True, exist_ok=True)
    velodyne = args.kitti_root / "dataset" / "sequences" / sequence / "velodyne"

    if not velodyne.is_dir() or len(list(velodyne.glob("*.bin"))) != gt_pose_count(sequence):
        code = run_logged(seq_record["commands"],
                          [sys.executable, str(REPO_ROOT / "evaluation/scripts/fetch_kitti_odometry_sequences.py"),
                           "--output-root", str(args.kitti_root), "--sequence", sequence],
                          work / "fetch.log", cwd=REPO_ROOT)
        if code != 0:
            raise RuntimeError(f"fetch failed for {sequence}")
    seq_record["input_before"] = scan_hashes(velodyne)
    if seq_record["input_before"]["scan_count"] != gt_pose_count(sequence):
        raise RuntimeError(f"scan count mismatch for {sequence}")

    link = REPO_ROOT / "dogfooding_results" / f"kitti_seq_{sequence}_full"
    link.parent.mkdir(exist_ok=True)
    if link.is_symlink() or link.exists():
        link.unlink()
    link.symlink_to(velodyne)

    manifest = json.loads(manifest_path(sequence).read_text())
    env = dict(os.environ, OMP_NUM_THREADS="1", OPENBLAS_NUM_THREADS="1", PYTHONDONTWRITEBYTECODE="1")
    processes = {}
    for arm in ARMS:
        arm_dir = work / arm
        workdir = arm_dir / "workdir"
        workdir.mkdir(parents=True, exist_ok=True)
        launcher = arm_dir / "launch.sh"
        write_launcher(launcher, workdir, args.binary)
        single = dict(manifest)
        single["variants"] = [v for v in manifest["variants"] if v["id"] == arm]
        scratch_manifest = arm_dir / manifest_path(sequence).name
        scratch_manifest.write_text(json.dumps(single, indent=2) + "\n")
        command = [sys.executable, str(REPO_ROOT / "evaluation/scripts/run_experiment_matrix.py"),
                   "--manifest", str(scratch_manifest), "--binary", str(launcher),
                   "--output-dir", str(arm_dir / "results"), "--docs-dir", str(arm_dir / "docs")]
        log = (arm_dir / "matrix.log").open("w")
        processes[arm] = (subprocess.Popen(command, cwd=REPO_ROOT, env=env, stdout=log,
                                           stderr=subprocess.STDOUT), command, time.time(), log)
    for arm, (process, command, started, log) in processes.items():
        code = process.wait()
        log.close()
        seq_record["commands"].append({"arm": arm, "command": command, "log": str(work / arm / "matrix.log"),
                                       "started_unix": started, "finished_unix": time.time(),
                                       "exit_code": code})
        if code != 0:
            raise RuntimeError(f"{arm} failed on {sequence}")

    stem = manifest_path(sequence).stem
    for arm in ARMS:
        run_dir = work / arm / "results" / "runs" / stem / arm
        stage = args.stage_root / "runs" / stem / arm
        if stage.exists():
            shutil.rmtree(stage)
        shutil.copytree(run_dir, stage)
        produced = work / arm / "workdir" / "dogfooding_results"
        shutil.copy2(produced / "ELO.txt", stage / "trajectory_kitti.txt")
        shutil.copy2(produced / f"kitti_seq_{sequence}_full_evaluated_gt.txt", stage / "evaluated_gt_kitti.txt")
        shutil.copy2(produced / f"kitti_seq_{sequence}_full_evaluation_manifest.json",
                     stage / "evaluation_manifest.json")
        code = run_logged(seq_record["commands"],
                          [sys.executable, str(REPO_ROOT / "evaluation/scripts/diagnose_kitti_steps.py"),
                           "--estimate", str(stage / "trajectory_kitti.txt"),
                           "--gt", str(stage / "evaluated_gt_kitti.txt"),
                           "--summary", str(stage / "summary.json"),
                           "--method", "ELO", "--output", str(stage / "step_diagnostics.json")],
                          work / f"diagnose_{arm}.log", cwd=REPO_ROOT)
        if code != 0:
            raise RuntimeError(f"diagnostics failed for {arm} on {sequence}")

    seq_record["input_after"] = scan_hashes(velodyne)
    if seq_record["input_after"] != seq_record["input_before"]:
        raise RuntimeError(f"scan hashes changed during {sequence}")
    if not args.keep_scans:
        shutil.rmtree(args.kitti_root / "dataset" / "sequences" / sequence)
        link.unlink()
        seq_record["scans_deleted"] = True
    seq_record["complete"] = True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--kitti-root", type=Path, required=True)
    parser.add_argument("--work-root", type=Path, required=True, help="Scratch working directories.")
    parser.add_argument("--stage-root", type=Path, required=True, help="Where staged runs/ are written.")
    parser.add_argument("--binary", type=Path, default=REPO_ROOT / "build/evaluation/pcd_dogfooding")
    parser.add_argument("--sequence", action="append", required=True)
    parser.add_argument("--keep-scans", action="store_true")
    parser.add_argument("--record", type=Path, required=True, help="JSON record of every command.")
    args = parser.parse_args()
    args.binary = args.binary.resolve()
    record = {"binary": str(args.binary),
              "binary_sha256": hashlib.sha256(args.binary.read_bytes()).hexdigest(),
              "git_head": subprocess.run(["git", "rev-parse", "HEAD"], cwd=REPO_ROOT, text=True,
                                         capture_output=True).stdout.strip(),
              "argv": sys.argv, "sequences": []}
    try:
        for sequence in args.sequence:
            run_sequence(sequence.zfill(2), args, record)
            args.record.write_text(json.dumps(record, indent=2) + "\n")
    finally:
        args.record.write_text(json.dumps(record, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
