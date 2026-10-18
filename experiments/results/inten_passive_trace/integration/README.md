# Passive trace integration with optional acceleration

The original passive API and the optional execution acceleration changed the
same solver loops. Their initial merge preflight exits 1 with source/test
conflicts; the recorded resolution preserves the ordered residual reduction
and adds observations after it. `accelerated` and the trace pointer retain
their off/null defaults. No registration threshold, gate, iteration budget,
map input or pose acceptance rule changes.

Native source is `eb5f0c47cc517f3e8d12d1bd36761f92e546faa0`, integrating the
speed branch `5beada3c` and the original API `45721068`. The reference executable
is pinned to speed source `82678117`, with native source identity and executable
hash checked. Historical evidence and results are retained separately.

## Committed checks

- `7aa83cff` records the full incremental Release rebuild, all 125 CTest entries
  with exit 0, and the InTEn entry with four workers and exit 0. Every passive
  lifecycle test now runs with acceleration disabled and enabled: moving poses
  and later map state, class counts, H/g/delta observations, fallback, skipped
  mapping, stopping reasons and forced-pose rejection.
- `fb9318f3` records all 8 real-fixture native exits 0. The default and unchanged
  KITTI07-selected plane-check profiles each compare the prior speed executable
  with integrated trace-off in both execution modes. Available accuracy/frame
  differences are zero; method, evaluated-GT and legacy-GT poses are byte-identical.
  Official long-segment KITTI RTE is unavailable on this short fixture.

The full [native commands and logs](evidence/native/commands.json),
[fixture commands and comparisons](evidence/fixture/summary.json),
[protocol](protocol.json) and [fixture driver](validate_fixture.py) are retained.
No new scan is fetched or borrowed. The plane check was selected on KITTI07,
with KITTI00 influencing the earlier diagnosis; this fixture selects no setting.

For the recorded build and pinned reference, repeat the compatibility check with:

```bash
python3 experiments/results/inten_passive_trace/integration/validate_fixture.py --output-dir /tmp/inten-api-integration-new
python3 experiments/results/inten_passive_trace/verify_preservation.py --base-commit 5beada3cbae215ceee54450da3085cb90bcffbb5 --output /tmp/inten-api-preservation-new.json
```

Each execution mode is compared with its own reference. These checks do not
prove fast/slow tie equivalence on arbitrary scans, full-sequence accuracy,
solver cause or a new speedup. The real trace-on export comparison belongs to
the CLI integration. The roadmap speed target and full failure diagnosis remain open.
