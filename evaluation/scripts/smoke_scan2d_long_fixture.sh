#!/usr/bin/env bash
# CI regression for P4 long Bonn train fixtures: 20-frame slices × all 9 methods.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/scan_dogfooding"
NFR=20
OUT="${ROOT}/experiments/results/runs/ci_scan2d_long_smoke"
mkdir -p "${OUT}"

if [[ ! -x "${BIN}" ]]; then
  echo "error: build scan_dogfooding first" >&2
  exit 1
fi

METHODS=(rf2o pl_icp psm ndt_2d idc mb_icp kinematic_icp csm karto_matcher)

FIXTURES=(mit_train_120 intel_train_150 fr079_train_200)

for fixture in "${FIXTURES[@]}"; do
  scan="${ROOT}/evaluation/fixtures/${fixture}"
  gt="${scan}/gt.csv"
  if [[ ! -f "${gt}" ]]; then
    echo "error: missing committed fixture ${scan}" >&2
    exit 1
  fi
  for m in "${METHODS[@]}"; do
    echo "=== CI scan2d long / ${fixture} / ${m} ==="
    extra=()
    if [[ "${m}" == "kinematic_icp" ]]; then
      extra+=(--wheel-odom-from-gt)
    fi
    "${BIN}" "${scan}" "${gt}" "${NFR}" --methods "${m}" "${extra[@]}" \
      --summary-json "${OUT}/${fixture}_${m}.json"
  done
done

echo "ci_scan2d_long_smoke: OK -> ${OUT}"
