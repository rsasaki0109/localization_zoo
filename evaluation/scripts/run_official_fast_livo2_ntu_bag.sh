#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
  echo "usage: run_official_fast_livo2_ntu_bag.sh SENSOR_ONLY_BAG OUTPUT_DIR [PLAYBACK_RATE]" >&2
  exit 2
fi

bag_path=$1
output_dir=$2
playback_rate=${3:-1.0}
official_result_dir=/opt/fast_livo2_ws/src/FAST-LIVO2/Log/result
official_trajectory=${official_result_dir}/eee_01.txt

if [[ ! -f "${bag_path}" ]]; then
  echo "sensor-only bag does not exist: ${bag_path}" >&2
  exit 2
fi
mkdir -p "${output_dir}"
if [[ -e "${official_trajectory}" ]]; then
  echo "refusing to overwrite official trajectory: ${official_trajectory}" >&2
  exit 2
fi

roscore_pid=
launch_pid=
cleanup() {
  if [[ -n "${launch_pid}" ]]; then
    kill "${launch_pid}" 2>/dev/null || true
    wait "${launch_pid}" 2>/dev/null || true
  fi
  if [[ -n "${roscore_pid}" ]]; then
    kill "${roscore_pid}" 2>/dev/null || true
    wait "${roscore_pid}" 2>/dev/null || true
  fi
}
trap cleanup EXIT

roscore >"${output_dir}/roscore.log" 2>&1 &
roscore_pid=$!
for _ in $(seq 1 60); do
  if rosparam list >/dev/null 2>&1; then
    break
  fi
  sleep 1
done
if ! rosparam list >/dev/null 2>&1; then
  echo "roscore did not become ready" >&2
  exit 1
fi

roslaunch fast_livo mapping_ouster_ntu.launch rviz:=false \
  >"${output_dir}/fast_livo2.log" 2>&1 &
launch_pid=$!
for _ in $(seq 1 120); do
  if rosnode list 2>/dev/null | grep -qx '/laserMapping'; then
    break
  fi
  if ! kill -0 "${launch_pid}" 2>/dev/null; then
    echo "FAST-LIVO2 launch exited before becoming ready" >&2
    exit 1
  fi
  sleep 1
done
if ! rosnode list 2>/dev/null | grep -qx '/laserMapping'; then
  echo "FAST-LIVO2 node did not become ready" >&2
  exit 1
fi

start_ns=$(date +%s%N)
rosbag play --clock --rate "${playback_rate}" "${bag_path}" \
  >"${output_dir}/rosbag_play.log" 2>&1
# Subscriber queues can outlive rosbag play briefly; allow them to drain before
# stopping the official node. Real-time playback keeps this bounded.
sleep 15
end_ns=$(date +%s%N)

if [[ ! -s "${official_trajectory}" ]]; then
  echo "official FAST-LIVO2 trajectory was not produced" >&2
  exit 1
fi
frames=$(wc -l <"${official_trajectory}")
elapsed_seconds=$(awk -v start="${start_ns}" -v end="${end_ns}" \
  'BEGIN { printf "%.9f", (end - start) / 1000000000.0 }')
printf '%s\n' "${frames}" >"${output_dir}/trajectory_frames.txt"
printf '%s\n' "${elapsed_seconds}" >"${output_dir}/runtime_seconds.txt"

