#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 4 ]]; then
  echo "usage: $0 <sequence> <identity-reference.csv> <output-root> <repetitions>" >&2
  exit 2
fi

benchmark_sequence="$1"
benchmark_identity_reference="$2"
benchmark_output_root="$3"
benchmark_repetitions="$4"
benchmark_ram_mount_point="/mnt/loc_zoo_ram"
benchmark_binary="/opt/loc_zoo_v12_build/evaluation/pcd_dogfooding"
benchmark_ssd_input="/mnt/e/datasets/loc_zoo/raw/kitti_odometry/dataset/sequences/${benchmark_sequence}/velodyne"
benchmark_ram_input="${benchmark_ram_mount_point}/velodyne"
benchmark_ram_identity="${benchmark_ram_mount_point}/identity_reference.csv"
benchmark_source_manifest="/opt/loc_zoo_v12_manifests/source${benchmark_sequence}.sha256"

if [[ ! "${benchmark_sequence}" =~ ^(00|05|07)$ ]]; then
  echo "unsupported sequence: ${benchmark_sequence}" >&2
  exit 2
fi
if [[ ! "${benchmark_repetitions}" =~ ^[1-9][0-9]*$ ]]; then
  echo "repetitions must be a positive integer" >&2
  exit 2
fi
if [[ ! -f "${benchmark_identity_reference}" ]]; then
  echo "identity reference is missing: ${benchmark_identity_reference}" >&2
  exit 2
fi
if [[ ! -d "${benchmark_ssd_input}" ]]; then
  echo "external SSD input is missing: ${benchmark_ssd_input}" >&2
  exit 2
fi
if [[ ! -x "${benchmark_binary}" ]]; then
  echo "benchmark binary is missing: ${benchmark_binary}" >&2
  exit 2
fi
if [[ ! -f "${benchmark_source_manifest}" ]]; then
  echo "source SHA-256 manifest is missing: ${benchmark_source_manifest}" >&2
  exit 2
fi
if pgrep -x pcd_dogfooding >/dev/null; then
  echo "another pcd_dogfooding process is already running" >&2
  exit 1
fi

mkdir -p \
  "${benchmark_ram_mount_point}" \
  "${benchmark_output_root}"
if mountpoint -q "${benchmark_ram_mount_point}"; then
  echo "refusing to replace an existing RAM staging mount: ${benchmark_ram_mount_point}" >&2
  exit 1
fi

mount -t tmpfs -o size=10G,mode=0755 tmpfs "${benchmark_ram_mount_point}"
mkdir -p "${benchmark_ram_input}"
/usr/bin/time -v \
  -o "${benchmark_output_root}/ram_staging_resource_time.txt" \
  cp -a "${benchmark_ssd_input}/." "${benchmark_ram_input}/"
cp "${benchmark_identity_reference}" "${benchmark_ram_identity}"
(
  cd "${benchmark_ram_input}"
  find . -maxdepth 1 -type f -print0 \
    | sort -z \
    | xargs -0 sha256sum \
    > "${benchmark_output_root}/ram_staging.sha256"
)
diff -u \
  "${benchmark_source_manifest}" \
  "${benchmark_output_root}/ram_staging.sha256" \
  > "${benchmark_output_root}/ram_staging.diff"
sha256sum "${benchmark_output_root}/ram_staging.sha256" \
  > "${benchmark_output_root}/ram_staging_manifest.sha256"

for ((benchmark_run = 1; benchmark_run <= benchmark_repetitions; ++benchmark_run)); do
  benchmark_run_output="${benchmark_output_root}/run${benchmark_run}"
  if [[ -e "${benchmark_run_output}" ]]; then
    echo "refusing to overwrite existing output: ${benchmark_run_output}" >&2
    exit 1
  fi
  mkdir -p "${benchmark_run_output}"
  (
    cd "${benchmark_run_output}"
    /usr/bin/time -v -o resource_time.txt \
      taskset -c 0-5 "${benchmark_binary}" \
      "${benchmark_ram_input}" "${benchmark_ram_identity}" \
      --methods cube_lio \
      --no-gt-seed \
      --association-mode strict \
      --summary-json summary.json \
      > run.log 2>&1
    sha256sum dogfooding_results/CUBE_LIO.txt > trajectory.sha256
    cat trajectory.sha256
  )
done
