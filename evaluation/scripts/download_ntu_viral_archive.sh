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
attempt=0
while ! unzip -tqq "${output}" >/dev/null 2>&1; do
  attempt=$((attempt + 1))
  before=0
  if [[ -f "${output}" ]]; then
    before=$(stat --format='%s' "${output}")
  fi
  printf 'resume_attempt=%s before_bytes=%s\n' "${attempt}" "${before}"
  curl --fail --location --retry 5 --retry-delay 5 --continue-at - \
    --output "${output}" "${url}"
  after=$(stat --format='%s' "${output}")
  printf 'resume_attempt=%s after_bytes=%s\n' "${attempt}" "${after}"
  if [[ "${after}" -le "${before}" ]]; then
    echo "download made no progress and ZIP validation still fails" >&2
    exit 1
  fi
done
printf 'completed_at_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
stat --format='bytes=%s' "${output}"
