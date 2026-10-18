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

## Compatibility with optional acceleration

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
