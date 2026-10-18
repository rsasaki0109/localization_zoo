from pathlib import Path
import json
import subprocess

W = Path('/workspace/localization-env/inten-passive-trace/worktree')
E = W / 'experiments/results/inten_passive_trace/integration'
for rel in ['evidence/fixture/summary.json', 'evidence/native/exit.json']:
    p = E / rel
    assert p.read_bytes() == subprocess.check_output(['git', 'show', 'HEAD:' + p.relative_to(W).as_posix()], cwd=W)
fixture = json.loads((E / 'evidence/fixture/summary.json').read_text())
assert fixture['native_exits'] == [0] * 8
assert json.loads((E / 'evidence/native/exit.json').read_text())['exit_code'] == 0
text = '''# Passive trace integration with optional acceleration

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
'''
(E / 'README.md').write_text(text)
for rel, addition in [
    ('papers/inten_loam/README.md', '''\n<!-- INTEN_PASSIVE_TRACE_ACCELERATION -->
## Passive trace and acceleration integration

The source conflict with optional acceleration is resolved while retaining the
original residual-addition order. All 125 CTest entries pass, and the InTEn entry
also passes with four workers; every passive lifecycle case runs in both
execution modes (`7aa83cff`). All 8 short real-fixture native calls exit 0,
with zero available metric differences and byte-identical saved poses against
each mode's prior executable (`fb9318f3`). These are compatibility checks;
long-segment RTE is unavailable, no speed is remeasured, and the full failure
cause remains open. Defaults and historical numbers are unchanged.
[Commands, settings provenance, evidence and limits](../../experiments/results/inten_passive_trace/integration/README.md).
<!-- /INTEN_PASSIVE_TRACE_ACCELERATION -->
'''),
    ('docs/inten_loam_passive_trace.md', '''\n## Compatibility with optional acceleration

The API also supports `InTenLoamPaperParams::accelerated = true` from the optional
speed implementation. Observation happens after residual queries and their
ordered serial reduction; it adds no parallel write to H/g or the trace buffer.
Both options keep their default off/null settings. Rebuild compiled clients
after integrating these API/parameter changes.

All 125 CTest entries and a four-worker InTEn check pass; all 8 real-fixture
compatibility calls exit 0, with identical saved poses within each execution
mode. Evidence was committed at `7aa83cff` / `fb9318f3` before this statement.
[Protocol, exact commands and limits](../experiments/results/inten_passive_trace/integration/README.md)
separate these checks from the pending full-sequence failure diagnosis.
'''),
    ('experiments/results/inten_passive_trace/README.md', '''\n## Integration with optional acceleration

The later [integration record](integration/README.md) preserves the original
study above and adds compatibility evidence for the combined speed/API source.
Both registration and execution profiles are identified explicitly.
''')]:
    p = W / rel
    assert addition.strip() not in p.read_text()
    p.write_text(p.read_text().rstrip() + '\n' + addition)
print('Documented committed API integration evidence; native source remains fixed.')
