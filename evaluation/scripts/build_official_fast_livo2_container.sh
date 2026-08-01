#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
image="${FAST_LIVO2_IMAGE:-localization-zoo/fast-livo2-official:0d2c0346}"

docker build \
  --pull \
  --tag "${image}" \
  "${repo_root}/evaluation/docker/fast_livo2_official"

docker run --rm "${image}" bash -lc \
  'test -x /opt/fast_livo2_ws/devel/lib/fast_livo/fastlivo_mapping && git -C /opt/fast_livo2_ws/src/FAST-LIVO2 rev-parse HEAD'
