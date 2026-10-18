from pathlib import Path
import json,subprocess
r=Path(__file__).resolve().parent;repo=r/'worktree';e=repo/'experiments/results/inten_trace_export'
# Numerical prose is derived only from closed evidence committed before this script.
for commit,path in [('4f235364','evidence/fixture/summary.json'),('086d70d0','evidence/skipped_input/summary.json')]:
 data=subprocess.check_output(['git','show',commit+':'+str((e/path).relative_to(repo))],cwd=repo)
 assert data==(e/path).read_bytes()
fixture=json.loads((e/'evidence/fixture/summary.json').read_text());skip=json.loads((e/'evidence/skipped_input/summary.json').read_text())
assert fixture['successful_native_exits']==[0]*6 and skip['native_exits']==[0]*3
assert fixture['negative_expected_exits']==[1,1,1,1,2,2,1]
p=e/'README.md';s=p.read_text();old='''The full CMake build and short-fixture runner comparison are still pending.
No full KITTI failure is explained and no speed or accuracy improvement is
claimed. The original whole-zoo replay continues in a separate frozen tree;
this study fetches no scans and changes no stored result.
''';
if old not in s and '## Closed runner compatibility checks' in s:
 print('Committed result documentation already applied');raise SystemExit(0)
assert old in s
s=s.replace(old,'''## Closed runner compatibility checks

Evidence was committed at `4f235364` before these results. The existing default
and unchanged KITTI 07-selected plane-check setting were transferred to the
committed short MCD fixture; KITTI 00 influenced the earlier plane diagnosis.
There is no new selection or tuning on this fixture.

All 6 native calls exit 0. For both settings, the pinned PR #140 reference,
new runner with tracing off, and new runner with tracing on have zero
absolute difference in every available accuracy/frame field and identical
status. Method and evaluated/legacy GT pose files are byte-identical.
Official KITTI segment RTE is null on this short fixture; this is a
compatibility check, not a full-sequence accuracy result.

| Setting | Off/on available metric differences | Saved pose bytes | Trace records | Observed iterations |
|---|---|---|---:|---:|
''')
for c in fixture['comparisons']:
 s+=f"| `{c['variant']}` | all zero | identical to reference | {c['trace']['records']} | {c['trace']['iterations']} |\n"
s+='''
Each exported file is strict JSONL and contains its header, every processed
frame and traversal end. Effective parameter keys are checked against the
C++ parameter struct; exported anchored poses agree with the existing
saved-pose precision. Counts, residuals and termination reasons are
observations, not accuracy or conditioning guarantees.

The candidate is a focused Release runner build linked with pinned,
unchanged PR #140 libraries; it is not described as a fresh whole-zoo
rebuild. [Compile/link commands](evidence/focused_build/commands.json) and
[all static/system link-input hashes](evidence/focused_build/pinned_link_inputs.json)
are retained. An initial incremental-build preflight exits 1 because it
looked for a container system library on the host. The corrected preflight
hashes inside the same container; no native source or setting changes.
Fresh full CMake/CTest and exact-head CI are tracked separately.

The exact validation command executed from the repository root is:

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_trace_export/validate_fixture.py \\
  --candidate-binary /workspace/localization-env/inten-trace-export/focused_build/pcd_dogfooding \\
  --output-dir /workspace/localization-env/inten-trace-export/fixture_run
```

[Driver receipt](evidence/fixture_command/command.json),
[complete comparison](evidence/fixture/summary.json),
[original input timestamp identity](evidence/fixture_timestamp_identity.json)
and each native command/log/summary/pose/trace are retained. Use a fresh
output directory when reproducing.

## Failure and skipped-input checks

All 7 negative CLI cases return their expected nonzero exit:

| Case | Native exit |
|---|---:|
| Missing trace path | 1 |
| Wrong selected method | 1 |
| Paper path not enabled | 1 |
| Wrong output extension | 1 |
| Missing parent directory | 2 |
| Existing output file | 2 |
| Trace/summary path collision | 1 |

The existing output bytes remain unchanged and the collision creates no
file. These are expected CLI rejections, not odometry accuracy failures.
[Commands and logs](evidence/fixture/negative/) preserve each invocation.

A separate synthetic lifecycle check retains the first/third committed MCD
scans and replaces the middle with an empty PCD. All 3 native calls exit 0
and save byte-identical poses. The trace has input indices 0/1/2, estimator
call indices 0/1, an explicit empty skip, and final counts of 2 processed /
1 skipped input. Evidence was committed at `086d70d0` before this prose.
The inherited runner removes the empty input without retiming GT, so none
of this synthetic case's accuracy fields are interpreted or advertised.

The first lifecycle attempt exits 1 before estimation because the test
builder omitted the original frame timestamps. Its command, log and
as-executed helper are [retained](evidence/skipped_input_initial_command/).
The corrected builder copies those timestamps unchanged; strict GT
association and estimator arguments are preserved.

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_trace_export/validate_skipped_input.py \\
  --candidate-binary /workspace/localization-env/inten-trace-export/focused_build/pcd_dogfooding \\
  --output-dir /workspace/localization-env/inten-trace-export/skipped_input_retry
```

[Lifecycle result](evidence/skipped_input/summary.json) and
[driver receipt](evidence/skipped_input_command/command.json) retain the
closed check. Generated input copies are not committed; the helper recreates
them from the committed fixture and records their hashes.

## Scope and remaining work

No full KITTI failure is explained and no speed or accuracy improvement is
claimed. The original whole-zoo replay continues in a separate frozen tree;
this study fetches no scans and changes no stored result. Its
[source, sealed-worker and binary integrity check](evidence/primary_integrity.json)
and the [publication/evidence guard](evidence/preservation_before_import.json)
preserve the ongoing run and prior results. No trace SDK/ROS export,
conditioning threshold, retry or estimator fix is added.

The local fresh whole-zoo build and CI are still pending at this checkpoint.
'''
p.write_text(s)
p=repo/'papers/inten_loam/README.md';s=p.read_text();needle='Full failure-sequence tracing and the solver-level cause\nremain open; this export capability is not an accuracy result.';assert needle in s
s=s.replace(needle,'''On the committed short MCD fixture, all 6 native comparisons exit 0:
reference, new trace-off and trace-on have zero available metric differences
and byte-identical poses with both the default and unchanged 07-selected
plane-check setting. Segment RTE is unavailable. A separate empty-middle-input
lifecycle check also has identical poses across all 3 calls; its GT timing
makes accuracy fields unsuitable for interpretation. The initial lifecycle
attempt exits 1 because its builder omitted timestamps; the corrected builder
copies the committed timestamps without relaxing association. Evidence was
committed at `4f235364` / `086d70d0` before these results.

Full failure-sequence tracing and the solver-level cause remain open;
this export capability is not an accuracy result.''')
p.write_text(s)
p=repo/'docs/roadmap.md';s=p.read_text();s=s.replace('track this separately from full failure-sequence diagnosis.','record short-fixture trace-on/off pose parity and explicit skipped-input\n    handling separately from full failure-sequence diagnosis.');p.write_text(s)
print('Documented committed closed compatibility evidence')
