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
The current [`checkpoints/01/`](checkpoints/01/) adds the committed, cleaned 01
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
