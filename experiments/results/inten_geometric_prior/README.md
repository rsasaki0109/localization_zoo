# Independent geometric motion/prior trial (hybrid, not paper InTEn)

The fixed default-off hybrid completes its first full sequence, **KITTI 01**.
Official RTE improves from **4.481189% to 0.967180%** and ATE from
**88.786456 m to 20.265326 m** against the frozen improved InTEn reference.
Every reference pose matches the archived full-precision trajectory exactly.
The independent saved-output audit passes. This hypothesis was selected on 01;
all KITTI data are development data, and the all-sequence retention gate is
still pending. The remaining sequences continue with the same configuration.

All earlier acquisition-interrupted attempts remain under `preflight_failures/`.
The successful run first acquired and verified the complete 01 input, then
started the unchanged estimator from zero. Processed scans were removed only
after hash, order and reference-control verification. This is a repository
InTEn + Zoo KISS-ICP hybrid result, not a paper-InTEn or upstream-KISS result.

## What the option does

An independent instance of the existing Zoo KISS-ICP point-to-point pipeline
maintains its own pose, map and motion history. Its current relative increment
initializes InTEn S2S and, when enabled, anchors the existing weak translation
prior. InTEn still performs its own S2S/S2M refinement and map update; KISS poses
are not substituted for InTEn outputs. The geometric input takes precedence
over the mapped-motion seed when usable. Existing fallback behavior remains.
This is an explicitly labeled repository hybrid extension, not a mechanism
claimed by the InTEn paper or the official upstream KISS implementation.

Default is off, with no geometric backend allocated or run. Enable with
`--inten-loam-paper-set s2s_geometric_prior=1` on the paper pipeline. It requires
`deskew=0` and rejects forced-pose diagnostic use before updating state. The
same parameter is available in `InTenLoamPaperParams`. The full fixed trial
settings are in the [01 manifest](../../inten_kitti_seq_01_full_geometric_prior_matrix.json),
with identical variants for 00–10.

The hypothesis was selected using the earlier committed
[independent onset comparison](../inten_independent_onset/README.md) on 01.
Input range/elevation and the reference's mapped prior, weak ratio, ground PCA
and facade PCA are inherited unchanged; upstream mechanics defaults supply the
geometric settings. There is no new sequence-specific parameter search. Every
KITTI sequence is development data.

## Full-sequence results with the fixed candidate

The [full native output and metrics](sequences/01/) were committed in `c99beb6a`
before this interpretation. The [independent audit](completed_audit_01.json)
(`11902445`) verifies all 1101 input hashes, pose exports, source/binary pins,
exact reference parity (maximum matrix difference 0), and reconstructs the
complete saved diagnostic objects. Native execution and the audit both exit 0.

| Sequence | Reference RTE [%] | Hybrid RTE [%] | Reference ATE [m] | Hybrid ATE [m] | Bad-length steps, reference / hybrid |
|---|---:|---:|---:|---:|---:|
| 01 | 4.481189 | 0.967180 | 88.786456 | 20.265326 | 44 / 7 |

00 and 02–10 remain pending. RTE is official KITTI translation error over
100–800 m segments. A bad-length step has more than 20% local translation-length
error on a GT step of at least 0.1 m. The matching bad-vector count is also
44 / 7. These full-sequence results do not establish all-sequence acceptance,
a held-out generalization result, SOTA, or a paper-score update.

The old prolonged failure at destination frames 204–247 is reduced to 209–210
and 214–215, but new bad steps occur at 432–433 and 436. The 200–300 block has
44 / 4 bad steps; 400–500 has 0 / 3. In the latter block, endpoint-relative
translation error rises from 0.701984% to 1.781673%. This block diagnostic is
not official KITTI RTE. The global improvement does not erase this local regression.

[Every remaining bad step](posthoc_bad_steps_01.json) (`5b13b243`) is retained
below. Selection is post hoc: the union of all candidate bad-length and
bad-vector frames from the unchanged complete-sequence diagnostics. Values
are translation length / GT step length. No setting was selected from them.

