#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
  echo "usage: download_ntu_viral_archive.sh URL OUTPUT LOG" >&2
  exit 2
fi

url=$1
output=$2
log=$3
mkdir -p "$(dirname "${output}")" "$(dirname "${log}")"
exec >"${log}" 2>&1
printf 'started_at_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
curl --fail --location --retry 5 --retry-delay 5 --output "${output}" "${url}"
printf 'completed_at_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
stat --format='bytes=%s' "${output}"
