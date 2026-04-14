#!/usr/bin/env bash
# Fast multimodal regression for GitHub CI: 3-frame MCD slice plus committed visual CSVs.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/build/evaluation/multimodal_dogfooding"
SEQ="${ROOT}/evaluation/fixtures/mcd_kth_smoke"
GT="${ROOT}/evaluation/fixtures/mcd_kth_smoke_gt.csv"
NFR=3
OUT="${ROOT}/experiments/results/runs/ci_multimodal_fixture_smoke"
SUMMARY="${OUT}/summary.json"
mkdir -p "${OUT}"

if [[ ! -x "${BIN}" ]]; then
  echo "error: build multimodal_dogfooding first" >&2
  exit 1
fi

"${BIN}" "${SEQ}" "${GT}" "${NFR}" \
  --methods okvis,fast_livo2 \
  --summary-json "${SUMMARY}" \
  --okvis-fast-profile \
  --fast-livo2-fast-profile \
  >"${OUT}/run.log" 2>&1

python3 - "${SUMMARY}" <<'PY'
import json
import sys

summary_path = sys.argv[1]
with open(summary_path) as handle:
    summary = json.load(handle)

if summary["num_frames"] != 3:
    raise SystemExit(f"expected 3 frames, got {summary['num_frames']}")

methods = {item["name"]: item for item in summary["methods"]}
for method_name in ("OKVIS", "FAST-LIVO2"):
    if method_name not in methods:
        raise SystemExit(f"missing method in summary: {method_name}")
    item = methods[method_name]
    if item["status"] != "OK":
        raise SystemExit(f"{method_name} status={item['status']} note={item['note']}")
    if item["frames"] != 3:
        raise SystemExit(f"{method_name} expected 3 frames, got {item['frames']}")

print(f"ci_multimodal_fixture_smoke: OK -> {summary_path}")
PY
