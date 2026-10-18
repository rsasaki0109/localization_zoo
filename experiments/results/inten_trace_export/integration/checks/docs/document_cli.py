from pathlib import Path
import json
import subprocess

W = Path('/workspace/localization-env/inten-trace-export/worktree')
E = W / 'experiments/results/inten_trace_export/integration'
for rel in ['evidence/fixture/summary.json', 'evidence/native/exit.json']:
    p = E / rel
    assert p.read_bytes() == subprocess.check_output(['git', 'show', 'HEAD:' + p.relative_to(W).as_posix()], cwd=W)
f = json.loads((E / 'evidence/fixture/summary.json').read_text())
assert f['successful_native_exits'] == [0] * 12
assert f['negative_expected_exits'] == [1, 1, 1, 1, 2, 2, 1]
assert json.loads((E / 'evidence/native/exit.json').read_text())['exit_code'] == 0
(E / 'README.md').write_text('''# Trace export integration with optional acceleration

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
''')
for rel, addition in [
    ('papers/inten_loam/README.md', '''\n<!-- INTEN_TRACE_EXPORT_ACCELERATION -->
## Passive export with optional acceleration

The integrated runner records `params.accelerated` in its trace header. All 12
real-fixture calls exit 0, with zero available metric differences and identical
saved poses within each default/accelerated registration profile (`2e6d7689`).
All 7 expected negative CLI exits are retained. The full incremental Release
build, all 126 CTest entries and four-worker InTEn/serializer checks also exit 0
(`79cd80a3`). Settings are reused unchanged from KITTI07, with KITTI00 influencing
the earlier diagnosis. These short checks provide no new speed or long-segment
accuracy result; the full failure cause remains open.
[Exact commands, hashes, traces and limits](../../experiments/results/inten_trace_export/integration/README.md).
<!-- /INTEN_TRACE_EXPORT_ACCELERATION -->
'''),
    ('docs/inten_loam_passive_trace.md', '''\n## Export with optional acceleration

`--paper-pipeline-accelerated` can be combined with passive export. The header
records its effective `params.accelerated` boolean. Acceleration and export both
remain off by default; this additive header field leaves archived records intact.

All 12 real-fixture calls and all 126 CTest entries pass, with saved-pose parity
within each execution mode. The 7 expected negative CLI exits are retained;
four-worker InTEn/serializer checks also pass. Evidence precedes this statement
at `2e6d7689` / `79cd80a3`. [Commands, traces and limits](../experiments/results/inten_trace_export/integration/README.md)
separate compatibility from full-sequence accuracy, speed and solver diagnosis.
'''),
    ('experiments/results/inten_trace_export/README.md', '''\n## Integration with optional acceleration

The [later integration record](integration/README.md) preserves all original
evidence above and checks the combined API/speed/export source. The full
effective parameter header includes the execution mode.
''')]:
    p = W / rel
    assert addition.strip() not in p.read_text()
    p.write_text(p.read_text().rstrip() + '\n' + addition)
print('Documented committed CLI integration evidence; no native edits.')
