#!/usr/bin/env bash
set -eo pipefail

source /opt/ros/humble/setup.bash
set -u
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec python3 "${script_dir}/run_mola_baseline.py" "$@"
