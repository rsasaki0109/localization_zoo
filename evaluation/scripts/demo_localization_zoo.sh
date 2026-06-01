#!/usr/bin/env bash
# One-command local demo for first-time users.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT}/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
LIDAR_METHODS_QUICK="litamin2,gicp,ndt,kiss_icp"
LIDAR_METHODS_BROAD="litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,dlo,dlio,aloam,floam,lego_loam,mulls,ct_icp,xicp,hdl_graph_slam,vgicp_slam,suma,balm2,isc_loam,loam_livox,lio_sam,lins,fast_lio_slam,point_lio"
LIDAR_METHODS_FULL="${LIDAR_METHODS_BROAD},fast_lio2"
MULTIMODAL_METHODS_QUICK="okvis,fast_livo2"
MULTIMODAL_METHODS_BROAD="vins_fusion,okvis,orb_slam3,lvi_sam,fast_livo2,r2live"
MULTIMODAL_METHODS_FULL="${MULTIMODAL_METHODS_BROAD}"
PROFILE="${DEMO_PROFILE:-broad}"
METHODS="${DEMO_METHODS:-}"
MULTIMODAL_METHODS="${DEMO_MULTIMODAL_METHODS:-}"
CUSTOM_METHODS=0
CUSTOM_MULTIMODAL_METHODS=0
if [[ -n "${METHODS}" ]]; then
  CUSTOM_METHODS=1
fi
if [[ -n "${MULTIMODAL_METHODS}" ]]; then
  CUSTOM_MULTIMODAL_METHODS=1
fi
FRAMES="${DEMO_FRAMES:-3}"
OUT="${ROOT}/experiments/results/runs/demo_localization_zoo"
SKIP_BUILD=0
RUN_MULTIMODAL=1
ORIGINAL_ARGS=("$@")
ORIGINAL_ARG_STRING=""
if [[ "${#ORIGINAL_ARGS[@]}" -gt 0 ]]; then
  printf -v ORIGINAL_ARG_STRING ' %q' "${ORIGINAL_ARGS[@]}"
fi
DEMO_COMMAND="bash evaluation/scripts/demo_localization_zoo.sh${ORIGINAL_ARG_STRING}"

if command -v nproc >/dev/null 2>&1; then
  JOBS="${JOBS:-$(nproc)}"
else
  JOBS="${JOBS:-2}"
fi

