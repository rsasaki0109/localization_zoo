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
