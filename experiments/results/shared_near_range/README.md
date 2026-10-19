# Shared optional sensor-frame near-range filter

Roadmap item 6's third component: the existing runner filter now lives in
[`common/range_filter.h`](../../../common/include/common/range_filter.h).
ELO and DiLO use this one implementation through the runner's XYZ loader;
the intensity-preserving and timed-point loaders use it too. No method-core
filter or parameter was added. The shared runner is the integration point;
standalone method libraries do not acquire automatic preprocessing.

`--input-min-range-m` still defaults to zero (disabled). When enabled, points
strictly inside the radius are removed **after sensor-frame conversion**,
before vertical-angle correction and voxelization. Float squared-distance
arithmetic, boundary inclusion, retained order/fields and cloud metadata follow
the old implementation. This is non-paper preprocessing and stays opt-in.
It does not sanitize non-finite points; consumers retain that policy.

The [current KITTI 00 saved-pose diagnostics](diagnostics/00/README.md) locate
error growth in the five replay paths that differ from stored metrics.
Referenced historical per-run summaries are missing at the inspected revision,
so these diagnostics do not identify the cause of the historical differences.

The [KITTI 01 saved-path diagnosis](diagnostics/01/README.md) compares the
available historical/current Mesh-LOAM paths and locates current CT-ICP error
growth. Its runtime and missing-path limits are explicit; it does not identify
the cause of historical metric differences.

The [KITTI02 saved-path diagnosis](diagnostics/02/README.md) now includes all
historical differences on that sequence and the available Mesh paired paths.

The [KITTI05 saved-path diagnosis](diagnostics/05/README.md) adds the fresh,
cleaned05 cohort, every historical numerical difference and the available Mesh
paired path. Its [cleaned checkpoint](checkpoints/05/summary.json)
retains incomplete whole-zoo coverage and the original historical audit exits.

The [KITTI06 saved-path diagnosis](diagnostics/06/README.md) adds CT-ICP/Mesh
numerical pairs and the available Mesh historical gap. Its earlier
[cleaned checkpoint](checkpoints/06/summary.json) includes only completed,
committed sequences with verified input cleanup.

The [KITTI07 saved-path diagnosis](diagnostics/07/README.md) adds all six
historical differences and current error-growth traces, including VLOM. Its
[current cleaned checkpoint](checkpoints/07/summary.json) excludes live runs;
referenced historical paths are unavailable in the inspected tree/history.

The [KITTI08 saved-path diagnosis](diagnostics/08/README.md) adds all four
historical differences, current error-growth traces and the available Mesh paired
path, with explicit missing-path/runtime limits. Its
[current cleaned checkpoint](checkpoints/08/summary.json) includes verified08
cleanup; live09/10 results remain excluded.

## Scope and status