| Frame | Reference final | Hybrid independent input | Hybrid S2S output | Hybrid final |
|---|---:|---:|---:|---:|
| 209 | 0.365882 | 1.007675 | 0.845556 | 0.798389 |
| 210 | 0.353933 | 0.984569 | 0.943412 | 0.621042 |
| 214 | 0.371042 | 0.993543 | 0.940296 | 0.742404 |
| 215 | 0.306116 | 1.003769 | 0.780277 | 0.373955 |
| 432 | 0.976234 | 0.990294 | 1.005920 | 0.734083 |
| 433 | 0.926296 | 1.007687 | 0.926461 | 0.564614 |
| 436 | 0.899063 | 0.996726 | 0.957629 | 0.735212 |

None uses S2S fallback; all accept mapping. At 432, the accurate S2S output
(1.005920) becomes 0.734083 after mapping. This locates an adverse correction,
without establishing which map support, residual or history caused it.
Complete the frozen trial before changing the estimator; the next causal
study should replay mapping with the same map/history.

## Frozen prospective trial

[protocol.json](protocol.json) was committed before the new measurements.
One candidate configuration is used across every sequence. Reference and
candidate run in the same persistent native process, with no GT input and no
state reset at acquisition-batch boundaries. Every reference frame must match
the corresponding archived full-precision trace within `1e-8`. Input hashes and
order must match the previous facade study. Only one sequence is fetched at a
time with the existing official fetch script; processed, verified raw scans
are removed to keep disk usage bounded.

After all full sequences finish, the predefined experimental retention gate
requires lower geometric-mean official RTE and no sequence exceeding
`max(1.10 * reference RTE, reference RTE + 0.05 percentage points)`.
All outcomes, including regressions and onset adverse cases, must be reported.
This gate does not change any historical failed gate, paper-variant eligibility
or Table 6. No gate verdict is available from the interrupted prefix.

## Evidence and current blocker

- [Passing build/test commands](build_retry_commands.json): actual production
  CMake targets for InTEn and KISS, including independent-history and
  forced-diagnostic rejection tests. The first synthetic test incorrectly
  required a prior on every well-constrained step; its failure remains in
  [build_commands.json](build_commands.json), followed by the corrected test.
  Estimator parameters were not changed to satisfy it.
- [Normal harness build](harness_build_commands.json) and
  [CLI fixture comparison](cli_fixture/check.json): option accepted, with
  disabled output poses byte-equal to the frozen old binary. Unchanged native
  libraries reused by this resource-bounded build are pinned in
  [harness_dependencies.json](harness_dependencies.json).
- [Interrupted prefix](preflight_failures/interrupted_01/): every raw native
  output, scan hash, reference/candidate pose and failed fetch is retained.
  Native exit `-9` is the controller terminating the process after acquisition
  exhausted its initial retries, not a solver crash or a successful full run.
- [Data connection probe](commands/acquisition_data_probe.stdout): the actual
  503 body identifies an Envoy `cloudflare_https_tunnel` connection failure.
  GitHub push initially failed with the same transport error. One push later
  succeeded, saving implementation commit `4fb78d02`, before the failure returned
  ([recorded push retry](commands/push_retry.json)). Actual scan batches also
  briefly succeeded; neither event established a stable recovered connection.

The retry uses the same binary/configuration from frame zero and only adds
bounded backoff for acquisition failures. Keep all failed attempts. A successful
retry must not be inferred from elapsed time or partial pose files. Full native
/ ROS2 CI and full-sequence acceptance are not claimed by the focused checks.

The recovery attempt and its exhausted acquisition retries are preserved in
[preflight_failures/recovery_interrupted_01/](preflight_failures/recovery_interrupted_01/).
All retained interrupted attempts ended because acquisition failed, not because
an estimator failed the prospective accuracy gate. The all-sequence gate remains
unevaluated after the completed 01 result.

The saved CLI results also pass an explicit [frame-count and metric audit](commands/cli_saved_validation.json).
The initial check used a nonexistent `frames_processed` key and compared two
missing values for that one field; `frames` and the root `num_frames` are now
required, with the pose line count checked separately. Its original receipt is
retained. This correction does not change any estimator output.

## Predeclared adverse cases reproduced in the complete 01 trial

The [complete-run case report](adverse_cases_complete_01.json) (`11902445`)
reproduces every predeclared stage diagnostic from the retained interrupted
prefixes. The cases below were named in the protocol before execution; they
remain distinct from the post-hoc full-run cases above. Values are local
translation length / GT length, not full-sequence accuracy. Reference final
and hybrid final include InTEn's mapping stage.

