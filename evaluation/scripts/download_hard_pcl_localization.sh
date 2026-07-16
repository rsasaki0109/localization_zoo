#!/usr/bin/env bash
set -euo pipefail

# Resumable downloader for the sequence archives in Zenodo record 10122133.
# Maps and gt.zip are intentionally omitted because setup already fetches them.

output_dir="${1:-data/hard_pcl_localization}"
base_url="https://zenodo.org/api/records/10122133/files"

mkdir -p "$output_dir"
exec >>"$output_dir/download.log" 2>&1
echo "[start] $(date --iso-8601=seconds)"

archives=(
  indoor_hard_01.zip
  indoor_easy_02.zip
  indoor_kidnap_01.zip
  indoor_kidnap_02.zip
  outdoor_hard_01a.zip
  outdoor_hard_01b.zip
  outdoor_hard_02a.zip
  outdoor_hard_02b.zip
  outdoor_kidnap_a.zip
  outdoor_kidnap_b.zip
)

# Zenodo record 10122133 metadata, fetched 2026-07-16. Verify both the size and
# checksum so an interrupted response cannot be mistaken for a completed ZIP.
declare -A expected_sizes=(
  [indoor_hard_01.zip]=1951298635
  [indoor_easy_02.zip]=1716486703
  [indoor_kidnap_01.zip]=2351148578
  [indoor_kidnap_02.zip]=1692338769
  [outdoor_hard_01a.zip]=1642263643
  [outdoor_hard_01b.zip]=1345802065
  [outdoor_hard_02a.zip]=1507761853
  [outdoor_hard_02b.zip]=1298771600
  [outdoor_kidnap_a.zip]=650850753
  [outdoor_kidnap_b.zip]=1250583886
)
declare -A expected_md5=(
  [indoor_hard_01.zip]=c5eca239e8635656af1ec5c5bb6532b7
  [indoor_easy_02.zip]=ecf01901f0e581c81d20e2bf86867925
  [indoor_kidnap_01.zip]=c1ce1dcab51ee6a92a8d9afd4b349e21
  [indoor_kidnap_02.zip]=8b68cd8d90bf623181e9353b1c94df35
  [outdoor_hard_01a.zip]=d126024c8f310c2c48239f136d7e0ed0
  [outdoor_hard_01b.zip]=d397c28cca763d34844189cb73de8e02
  [outdoor_hard_02a.zip]=8f56fdfa93f4fa456e234f58e193a08c
  [outdoor_hard_02b.zip]=267377a88402f825ab70a161cc48983c
  [outdoor_kidnap_a.zip]=3c6941b8c70ca41c79dae83758632625
  [outdoor_kidnap_b.zip]=893ed4a732c5c0c9dc38d069e7056a69
)

download_archive() {
  local archive="$1"
  if command -v aria2c >/dev/null 2>&1; then
    aria2c --continue=true --auto-file-renaming=false --file-allocation=none \
      --max-connection-per-server=16 --split=16 --min-split-size=1M \
      --dir="$output_dir" --out="$archive" \
      "$base_url/$archive/content"
  else
    wget --continue --output-document "$output_dir/$archive" \
      "$base_url/$archive/content"
  fi
}

for archive in "${archives[@]}"; do
  for attempt in 1 2; do
    echo "[download] $archive attempt=$attempt"
    download_archive "$archive"
    actual_size="$(stat --format=%s "$output_dir/$archive")"
    if [[ "$actual_size" == "${expected_sizes[$archive]}" ]] &&
       echo "${expected_md5[$archive]}  $output_dir/$archive" |
         md5sum --check --status; then
      echo "[verified] $archive size=$actual_size md5=${expected_md5[$archive]}"
      break
    fi
    echo "[error] $archive size=$actual_size expected_size=${expected_sizes[$archive]} checksum_mismatch"
    if [[ "$attempt" == 2 ]]; then
      exit 1
    fi
    echo "[retry] removing invalid completed download: $output_dir/$archive"
    rm -f -- "$output_dir/$archive" "$output_dir/$archive.aria2"
  done
done
