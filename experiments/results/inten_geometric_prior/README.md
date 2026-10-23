# Independent geometric motion/prior trial (hybrid, not paper InTEn)

The default-off `s2s_geometric_prior` option is implemented and the focused
native tests and normal CLI integration pass. **Full-sequence accuracy is not
yet established.** The first KITTI 01 run was interrupted by acquisition HTTP
503 failures after 768 frames. Its disabled/reference path matches the archived
full-precision facade-PCA trajectory exactly through that prefix (maximum
matrix difference 0). The incomplete candidate output is retained, not presented
as a full KITTI result or an adoption decision. After a brief connection
recovery, a fresh unchanged run stopped after 128 frames when HTTP 503 returned.
The independent [prefix audit](interrupted_audit.json) verifies raw hashes,
order, pose exports and exact reference parity for both nonempty attempts.
The intervening acquisition-only retry processed no frames; it establishes no
estimator parity or accuracy.

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
All attempts ended because acquisition failed, not because an estimator failed
the prospective accuracy gate. The gate remains unevaluated.

The saved CLI results also pass an explicit [frame-count and metric audit](commands/cli_saved_validation.json).
The initial check used a nonexistent `frames_processed` key and compared two
missing values for that one field; `frames` and the root `num_frames` are now
required, with the pose line count checked separately. Its original receipt is
retained. This correction does not change any estimator output.

## Predeclared adverse cases in the incomplete continuous trial

The latest acquisition-interrupted attempt contains 832 continuous frames
from frame zero. The [complete prefix audit](interrupted_audit_final.json)
verifies every retained raw hash, order and pose export, and reference matrix
error remains 0. No full-sequence RTE/ATE or retention verdict is inferred.

[adverse_cases.json](adverse_cases.json) reports the cases named in the protocol
before execution. Each entry below is local translation length divided by GT
translation length; 1 is correct length. These are stage diagnostics, not
paper scores. Reference final and hybrid final include InTEn's mapping stage.

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

## Full Python validation is not yet green

The full discovery and all failures are preserved in
[publication_retry/](publication_retry/). The isolated checkout initially lacked
SciPy, real verifier paths and sparse manifests. The corrected checkout reads
all manifests directly from Git and preserves Python module paths; the
[recheck](publication_repaired/) resolves those setup failures.

One historical verification remains unresolved. Its protocol hashes the
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

All seven [source-binding regressions](commands/historical_binding_regressions.json)
pass: development changes are allowed, but changed/missing archived source,
changed manifests and changed historical verification code are rejected. Module
state is restored after a verification failure. These results are committed in
`19b9c862`. Complete Python discovery is being rerun; the adapter's focused
checks alone do not establish a full-suite pass.

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
--commit-results` processes the frozen order, beginning with 01. Run it only
once for a given output directory. On a transport failure, preserve that entire
attempt under `preflight_failures/`, then restart the affected sequence from
frame zero with the unchanged binary/settings. Do not resume an estimator from
an arbitrary scan with empty map/history.

The controller's source checkout must contain the existing `inten_facade_pca`
input manifests and trace shards and the reference GT CSVs. The normal per-sequence
manifests also support reproducing through `run_experiment_matrix.py` when a
complete sequence fits on disk. The continuous runner exists to avoid that
storage requirement, not to change input selection or evaluator conventions.
