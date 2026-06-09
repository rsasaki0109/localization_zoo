#!/usr/bin/env bash
# Export longer Bonn 2D-SLAM train windows for P4/P8 benchmarks (MIT / Intel / fr079).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PREP="${ROOT}/evaluation/scripts/prepare_bonn_2dslam_inputs.py"
BONN="${BONN_2DSLAM_DIR:-/tmp/2dslam}"

if [[ ! -f "${PREP}" ]]; then
  echo "error: missing ${PREP}" >&2
  exit 1
fi

if [[ ! -f "${BONN}/mit/train.json" ]]; then
  echo "error: ${BONN}/mit/train.json not found." >&2
  echo "  wget -O /tmp/2dslam.zip https://www.ipb.uni-bonn.de/html/projects/kuang2023ral/2dslam.zip" >&2
  echo "  unzip /tmp/2dslam.zip 'mit/train.json' 'intel/train.json' 'fr079/train.json' -d /tmp/2dslam" >&2
  exit 1
fi

echo "=== mit_train_120 (Bonn MIT train, first 120 frames) ==="
python3 "${PREP}" \
  --json "${BONN}/mit/train.json" \
  --output-dir "${ROOT}/evaluation/fixtures/mit_train_120" \
  --max-frames 120

echo "=== intel_train_150 (Bonn Intel train, first 150 frames) ==="
python3 "${PREP}" \
  --json "${BONN}/intel/train.json" \
  --output-dir "${ROOT}/evaluation/fixtures/intel_train_150" \
  --max-frames 150

if [[ ! -f "${BONN}/fr079/train.json" ]]; then
  echo "error: ${BONN}/fr079/train.json not found (needed for fr079_train_200)." >&2
  exit 1
fi

echo "=== fr079_train_200 (Bonn fr079 train, first 200 frames) ==="
python3 "${PREP}" \
  --json "${BONN}/fr079/train.json" \
  --output-dir "${ROOT}/evaluation/fixtures/fr079_train_200" \
  --max-frames 200

echo "long fixtures ready under evaluation/fixtures/{mit_train_120,intel_train_150,fr079_train_200}"
