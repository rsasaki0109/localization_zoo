# Passive InTEn-LOAM CLI export

The runner exports the existing passive solver API when
`--inten-loam-paper-trace-jsonl NEW.jsonl` is supplied. It is default off,
outside the paper, and supplies no pose or solver gate. This branch depends
on [PR #140](https://github.com/rsasaki0109/localization_zoo/pull/140).

[Usage and record semantics](../../../docs/inten_loam_passive_trace.md#streaming-export-from-the-evaluation-runner)
explain coordinate frames, incomplete traces, skipped inputs and explicit
nonfinite values. The [protocol](protocol.json) pins the source and settings.

The committed [serializer checks](evidence/serializer_check/summary.json)
pass all 3 cases with actual compile/test exit codes zero. They cover failed
step values, matrix order and precision, escaped paths, effective settings,
existing-file protection and the completion record. Evidence was committed
at `7e74cb76` before this prose. The exact [commands](evidence/serializer_check/commands.json)
and losslessly compressed logs are retained.

## Closed runner compatibility checks

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
| `paper_default` | all zero | identical to reference | 5 | 36 |
| `paper_odometry_plane_check` | all zero | identical to reference | 5 | 36 |

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
/workspace/localization-env/host-venv/bin/python experiments/results/inten_trace_export/validate_fixture.py \
  --candidate-binary /workspace/localization-env/inten-trace-export/focused_build/pcd_dogfooding \
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
/workspace/localization-env/host-venv/bin/python experiments/results/inten_trace_export/validate_skipped_input.py \
  --candidate-binary /workspace/localization-env/inten-trace-export/focused_build/pcd_dogfooding \
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

Fresh whole-zoo CMake/CTest evidence is maintained separately from the
focused binary comparison above; the PR links the exact-head CI status.

## Fresh whole-zoo native validation

The separate fresh Release configure/build and all 126 CTest entries
exit 0. A fresh user-owned `TEST_TMPDIR` is selected before the run.
[Commands and actual exits](evidence/full_native/commands.json),
[whole-suite evidence](evidence/full_native/summary.json), lossless logs and
the as-executed driver are retained. This evidence was committed at
`c679dde4837dc097b697f18654c6cc51da0ffa0f` before this prose. The closed build/CTest and fixture
checks were not rerun during import recovery.

The first postprocessor stops when its preservation subprocess exits 120
after writing a successful result JSON. The controlled same-command check
reproduces exit 120 and `BrokenPipeError` with a closed stdout pipe, while
regular-file capture exits 0 and produces an identical success report.
Original stderr is unavailable, so the original pipe state is not proven.
[Failure and recovery evidence](evidence/full_native/recovery/summary.json)
retain this distinction; the recovery captures subprocess output to files.
