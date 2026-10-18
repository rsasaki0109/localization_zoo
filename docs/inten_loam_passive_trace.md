# Observing the normal InTEn-LOAM estimator

`InTenLoamPaper::process` accepts an optional `InTenLoamPaperTrace*` as its
fourth argument. Leaving it null keeps observation off. This analysis API is
repository tooling, outside the paper's estimator; it supplies no pose,
changes no solver gate and performs no additional registration solve.

```cpp
#include "inten_loam/inten_loam_paper.h"

using namespace localization_zoo::inten_loam;
InTenLoamPaper odometry;
InTenLoamPaperTrace trace;
// Reuse the same odometry instance for the complete sequence.
const auto result = odometry.process(points, normalized_intensities,
                                     nullptr, &trace);
// Consume or copy trace here, before reusing it on the next call.
```

The optional third argument, `InTenLoamPaperDiagnostics`, remains the existing
forced-pose experiment. Its `forced_pose` affects placement, mapping and the
next prediction. Combining a non-null `forced_pose` with the passive trace
throws `std::invalid_argument` before estimator state changes. Passing the
older diagnostics object without a forced pose is allowed; its legacy fields
retain their previous meaning.

The trace resets at the start of every successful call. It has a frame index,
an initialization flag and the final world-from-sensor pose. The estimator
does not retain the trace pointer. Callers choose how to export or store it;
this change adds the C++ observation API.

## Stage and iteration records

For S2S, `input` is the previous accepted relative motion, `output` is the
transform actually returned by the solver before caller fallback, and
`accepted_relative` is the transform retained for the odometry chain.
`s2s_fallback` records the existing caller decision, including a nonfinite
solver-step failure. A solver can make earlier updates and then fail; its
output should therefore be read together with its return count and fallback
flag.

`s2m_prediction` is the existing drift-corrected odometry prediction. When
mapping is scheduled, S2M `input` and `output` record its actual world poses;
`s2m_accepted` reports whether the caller applied that output. The final pose
reflects this decision. On initialization or a frame where mapping is
disabled/skipped, the corresponding solver has `called == false` and
`NotRun`; its default matrices are placeholders, not observations.

Each attempted solver iteration records its linearization pose, total
unregularized normal matrix `H`, gradient `g`, and the existing update
`delta`. The left perturbation is ordered translation then rotation. S2S
uses the previous sensor frame as target; S2M uses world coordinates.
`step_attempted`, `step_finite` and `update_applied` distinguish insufficient
support, a failed numerical step and an applied update. A nonfinite `delta`
can contain nonfinite entries; exporters must represent them explicitly
rather than silently replacing a failed step with a zero update.

The class counts refer to edge, ground, facade and intensity residuals at
that same linearization. Their sum equals the total count. They can change
between iterations as correspondences are rebuilt. `returned_pairs` retains
the solver's existing return value, which becomes zero on a nonfinite step
even when the last recorded system has support.

`weighted_squared_residual` is the sum of the existing Huber IRLS weight
times squared residual at the linearization. It is not the Huber objective
or a post-update score. Residual counts, intensity scaling, accepted
correspondences and coordinate frames matter when comparing it between
records. `H` permits downstream conditioning analysis, but its mixed
translation/rotation units require an explicit analysis convention.

The stop enum distinguishes no call, a zero iteration budget, insufficient
pairs, a nonfinite step, convergence and exhausted iteration budget. It
reports the branch taken; convergence and high support alone do not
establish correct motion or reliable correspondences.

## Validation and remaining diagnosis

The [validation protocol](../experiments/results/inten_passive_trace/protocol.json)
pins the implementation and compatibility comparison. The new mechanism
tests check moving trace-on/off pose and retained-map parity, fallback and
mapping rejection, trace reset, skipped stages, termination distinctions,
and rejection of forced-pose mixing. Existing forced-pose tests remain in
the same suite.