usage() {
  cat <<'EOF'
Usage: bash evaluation/scripts/demo_localization_zoo.sh [options]

Build the repository, run the synthetic benchmark, then run a small committed
MCD fixture through selected LiDAR and multimodal methods. The run also writes
a browser-friendly report.html under experiments/results/runs/demo_localization_zoo/.

Options:
  --skip-build          Reuse an existing build directory.
  --build-dir PATH      Build directory. Default: ./build
  --profile NAME        Method profile: quick, broad, or full.
                        Default: broad
  --methods LIST        Comma-separated pcd_dogfooding methods.
                        Overrides the selected profile's LiDAR methods.
  --multimodal-methods LIST
                        Comma-separated multimodal_dogfooding methods.
                        Overrides the selected profile's multimodal methods.
  --frames N            Fixture frame count. Default: 3
  --no-multimodal       Skip the multimodal fixture smoke.
  -h, --help            Show this help.

Environment:
  BUILD_TYPE=Release    CMake build type.
  DEMO_PROFILE=broad    Same values as --profile.
  DEMO_METHODS          Same value as --methods.
  DEMO_MULTIMODAL_METHODS
                        Same value as --multimodal-methods.
  JOBS=$(nproc)         Build parallelism.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --skip-build)
      SKIP_BUILD=1
      shift
      ;;
    --build-dir)
      if [[ $# -lt 2 ]]; then
        echo "error: --build-dir requires a path" >&2
        exit 2
      fi
      BUILD_DIR="$2"
      shift 2
      ;;
    --profile)
      if [[ $# -lt 2 ]]; then
        echo "error: --profile requires quick, broad, or full" >&2
        exit 2
      fi
      PROFILE="$2"
      shift 2
      ;;
    --methods)
      if [[ $# -lt 2 ]]; then
        echo "error: --methods requires a comma-separated list" >&2
        exit 2
      fi
      METHODS="$2"
      CUSTOM_METHODS=1
      shift 2
      ;;
    --multimodal-methods)
      if [[ $# -lt 2 ]]; then
        echo "error: --multimodal-methods requires a comma-separated list" >&2
        exit 2
      fi
      MULTIMODAL_METHODS="$2"
      CUSTOM_MULTIMODAL_METHODS=1
      shift 2
      ;;
    --frames)
      if [[ $# -lt 2 ]]; then
        echo "error: --frames requires a positive integer" >&2
        exit 2
      fi
      FRAMES="$2"
      shift 2
      ;;
    --no-multimodal)
      RUN_MULTIMODAL=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if ! [[ "${FRAMES}" =~ ^[1-9][0-9]*$ ]]; then
  echo "error: --frames must be a positive integer" >&2
  exit 2
fi

case "${PROFILE}" in
  quick)
    DEFAULT_METHODS="${LIDAR_METHODS_QUICK}"
    DEFAULT_MULTIMODAL_METHODS="${MULTIMODAL_METHODS_QUICK}"
    ;;
  broad)
    DEFAULT_METHODS="${LIDAR_METHODS_BROAD}"
    DEFAULT_MULTIMODAL_METHODS="${MULTIMODAL_METHODS_BROAD}"
    ;;
  full)
    DEFAULT_METHODS="${LIDAR_METHODS_FULL}"
    DEFAULT_MULTIMODAL_METHODS="${MULTIMODAL_METHODS_FULL}"
    ;;
  *)
    echo "error: --profile must be quick, broad, or full" >&2
    exit 2
    ;;
esac

if [[ -z "${METHODS}" ]]; then
  METHODS="${DEFAULT_METHODS}"
fi
if [[ -z "${MULTIMODAL_METHODS}" ]]; then
  MULTIMODAL_METHODS="${DEFAULT_MULTIMODAL_METHODS}"
fi

RUN_PROFILE="${PROFILE}"
if [[ "${CUSTOM_METHODS}" -eq 1 || "${CUSTOM_MULTIMODAL_METHODS}" -eq 1 ]]; then
  RUN_PROFILE="${PROFILE}+custom"
fi

if [[ "${BUILD_DIR}" != /* ]]; then
  BUILD_DIR="${ROOT}/${BUILD_DIR}"
fi

mkdir -p "${OUT}"

echo "== Localization Zoo demo =="
echo "repo: ${ROOT}"
echo "build: ${BUILD_DIR}"
echo "output: ${OUT}"
echo "profile: ${RUN_PROFILE}"
echo

if [[ "${SKIP_BUILD}" -eq 0 ]]; then
  echo "== Build =="
  cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
  cmake --build "${BUILD_DIR}" -j"${JOBS}"
  echo
else
  echo "== Build =="
  echo "skip: reusing ${BUILD_DIR}"
  echo
fi

SYNTHETIC_BIN="${BUILD_DIR}/evaluation/synthetic_benchmark"
PCD_BIN="${BUILD_DIR}/evaluation/pcd_dogfooding"
MULTIMODAL_BIN="${BUILD_DIR}/evaluation/multimodal_dogfooding"

for bin in "${SYNTHETIC_BIN}" "${PCD_BIN}"; do
  if [[ ! -x "${bin}" ]]; then
    echo "error: expected executable not found: ${bin}" >&2
    echo "hint: rerun without --skip-build" >&2
    exit 1
  fi
done

echo "== Synthetic benchmark =="
(
  cd "${OUT}"
  "${SYNTHETIC_BIN}"
) | tee "${OUT}/synthetic_benchmark.log"
echo

PCD="${ROOT}/evaluation/fixtures/mcd_kth_smoke"
GT="${ROOT}/evaluation/fixtures/mcd_kth_smoke_gt.csv"
LIDAR_SUMMARY="${OUT}/lidar_fixture_summary.json"

echo "== LiDAR fixture (${FRAMES} frames) =="
echo "methods: ${METHODS}"
(
  cd "${OUT}"
  "${PCD_BIN}" "${PCD}" "${GT}" "${FRAMES}" \
    --methods "${METHODS}" \
    --summary-json "${LIDAR_SUMMARY}"
) | tee "${OUT}/lidar_fixture.log"

python3 - "${LIDAR_SUMMARY}" <<'PY'
import json
import sys

summary_path = sys.argv[1]
with open(summary_path) as handle:
    summary = json.load(handle)

methods = summary.get("methods", [])
if not methods:
    raise SystemExit("summary has no methods")

print("")
print("LiDAR fixture summary")
print("method          status    ate_m      fps")
print("------------------------------------------")
bad = []
for item in methods:
    status = str(item.get("status", ""))
    ate = item.get("ate_m")
    fps = item.get("fps")
    ate_text = "-" if ate is None else f"{ate:.3f}"
    fps_text = "-" if fps is None else f"{fps:.1f}"
    print(f"{item.get('name', '-'):<15} {status:<8} {ate_text:>7} {fps_text:>8}")
    if status.upper() != "OK":
        bad.append(f"{item.get('name', '-')}={status}")

if bad:
    raise SystemExit("non-OK LiDAR demo methods: " + ", ".join(bad))
PY
echo

if [[ "${RUN_MULTIMODAL}" -eq 1 ]]; then
  if [[ ! -x "${MULTIMODAL_BIN}" ]]; then
    echo "error: expected executable not found: ${MULTIMODAL_BIN}" >&2
    echo "hint: rerun without --skip-build or use --no-multimodal" >&2
    exit 1
  fi

  MULTIMODAL_SUMMARY="${OUT}/multimodal_fixture_summary.json"
  echo "== Multimodal fixture (${FRAMES} frames) =="
  echo "methods: ${MULTIMODAL_METHODS}"
  (
    cd "${OUT}"
    "${MULTIMODAL_BIN}" "${PCD}" "${GT}" "${FRAMES}" \
      --methods "${MULTIMODAL_METHODS}" \
      --summary-json "${MULTIMODAL_SUMMARY}" \
      --okvis-fast-profile \
      --fast-livo2-fast-profile \
      --r2live-fast-profile
  ) >"${OUT}/multimodal_fixture.log" 2>&1

  python3 - "${MULTIMODAL_SUMMARY}" <<'PY'
import json
import sys

summary_path = sys.argv[1]
with open(summary_path) as handle:
    summary = json.load(handle)

methods = summary.get("methods", [])
print("multimodal summary")
print("method          status    frames")
print("---------------------------------")
bad = []
for item in methods:
    status = str(item.get("status", ""))
    frames = item.get("frames", "-")
    print(f"{item.get('name', '-'):<15} {status:<8} {frames}")
    if status.upper() != "OK":
        bad.append(f"{item.get('name', '-')}={status}")

if bad:
    raise SystemExit("non-OK multimodal demo methods: " + ", ".join(bad))
PY
  echo
else
  rm -f "${OUT}/multimodal_fixture_summary.json" "${OUT}/multimodal_fixture.log"
fi

REPORT="${OUT}/report.html"
echo "== Report =="
REPORT_ARGS=(
  --demo-dir "${OUT}"
  --output "${REPORT}"
  --command "${DEMO_COMMAND}"
  --profile "${RUN_PROFILE}"
  --lidar-methods "${METHODS}"
)
VALIDATE_ARGS=(
  --demo-dir "${OUT}"
  --expected-lidar-methods "${METHODS}"
)
if [[ "${RUN_MULTIMODAL}" -eq 1 ]]; then
  REPORT_ARGS+=(--multimodal-methods "${MULTIMODAL_METHODS}")
  VALIDATE_ARGS+=(--expected-multimodal-methods "${MULTIMODAL_METHODS}")
else
  REPORT_ARGS+=(--skip-multimodal)
  VALIDATE_ARGS+=(--skip-multimodal)
fi

python3 "${ROOT}/evaluation/scripts/generate_demo_report.py" "${REPORT_ARGS[@]}"
python3 "${ROOT}/evaluation/scripts/validate_demo_artifacts.py" "${VALIDATE_ARGS[@]}"
echo

echo "== Done =="
echo "Report: ${REPORT}"
echo "Manifest: ${OUT}/manifest.json"
echo "Synthetic log: ${OUT}/synthetic_benchmark.log"
echo "Synthetic trajectories: ${OUT}/benchmark_results"
echo "LiDAR summary: ${LIDAR_SUMMARY}"
echo "LiDAR trajectories: ${OUT}/dogfooding_results"
if [[ "${RUN_MULTIMODAL}" -eq 1 ]]; then
  echo "Multimodal summary: ${MULTIMODAL_SUMMARY}"
fi
