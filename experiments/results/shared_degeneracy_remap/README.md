# Shared local translation degeneracy remap

The first component of roadmap item 6 is implemented and evaluated: the existing
DiLO opt-in remap is an Eigen-only helper in
[`common/degeneracy.h`](../../../common/include/common/degeneracy.h), also used by
opt-in SuMa. Both defaults stay off. Coarse initial registration, near-range
sharing and NCLT evaluation remain open.

Full KITTI 00–10 is complete with one fixed SuMa configuration:
`--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`.
The value comes unchanged from DiLO's existing 01 calibration, without a SuMa
sweep. Sequence 01 is development evidence; historical SuMa results and 02
diagnostics were already known. No new held-out claim is made.
The paired measurements reject this transfer as a general SuMa setting.
For example, 01 official RTE rises from 13.792690% to 95.257580% and ATE from
175.826760 m to 1398.495188 m. No parameter is reselected.

[Full accuracy table](report/table.md), [diagnostic table](report/diagnostics.md),
[numeric CSV](report/comparison.csv) and [report with immutable input hashes](report/summary.json)
retain all sequences. [SuMa's README](../../../papers/suma/README.md) records
negative results and failure intervals. Saved-pose diagnostics locate startup
and later failures without establishing a solver-level cause.
[01](plots/01/trace.png) and [02](plots/02/trace.png) traces retain SVG, numeric CSV,
input provenance and checked commands beside the figures; no path fitting or
rescaling is applied. Pointwise position error is distinct from official RTE.

[Protocol](protocol.json), [candidate](candidate.json) and [baseline](baseline.json)
pin scope, settings, selection provenance and frozen binary hashes. The native
implementation stays at `52aef810ac4f9fe3600188f08ebb943d001bc0fe`; later commits
add evidence and documentation. All 22 existing selected DiLO/SuMa full-sequence
cells match RPE, ATE, frame count and official translation/rotation RTE, as
checked by the full report. The [frozen pre-change full 01 comparison](sequences/01/frozen_comparison.json)
also has identical saved-pose bytes for both methods. The audit covers the
helper's two native consumers; existing common loaders and other cores are
unchanged. This is not a new whole-zoo audit.

New measurements stay in this nested evidence directory and are not promoted
into published leaderboard/Table 6 selection or paper-reference metadata.
Nested aggregates retain the runner's automatic `Adopt as current default`
label for the sole variant in each experimental manifest. That within-manifest
ranking is retained verbatim; the separately measured raw control determines
the rejection here. No paper-ratio or speed claim is made.

`run_all.py` records exact commands and actual process exits. It uses
`evaluation/scripts/fetch_kitti_odometry_sequences.py` one sequence at a time,
checks full indices/GT frame counts and scan hashes, evaluates and audits, then
rechecks hashes before removing only its owned scans and alias. Every sequence
has input, comparison, command, diagnostic and cleanup records under
[`sequences/`](sequences/). [Final controller completion](attempts/final_completion.json)
records successful completion. Concurrent independent jobs have one
OpenMP/OpenBLAS worker each and are unsuitable for speed comparisons.

`native_wrapper.py` forwards arguments unchanged, isolates each output cwd and
records executable SHA, argv, environment and exit. Direct/wrapped fixture parity
is retained under [`wrapper_validation/`](wrapper_validation/). Replays should
use fresh artifact directories and a binary built from the pinned source;
per-cell command receipts specify full arguments. The controller has fixed
workspace paths and refuses pre-existing native artifacts, so replay its cell
commands into an isolated copy rather than overwrite the committed records.

Truncated HTTP Range responses required fetch-only continuation on 03, 06 and
08. [Failure and launch receipts](attempts/) and per-sequence `fetch_attempts/`
retain every failed command/log/exit. The official downloader's CRC-checked
atomic files were reused. `--resume-from NN --check-resume-only` validates
committed earlier comparisons/cleanup without state writes; actual continuation
is limited to fetch-stage checkpoints and refuses native artifacts.
[Initial guard checks](checks/resume_guard/commands.json),
[06](checks/resume_06_guard/command.json) and [08](checks/resume_08_guard/command.json)
retain the actual checks. Native binaries and method settings stayed fixed.

The full report was generated from committed inputs, before writing the numeric
documentation, with:

```bash
python experiments/results/shared_degeneracy_remap/summarize_results.py \
  --output-dir experiments/results/shared_degeneracy_remap/report
```

[Report command and checked exit](report/command.json) records the exact Python
path and cwd. The default requires committed full 00–10 coverage and validates
binary hashes, complete argv, threads, frame counts, GT CSV identity, diagnostic
definitions, official RTE agreement, old selected metrics and owned-input cleanup.
`--allow-partial` was used only for explicitly incomplete historical snapshots
under `checks/reporting*`; those are not the completed report.

[Release receipts](checks/commands.json) record all 123 CTest entries and 290
Python regressions passing. [Final regeneration and checks](checks/final/commands.json)
record the mandated three doc generators and subsequent preservation, Python,
showcase, leaderboard, paper-ready and whitespace checks with actual exits.
[Final preservation](checks/final/preservation.json) checks every historical
aggregate hash, published README/HTML/Table 6/paper metadata bytes, fixed
manifests and native source identity. Generated evidence is collapsed in GitHub
diffs; handwritten orchestration, protocol and implementation remain visible.