The API enables a later continuous observation of the full failure
sequences using their unchanged selected setting. The solver-level cause
in [roadmap item 12](roadmap.md) remains open. Synthetic parity tests and
short-fixture compatibility do not establish that cause or replace a full
sequence replay.

[Committed validation results and exact commands](../experiments/results/inten_passive_trace/README.md) retain the native tests, compatibility comparison and their scope.

## Streaming export from the evaluation runner

`pcd_dogfooding` enables observation only when
`--inten-loam-paper-trace-jsonl NEW.jsonl` is supplied together with
`--methods inten_loam --inten-loam-paper-odometry`. Use a new path with an
existing parent directory. The exporter refuses to overwrite a file, requires
`.jsonl`, and rejects a path shared with `--summary-json`. Tracing is off when
the flag is absent. It is repository analysis tooling, outside the paper.
Keep the estimator arguments from the selected manifest unchanged when
observing a failure sequence. For the existing plane-check setting:

```bash
./build/evaluation/pcd_dogfooding DATA GT.csv \
  --methods inten_loam --no-gt-seed --inten-loam-paper-odometry \
  --input-max-range-m 120 --input-vertical-angle-correction-deg 0.17 \
  --inten-loam-paper-set s2m_plane_max_dist=0.05 \
  --summary-json run-summary.json \
  --inten-loam-paper-trace-jsonl run-trace.jsonl
```

Those arguments were selected on KITTI 07, with 00 influencing the earlier
diagnosis; the exporter chooses no new setting. The command preserves the
usual input loading and continuous estimator state. It does not accept a
forced pose, restart at a diagnostic window, or add a quality gate. Runtime
includes recording overhead and should not be used as a speed result.

The first JSONL record has `type: header`, a schema/version, full command
arguments, effective paper parameters, loader settings and the output anchor.
Matrices are flat arrays in row-major order. `pose` and S2M matrices use the
estimator world frame; S2S maps into the previous sensor frame.
`anchored_pose` uses the runner's existing output anchor, so it can be compared
with the saved trajectory; that anchor is not fed into the solver.

Every input has either a `frame` record or a `skipped_frame` record. The
latter preserves the loader's existing empty-cloud skip and supplies no
solver observation. `input_index` addresses the selected input list, while
`frame_index` counts actual estimator calls. Do not silently equate them
when an input was skipped. Scan paths identify inputs but are not hashes;
retain the native command, binary/source identity and input hashes with the
experiment, as in the validation records.

Each processed frame contains the passive API's stage and iteration data,
feature counts, DOR removal count and both pose conventions. A stage with
`called: false` has placeholder matrices, not a solved identity transform.
Numbers use round-trip double precision. Nonfinite entries are the JSON
strings `NaN`, `+Infinity` or `-Infinity`; they must not be converted to zero
or treated as finite measurements. The residual definition and mixed units
of `H` remain those described above.

Records are flushed per input and only the current frame is retained in
memory. The final `end` record counts processed and skipped inputs. It marks
completion of estimator traversal, not successful metric output: require
both native exit code zero and a valid summary as well. A truncated trace
without `end` is incomplete. File creation, write and close failures fail
the invocation; an interrupted file is kept as evidence.

[Exporter validation protocol and execution records](../experiments/results/inten_trace_export/README.md)
cover the CLI separately from the underlying observation API. Full failure
sequence diagnosis remains open.

## Export with optional acceleration

`--paper-pipeline-accelerated` can be combined with passive export. The header
records its effective `params.accelerated` boolean. Acceleration and export both
remain off by default; this additive header field leaves archived records intact.

All 12 real-fixture calls and all 126 CTest entries pass, with saved-pose parity
within each execution mode. The 7 expected negative CLI exits are retained;
four-worker InTEn/serializer checks also pass. Evidence precedes this statement
at `2e6d7689` / `79cd80a3`. [Commands, traces and limits](../experiments/results/inten_trace_export/integration/README.md)
separate compatibility from full-sequence accuracy, speed and solver diagnosis.
