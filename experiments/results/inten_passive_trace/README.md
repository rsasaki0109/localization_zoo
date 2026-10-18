# Passive InTEn-LOAM solver observation

Roadmap item 12 needs observations from the normal estimator. The existing
forced-pose diagnostic changes frame placement, mapping and prediction, so it
cannot supply that observation. The new optional C++ trace records the actual
S2S/S2M solves, branch decisions and normal equations without supplying a pose.
It is analysis tooling outside the paper, off by default.

[API usage and field semantics](../../../docs/inten_loam_passive_trace.md)
describe the stage frames, return counts, residual convention, stop reasons
and forced-pose exclusion. No estimator parameter is selected by this work.

## Committed validation

Native source `67cb05b24a7566c6cf00415784f36cb03593ea71`; result evidence committed at `ad6010cf1c1cecb5c7012f37d92558126132a83e` before
these numerical statements. Release configure/build and all
**125 CTest entries** exit 0. The recorded successful
cases include 5 new passive-trace tests:
moving trace-on/off pose and retained-state equality, fallback/rejection,
reset/skipped stages, termination distinctions, and rejection of forced-pose
mixing before state updates. Existing forced-pose tests also pass.

The committed MCD fixture comparison uses the default paper pipeline and the
existing `paper_odometry_plane_check` manifest arguments unchanged. Those
plane-check arguments were selected on KITTI 07 windows with 00 influencing
the diagnosis, then transferred here without tuning. All
**4 native commands** exit 0. Both comparisons have
**zero differences in all available accuracy/frame fields**, with identical
method and evaluated-GT pose bytes. Official KITTI segment RTE is unavailable
on this short fixture; unavailable values remain null. This checks the
tracing-disabled new binary against the pinned old executable. Tracing-on
retained-state equality is covered by the moving synthetic tests.

Frozen reference SHA256: `e9ed1f650d42b73d1156e8b3eda3a145ebed4ad382323863f0d89d886d5d166d`.
Candidate SHA256: `e34c6294bdd41c5d1efe64b576283511cbe6d12dfb138ae62a5bf84d110b8459`.
Their source and input hashes, actual argv/cwd, native exits, saved poses,
all metric deltas and unavailable fields are in
[fixture/summary.json](evidence/fixture/summary.json).
OMP and BLAS settings are fixed to 1. Concurrent builds/replays make these
execution times unsuitable for a speed claim.

The normal equations permit later conditioning analysis. High support or a
convergence stop does not establish correct motion. No full failure sequence
was instrumented here, and the KITTI solver-level cause remains open.
Original aggregates, published tables and selected settings are preserved.
No scan is fetched or read from the controller-owned KITTI sequence.

The initial full CTest run exits **8**: the unchanged
`test_posture_confirmation` calls `fputs` after an unchecked failed `fopen` on
the root-owned `/tmp/posture_profile.yaml`. The execution user cannot write
that file. The [initial attempt](evidence/initial_attempt/) retains its exact
failed log, full CTest output and original build/command receipts.
Setting GoogleTest's supported `TEST_TMPDIR` to a fresh user-owned directory
makes the targeted test and the complete suite pass with the same source and
binaries. [Environment probe and corrected commands](evidence/native_checks/commands.json)
record that change; no test is skipped or threshold changed.

The initial source-scope preservation check exits
**1** because its nonrecursive Git pathspec
omits the changed paper files. The [original report](evidence/preservation.json)
and helper as executed are retained. Explicit recursive globs now verify the
allowed native changes, reference-source identity and the untouched original
replay source; [corrected checks](evidence/final_source_checks/commands.json)
and [preservation report](evidence/final_source_checks/preservation.json) exit 0.
Stored aggregates and publication bytes match throughout. This corrects the
verification helper without changing the tested binaries or repeating their
already-closed fixture runs.

## Commands and evidence

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
TEST_TMPDIR=/workspace/localization-env/inten-passive-trace/native_checks_retry/tmp ctest --test-dir build --output-on-failure
/workspace/localization-env/host-venv/bin/python experiments/results/inten_passive_trace/validate_fixture.py --output-dir /workspace/localization-env/inten-passive-trace/native_fixture
/workspace/localization-env/host-venv/bin/python experiments/results/inten_passive_trace/verify_preservation.py --output /workspace/localization-env/inten-passive-trace/preservation.json
```

[protocol.json](protocol.json) pins scope, source and comparison rules.
[Native commands](evidence/native_checks/commands.json), the complete CTest
log, fixture commands, summaries and poses are retained in [evidence/](evidence/).
The host drivers use the existing `localization-zoo-dev` container; exact
container argv and environment settings are in the command receipts.
[import.json](evidence/import.json) records original/stored SHA256 and verifies
every compressed log decompresses to its original bytes. Actual process exits
determine success.

This C++ API change adds no command-line exporter or Python/ROS binding.
Further diagnosis needs continuous full-sequence traces using the unchanged
selected settings and a separate controlled interpretation of their records.
