#!/usr/bin/env bash
# Run P4 long Bonn train fixtures through scan_dogfooding.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/scan_dogfooding"
OUT="${ROOT}/docs/benchmarks/scan2d"
METHODS="rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc,mb_icp,karto_matcher"

if [[ ! -x "${BIN}" ]]; then
  echo "error: build scan_dogfooding first" >&2
  exit 1
fi

mkdir -p "${OUT}"

run_fixture() {
  local name="$1"
  local scan="${ROOT}/evaluation/fixtures/${name}"
  local gt="${scan}/gt.csv"
  local json="${OUT}/${name}.json"
  echo "=== scan2d long benchmark: ${name} ==="
  "${BIN}" "${scan}" "${gt}" \
    --methods "${METHODS}" \
    --wheel-odom-from-gt \
    --summary-json "${json}"
}

run_fixture mit_train_120
run_fixture intel_train_150
run_fixture fr079_train_200

echo "long scan2d benchmarks written under ${OUT}"
