from pathlib import Path
import json
import subprocess

r = Path(__file__).resolve().parent
repo = r / 'worktree'
e = repo / 'experiments/results/inten_passive_trace'
path = e / 'evidence/summary.json'
rel = path.relative_to(repo).as_posix()
assert path.read_bytes() == subprocess.check_output(['git', 'show', 'HEAD:' + rel], cwd=repo)
x = json.loads(path.read_text())
assert x['exit_code'] == 0
f = json.loads((e / 'evidence/fixture/summary.json').read_text())
assert f['exit_code'] == 0 and all(code == 0 for code in f['native_exits'])
assert all(all(v == 0 for v in c['metric_abs_delta'].values()) and
           all(p['identical'] for p in c['poses'].values()) for c in f['comparisons'])
assert all({'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m'} <=
           set(c['unavailable_metrics']) for c in f['comparisons'])
commit = subprocess.check_output(['git', 'log', '-1', '--format=%H', '--', rel], cwd=repo, text=True).strip()
native = x['native_source_commit']
text = f'''# Passive InTEn-LOAM solver observation

Roadmap item 12 needs observations from the normal estimator. The existing
forced-pose diagnostic changes frame placement, mapping and prediction, so it
cannot supply that observation. The new optional C++ trace records the actual
S2S/S2M solves, branch decisions and normal equations without supplying a pose.
It is analysis tooling outside the paper, off by default.

[API usage and field semantics](../../../docs/inten_loam_passive_trace.md)
describe the stage frames, return counts, residual convention, stop reasons
and forced-pose exclusion. No estimator parameter is selected by this work.

## Committed validation

Native source `{native}`; result evidence committed at `{commit}` before
these numerical statements. Release configure/build and all
**{x['ctest_entries_passed']} CTest entries** exit 0. The recorded successful
cases include {len(x['new_passive_test_cases_passed'])} new passive-trace tests:
moving trace-on/off pose and retained-state equality, fallback/rejection,
reset/skipped stages, termination distinctions, and rejection of forced-pose
mixing before state updates. Existing forced-pose tests also pass.

The committed MCD fixture comparison uses the default paper pipeline and the
existing `paper_odometry_plane_check` manifest arguments unchanged. Those
plane-check arguments were selected on KITTI 07 windows with 00 influencing
the diagnosis, then transferred here without tuning. All
**{len(f['native_exits'])} native commands** exit 0. Both comparisons have
**zero differences in all available accuracy/frame fields**, with identical
method and evaluated-GT pose bytes. Official KITTI segment RTE is unavailable
on this short fixture; unavailable values remain null. This checks the
tracing-disabled new binary against the pinned old executable. Tracing-on
retained-state equality is covered by the moving synthetic tests.

Frozen reference SHA256: `{f['binaries']['reference']['sha256']}`.
Candidate SHA256: `{f['binaries']['candidate']['sha256']}`.
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

The initial full CTest run exits **{x['initial_ctest_exit_code']}**: the unchanged
`test_posture_confirmation` calls `fputs` after an unchecked failed `fopen` on
the root-owned `/tmp/posture_profile.yaml`. The execution user cannot write
that file. The [initial attempt](evidence/initial_attempt/) retains its exact
failed log, full CTest output and original build/command receipts.
Setting GoogleTest's supported `TEST_TMPDIR` to a fresh user-owned directory
makes the targeted test and the complete suite pass with the same source and
binaries. [Environment probe and corrected commands](evidence/native_checks/commands.json)
record that change; no test is skipped or threshold changed.

The initial source-scope preservation check exits
**{x['initial_preservation_exit_code']}** because its nonrecursive Git pathspec
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
'''
(e / 'README.md').write_text(text)

p = repo / 'papers/inten_loam/README.md'
text = p.read_text()
assert '<!-- INTEN_PASSIVE_TRACE -->' not in text
text += f'''
<!-- INTEN_PASSIVE_TRACE -->
## Optional passive solver trace

The C++ `InTenLoamPaper::process` API accepts an optional fourth argument,
`InTenLoamPaperTrace*`. It observes the normal S2S/S2M path, accepted poses,
fallback decisions, iteration normal equations, weighted residual sums and
stop reasons. This repository analysis tool is outside the paper and off by
default. Combining it with `forced_pose` throws before estimator state changes;
the older forced-pose diagnostic remains an intervention.

All {x['ctest_entries_passed']} CTest entries pass, including moving trace-on/off
pose and retained-map parity. The default and unchanged 07-selected plane-check
settings also match the pinned old executable on the committed short MCD
fixture: all {len(f['native_exits'])} native calls exit 0, available accuracy/frame
differences are zero and saved poses are byte-identical. Short-fixture KITTI
segment RTE remains unavailable. Evidence was committed at `{commit}` before
this prose. These checks preserve behavior; the full KITTI solver-level cause
remains open.

The initial whole-suite run exits {x['initial_ctest_exit_code']} because an
unrelated existing test tries to overwrite a root-owned shared temporary file.
Its failure is retained; the same tests and binaries pass after selecting a
fresh user-owned `TEST_TMPDIR`. The study documents both attempts.

[API semantics](../../docs/inten_loam_passive_trace.md) and
[protocol, complete evidence, commands and limits](../../experiments/results/inten_passive_trace/README.md)
describe the scope. Published results and estimator defaults are unchanged.
<!-- /INTEN_PASSIVE_TRACE -->
'''
p.write_text(text)

p = repo / 'docs/roadmap.md'
text = p.read_text()
anchor = '''    with 00 influencing the diagnosis, so 00 is not fully held out.
13. **CT-ICP'''
replacement = '''    with 00 influencing the diagnosis, so 00 is not fully held out.
    An opt-in [passive C++ solver trace](inten_loam_passive_trace.md) now records
    the normal S2S/S2M stages, fallback and acceptance decisions, normal
    equations and residuals without a forced pose. Moving synthetic parity
    and short-fixture compatibility checks preserve the existing behavior;
    full failure-sequence tracing and the solver-level cause remain open.
    [Validation evidence and commands](../experiments/results/inten_passive_trace/README.md).
13. **CT-ICP'''
assert anchor in text
p.write_text(text.replace(anchor, replacement))

p = repo / 'docs/inten_loam_passive_trace.md'
text = p.read_text()
text += '\n[Committed validation results and exact commands](../experiments/results/inten_passive_trace/README.md) retain the native tests, compatibility comparison and their scope.\n'
p.write_text(text)
print('Documented committed passive trace evidence; full-sequence solver cause remains open.')
