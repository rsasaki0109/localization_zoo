#!/usr/bin/env bash
# Run DLIO + CT-LIO on any dogfooding tree that already has imu.csv (KITJI 非必須).
#
# Contract: pcd_dogfooding の IMU 窓は evaluation/scripts/kitti_oxts_imu_for_dogfooding.py と
# 同じ imu.csv 列・frame_timestamps.csv の時刻軸を前提にする（データソースは任意）。
#
# Default paths = 公開 MCD KTH（imu は通常入っていない → 先に imu を置くか env で別ツリーを指定）。
#
# Env:
#   PCD_DIR       dogfooding ルート（既定: dogfooding_results/mcd_kth_day_06_108）
#   GT_CSV        対応 GT CSV（既定: experiments/reference_data/mcd_kth_day_06_108_gt.csv）
#   NFR           フレーム数（既定: 40）
#   USE_DLIO_PROFILE  1 で --dlio-kitti-profile（CLI 名は過去互換）。0 ならプロファイルなし。
#   OUT_TAG           出力サブディレクトリ名（既定: lio_imu_smoke）
#
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/pcd_dogfooding"
PCD_DIR="${PCD_DIR:-${ROOT}/dogfooding_results/mcd_kth_day_06_108}"
GT_CSV="${GT_CSV:-${ROOT}/experiments/reference_data/mcd_kth_day_06_108_gt.csv}"
NFR="${NFR:-40}"
USE_DLIO_PROFILE="${USE_DLIO_PROFILE:-1}"
OUT_TAG="${OUT_TAG:-lio_imu_smoke}"

if [[ ! -x "${BIN}" ]]; then
  echo "error: missing ${BIN}" >&2
  exit 1
fi
if [[ ! -d "${PCD_DIR}" ]] || [[ ! -f "${GT_CSV}" ]]; then
  echo "error: PCD_DIR or GT_CSV missing" >&2
  echo "  PCD_DIR=${PCD_DIR}" >&2
  echo "  GT_CSV=${GT_CSV}" >&2
  exit 1
fi
if [[ ! -f "${PCD_DIR}/imu.csv" ]]; then
  echo "error: ${PCD_DIR}/imu.csv not found."
  echo "KITTI でなくてもよいが、本リポの契約に合わせた imu.csv が必要です。"
  echo "  - KITTI Raw *sync* + OXTS から作る例: evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md"
  echo "  - 他データ: 同じ列・タイムラインに揃えて ${PCD_DIR}/imu.csv を置く"
  exit 2
fi

OUT="${ROOT}/experiments/results/runs/${OUT_TAG}"
mkdir -p "${OUT}"
dlio_cmd=( "${BIN}" "${PCD_DIR}" "${GT_CSV}" "${NFR}" --methods dlio )
if [[ "${USE_DLIO_PROFILE}" == 1 ]]; then
  dlio_cmd+=( --dlio-kitti-profile )
fi
dlio_cmd+=( --summary-json "${OUT}/dlio.json" )
echo "=== DLIO — IMU samples should be >0 ==="
"${dlio_cmd[@]}"

echo "=== CT-LIO ==="
"${BIN}" "${PCD_DIR}" "${GT_CSV}" "${NFR}" --methods ct_lio \
  --summary-json "${OUT}/ct_lio.json"

echo "lio_imu_smoke: done -> ${OUT} (check stdout for IMU samples / imu=y)"
