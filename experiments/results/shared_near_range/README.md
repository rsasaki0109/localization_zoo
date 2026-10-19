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

The [KITTI03 Mesh diagnosis](diagnostics/03/README.md) retains the paired
historical gap and current error growth. The
[KITTI04 CT/Mesh diagnosis](diagnostics/04/README.md) retains both numerical
pairs, the available Mesh pair and CT's historical-path availability limits.
Their runtime limits and unresolved causes remain explicit.

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
[earlier cleaned checkpoint](checkpoints/07/summary.json) excludes live runs;
referenced historical paths are unavailable in the inspected tree/history.

The [KITTI08 saved-path diagnosis](diagnostics/08/README.md) adds all four
historical differences, current error-growth traces and the available Mesh paired
path, with explicit missing-path/runtime limits. Its
[earlier cleaned checkpoint](checkpoints/08/summary.json) includes verified08
cleanup; later sequences are excluded from that checkpoint.

The [KITTI09 saved-path diagnosis](diagnostics/09/README.md) adds the Mesh
historical difference, current error-growth block and available paired path.
Its [earlier cleaned checkpoint](checkpoints/09/summary.json) includes verified09
cleanup; later sequences are excluded from that checkpoint. The historical cause remains unresolved.

The [runtime restoration and backup record](runtime295_restore/README.md)
retains the environment loss, exact executable restoration, fresh restart and
normal-push backup limits. Closed evidence excludes unavailable interrupted work.

The [KITTI10 CT/Mesh diagnosis](diagnostics/10/README.md) retains both historical
differences, current error-growth blocks and the available Mesh paired path.
The [final cleaned checkpoint](checkpoints/10/summary.json) includes every
selected sequence and verified cleanup. Missing-path/runtime limits remain explicit.

## Complete KITTI replay

All 177 frozen selected cells across full KITTI 00-10 have completed, including 37 frozen-executable controls. All owned scans/aliases were verified and removed one sequence at a time. 142 cells match historical RPE/ATE/frame values; 35 differ. Every differing cell matches the pre-extraction executable's recorded errors/frame count, official translation/rotation RTE and saved-pose bytes. Historical causes remain unresolved. These are historical per-cell settings, not a new fixed-setting accuracy study.

| Method | Replayed cells | Historical matches | Historical differences | RPE worse | ATE worse |
|---|---:|---:|---:|---:|---:|
| A-LOAM | 11 | 6 | 5 | 3 | 3 |
| CT-ICP | 11 | 4 | 7 | 5 | 2 |
| DiLO | 11 | 11 | 0 | 0 | 0 |
| ELO | 11 | 11 | 0 | 0 | 0 |
| F-LOAM | 5 | 0 | 5 | 3 | 3 |
| IMLS-SLAM | 11 | 11 | 0 | 0 | 0 |
| InTEn-LOAM | 11 | 11 | 0 | 0 | 0 |
| KISS-ICP | 11 | 11 | 0 | 0 | 0 |
| L-LO | 10 | 10 | 0 | 0 | 0 |
| LF-GICP | 11 | 11 | 0 | 0 | 0 |
| LeGO-LOAM | 5 | 0 | 5 | 1 | 2 |
| LiTAMIN2 | 11 | 11 | 0 | 0 | 0 |
| MULLS | 11 | 11 | 0 | 0 | 0 |
| Mesh-LOAM | 11 | 0 | 11 | 5 | 6 |
| NDT | 2 | 2 | 0 | 0 | 0 |
| Quadric-LO | 11 | 11 | 0 | 0 | 0 |
| Small-GICP | 5 | 5 | 0 | 0 | 0 |
| SuMa | 11 | 11 | 0 | 0 | 0 |
| VLOM | 2 | 0 | 2 | 1 | 1 |
| Voxel-GICP | 5 | 5 | 0 | 0 | 0 |

[Full committed report](completion/03/report/summary.json) and [all numeric pairs](completion/03/report/comparison.csv) retain every outcome. Each historical audit's actual nonzero exit remains in its sequence folder; successful completion of the orchestration/report is not a claim that historical audits all pass. Every affected method README retains its numeric differences. No published value is replaced.

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

Fresh real-NCLT validation remains open. The launch-time enforced policy lacked the official host despite prior approval; the policy observations are retained in `checks/`. This KITTI evidence does not establish current NCLT access or accuracy.

## Commands and validation

`protocol.json`, `selection.json`, binary pins and `sequences/` retain the frozen source, arguments, actual process exits, logs/poses, official input hashes, fetch failures and cleanup. Defaults stay off. Concurrent processes do not support a speed claim.

Finalization commands, report generation, document changes and required checks are retained under `completion/03/`. Initial Release/125 CTest and 290 Python checks are in `checks/preflight/` and `checks/python/`; final actual check exits are recorded separately. Raw native log padding is preserved through narrowly scoped Git attributes.

## Integration with current main

The original frozen-source report was committed at `28406fe8`, imported as
`b07958a4`; original completion receipts from `ee225d09` are imported at
`57e19026`. Their [identity and terminal receipts](checks/final_integration/native_completion/)
retain the controller exit, cleanup order and byte-identical completion evidence.
The original completion PR body is retained verbatim as execution evidence; its
PR137-ready wording is historical and is not current PR metadata.

This audit measures native source `0b079dbb` with the original per-cell settings.
Current-main acceleration, passive tracing and bindings have separate compatibility
evidence; their presence in this branch does not turn the frozen replay into a
whole-zoo measurement of current-main native code. The later InTEn cause study and
fresh NCLT work remain open. No estimator, registration argument, published result
or paper number is changed by this completion.