| Frame | Reference final | Hybrid S2S input | Hybrid S2S output | Hybrid final |
|---|---:|---:|---:|---:|
| 202 | 0.949347 | 1.006358 | 0.799373 | 0.968400 |
| 215 | 0.306116 | 1.003769 | 0.780277 | 0.373955 |
| 420 | 0.999430 | 1.004785 | 0.589326 | 0.986409 |

At 215, the independent motion enters at 1.003769, S2S shortens it to 0.780277,
and the accepted S2M result ends at 0.373955. The reference is also bad
(0.306116); the hybrid still fails the >20% length diagnostic. This is direct
negative evidence that an accurate independent seed alone does not eliminate
this continuous trajectory's failure. At 420, S2S worsens relative to the
reference S2S output (0.589326 versus 0.606071), although mapping recovers the
final step to 0.986409. At 202, S2S is still near the diagnostic boundary
(0.799373), while mapping recovers 0.968400. All these stages are retained;
none is used to tune a new setting. A next causal diagnosis should distinguish
S2S distortion from the accepted mapping correction and map-history effects.
This does not replace completing the frozen all-sequence trial after transport
recovers.

The production code supplies the independent motion to S2S only. `solveS2M`
optimizes its own feature-map objective without that motion prior, and mapping
acceptance checks the returned correspondence count. This identifies where to
inspect the accepted correction; it does not establish which residual or map
history causes the observed shortening. A passive same-history mapping replay
should separate these effects before proposing another estimator change.

The later 960-frame attempt (`5984759a`) also restarts from zero with the
[same frozen binary](commands/frozen_binary_recovery_check.json). Its
[predeclared cases](adverse_cases_after_approval.json) retain the same negative
diagnosis and displayed ratios. Acquisition exhausts the original retries;
native exit -9 is the controller stopping the waiting estimator after that
failure. Small data probes and Git push briefly recover, then the saved
[range probes](commands/active_retry_range_probe.stdout) again identify
`cloudflare_https_tunnel` HTTP 503. No estimator setting or accuracy gate changes.
A further [retry](commands/full_trial_after_serial_fix.json), started after
successful small-range probes, fails before any native frame (`19a6a1a7`).
Small-range success is therefore insufficient evidence of recovered scan
transport. Retain this acquisition-only failure as well as the nonempty prefixes.

A subsequent [real-batch fetch](commands/real_batch_transport_after_runtime_fix.json)
and [input/binary hash check](commands/recovered_batch_hash_validation.json)
both pass (`b2d9cccc`). Restarting the same continuous trial nevertheless
stops at 960 frames again (`c991f723`), after the next acquisition batch exhausts
its retries. A separate contiguous-batch probe fails while reading the remote
ZIP directory, before any scan is extracted. The [new case report](adverse_cases_after_verified_batch.json)
retains the previous diagnosis; all predeclared case diagnostics match the
earlier report. The latest audit and case report are committed in `351353fa`.
At that stage no complete sequence was available. Even a successful real-batch preflight does
not guarantee sustained transport; avoid repeated estimator restarts while
the recorded acquisition failure remains unresolved.

The last interrupted 01 [attempt](commands/full_trial_after_tail_recovery.json) (`c960a788`)
first acquires and hash-verifies all remaining 01 tail scans, frames 960–1100
(`622b4bd2`), then restarts the frozen estimator from zero. Transport fails
earlier this time: the run stops after 832 frames while fetching 832–895.
Its [independent audit](interrupted_audit_after_tail_recovery.json) and
[predeclared cases](adverse_cases_after_tail_recovery.json) (`b375dfbb`) retain
exact reference parity and the same adverse-case diagnostics. The native exit
is -9 because the controller stops the waiting process after acquisition fails.
The subsequent successful run first makes the entire sequence locally available
and [hash-verifies every input and the frozen binary](full_01_input_preflight_20261024.json)
(`3c8fd971`). This acquisition change does not change estimator settings or
allow a mid-sequence state reset.

## Python validation: complete fresh discovery

