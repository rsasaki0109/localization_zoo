#!/usr/bin/env bash

set -euo pipefail

bievr_ref="${BIEVR_REF:-21121698f273d6fbfffca57546b940edb1de2ff0}"
bievr_source="${BIEVR_SOURCE:-/root/bievr_lio_upstream}"
deps_root="${BIEVR_DEPS_ROOT:-/root/bievr_lio_deps}"
ceres_source="${deps_root}/ceres-src"
ceres_prefix="${deps_root}/ceres-2.2"
adapter_source="${ADAPTER_SOURCE:-/root/localization_zoo/evaluation/external/bievr_lio_adapter}"
adapter_build="${ADAPTER_BUILD:-/root/bievr_lio_adapter_build}"

for package in libpcl-dev libyaml-cpp-dev libeigen3-dev libtbb-dev; do
  if ! dpkg-query -W -f='${Status}' "${package}" 2>/dev/null | grep -q "install ok installed"; then
    echo "[error] missing package: ${package}" >&2
    echo "Install it with: apt-get install -y ${package}" >&2
    exit 1
  fi
done

if [[ ! -d "${bievr_source}/.git" ]]; then
  git clone https://github.com/ethz-asl/BIEVR-LIO.git "${bievr_source}"
fi
git -C "${bievr_source}" fetch origin "${bievr_ref}"
git -C "${bievr_source}" checkout --detach "${bievr_ref}"

if [[ ! -f "${ceres_prefix}/lib/cmake/Ceres/CeresConfig.cmake" ]]; then
  mkdir -p "${deps_root}"
  if [[ ! -d "${ceres_source}/.git" ]]; then
    git clone --branch 2.2.0 --depth 1 \
      https://github.com/ceres-solver/ceres-solver.git "${ceres_source}"
  fi
  cmake -S "${ceres_source}" -B "${ceres_source}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_INSTALL_PREFIX="${ceres_prefix}" \
    -DBUILD_TESTING=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_BENCHMARKS=OFF \
    -DBUILD_DOCUMENTATION=OFF \
    -DSUITESPARSE=ON \
    -DEIGENSPARSE=ON \
    -DSCHUR_SPECIALIZATIONS=OFF \
    -DUSE_CUDA=OFF
  cmake --build "${ceres_source}/build" --parallel 2
  cmake --install "${ceres_source}/build"
fi

cmake -S "${adapter_source}" -B "${adapter_build}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBIEVR_LIO_SOURCE_DIR="${bievr_source}" \
  -DCeres_DIR="${ceres_prefix}/lib/cmake/Ceres"
cmake --build "${adapter_build}" --parallel 2

echo "[done] ${adapter_build}/localization_zoo_bievr_runner"
