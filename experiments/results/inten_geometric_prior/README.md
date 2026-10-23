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

## Reproduction and resumption

Exact commands, working directories, commits and exits live in [commands/](commands/).
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