The [fresh complete discovery](publication_final_discovery/) succeeds with
**448 tests, five skipped, exit 0** (`a9f2991b`). It runs in the recorded
Python/NumPy/SciPy environment, including the corrected nested source binding
and every original numerical worker. All six documentation generation/check
commands also exit 0; regenerated historical tables are byte-identical.
The [independent artifact audit](fresh_discovery_audit.json) (`73edf50f`)
checks command/log hashes, complete worker payloads and unchanged tested
production/test sources. Worker outputs were captured during this execution,
not recovered from historical expected answers. All four match the earlier
executed payloads exactly. Full native/ROS2 CI is not claimed.

### Earlier failures and separate post-run checks

The earlier recorded-runtime discovery runs 445 tests with one final
source-binding failure and five skips. Every numerical reconstruction worker
passes. After fixing the nested binding, all 13 binding regressions and the
original global audit pass separately. Those checks alone did not establish
a fresh all-green discovery; the complete execution above now supplies it.
All earlier failures remain unchanged.

The full discovery and all failures are preserved in
[publication_retry/](publication_retry/). The isolated checkout initially lacked
SciPy, real verifier paths and sparse manifests. The corrected checkout reads
all manifests directly from Git and preserves Python module paths; the
[recheck](publication_repaired/) resolves those setup failures.

The initial historical verification failed because its protocol hashes the
production source in the working tree, including the InTEn source/header and
trace serializer. The new option intentionally changes those files; the
[deterministic preflight](commands/historical_source_preflight.stdout) shows
that this historical check cannot pass against the new source. Its original
protocol, source hashes, archived outputs and numerical tolerances are **not**
changed. Expensive rechecking was explicitly stopped after that precondition
failed ([stop receipt](commands/stop_invalid_source_verification.json)); this
is not a successful full Python/CI run. An earlier worker also terminated
abnormally during the full discovery, whose failure is retained.

The [historical-source adapter](../../../evaluation/scripts/verify_inten_support_history.py)
now restores production sources from the study's existing build archive and
checks them against the unchanged protocol and manifest hashes. Experimental
sources and data still resolve to the current checkout, so the original frozen
verifier, recovery-source checks, full group reconstruction and strict numerical
tolerances remain active. The current estimator stays covered by its separate
native/CLI checks. The adapter does not assert that today's production source
matches the historical build.

The subsequent [complete discovery](publication_historical_binding/) retains
its source-binding failures in prior-control and local-rigidity checks and the
support worker's exit -9, including saved worker receipts and memory-event
snapshot. The adapter now covers all affected studies. The passive rigidity
study uses its parent's production archive; its corrected historical test
source is preserved byte-for-byte in [historical_sources/](historical_sources/)
against the unchanged protocol hash before adding the current test wrapper.

All ten [source-binding/scheduling regressions](commands/historical_all_sources_preflight.json)
pass (`1d99e284`). Changed/missing archived source, changed manifests, changed
historical verification code and a changed historical test snapshot are still
rejected. Those preflights did not cover the nested recovery verifier's final
source audit; the additional regression below covers it. Frozen worker commands,
group order, populations and arithmetic
remain unchanged; the adapter runs one worker at a time to bound memory and
restores module state after errors. These focused checks alone did not establish
a full-suite pass. The earlier
failures and deliberately stopped incomplete retry remain archived.

The completed [serial discovery](publication_all_bindings_serial/) (`1f973bd8`)
runs 445 tests with one failure and five skips. It reaches the unchanged native Hessian reconstruction
check for query `(08, 46, 14, false, 1, 2)`. All documentation commands pass
and regenerated historical tables remain byte-identical.

The [runtime witness](runtime_witness/) (`7aa7b0a8`) extracts that exact query
and its source/target features from the hash-verified archive. Python 3.13.5,
NumPy 2.2.4 and SciPy 1.15.3 fail the original check; Python 3.12.14, NumPy
1.26.4 and SciPy 1.17.0, the combination recorded by the original experiment,
pass it. This identifies a runtime-dependent numerical difference, without
isolating a single library as its cause. The witness uses the same frozen
reconstruction and unchanged `rtol=1e-10, atol=1e-12`; it does not establish a
full-suite pass. The subsequent complete discovery uses the recorded runtime.
The command explicitly sets `PYTHONPATH` to select its NumPy installation,
while using the original base interpreter's SciPy. No archived result, hash,
numerical tolerance or estimator setting is changed.

