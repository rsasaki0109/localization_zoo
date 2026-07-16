#!/usr/bin/env bash

# Finish the slow Hard PCL outdoor DegenSense matrix without the memory
# pressure of launching all configurations concurrently.

set -euo pipefail

repo_root="${REPO_ROOT:-/root/localization_zoo}"
python_bin="${PYTHON_BIN:-/root/lz-venv/bin/python3}"
windows_root="${WINDOWS_REPO_ROOT:-}"

cd "${repo_root}"

run_one() {
  local sequence="$1"
  local variant="$2"
  local imu_flag=()
  if [[ "${variant}" == "no_imu" ]]; then
    imu_flag=(--no-imu)
  fi

  local pcd_dir="dogfooding_results/hard_pcl_localization/${sequence}"
  local gt_csv="experiments/reference_data/hard_pcl_${sequence}_gt.csv"
  local result="experiments/results/hard_pcl_localization/${sequence}/full/degen_sense_${variant}.json"

  if [[ -s "${result}" ]]; then
    echo "[skip] ${result} already exists"
  else
    echo "[run] ${sequence} ${variant}"
    build/evaluation/degen_sense_window_odometry \
      "${pcd_dir}" "${gt_csv}" "${result}" -1 "${imu_flag[@]}"
  fi

  "${python_bin}" evaluation/scripts/evaluate_window_odometry_gt.py \
    --result "${result}"
  if [[ -n "${windows_root}" ]]; then
    local windows_result="${windows_root}/${result}"
    mkdir -p "$(dirname "${windows_result}")"
    cp "${result}" "${windows_result}"
    echo "[done] ${windows_result}"
  else
    echo "[done] ${result}"
  fi
}

if (( $# > 0 )); then
  if (( $# % 2 != 0 )); then
    echo "usage: $0 [SEQUENCE VARIANT]..." >&2
    exit 2
  fi
  while (( $# > 0 )); do
    run_one "$1" "$2"
    shift 2
  done
else
  run_one outdoor_kidnap imu
  run_one outdoor_hard_01 imu
  run_one outdoor_hard_01 no_imu
  run_one outdoor_hard_02 imu
  run_one outdoor_hard_02 no_imu
fi

echo "[complete] remaining Hard PCL outdoor DegenSense matrix"
