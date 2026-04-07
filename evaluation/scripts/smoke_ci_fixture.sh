#!/usr/bin/env bash
# Fast regression for GitHub CI: 3-frame MCD slice (committed under evaluation/fixtures/).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/pcd_dogfooding"
PCD="${ROOT}/evaluation/fixtures/mcd_kth_smoke"
GT="${ROOT}/evaluation/fixtures/mcd_kth_smoke_gt.csv"
NFR=3
OUT="${ROOT}/experiments/results/runs/ci_fixture_smoke"
mkdir -p "${OUT}"

if [[ ! -x "${BIN}" ]]; then
  echo "error: build pcd_dogfooding first" >&2
  exit 1
fi

METHODS=(
  litamin2 gicp small_gicp voxel_gicp ndt kiss_icp
  dlo dlio aloam floam lego_loam mulls ct_icp
  xicp hdl_graph_slam vgicp_slam
  suma balm2 isc_loam loam_livox lio_sam lins
  fast_lio_slam point_lio
)

for m in "${METHODS[@]}"; do
  echo "=== CI fixture / ${m} ==="
  "${BIN}" "${PCD}" "${GT}" "${NFR}" --methods "${m}" \
    --summary-json "${OUT}/${m}.json"
done

if [[ -f "${PCD}/imu.csv" ]]; then
  echo "=== CI fixture / ct_lio (imu present) ==="
  "${BIN}" "${PCD}" "${GT}" "${NFR}" --methods ct_lio \
    --summary-json "${OUT}/ct_lio.json"
  echo "=== CI fixture / fast_lio2 (imu present) ==="
  "${BIN}" "${PCD}" "${GT}" "${NFR}" --methods fast_lio2 \
    --summary-json "${OUT}/fast_lio2_imu.json"
  echo "=== CI fixture / clins (imu present) ==="
  "${BIN}" "${PCD}" "${GT}" "${NFR}" --methods clins \
    --summary-json "${OUT}/clins.json"
fi

echo "ci_fixture_smoke: OK -> ${OUT}"
