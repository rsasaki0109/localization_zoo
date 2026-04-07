#!/usr/bin/env bash
# Smoke-test every pcd_dogfooding method on public MCD windows (KTH / NTU / TUHH).
#
# Usage:
#   ./evaluation/scripts/smoke_mcd_pcd_dogfooding.sh [num_frames]              # default: KTH, 30 frames (compat)
#   ./evaluation/scripts/smoke_mcd_pcd_dogfooding.sh kth [num_frames]
#   ./evaluation/scripts/smoke_mcd_pcd_dogfooding.sh ntu 25
#   ./evaluation/scripts/smoke_mcd_pcd_dogfooding.sh tuhh 25
#   ./evaluation/scripts/smoke_mcd_pcd_dogfooding.sh all 15                  # all three sequences
#
# Requires: build/evaluation/pcd_dogfooding
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/pcd_dogfooding"

DATASET="kth"
NFR="30"
if [[ $# -ge 1 ]]; then
  if [[ "$1" =~ ^[0-9]+$ ]]; then
    NFR="$1"
  elif [[ "$1" == "kth" || "$1" == "ntu" || "$1" == "tuhh" || "$1" == "all" ]]; then
    DATASET="$1"
    if [[ $# -ge 2 && "$2" =~ ^[0-9]+$ ]]; then
      NFR="$2"
    fi
  else
    echo "usage: $0 [kth|ntu|tuhh|all] [num_frames]   OR   $0 [num_frames]" >&2
    exit 1
  fi
fi

if [[ ! -x "${BIN}" ]]; then
  echo "error: missing ${BIN} (cmake --build build --target pcd_dogfooding)" >&2
  exit 1
fi

METHODS_BASE=(
  litamin2 gicp small_gicp voxel_gicp ndt kiss_icp
  dlo dlio aloam floam lego_loam mulls ct_icp
  xicp fast_lio2 hdl_graph_slam vgicp_slam
  suma balm2 isc_loam loam_livox lio_sam lins
  fast_lio_slam point_lio
)

run_one_dataset() {
  local slug="$1"
  local pcd_rel gt_rel out_rel
  case "${slug}" in
    kth)
      pcd_rel="dogfooding_results/mcd_kth_day_06_108"
      gt_rel="experiments/reference_data/mcd_kth_day_06_108_gt.csv"
      out_rel="experiments/results/runs/smoke_mcd_kth"
      ;;
    ntu)
      pcd_rel="dogfooding_results/mcd_ntu_day_02_108"
      gt_rel="experiments/reference_data/mcd_ntu_day_02_108_gt.csv"
      out_rel="experiments/results/runs/smoke_mcd_ntu"
      ;;
    tuhh)
      pcd_rel="dogfooding_results/mcd_tuhh_night_09_108"
      gt_rel="experiments/reference_data/mcd_tuhh_night_09_108_gt.csv"
      out_rel="experiments/results/runs/smoke_mcd_tuhh"
      ;;
    *)
      echo "internal error: bad slug ${slug}" >&2
      return 1
      ;;
  esac

  local PCD="${ROOT}/${pcd_rel}"
  local GT="${ROOT}/${gt_rel}"
  local OUT="${ROOT}/${out_rel}"
  mkdir -p "${OUT}"

  if [[ ! -d "${PCD}" ]] || [[ ! -f "${GT}" ]]; then
    echo "error: missing ${pcd_rel} or ${gt_rel}" >&2
    return 1
  fi

  local METHODS=("${METHODS_BASE[@]}")
  if [[ -f "${PCD}/imu.csv" ]]; then
    METHODS+=(ct_lio)
  fi

  echo "========== MCD smoke: ${slug} (${NFR} frames) -> ${out_rel} =========="
  local fail=0
  for m in "${METHODS[@]}"; do
    local json="${OUT}/${m}_n${NFR}.json"
    echo "=== ${slug} / ${m} ==="
    if "${BIN}" "${PCD}" "${GT}" "${NFR}" --methods "${m}" --summary-json "${json}"; then
      if command -v jq >/dev/null 2>&1; then
        jq -r '.methods[0] | "\(.name) status=\(.status) ate_m=\(.ate_m) fps=\(.fps)"' "${json}" 2>/dev/null || true
      fi
    else
      echo "FAILED: ${slug} ${m}" >&2
      fail=1
    fi
  done
  return "${fail}"
}

overall=0
if [[ "${DATASET}" == "all" ]]; then
  for s in kth ntu tuhh; do
    run_one_dataset "${s}" || overall=1
  done
else
  run_one_dataset "${DATASET}" || overall=1
fi

if [[ "${overall}" -ne 0 ]]; then
  echo "smoke_mcd: one or more runs failed" >&2
  exit 1
fi
echo "smoke_mcd: all OK (dataset=${DATASET} n=${NFR})"
