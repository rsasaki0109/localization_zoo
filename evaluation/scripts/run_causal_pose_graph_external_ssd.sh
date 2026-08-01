#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 5 ]]; then
  echo "usage: $0 <sequence> <identity-reference.csv> <external-poses.txt> <pcd-dogfooding-bin> <output-root>" >&2
  exit 2
fi

benchmark_sequence="$1"
benchmark_identity_reference="$2"
benchmark_external_poses="$3"
benchmark_binary="$4"
benchmark_output_root="$5"
benchmark_ram_mount_point="/mnt/loc_zoo_ram_causal"
benchmark_ssd_input="/mnt/e/datasets/loc_zoo/raw/kitti_odometry/dataset/sequences/${benchmark_sequence}/velodyne"
benchmark_ram_input="${benchmark_ram_mount_point}/velodyne"
benchmark_ram_identity="${benchmark_ram_mount_point}/identity_reference.csv"
benchmark_source_manifest="/opt/loc_zoo_v12_manifests/source${benchmark_sequence}.sha256"

if [[ ! "${benchmark_sequence}" =~ ^(00|07)$ ]]; then
  echo "unsupported sequence: ${benchmark_sequence}" >&2
  exit 2
fi
for benchmark_required_file in \
  "${benchmark_identity_reference}" \
  "${benchmark_external_poses}" \
  "${benchmark_binary}" \
  "${benchmark_source_manifest}"; do
  if [[ ! -e "${benchmark_required_file}" ]]; then
    echo "required input is missing: ${benchmark_required_file}" >&2
    exit 2
  fi
done
if [[ ! -d "${benchmark_ssd_input}" ]]; then
  echo "external SSD input is missing: ${benchmark_ssd_input}" >&2
  exit 2
fi
if [[ -e "${benchmark_output_root}" ]]; then
  echo "refusing to overwrite existing output: ${benchmark_output_root}" >&2
  exit 1
fi
if pgrep -x pcd_dogfooding >/dev/null; then
  echo "another pcd_dogfooding process is already running" >&2
  exit 1
fi

mkdir -p "${benchmark_ram_mount_point}" "${benchmark_output_root}"
if mountpoint -q "${benchmark_ram_mount_point}"; then
  echo "refusing to replace an existing RAM staging mount: ${benchmark_ram_mount_point}" >&2
  exit 1
fi
benchmark_mounted=0
cleanup() {
  if [[ "${benchmark_mounted}" -eq 1 ]] && mountpoint -q "${benchmark_ram_mount_point}"; then
    umount "${benchmark_ram_mount_point}"
  fi
}
trap cleanup EXIT

sha256sum "${benchmark_binary}" > "${benchmark_output_root}/binary.sha256"
sha256sum "${benchmark_external_poses}" > "${benchmark_output_root}/external_poses.sha256"
sha256sum "${benchmark_identity_reference}" > "${benchmark_output_root}/identity_reference.sha256"

mount -t tmpfs -o size=10G,mode=0755 tmpfs "${benchmark_ram_mount_point}"
benchmark_mounted=1
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

(
  cd "${benchmark_output_root}"
  /usr/bin/time -v -o resource_time.txt \
    env OMP_NUM_THREADS=4 taskset -c 0-5 "${benchmark_binary}" \
    "${benchmark_ram_input}" "${benchmark_ram_identity}" \
    --methods kiss_pose_graph_causal \
    --kiss-fast-profile \
    --kiss-source-voxel-size 0.8 \
    --kiss-voxel-size 1.2 \
    --kiss-model-deviation-threshold \
    --kiss-model-deviation-correspondence-multiplier 2.0 \
    --kiss-max-iterations 15 \
    --kiss-max-source-points 2000 \
    --kiss-max-points-per-voxel 6 \
    --kiss-motion-guard \
    --kiss-adaptive-motion-guard \
    --kiss-max-step-translation-m 2.0 \
    --kiss-max-step-rotation-deg 20.0 \
    --kiss-vertical-angle-correction-deg 0.205 \
    --kiss-pg-scan-context-threshold 0.18 \
    --kiss-pg-external-poses "${benchmark_external_poses}" \
    --no-gt-seed \
    --association-mode strict \
    --summary-json summary.json \
    > run.log 2>&1
  sha256sum dogfooding_results/External_KISS_Pose_Graph_Causal.txt \
    > trajectory.sha256
)

cat "${benchmark_output_root}/trajectory.sha256"
