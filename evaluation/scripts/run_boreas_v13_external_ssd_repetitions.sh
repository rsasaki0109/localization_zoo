#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 7 ]]; then
  echo "usage: $0 <sensor-pcd-dir> <identity-reference.csv> <source-pcd.sha256> <candidate-xyzi-bin-dir> <source-bin.sha256> <output-root> <repetitions>" >&2
  exit 2
fi

benchmark_sensor_pcd="$1"
benchmark_identity_reference="$2"
benchmark_source_manifest="$3"
benchmark_candidate_bin="$4"
benchmark_candidate_manifest="$5"
benchmark_output_root="$6"
benchmark_repetitions="$7"
benchmark_ram_mount_point="/mnt/loc_zoo_ram_boreas_v13"
benchmark_ram_input="${benchmark_ram_mount_point}/sensor_only_pcd"
benchmark_ram_candidate_bin="${benchmark_ram_mount_point}/candidate_xyzi_bin"
benchmark_ram_identity="${benchmark_ram_mount_point}/identity_reference.csv"
benchmark_cube_binary="/opt/loc_zoo_v12_build/evaluation/pcd_dogfooding"
benchmark_candidate_root="/mnt/e/datasets/loc_zoo/results/v12_sota_head_to_head/v12_dynamic_voxel_radius/bin"
benchmark_odometry_binary="${benchmark_candidate_root}/kiss_bin_odometry"
benchmark_pose_graph_binary="${benchmark_candidate_root}/kiss_bin_pose_graph"

if [[ ! "${benchmark_repetitions}" =~ ^[1-9][0-9]*$ ]]; then
  echo "repetitions must be a positive integer" >&2
  exit 2
fi
for benchmark_required in \
  "${benchmark_sensor_pcd}" \
  "${benchmark_identity_reference}" \
  "${benchmark_source_manifest}" \
  "${benchmark_candidate_bin}" \
  "${benchmark_candidate_manifest}" \
  "${benchmark_cube_binary}" \
  "${benchmark_odometry_binary}" \
  "${benchmark_pose_graph_binary}"; do
  if [[ ! -e "${benchmark_required}" ]]; then
    echo "required input is missing: ${benchmark_required}" >&2
    exit 2
  fi
done
if [[ -e "${benchmark_output_root}" ]]; then
  echo "refusing to overwrite existing output: ${benchmark_output_root}" >&2
  exit 1
fi
if pgrep -x pcd_dogfooding >/dev/null; then
  echo "another pcd_dogfooding process is already running" >&2
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

sha256sum \
  "${benchmark_identity_reference}" \
  "${benchmark_source_manifest}" \
  "${benchmark_candidate_manifest}" \
  "${benchmark_cube_binary}" \
  "${benchmark_odometry_binary}" \
  "${benchmark_pose_graph_binary}" \
  > "${benchmark_output_root}/frozen_inputs.sha256"

mount -t tmpfs -o size=8G,mode=0755 tmpfs "${benchmark_ram_mount_point}"
benchmark_mounted=1
mkdir -p "${benchmark_ram_input}"
mkdir -p "${benchmark_ram_candidate_bin}"
/usr/bin/time -v \
  -o "${benchmark_output_root}/ram_staging_resource_time.txt" \
  cp -a "${benchmark_sensor_pcd}/." "${benchmark_ram_input}/"
/usr/bin/time -v \
  -o "${benchmark_output_root}/candidate_bin_staging_resource_time.txt" \
  cp -a "${benchmark_candidate_bin}/." "${benchmark_ram_candidate_bin}/"
cp "${benchmark_identity_reference}" "${benchmark_ram_identity}"
(
  cd "${benchmark_ram_input}"
  find . -type f -name '*.pcd' -print0 \
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
(
  cd "${benchmark_ram_candidate_bin}"
  find . -maxdepth 1 -type f -name '*.bin' -print0 \
    | sort -z \
    | xargs -0 sha256sum \
    > "${benchmark_output_root}/candidate_bin_staging.sha256"
)
diff -u \
  "${benchmark_candidate_manifest}" \
  "${benchmark_output_root}/candidate_bin_staging.sha256" \
  > "${benchmark_output_root}/candidate_bin_staging.diff"
sha256sum "${benchmark_output_root}/candidate_bin_staging.sha256" \
  > "${benchmark_output_root}/candidate_bin_staging_manifest.sha256"

for ((benchmark_run = 1; benchmark_run <= benchmark_repetitions; ++benchmark_run)); do
  benchmark_cube_output="${benchmark_output_root}/cube_lio/run${benchmark_run}"
  benchmark_candidate_output="${benchmark_output_root}/candidate_frontend/run${benchmark_run}"
  mkdir -p "${benchmark_cube_output}" "${benchmark_candidate_output}"
  (
    cd "${benchmark_cube_output}"
    /usr/bin/time -v -o resource_time.txt \
      taskset -c 0-5 "${benchmark_cube_binary}" \
      "${benchmark_ram_input}" "${benchmark_ram_identity}" \
      --methods cube_lio \
      --no-gt-seed \
      --association-mode strict \
      --summary-json summary.json \
      > run.log 2>&1
    sha256sum dogfooding_results/CUBE_LIO.txt > trajectory.sha256
  )
  (
    cd "${benchmark_candidate_output}"
    /usr/bin/time -v -o odometry_resource_time.txt \
      env OMP_NUM_THREADS=4 taskset -c 0-5 "${benchmark_odometry_binary}" \
      "${benchmark_ram_candidate_bin}" raw.txt raw_manifest.json \
      > odometry.log 2>&1
    /usr/bin/time -v -o pose_graph_resource_time.txt \
      env OMP_NUM_THREADS=4 taskset -c 0-5 "${benchmark_pose_graph_binary}" \
      "${benchmark_ram_candidate_bin}" raw.txt corrected.txt pose_graph_manifest.json \
      > pose_graph.log 2>&1
    sha256sum raw.txt corrected.txt > trajectories.sha256
  )
done

cat "${benchmark_output_root}/ram_staging_manifest.sha256"
cat "${benchmark_output_root}/candidate_bin_staging_manifest.sha256"
cat "${benchmark_output_root}/cube_lio/"run*/trajectory.sha256
cat "${benchmark_output_root}/candidate_frontend/"run*/trajectories.sha256
