#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
  echo "usage: $0 <sequence> <identity-reference.csv> <output-root>" >&2
  exit 2
fi

benchmark_sequence="$1"
benchmark_identity_reference="$2"
benchmark_output_root="$3"
benchmark_ram_mount_point="/mnt/loc_zoo_ram"
benchmark_binary="/opt/loc_zoo_v12_build/evaluation/pcd_dogfooding"
benchmark_official_python="/root/.venvs/kiss-icp-official/bin/python"
benchmark_script_dir="$(cd "$(dirname "$0")" && pwd)"
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
  "${benchmark_binary}" \
  "${benchmark_official_python}" \
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

mkdir -p "${benchmark_output_root}/primary"
(
  cd "${benchmark_output_root}/primary"
  /usr/bin/time -v -o resource_time.txt \
    env OMP_NUM_THREADS=4 taskset -c 0-5 "${benchmark_binary}" \
    "${benchmark_ram_input}" "${benchmark_ram_identity}" \
    --methods kiss_icp \
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
    --no-gt-seed \
    --association-mode strict \
    --summary-json summary.json \
    > run.log 2>&1
  sha256sum dogfooding_results/KISS_ICP.txt > trajectory.sha256
)

mkdir -p "${benchmark_output_root}/official"
/usr/bin/time -v \
  -o "${benchmark_output_root}/official/resource_time.txt" \
  taskset -c 0-5 "${benchmark_official_python}" \
  "${benchmark_script_dir}/run_official_kiss_pcd_odometry.py" \
  --pcd-dir "${benchmark_ram_input}" \
  --output-dir "${benchmark_output_root}/official" \
  --thread-policy timestamp_adaptive \
  > "${benchmark_output_root}/official/run.log" 2>&1
sha256sum "${benchmark_output_root}/official/estimate.txt" \
  > "${benchmark_output_root}/official/trajectory.sha256"

cat \
  "${benchmark_output_root}/primary/trajectory.sha256" \
  "${benchmark_output_root}/official/trajectory.sha256" \
  > "${benchmark_output_root}/component_trajectories.sha256"

"${benchmark_official_python}" \
  "${benchmark_script_dir}/fuse_odometry_direction_consistent_rotation.py" \
  --primary-poses "${benchmark_output_root}/primary/dogfooding_results/KISS_ICP.txt" \
  --reference-poses "${benchmark_output_root}/official/estimate.txt" \
  --output "${benchmark_output_root}/v10_estimate.txt" \
  --window-frames 10 \
  --min-window-displacement-m 2.0 \
  --min-direction-disagreement-deg 0.05 \
  --low-speed-rotation-blend 1.0 \
  --high-speed-rotation-blend 0.05 \
  --low-speed-threshold-m 1.0 \
  --strong-blend-max-disagreement-deg 0.1 \
  --max-strong-blend-fraction 0.05 \
  --max-increment-disagreement-deg 5.0 \
  --max-cumulative-correction-deg 0.1 \
  --manifest "${benchmark_output_root}/v10_fusion_manifest.json"
sha256sum "${benchmark_output_root}/v10_estimate.txt" \
  > "${benchmark_output_root}/v10_trajectory.sha256"

cat \
  "${benchmark_output_root}/component_trajectories.sha256" \
  "${benchmark_output_root}/v10_trajectory.sha256"
