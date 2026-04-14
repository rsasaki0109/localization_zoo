#!/usr/bin/env python3

"""Verify that the build environment meets benchmark requirements."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent.parent


def check(label: str, ok: bool, detail: str = "") -> bool:
    status = "OK" if ok else "FAIL"
    msg = f"[{status}] {label}"
    if detail:
        msg += f" ({detail})"
    print(msg)
    return ok


def run_version(cmd: list[str]) -> str:
    try:
        return subprocess.check_output(cmd, stderr=subprocess.STDOUT, text=True).strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return ""


def main() -> int:
    all_ok = True

    # C++ toolchain
    cmake_ver = run_version(["cmake", "--version"]).split("\n")[0] if run_version(["cmake", "--version"]) else ""
    all_ok &= check("cmake", bool(cmake_ver), cmake_ver)

    gcc_ver = run_version(["g++", "--version"]).split("\n")[0] if run_version(["g++", "--version"]) else ""
    all_ok &= check("g++", bool(gcc_ver), gcc_ver)

    # Build artifact
    for binary_name in ["pcd_dogfooding", "multimodal_dogfooding"]:
        binary = REPO_ROOT / "build" / "evaluation" / binary_name
        all_ok &= check(f"{binary_name} binary", binary.exists(), str(binary))

    # Python packages
    for pkg in ["numpy", "matplotlib"]:
        try:
            mod = __import__(pkg)
            ver = getattr(mod, "__version__", "unknown")
            all_ok &= check(f"python: {pkg}", True, ver)
        except ImportError:
            all_ok &= check(f"python: {pkg}", False, "not installed")

    # Key directories
    experiments_dir = REPO_ROOT / "experiments"
    all_ok &= check("experiments/ dir", experiments_dir.is_dir())

    manifests = list(experiments_dir.glob("*_matrix.json"))
    all_ok &= check("manifests found", len(manifests) > 0, f"{len(manifests)} files")

    results_dir = experiments_dir / "results"
    all_ok &= check("experiments/results/ dir", results_dir.is_dir())

    # Scripts
    for script in [
        "run_experiment_matrix.py",
        "refresh_study_docs.py",
        "generate_reproduction_status.py",
        "run_multimodal_study.py",
        "prepare_kitti_multimodal_inputs.py",
        "prepare_kitti_odometry_inputs.py",
        "prepare_mulran_inputs.py",
        "mulran_global_pose_to_gt_csv.py",
        "mulran_pose_io.py",
        "prepare_newer_college_inputs.py",
        "newer_college_pose_io.py",
        "smoke_ci_fixture.sh",
        "smoke_multimodal_fixture.sh",
        "kitti_bin_to_pcd_dir.py",
        "kitti_poses_to_gt_csv.py",
    ]:
        path = REPO_ROOT / "evaluation" / "scripts" / script
        all_ok &= check(f"script: {script}", path.exists())

    print()
    if all_ok:
        print("All checks passed.")
    else:
        print("Some checks FAILED. See above for details.")
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
