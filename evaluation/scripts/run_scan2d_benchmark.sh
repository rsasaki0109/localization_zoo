#!/usr/bin/env bash
# Run all committed 2D scan fixtures through scan_dogfooding and refresh benchmark JSON.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/scan_dogfooding"
OUT="${ROOT}/docs/benchmarks/scan2d"
METHODS="rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc,mb_icp,karto_matcher"

if [[ ! -x "${BIN}" ]]; then
  echo "error: build scan_dogfooding first (cmake --build build --target scan_dogfooding)" >&2
  exit 1
fi

mkdir -p "${OUT}"

run_fixture() {
  local name="$1"
  local scan="${ROOT}/evaluation/fixtures/${name}"
  local gt="${scan}/gt.csv"
  local json="${OUT}/${name}.json"
  echo "=== scan2d benchmark: ${name} ==="
  "${BIN}" "${scan}" "${gt}" \
    --methods "${METHODS}" \
    --wheel-odom-from-gt \
    --summary-json "${json}"
}

# Public Bonn 2D-SLAM exports (committed fixtures)
run_fixture intel_val_73
run_fixture fr079_val_384
run_fixture mit_val_33

# Synthetic slow-motion corridor (optional refresh)
run_fixture rf2o_corridor

echo "scan2d benchmarks written under ${OUT}"