That [recorded-runtime discovery](publication_recorded_runtime/) (`b3c22c0b`)
passes every original numerical worker, including 08, then fails at the nested
full-field recovery verifier's final production-source hash check. The adapter
previously rebound only the main checker. It now binds every nested recovery
module and restores each root on exit, while preserving the scheduler's original
worker commands and working directory.

All 13 [final binding regressions](commands/nested_source_binding_final.json)
pass, including the real nested global-source path, corrupted recovery-source
rejection and restoration after failure. The [post-run global audit](global_audit_final.json)
also passes. Each executed worker's committed receipt has exit 0 and an output
SHA-256 identical to the corresponding lossless historical worker payload.
The recheck requires that exact equality before recovering the temporary output
bytes, then runs the original scheduler bindings, global recovery/source/build
checks, report comparison and original test result assertions. It neither reruns
nor replaces numerical reconstruction with an assumed answer. See
[the executed command](commands/global_audit_final.json) and
[runtime inventory](commands/recorded_runtime_inventory.stdout).
The failed discovery remains unchanged; these separate passing checks do not
rewrite its exit code or establish portability to the system NumPy/SciPy stack.

### Same-version numerical build control

The [CI-locked library witness](runtime_witness/ci_locked_libraries.json)
(`576b6f1d`) also passes with Python 3.12.14, NumPy 1.26.4 and SciPy 1.15.3.
This is a single-query control, not a second complete discovery in CI's environment.

A further controlled comparison (`23ebd429`) keeps the same Python executable
and Python 3.13.5 / NumPy 2.2.4 / SciPy 1.15.3 versions. The
[system NumPy installation still fails](runtime_witness/system_control_repeat.json),
while the [isolated PyPI NumPy wheel passes](runtime_witness/numpy2_same_versions.json).
The SciPy installation stays at the same path. The saved
[system build inventory](commands/system_build_inventory.stdout) and
[wheel build inventory](commands/wheel_same_versions_build_inventory.stdout)
show different numerical builds and BLAS/LAPACK dependencies. Changing the
NumPy installation is sufficient to change this witness's verdict; version
numbers alone do not explain it. This does not isolate the responsible compiler,
BLAS, instruction path or operation. The Python 3.12 NumPy 2.2.4 wheel
[also passes](runtime_witness/numpy2_control.json). Preserve original tolerances
and build provenance rather than treating this as a NumPy-version-only failure.
No trajectory improvement follows from these runtime controls.

## Reproduction and resumption

Exact commands, working directories, commits and exits live in [commands/](commands/).
Normal Git publication recovered after the API upload. The verified history
merge `0f34cb25` preserves both API and original local commits; the recorded
[push](commands/push_source_binding.json) succeeded. Original source IDs are
now ancestors of the PR branch, with the older
[transport aliases](transport_commit_mapping.json) retained for provenance.
The frozen runner/source hashes are in [binaries.json](binaries.json).
`build.py` stages a dedicated temporary source/build tree in the existing
container; `build_harness.py` independently compiles the normal CLI.
`run_sequences.py --source-root <full evidence checkout> --scratch <raw root>
--commit-results` processes the frozen order, beginning with 01. The complete 01
result already exists: do not overwrite or rerun it. A restarted controller must
receive `--sequence` for each remaining sequence only. Do not start another
controller while the current trial is running. Run each output directory once. On a transport failure, preserve that entire
attempt under `preflight_failures/`, then restart the affected sequence from
frame zero with the unchanged binary/settings. Do not resume an estimator from
an arbitrary scan with empty map/history.
The successful 01 attempt uses the official fetch script to acquire the complete
sequence before starting, after the memory-heavy checks finish. Verify every
raw hash against the archived input manifest. A small range or successful batch
has repeatedly failed to establish sustained transport. Other sequences still
use bounded acquisition when a full sequence does not fit. Existing valid raw
files can be reused; estimator history starts at zero, and every input is
verified again and removed after processing.

The controller's source checkout must contain the existing `inten_facade_pca`
input manifests and trace shards and the reference GT CSVs. The normal per-sequence
manifests also support reproducing through `run_experiment_matrix.py` when a
complete sequence fits on disk. The continuous runner exists to avoid that
storage requirement, not to change input selection or evaluator conventions.
