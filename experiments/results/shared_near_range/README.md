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

## Scope and status

Implementation and the regression protocol are committed. Full KITTI replay
is running; no full audit completion or new NCLT result is claimed here.
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

[`checkpoints/04/`](checkpoints/04/) is generated only from committed,
cleaned sequence checkpoints; live 00 outputs are excluded. The checkpoint's
verification exit 0 does not replace the retained native audit exit 1.
[`checks/checkpoint04/`](checks/checkpoint04/) retains the required document
generators, Python/doc checks and preservation review. Staged whitespace checks
caught CRLF in the derived CSV and then its captured error log. CSV writers now
emit LF; exact failed logs and their nonzero exits are retained as gzip archives,
and the corrected staged check exits 0. Native results are unchanged.

## Completed 00 runs; frozen comparison and cleanup pending

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

Mesh-LOAM and VLOM's frozen replays are **still running**; the extraction's
effect on those two cells is not yet established. The parent frozen audit has
no terminal exit yet. No setting is changed or historical result replaced.
Summary `evaluator_git_commit` records the working-tree HEAD at write time;
native provenance comes from the frozen executable SHA256/source pins, not
that metadata field.

Completed candidate results, exact commands/logs, executable hashes and input
provenance were committed at `485fa00601644731827dae601eb3b43c7543efeb`:
[`sequences/00/audit/`](sequences/00/audit/),
[`inputs.json`](sequences/00/inputs.json),
[`fetch_attempts/`](sequences/00/fetch_attempts/) and
[`checks/00_primary/verification.json`](checks/00_primary/verification.json).
The official scans remain available for the running frozen comparisons. No 01
inputs are fetched before 00 comparisons and verified cleanup complete.
The completed, cleaned sequence checkpoint count therefore remains 14 cells
(04); these 20 completed candidate runs are reported separately.

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

Outputs go to `completion/01/`; the prepared `PR_BODY.md` and pushed-head CI need
the connected GitHub tool's final review/update. The ordinary GitHub CLI API
probe was refused; its actual exit is retained, and no alternate access path is
attempted by the worker. [Guard and renderer validation](checks/finalizer_preflight/)
uses committed partial 04/00 evidence: partial reports and the still-running
controller are rejected before public documentation changes. Rendering previews
retain the real 04 numeric differences and historical NCLT confound; they are
validation artifacts, not full-study completion.
