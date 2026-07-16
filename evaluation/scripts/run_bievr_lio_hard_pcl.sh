#!/usr/bin/env bash

set -euo pipefail

if (( $# < 1 || $# > 2 )); then
  echo "usage: $0 SEQUENCE [MAX_FRAMES]" >&2
  exit 2
fi

sequence="$1"
max_frames="${2:--1}"
repo_root="${REPO_ROOT:-/root/localization_zoo}"
adapter_build="${BIEVR_ADAPTER_BUILD:-/root/bievr_lio_adapter_build}"
bievr_source="${BIEVR_SOURCE:-/root/bievr_lio_upstream}"
python_bin="${PYTHON_BIN:-/root/lz-venv/bin/python3}"
windows_root="${WINDOWS_REPO_ROOT:-}"

pcd_dir="${repo_root}/dogfooding_results/hard_pcl_localization/${sequence}"
params="${repo_root}/evaluation/config/bievr_hard_pcl_params.yaml"
sensor="${repo_root}/evaluation/config/bievr_hard_pcl_indoor.yaml"
result_name="upstream_bievr_lio"
if (( max_frames >= 0 )); then
  result_name="upstream_bievr_lio_smoke_${max_frames}"
fi
result_dir="${repo_root}/experiments/results/hard_pcl_localization/${sequence}/${result_name}"
trajectory="${result_dir}/trajectory.tum"
summary="${result_dir}/summary.json"
gt_csv="${repo_root}/experiments/reference_data/hard_pcl_${sequence}_gt.csv"

mkdir -p "${result_dir}"
start_s="${SECONDS}"
"${adapter_build}/localization_zoo_bievr_runner" \
  "${pcd_dir}" "${params}" "${sensor}" "${trajectory}" "${max_frames}"
runtime_s="$((SECONDS - start_s))"
upstream_commit="$(git -C "${bievr_source}" rev-parse HEAD)"
"${python_bin}" "${repo_root}/evaluation/scripts/evaluate_bievr_tum_gt.py" \
  --trajectory "${trajectory}" \
  --gt-csv "${gt_csv}" \
  --sensor-yaml "${sensor}" \
  --upstream-commit "${upstream_commit}" \
  --runtime-s "${runtime_s}" \
  --output "${summary}"

if [[ -n "${windows_root}" ]]; then
  windows_result="${windows_root}/experiments/results/hard_pcl_localization/${sequence}/${result_name}"
  mkdir -p "${windows_result}"
  cp "${trajectory}" "${summary}" "${windows_result}/"
fi

echo "[complete] ${sequence} max_frames=${max_frames}"
