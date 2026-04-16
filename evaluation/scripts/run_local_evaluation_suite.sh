#!/usr/bin/env bash
# Local evaluation stack: ctest → synthetic_benchmark → smoke_ci_fixture (same order as CI intent).
# Run from anywhere; resolves repository root from this script location.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="${ROOT}/build"

SKIP_CTEST=0
SKIP_SYNTHETIC=0
SKIP_FIXTURE=0

usage() {
  cat <<'EOF'
Run the standard local evaluation stack: ctest, synthetic_benchmark, smoke_ci_fixture.

Usage: run_local_evaluation_suite.sh [options]

Options:
  --skip-ctest       Skip ctest (unit / integration tests)
  --skip-synthetic   Skip build/evaluation/synthetic_benchmark
  --skip-fixture     Skip evaluation/scripts/smoke_ci_fixture.sh
  -h, --help         Show this help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --skip-ctest) SKIP_CTEST=1 ;;
    --skip-synthetic) SKIP_SYNTHETIC=1 ;;
    --skip-fixture) SKIP_FIXTURE=1 ;;
    -h|--help) usage; exit 0 ;;
    *)
      echo "unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
  shift
done

cd "${ROOT}"

if [[ ${SKIP_CTEST} -eq 0 ]]; then
  if [[ -d "${BUILD}" ]] && [[ -f "${BUILD}/CTestTestfile.cmake" ]]; then
    echo "=== [1/3] ctest (build directory: ${BUILD}) ==="
    (cd "${BUILD}" && ctest --output-on-failure)
  else
    echo "warning: skipping ctest (no ${BUILD}/CTestTestfile.cmake); configure and build first" >&2
  fi
else
  echo "=== [1/3] ctest skipped ==="
fi

if [[ ${SKIP_SYNTHETIC} -eq 0 ]]; then
  SYN="${BUILD}/evaluation/synthetic_benchmark"
  if [[ -x "${SYN}" ]]; then
    echo "=== [2/3] synthetic_benchmark ==="
    "${SYN}"
  else
    echo "warning: skipping synthetic_benchmark (not found or not executable: ${SYN})" >&2
  fi
else
  echo "=== [2/3] synthetic_benchmark skipped ==="
fi

if [[ ${SKIP_FIXTURE} -eq 0 ]]; then
  echo "=== [3/3] smoke_ci_fixture ==="
  bash "${ROOT}/evaluation/scripts/smoke_ci_fixture.sh"
else
  echo "=== [3/3] smoke_ci_fixture skipped ==="
fi

echo "run_local_evaluation_suite.sh: OK"
