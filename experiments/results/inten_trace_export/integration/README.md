# Trace export integration with optional acceleration

The combined runner accepts the default-off acceleration and passive export
options together. The trace header records the effective boolean
`params.accelerated`, including false when disabled. Residual queries retain
their ordered serial reduction; recording changes no solver setting or decision.
The new field is additive to schema version 1; existing archived traces are retained.

Native source is `1dd35f5cd4a052c275321ba52e99c399dc4aea62`. The executable
reference uses integrated API source `eb5f0c47`, identified by source and binary
hash, with export disabled. Both CLI usage flags survive the source merge.

## Committed checks

- `2e6d7689` records all 12 successful native exits 0: reference, new trace-off
  and trace-on for default and unchanged KITTI07-selected plane-check registration,
  each with acceleration disabled and enabled. Within every execution profile,
  all available accuracy/frame differences are zero and every saved pose file
  is byte-identical. All four traces have the full effective parameter keys,
  the correct acceleration boolean, complete frame/iteration data and end record.
- The same commit retains all 7 expected negative CLI exits 1/2, including
  exclusive output creation and trace/summary collision checks.
- `79cd80a3` records the incremental full Release build, all 126 CTest entries
  with exit 0, and four-worker InTEn/serializer checks with exit 0. The serializer
  checks true and default false acceleration settings.

[Protocol](protocol.json), [driver](validate_fixture.py),
[native commands](evidence/native/commands.json), and
[fixture comparisons, commands, hashes and traces](evidence/fixture/summary.json)
preserve the exact execution evidence. Repeat the fixture check with the recorded
build and pinned API reference:

```bash
python3 experiments/results/inten_trace_export/integration/validate_fixture.py --output-dir /tmp/inten-export-integration-new
```

Registration settings are reused from KITTI07; KITTI00 influenced the original
diagnosis. This short MCD fixture selects no setting, and long-segment KITTI RTE
is unavailable. Comparisons are within each execution mode, without assuming
fast/slow equality for arbitrary nearest-neighbour ties. No throughput is
remeasured and no full-sequence accuracy or failure cause is established.
The frozen 177-cell replay and its raw data remain separate.
