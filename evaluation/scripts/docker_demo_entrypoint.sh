#!/usr/bin/env bash
# Container entrypoint: run the one-command demo against the prebuilt binaries,
# then copy the report to /out when the caller mounted a host directory there.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT_DIR="${ROOT}/experiments/results/runs/demo_localization_zoo"

bash "${ROOT}/evaluation/scripts/demo_localization_zoo.sh" --skip-build "$@"

if [[ -d /out ]]; then
  out_owner="$(stat -c '%u:%g' /out 2>/dev/null || true)"
  cp -r "${OUT_DIR}/." /out/
  if [[ -n "${out_owner}" ]]; then
    find /out -mindepth 1 -maxdepth 1 -exec chown -R "${out_owner}" {} +
  fi
  echo
  echo "Demo report copied to the mounted /out directory."
  echo "Open report.html in a browser to inspect the results."
fi