Implementation and the regression protocol were merged in
[PR #137](https://github.com/rsasaki0109/localization_zoo/pull/137). Full KITTI
replay is running; no full audit completion or new NCLT result is claimed here.
[`selection.json`](selection.json) freezes all stored full-sequence best-RPE
cells across the zoo; the selection command and its actual exit are retained.
These are replays of historical arguments, which can differ by cell. They do
not select a new setting or constitute a fixed-setting accuracy comparison.

[`protocol.json`](protocol.json) pins source/base commits, scope and controls.
[`check_fixture_parity.py`](check_fixture_parity.py) compares the pre-extraction
binary and new binary through ELO, DiLO and InTEn-LOAM on the committed short
MCD fixture: disabled/explicit-zero, the historical NCLT cutoff, sensor-frame
conversion and elevation correction. This is a regression fixture, not NCLT
validation. Common mechanism tests cover the spherical boundary, float
threshold, retained intensity/order, default-off metadata and empty inputs.

Preflight: Release configure/build and all 125 CTest entries exit 0; all 290
Python regressions exit 0. The 15 fixture method/case comparisons have identical
accuracy/frame fields and byte-identical saved poses. Actual native commands,
hashes, logs, exits and the complete comparison are committed under `checks/`.

## Completed KITTI checkpoint: 04

All 14 selected cells completed on full 04. Twelve match the stored RPE, ATE
and frame count. The audit exits **1**, because the following two historical
cells differ; this is not reported as a passing historical audit.

| Method | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---:|---:|---:|---:|
| CT-ICP | 0.713006 | 0.733247 | 2.184636 | 2.191953 |
| Mesh-LOAM | 0.523782 | 0.520627 | 10.602073 | 10.252902 |

The frozen pre-extraction executable reproduces both differences exactly; its
audit also exits 1. For these two methods plus ELO and DiLO, extraction changes
RPE, ATE, frames and official translation/rotation RTE by zero, and saved poses
are byte-identical. This establishes that the differences predate extraction;
it does not identify why the original historical runs differ. The CT-ICP
reproduction is worse; the Mesh-LOAM reproduction improves but still differs.
Both method READMEs retain the values. Stored aggregates are unchanged.

Evidence committed at `7458c24d16096cc305889ed0864a5ef8bfb6bdf4`:
[`sequences/04/audit/`](sequences/04/audit/),
[`frozen_baseline/`](sequences/04/frozen_baseline/),
[`frozen_comparison.json`](sequences/04/frozen_comparison.json),
[`inputs.json`](sequences/04/inputs.json) and
[`cleanup.json`](sequences/04/cleanup.json). Inputs match the previous official
scan/GT hashes and were verified and removed before fetching 00. The remaining
cells are pending; this checkpoint does not establish whole-zoo completion.

[`checkpoints/04/`](checkpoints/04/) records the earlier 04-only checkpoint.
[`checkpoints/00/`](checkpoints/00/) records the earlier cleaned 04/00 checkpoint.
The earlier [`checkpoints/01/`](checkpoints/01/) adds the committed, cleaned 01
sequence. Its verification exit 0 does not replace the retained historical
audit exits 1.
[`checks/checkpoint04/`](checks/checkpoint04/) retains the required document
generators, Python/doc checks and preservation review. Staged whitespace checks
caught CRLF in the derived CSV and then its captured error log. CSV writers now
emit LF; exact failed logs and their nonzero exits are retained as gzip archives,
and the corrected staged check exits 0. Native results are unchanged.

## Completed KITTI checkpoint: 00

The full 00 candidate audit has finished all 20 selected cells. Fifteen match
the historical RPE, ATE and frame count; the following five differ. All native
processes exit 0 and complete 4541 frames, but the historical audit exits **1**.

| Method | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---:|---:|---:|---:|
| A-LOAM | 0.891327 | 0.914174 | 12.043760 | 9.520570 |
| F-LOAM | 0.921410 | 0.990961 | 9.833596 | 8.556112 |
| LeGO-LOAM | 0.838536 | 0.842687 | 12.833331 | 12.378148 |
| Mesh-LOAM | 0.773243 | 0.774034 | 10.180105 | 10.099834 |
| VLOM | 0.875872 | 0.914174 | 9.391448 | 9.520570 |

RPE worsens for all five; ATE improves for the first four and worsens for
VLOM. These values and limits are recorded in each method README. The frozen
pre-extraction replays for A-LOAM, F-LOAM and LeGO-LOAM now finish with native
exits 0. All three have zero old-vs-extracted differences in RPE, ATE, frames
and official translation/rotation RTE, with byte-identical saved poses. Their
historical differences predate the extraction; their original causes remain
unresolved. [Closed control evidence](checks/00_closed_controls/summary.json),
including commands, native exits and artifact hashes, was committed at
`7d55897901c17b0c64567115a6f59db06cf585b0`.

VLOM's frozen replay also finishes with native exit 0. Its RPE, ATE, frames
and official translation/rotation RTE have zero old-vs-extracted differences,
with byte-identical saved poses. Thus both its RPE and ATE worsening predate
extraction; their original cause remains unresolved. The
[VLOM control and verification](checks/00_vlom_control/summary.json) were
committed at `1f7b554f9dfe5878067dc42ae18ed45c51a58e35`.

Mesh-LOAM's frozen replay now also completes with native exit 0. Its RPE,
ATE, frames and official translation/rotation RTE have zero recorded differences
from the extracted implementation, with byte-identical saved poses. Therefore
all five historical 00 differences predate extraction; their original causes
remain unresolved. The parent frozen historical audit exits **1**, retaining
the same differences rather than reporting a clean historical pass.
The complete [frozen comparison](sequences/00/frozen_comparison.json),
[commands and exits](sequences/00/frozen_baseline/) and
[verified input cleanup](sequences/00/cleanup.json) were committed at
`10cf31616365b41878ec9311f0b705e798716c51`. No setting is changed or historical
result replaced.
Summary `evaluator_git_commit` records the working-tree HEAD at write time;
native provenance comes from the frozen executable SHA256/source pins, not
that metadata field.

Completed candidate results, exact commands/logs, executable hashes and input
provenance were committed at `485fa00601644731827dae601eb3b43c7543efeb`:
[`sequences/00/audit/`](sequences/00/audit/),
[`inputs.json`](sequences/00/inputs.json),
[`fetch_attempts/`](sequences/00/fetch_attempts/) and
[`checks/00_primary/verification.json`](checks/00_primary/verification.json).
All 00 scan hashes were verified before deleting the owned scans and alias.
Subsequent sequences are excluded until their own comparison and cleanup
checkpoints are committed. The earlier [00 checkpoint summary](checkpoints/00/summary.json)
and [comparison CSV](checkpoints/00/comparison.csv), committed at `57fb04d3`,
contain **34 of 177 cells** across 04 and 00:
27 historical matches and 7 differences. All 9 frozen controls have zero
recorded metric differences and byte-identical saved poses. Both historical
sequence audits exit 1; checkpoint verification exits 0. Whole-zoo coverage is
still incomplete. The required generators, Python/doc contracts, preservation
and whitespace checks are retained under [checks/checkpoint00/](checks/checkpoint00/),
with actual process exits and losslessly compressed logs.

## Completed KITTI checkpoint: 01

All 13 selected candidate cells finish, with native exits 0 and
1101 evaluated frames each. 11 match stored RPE, ATE and frame count;
the following 2 differ, so the historical audit exits **1**.

| Method | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---:|---:|---:|---:|
| CT-ICP | 0.826792 | 0.830115 | 58.190972 | 57.923334 |
| Mesh-LOAM | 0.849506 | 0.850998 | 30.862639 | 30.940997 |

RPE worsens for both. CT-ICP ATE improves; Mesh-LOAM ATE worsens. The original
arguments are replayed without selecting a setting. The completed candidate
results, commands, native hashes, input hashes and verification were committed
at `9555f01fda68a0294fc7dd5a828a85fac207f68f`:
[candidate evidence](sequences/01/audit/), [inputs](sequences/01/inputs.json),
[verification](checks/01_primary/verification.json).

Both frozen pre-extraction controls now complete with native exits 0. RPE,
ATE, frames and official translation/rotation RTE have zero recorded differences
from the extracted implementation, with byte-identical saved poses. The frozen
historical audit also exits 1. Both historical differences therefore predate
extraction; their original causes remain unresolved.
The [full comparison](sequences/01/frozen_comparison.json),
[commands and exits](sequences/01/frozen_baseline/) and
[hash-verified input cleanup](sequences/01/cleanup.json) were committed at
`6188b71d8e48933a4dffce54afdde7a01382f4b0`. The owned scans and alias are removed.

The [current checkpoint](checkpoints/01/summary.json) and
[comparison CSV](checkpoints/01/comparison.csv), committed at `d63dd3dfe5d25f9444442c3809b13ea3e9c05238`,
cover **47/177 cells** across 04, 00 and 01:
38 historical matches and 9 differences. All 11 frozen controls have
zero recorded metric differences and byte-identical saved poses. All three
historical audits retain exit 1; checkpoint verification exits 0. Remaining
sequences are excluded until their own comparisons and cleanup are committed.
Stored aggregates, Table 6, paper metadata and defaults are unchanged; whole-zoo
coverage remains incomplete. Required checks are retained under
[checks/checkpoint01/](checks/checkpoint01/).

[Committed saved-path diagnostics](diagnostics/01/README.md) pair Mesh-LOAM's
historical/current paths against byte-identical evaluated GT, and locate current
CT-ICP error growth. The Mesh source/header match its recorded original source,
but its original/current OpenMP settings differ; this is not a controlled
thread-count experiment or a cause attribution. CT-ICP's referenced legacy
summary/path is unavailable in the inspected checkout. Diagnostic evidence was
committed at `1426a851c9f021cbffae239e814b6379a5788b25` before numeric prose.

Native stdout contains padded tables. Git whitespace attributes are scoped to
native replay/fixture logs so their exact bytes are retained; code, CSV and
other evidence retain ordinary whitespace checks. Initial failing branch checks
and exact gzip-compressed outputs are retained under `checks/00_primary/`.

## Historical NCLT evidence and limits

The cutoff is transferred unchanged from the existing NCLT full-sequence and
restart-window diagnosis. It is not reselected on KITTI. That NCLT sequence
already influenced the choice; there is no new held-out claim.

- ELO's committed full-sequence summaries record official translation RTE
  18.132951% without the cut and 3.361084% with it; ATE remains 51.639158 m
  with the cut. These are historical results, not a rerun of this extraction.
- DiLO's historical summaries record RTE 8.533284% / 7.369821% and ATE
  54.344593 m / 75.832748 m. **The recorded profiles also differ**: the first
  note says height 192, while the cut summary says height 240, top elevation
  31 degrees and maximum fill gap 8. This is not a controlled cut-only pair.
  The cut-profile ATE is worse and substantial drift remains. Do not attribute
  the RTE difference solely to filtering.

Sources: [`docs/benchmarks/nclt_paper_paths/`](../../../docs/benchmarks/nclt_paper_paths/)
(`elo.json`, `elo_m5.json`, `dilo.json`, `dilo_m5.json`); recorded commands and
failure locations are in [`nclt_paper_paths.md`](../../../docs/nclt_paper_paths.md).
The original summaries have no evaluator commit/argv; the documented commands
and summary notes are retained without inventing missing provenance.

Real NCLT validation remains blocked: its official host
`s3.us-east-2.amazonaws.com` is still absent from the enforced environment
allowlist despite earlier approval to add it. KITTI/MCD regression checks do
not close that item. No alternate host or proxy bypass is used.

## Commands and evidence

The build driver and actual configure/build/CTest exits are retained under
[`checks/preflight/`](checks/preflight/). Baseline and candidate JSON files pin
the frozen native executable hashes. A wrapper preserves all native arguments,
records the actual executable/hash/environment/exit, and isolates saved poses.

```bash
python3 experiments/results/shared_near_range/check_fixture_parity.py
/workspace/localization-env/host-venv/bin/python experiments/results/shared_near_range/run_all.py
python3 experiments/results/shared_near_range/verify_preservation.py --output /tmp/range-preservation.json
```

`run_all.py` uses the official selective KITTI downloader one sequence at a
time, checks every scan and GT hash against the previous official-input study,
replays the frozen cell selection with `audit_kitti_drift.py`, then verifies
hashes and deletes its owned scans/alias before continuing. No stored aggregate
is replaced. Any historical-metric drift is retained and compared against the
frozen pre-extraction executable before proceeding. A failed replay is an error.

Fetch failures (including the initial 04 prefetch) retain their nonzero exits
and logs. Official CRC-checked completed files may be reused by the same
downloader. A fetch-only continuation never changes the source or arguments.

Tests and checks are judged by actual process exits, not log tails. Concurrent
replays use one OMP/BLAS thread per process; no speed comparison is made.

## KITTI 02 saved-path diagnosis preparation

The sequence-specific [source](diagnose_saved02.py) and
[protocol](diagnostics/02_protocol.json) prepare the next offline diagnosis.
The script requires committed, unchanged candidate and frozen reports, native
command receipts, matching saved paths and verified input cleanup before creating
diagnostic files. Owned scans and their alias must already be absent. Missing or
modified required inputs stop the command; the script never invokes an estimator
or reads scans.

```bash
python3 experiments/results/shared_near_range/diagnose_saved02.py \
  --output-dir /absolute/path/to/fresh-diagnostics
```

It includes every historical difference named by the protocol, inherits the
previous diagnostic parameters, and retains all blocks and frames. Historical
pairing requires committed trajectories with the same evaluated GT. Runtime and
missing-path limits remain explicit. The regression fixtures exercise publication
gates and saved-path pairing on synthetic data; they do not add dataset coverage.
The actual diagnosis awaits the committed frozen comparison and cleanup. Commit
its raw outputs before adding numerical prose or updating the cleaned checkpoint.

## One-shot completion of the existing run

`finish_when_complete.py --wait --push --attempt 01` waits for this controller's
terminal success, all committed sequence checkpoints and verified input cleanup.
It starts no native replay. It seals execution scripts/configuration, checks the
working branch and clean tree, and refuses partial data, failed controllers,
changed binaries/source or an existing completion attempt.

After completion it generates and commits the full report before using its
numbers in documentation. `finish_docs.py` retains every historical difference
in the method READMEs, including improvements and original audit exits 1, and
requires zero recorded old-vs-extracted control deltas and identical saved poses.
The required generators, Python/doc checks, preservation and full-branch
whitespace checks must then pass before the final documentation is committed
and pushed. It does not merge, mark the PR ready, or claim new NCLT validation.

Outputs go to `completion/01/`; later evidence needs a new follow-up PR against
`main`, because implementation PR #137 is already merged. The prepared
`PR_BODY.md` contains an old PR137 readiness reference that must be adapted for
the follow-up; pushed-head CI also needs the connected GitHub tool's final
review/update. The ordinary GitHub CLI API
probe was refused; its actual exit is retained, and no alternate access path is
attempted by the worker. [Guard and renderer validation](checks/finalizer_preflight/)
uses committed partial 04/00 evidence: partial reports and the still-running
controller are rejected before public documentation changes. Rendering previews
retain the real 04 numeric differences and historical NCLT confound; they are
validation artifacts, not full-study completion.

## Earlier cleaned checkpoint: 02 and 03 added

The [checkpoint](checkpoints/03/summary.json) and [comparison CSV](checkpoints/03/comparison.csv),
committed at `8f5841e3946995cedfa9c0e2fc484dbd2c02373e`, cover **79/177 cells**
across 04, 00, 01, 02 and 03: **64** historical matches and
**15** differences. All **17** frozen controls have zero
recorded metric differences and byte-identical saved poses. All five historical
audits retain exit **1**; checkpoint verification exits **0**. Coverage is incomplete.

New historical differences, retained in each method README:

| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| A-LOAM | 02 | 0.923231 | 0.926292 | 50.114358 | 50.789841 |
| CT-ICP | 02 | 0.810338 | 0.813023 | 20.606805 | 19.687922 |
| F-LOAM | 02 | 0.937750 | 0.945974 | 52.068212 | 53.822119 |
| LeGO-LOAM | 02 | 0.883497 | 0.883304 | 41.755869 | 42.208784 |
| Mesh-LOAM | 02 | 0.785133 | 0.785817 | 22.288359 | 22.328196 |
| Mesh-LOAM | 03 | 0.877028 | 0.877431 | 3.992708 | 3.995116 |

Candidate/frozen reports and native receipts are committed. Scan hashes were
verified before deleting owned inputs and aliases. The sequence commits are
`a821eaee` (02) and `eebaf3d7` (03). These differences predate extraction;
their original causes remain unresolved. This replays historical per-cell
arguments rather than defining a new single-setting accuracy study.
[Saved02 diagnostics](diagnostics/02/README.md) locate current error growth and
the available Mesh path gap, with missing-path/runtime limits and no cause attribution.
[Saved03 Mesh diagnostics](diagnostics/03/README.md) add the paired historical gap,
all inherited fixed blocks and explicit runtime limits from committed paths.
[Saved04 CT/Mesh diagnostics](diagnostics/04/README.md) retain the complete
historical-difference cohort, the paired Mesh gap and CT missing-path limits;
all recorded additional frozen controls remain validated.

## Environment interruption and remaining-sequence recovery

An earlier managed-environment restart during05 removed the original controller,
native processes and completion worker. At that recovery boundary the interrupted
audit was preserved locally under `sequences/05/interrupted_20261018/`.
Unknown exits remain null, and that attempt contributes no completed cells.

The [recovery protocol](recovery05_protocol.json) and [source](recover_interrupted05.py)
verified frozen source/binary/sealed pins and the retained owned official05
scan/GT hashes, then reran every selected05 cell from fresh native state. That
attempt retained the original audit/comparison helpers, arguments, one-thread
settings and cleanup policy. It was an explicit environment-interruption
addendum to the original fetch-failure-only restart procedure.

[Launch receipts](checks/recovery05_launch/) identify the recovery and replacement
completion worker. Unfinished work is excluded from the checkpoint. Historical
aggregates, paper numbers, Table 6 and defaults are preserved. This replay retains
its original frozen implementation; current-main acceleration/passive tracing
and real NCLT validation keep their previously documented scope.

The subsequent full environment recreation removed that local interrupted
archive, uncommitted05 outputs, owned scans and pinned executables. Both
executables have now been rebuilt at their original sources with exact SHA256
matches, and the original sealed controller restarted at the fresh05 fetch
boundary. Successfully committed and cleaned sequence checkpoints are backed
up by normal pushes to a separate branch. Unavailable interrupted05 work remains
excluded; that restart retained the earlier cleaned checkpoint above. The fresh
completed05 is recorded below. The earlier retained-input recovery
was not invoked again. [Restoration receipts, fresh restart and backup limits](runtime295_restore/README.md).

## Earlier cleaned checkpoint: fresh05 completed

The fresh05 sequence commit `e9822bcf`, imported unchanged as `1d4a37a4`, contains
all **18** selected candidate cells, the completed frozen controls and verified
input cleanup. **13** match the historical metrics; these **5** differ:

| Method | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---:|---:|---:|---:|
| A-LOAM | 0.528651 | 0.512096 | 4.839948 | 5.192731 |
| CT-ICP | 0.469581 | 0.469523 | 4.264453 | 4.079794 |
| F-LOAM | 0.537259 | 0.538788 | 6.035067 | 6.038695 |
| LeGO-LOAM | 0.559431 | 0.515567 | 6.449096 | 5.218196 |
| Mesh-LOAM | 0.397437 | 0.397399 | 5.757510 | 5.772741 |

Both the candidate and frozen historical audits retain exit **1**. Every native
process exits **0** with **2,761** frames. All five frozen comparisons have
zero RPE, ATE, frame and official translation/rotation RTE differences, with
byte-identical saved poses. The differences predate extraction; their original
causes remain unresolved. The table includes both improvements and deteriorations.
Every affected method README retains its full numerical pair.

The [checkpoint](checkpoints/05/summary.json) and [CSV](checkpoints/05/comparison.csv),
committed at `6285c60b`, cover **97/177 cells** across 04/00/01/02/03/05:
**77** historical matches and **20** differences. All **22** frozen controls
have zero metric differences and byte-identical saved poses. All six historical
audits retain exit **1**; checkpoint verification exits **0**. Remaining
06/07/08/09/10 cells are not counted until their own committed cleanup exists.

The [saved05 diagnosis](diagnostics/05/README.md), also committed before numerical
prose, locates current error growth and the available Mesh historical path gap.
Missing paths and differing original/replay thread counts remain explicit
limits. Analysis source/protocol `35bd3ecf` precedes execution and transfers the
existing diagnostic parameters unchanged. This is an offline analysis of
committed paths, with no estimator run or parameter selection.

Official05 scan hashes were verified before removing the owned scans and alias.
The existing controller has advanced to06; its source, binaries and active data
remain untouched. Stored aggregates, Table 6, paper metadata and defaults are
preserved. Historical per-cell replay is not a new fixed-setting accuracy study
or a whole-zoo validation of current-main acceleration/passive tracing.

[Required regeneration, regression and preservation checks](checks/saved05/docs/)
retain actual commands and exits, full lossless logs and exact raw05 import hashes.
The separate checkout's built-evaluator skips remain visible; full native and
ROS coverage runs in final-head CI. The original running checkout is preserved.

## Earlier cleaned checkpoint: 06 completed

The06 sequence commit `27354aa4`, imported unchanged as `54eaeab6`, contains all
**14** selected candidate cells, the completed frozen controls and verified
input cleanup. **12** match stored RPE, ATE and frames; these **2** differ:

| Method | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---:|---:|---:|---:|
| CT-ICP | 0.388505 | 0.391531 | 3.963097 | 3.891527 |
| Mesh-LOAM | 0.505393 | 0.505226 | 2.018974 | 2.024646 |

Both historical audits retain exit **1**. Every native process exits **0** with
**1,101** frames. Both frozen comparisons have zero RPE, ATE, frame and official
translation/rotation RTE differences, with byte-identical saved poses. The
differences predate extraction; their original causes remain unresolved.
CT-ICP RPE worsens and ATE improves; Mesh RPE improves and ATE worsens. Both
method READMEs retain all numerical pairs.

The [checkpoint](checkpoints/06/summary.json) and [CSV](checkpoints/06/comparison.csv),
committed at `e4c528fe`, cover **111/177 cells** across 04/00/01/02/03/05/06:
**89** historical matches and **22** differences. All **24** frozen controls
have zero metric differences and byte-identical saved poses. All seven
historical audits retain exit **1**; checkpoint verification exits **0**.
Remaining07/08/09/10 cells are excluded until their own committed cleanup exists.

The [saved06 diagnosis](diagnostics/06/README.md) locates current error growth
and the available Mesh historical path gap, preserving CT's missing-path and
Mesh's differing original/replay runtime limits. Analysis source/protocol
`45cbb341` precedes execution and transfers existing diagnostic parameters
unchanged. This is an offline analysis of committed paths, with no estimator
run or parameter selection. Raw results precede the numerical prose.

Official06 scan hashes were verified before removing the owned scans and alias.
The existing controller has advanced to07; its source, binaries and active data
remain untouched. Stored aggregates, Table 6, paper metadata and defaults are
preserved. Historical per-cell replay is not a new fixed-setting accuracy study
or a whole-zoo validation of current-main acceleration/passive tracing.

## Earlier cleaned checkpoint: 07 completed

Native/cleanup commit `ed37e2fd`, imported unchanged as `9c9976c6`, contains all
**20** selected cells and **6** frozen controls. **14** match historical RPE,
ATE and frames; these **6** differ:

| Method | Stored 100 m RPE [%] | Replay RPE [%] | Stored ATE [m] | Replay ATE [m] |
|---|---:|---:|---:|---:|
| A-LOAM | 0.607458 | 0.605313 | 2.544355 | 2.505243 |
| CT-ICP | 0.455389 | 0.450827 | 1.256652 | 1.243500 |
| F-LOAM | 0.612719 | 0.590347 | 2.971748 | 3.173641 |
| LeGO-LOAM | 0.527758 | 0.526845 | 2.589775 | 2.557870 |
| Mesh-LOAM | 0.469625 | 0.469538 | 0.852913 | 0.843704 |
| VLOM | 0.607458 | 0.605313 | 2.544355 | 2.505243 |

RPE improves on all six; F-LOAM ATE worsens while the other ATEs improve. Both
historical audits retain exit **1**. All native runs finish with exit **0** and
**1,101** frames. Frozen controls have zero recorded metric/frame differences
and byte-identical saved poses; comparison exits **0**. These differences
predate extraction and their original causes remain unresolved. Every affected
method README retains its numerical pair.

The [checkpoint](checkpoints/07/summary.json) and [CSV](checkpoints/07/comparison.csv),
committed at `6db8522a`, cover **131/177** cells across 04/00/01/02/03/05/06/07:
**103** historical matches and **28** differences, reproduced by **30** exact
frozen controls. All eight historical audits retain exit **1**; checkpoint
verification exits **0**. Whole-zoo coverage is incomplete; remaining08/09/10
cells are excluded until their own committed cleanup exists.

The [saved07 diagnosis](diagnostics/07/README.md) retains all six current
error-growth traces. Every largest inherited block increase occurs at
**800-900**. None of the exact referenced historical paths is available in the
inspected tree/history; no paired historical gap or cause is inferred.
Analysis source/protocol `59e69241` transfers diagnostic parameters unchanged;
raw evidence is committed before numerical interpretation.

Official07 inputs were hash-verified before deleting owned scans and alias.
The existing frozen controller advances to08 without changes to source,
binaries or settings. Historical per-cell replay is not a new fixed-setting
accuracy study or a whole-zoo validation of current-main acceleration/tracing.
Table 6, published accuracy, paper metadata and defaults are preserved.

## Current cleaned checkpoint: 08 completed

Original native/cleanup `15157bab`, imported unchanged as `d0a76218`, contains
all **18** selected candidate cells, **4** completed frozen controls and verified
input cleanup. **14** match historical RPE, ATE and frames; these **4** differ:

| Method | Stored 100 m RPE [%] | Replay RPE [%] | Stored ATE [m] | Replay ATE [m] |
|---|---:|---:|---:|---:|
| A-LOAM | 1.390107 | 1.394630 | 18.110228 | 18.661434 |
| F-LOAM | 1.399647 | 1.372876 | 20.440418 | 17.318202 |
| LeGO-LOAM | 1.375340 | 1.374146 | 17.896468 | 19.120563 |
| Mesh-LOAM | 1.331902 | 1.332030 | 22.402218 | 22.431217 |

A-LOAM/Mesh worsen in both metrics; F-LOAM improves in both; LeGO RPE improves
while ATE worsens. All method READMEs retain their numerical pairs. Both
historical audits exit **1**; all native runs exit **0** with **4,071** frames.
Frozen controls have zero recorded metric/frame differences and byte-identical
saved poses. Comparison exits **0**; the differences predate extraction and
their original causes remain unresolved.

The [checkpoint](checkpoints/08/summary.json) and [CSV](checkpoints/08/comparison.csv),
committed at `9197f428`, cover **149/177** cells: **117** historical matches and
**32** differences, reproduced by **34** exact frozen controls. All nine
historical audits retain exit **1**; checkpoint verification exits **0**.
Remaining09/10 are excluded until their own committed cleanup exists.

The [saved08 diagnosis](diagnostics/08/README.md) records current error growth
at **0-100** for all four methods. GT travels **50.647955 m** in this inherited
block, shorter than official KITTI segments, so its block error is not official
RTE. The available historical/current Mesh gap reaches **0.536423 m** at final
frame **4070**; original/replay OpenMP counts **2 / 1** remain a confound.
Missing exact A/F/LeGO historical paths prevent paired diagnosis for those methods.
All blocks/frames and missing-path/runtime limits are retained without cause claims.

Source/protocol `d969608e` precedes analysis; the real incomplete-input refusal
and synthetic gates are retained at `9f94c9c4`. Original08 input hashes were
verified before deleting the scans/alias and advancing the controller to09.
No estimator or setting selection occurs in this offline analysis. Frozen native
source `0b079dbb`, executables, sealed controller and active09 inputs are untouched.
Published accuracy, paper metadata and defaults remain unchanged. The
[final integration protocol](final_integration_protocol.json) groups the remaining
actual results into one audit-completion PR after every terminal gate passes.

## Checkpoint integration while the frozen replay continues

The [08 regeneration, regression and preservation checks](checks/saved08/docs/),
committed at `ffca75b8`, retain exact commands, actual exits and full lossless
logs. All required generators, full Python and documentation checks exit **0**;
the **5** built-evaluator skips remain explicit. Preservation confirms every
historical aggregate, all original native08 files, published table/paper bytes,
prior method README content and the primary's frozen source/sealed files/binaries.
Final-head native/ROS CI will run for the completed integration PR.

The earlier [07 regeneration, regression and preservation checks](checks/saved07/docs/),
committed at `88c21dd9`, retain exact commands and actual exits, including the
corrected private-driver reference failure. All required generators, full
Python and documentation checks pass. Existing aggregate/publication bytes,
raw native07 evidence, prior method READMEs, active source/sealed files and
frozen executable hashes are preserved. Native and ROS coverage run in CI.

The earlier [06 regeneration, regression and preservation checks](checks/saved06/docs/)
retain exact commands, actual exits, complete lossless logs and raw-import hashes.
The separate checkout's built-evaluator skips remain visible; full native and
ROS coverage runs in final-head CI. The original running checkout is preserved.

The committed checkpoint and diagnostics can be integrated using a separate
worktree while the original replay retains its frozen checkout and executables.
The running controller and completion worker keep their original files and
scan ownership. Later completed sequences are follow-up evidence. Recorded
numerical results retain their original source pins; integrating their documents
does not re-run them on the current native implementations.

[Isolated integration checks and exact commands](checks/merge138/) retain the
native-main identity, historical result/publication preservation and independent
primary-checkout source/binary/sealed-file guards. Full native/Python and ROS
validation runs in CI for the integrated PR head.
