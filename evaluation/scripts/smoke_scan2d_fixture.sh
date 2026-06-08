#!/usr/bin/env bash
# Fast regression for GitHub CI: 20-frame Intel Lab 2D scan slice (committed fixture).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/scan_dogfooding"
SCAN="${ROOT}/evaluation/fixtures/intel_val_73"
GT="${ROOT}/evaluation/fixtures/intel_val_73/gt.csv"
NFR=20
OUT="${ROOT}/experiments/results/runs/ci_scan2d_smoke"
mkdir -p "${OUT}"

if [[ ! -x "${BIN}" ]]; then
  echo "error: build scan_dogfooding first" >&2
  exit 1
fi

METHODS=(rf2o pl_icp psm ndt_2d idc kinematic_icp csm)

for m in "${METHODS[@]}"; do
  echo "=== CI scan2d / ${m} ==="
  extra=()
  if [[ "${m}" == "kinematic_icp" ]]; then
    extra+=(--wheel-odom-from-gt)
  fi
  "${BIN}" "${SCAN}" "${GT}" "${NFR}" --methods "${m}" "${extra[@]}" \
    --summary-json "${OUT}/${m}.json"
done

echo "ci_scan2d_smoke: OK -> ${OUT}"
