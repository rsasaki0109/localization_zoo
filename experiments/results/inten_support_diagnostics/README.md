# InTEn-LOAM: correspondence support at saved failure steps

The committed native logs show substantial correspondence support at the sampled
steps with large length errors. This excludes the **low-count
branch at those recorded frames**. It does not explain the motion deficit or
exclude an earlier fallback influencing later estimates.

This is an offline join of the existing fixed-setting runs, not an estimator
experiment. No scans are read or downloaded and no native code, parameter,
stored aggregate, paper value or published accuracy table is changed.

## Scope and reproducibility

[`protocol.json`](protocol.json) records the new fixed-setting coverage runs
01-06 and 08-10 included in this analysis. The low-count hypothesis was formed
while inspecting these saved failures; this is an exploratory diagnostic.
Historical 00/07 runs are outside this log
analysis. Every included manifest uses the unchanged `paper_odometry_plane_check`
arguments: the plane distance and elevation correction were selected on 07,
with 00 influencing the diagnosis. No new setting is selected here. The focus
01/02/10 failures were already identified by their paper comparisons.

Run from the repository root, with NumPy available:

```bash
python3 experiments/results/inten_support_diagnostics/analyze.py --output-dir <fresh-directory>
python3 experiments/results/inten_support_diagnostics/render_docs.py
```

The exact executed interpreter, command, working directory, analysis revision,
input hashes and recorded native build/binary pins are in
[`analysis/summary.json`](analysis/summary.json). Native execution commands and
successful exits come from the existing coverage study's `parallel_execution.json`.
The original code at `d1b7288ff46964bde86ef15ab34b2ae8b4f389da` is read directly from Git;
the InTEn paper source/header match this branch byte for byte. The recorded
build metadata is distinguished from the analysis commit.

The analysis and raw check receipts were committed at `283bc86fa939062d80b36057ccca2ce42a891f9c` before this
numeric prose. [`checks/preflight/`](checks/preflight/) preserves actual exits
and losslessly compressed logs. Existing step ranges/counts are independently
recomputed from saved poses and matched, and the saved-pose accuracy metrics
agree with their recorded summaries within the declared analysis tolerance.

## Observed support and missing observations

The native runner prints progress every 10 frames. Frame 0 is initialization
and is excluded from step statistics. A bad-length step has GT travel at least
0.1 m and an absolute length-ratio error above 20%, exactly as in the existing
saved-pose diagnostics. Its index is the destination frame.

| Sequence | All bad-length steps | Logged bad steps | Unlogged bad steps | Min S2S pairs on logged bad steps | Min S2M pairs on logged bad steps |
|---|---:|---:|---:|---:|---:|
| 01 | 136 | 14 | 122 | 694 | 1381 |
| 02 | 133 | 14 | 119 | 927 | 1413 |
| 03 | 2 | 0 | 2 | - | - |
| 04 | 0 | 0 | 0 | - | - |
| 05 | 0 | 0 | 0 | - | - |
| 06 | 0 | 0 | 0 | - | - |
| 08 | 20 | 2 | 18 | 1061 | 1951 |
| 09 | 0 | 0 | 0 | - | - |
| 10 | 0 | 0 | 0 | - | - |

Across these sequences, all 1737 logged moving steps, including 30 logged
bad-length steps, have S2S and S2M returned counts at least 30. The recorded
code replaces an S2S solution with the previous relative motion only below 30
pairs; S2M accepts its correction at or above 30 pairs. The selected pipeline
has mapping enabled at every frame. Thus neither low-count condition holds
at the sampled failure frames. A dash means no logged bad step, not a measured
zero minimum.

The [per-logged-step CSV](analysis/logged_steps.csv) also retains vector error,
feature counts and S2M class counts. The
[complete bad-length interval CSV](analysis/bad_length_intervals.csv) includes
the singleton and unobserved intervals, without filling missing counts.

## Limits and next diagnostic

The large counts rule out correspondence-count shortage at the observed steps
only. The unlogged bad steps in the table remain unobserved, including earlier
frames that might influence prediction or the map. No claim is made about
their fallback branches. Sequence 10 has no step beyond the existing 20%
threshold despite its paper gap; this threshold does not describe all drift.

Many correspondences can still be ambiguous, poorly distributed or associated
incorrectly. These progress logs contain neither per-iteration residuals nor
Jacobian conditioning, so they do not establish observability, convergence or
the solver-level cause. Ground truth here labels saved steps only; it never
initializes an estimator.

The merged [passive trace API and streaming workflow](../../../docs/inten_loam_passive_trace.md)
now provide stage motions, fallback/acceptance decisions, normal equations and
iteration residuals for continuous observation. Tracing stays opt-in and default
off. The legacy forced-pose diagnostics change placement and prediction and
cannot serve as passive observation. The streaming checker validates record/run
consistency; it does not establish solver quality or a failure cause. Conditioning
analysis also requires an explicit convention for mixed translation/rotation units.

Continuous failure-sequence collection remains pending. Use the unchanged
selected registration setting after the ongoing frozen replay releases native
and input ownership; do not borrow its live scans. Existing progress records
still leave earlier fallback and unlogged frames unobserved. Short-fixture
trace compatibility is separate from this full failure diagnosis.

## Reproduction after main integration

Source `3d7e3f91d4e0c28d8c286fef68e72a49ca5c14fe` follows integration of main
`a597e4c95614e733fc20977321fe4d6aea0fba46`. The analyzer reads native source/header
blobs from the recorded evaluator commit and the protocol's pinned input commit,
and requires those historical blobs to match. Current native files can evolve
without changing the interpretation of recorded progress. Stored logs, poses,
manifests and analysis inputs retain their committed-byte checks.

The pre-fix command exits 1 because the current native source differs from the
historical execution; [its receipt and log](checks/main_refresh/before_fix/command.json)
are retained. Raw fix evidence was committed at `70bac660` before this prose.
The [reproduction commands](checks/main_refresh/analysis/commands.json) succeed;
[numerical identity](checks/main_refresh/analysis/numerical_identity.json) records
byte-identical CSVs and unchanged protocol, selected arguments, sequence counts,
step intervals and metric comparisons. The original analysis above remains
attached to its original evidence commit. Re-analysis adds explicit historical
source provenance without a new estimator execution or accuracy measurement.

The [new metadata and outputs](checks/main_refresh/analysis/analysis/summary.json)
record the actual analysis/source/input pins. Core algorithms and evaluation
sources match integrated main; original numerical artifacts remain unchanged.
