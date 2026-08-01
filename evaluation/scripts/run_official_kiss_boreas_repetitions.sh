#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 5 ]]; then
  echo "usage: $0 <sensor-pcd-dir> <source-pcd.sha256> <runner.py> <output-root> <repetitions>" >&2
  exit 2
fi

benchmark_sensor_pcd="$1"
benchmark_source_manifest="$2"
benchmark_runner="$3"
benchmark_output_root="$4"
benchmark_repetitions="$5"
benchmark_python="/root/.venvs/kiss-icp-official/bin/python"
benchmark_ram_mount_point="/mnt/loc_zoo_ram_boreas_official"
benchmark_ram_input="${benchmark_ram_mount_point}/sensor_only_pcd"

if [[ ! "${benchmark_repetitions}" =~ ^[1-9][0-9]*$ ]]; then
  echo "repetitions must be a positive integer" >&2
  exit 2
fi
for benchmark_required in \
  "${benchmark_sensor_pcd}" \
  "${benchmark_source_manifest}" \
  "${benchmark_runner}" \
  "${benchmark_python}"; do
  if [[ ! -e "${benchmark_required}" ]]; then
    echo "required input is missing: ${benchmark_required}" >&2
    exit 2
  fi
done
if [[ -e "${benchmark_output_root}" ]]; then
  echo "refusing to overwrite existing output: ${benchmark_output_root}" >&2
  exit 1
fi
if mountpoint -q "${benchmark_ram_mount_point}"; then
  echo "refusing to replace an existing RAM staging mount: ${benchmark_ram_mount_point}" >&2
  exit 1
fi

mkdir -p "${benchmark_output_root}" "${benchmark_ram_mount_point}"
benchmark_mounted=0
cleanup() {
  if [[ "${benchmark_mounted}" -eq 1 ]] && mountpoint -q "${benchmark_ram_mount_point}"; then
    umount "${benchmark_ram_mount_point}"
  fi
}
trap cleanup EXIT

sha256sum "${benchmark_source_manifest}" "${benchmark_runner}" \
  > "${benchmark_output_root}/frozen_inputs.sha256"
mount -t tmpfs -o size=6G,mode=0755 tmpfs "${benchmark_ram_mount_point}"
benchmark_mounted=1
mkdir -p "${benchmark_ram_input}"
/usr/bin/time -v \
  -o "${benchmark_output_root}/ram_staging_resource_time.txt" \
  cp -a "${benchmark_sensor_pcd}/." "${benchmark_ram_input}/"
(
  cd "${benchmark_ram_input}"
  find . -type f -name '*.pcd' -print0 \
    | sort -z \
    | xargs -0 sha256sum \
    > "${benchmark_output_root}/ram_staging.sha256"
)
diff -u "${benchmark_source_manifest}" "${benchmark_output_root}/ram_staging.sha256" \
  > "${benchmark_output_root}/ram_staging.diff"
sha256sum "${benchmark_output_root}/ram_staging.sha256" \
  > "${benchmark_output_root}/ram_staging_manifest.sha256"

for ((benchmark_run = 1; benchmark_run <= benchmark_repetitions; ++benchmark_run)); do
  benchmark_run_output="${benchmark_output_root}/run${benchmark_run}"
  mkdir -p "${benchmark_run_output}"
  /usr/bin/time -v -o "${benchmark_run_output}/resource_time.txt" \
    env OMP_NUM_THREADS=8 taskset -c 0-5 "${benchmark_python}" "${benchmark_runner}" \
    --pcd-dir "${benchmark_ram_input}" \
    --output-dir "${benchmark_run_output}" \
    --thread-policy timestamp_adaptive \
    > "${benchmark_run_output}/run.log" 2>&1
  sha256sum "${benchmark_run_output}/estimate.txt" \
    > "${benchmark_run_output}/trajectory.sha256"
done

cat "${benchmark_output_root}/ram_staging_manifest.sha256"
cat "${benchmark_output_root}/"run*/trajectory.sha256
