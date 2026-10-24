# IMU DR visibility and Scan Context → KISS-Matcher verification

## IMU-only dead reckoning in the main README

The main README now summarizes pure IMU-only dead reckoning and the same
NHC+ZUPT flags on NCLT and the archived KITTI Raw fixture. Values come directly
from committed aggregate rows, retained with commands and hashes in
[imu_dr_sources.json](imu_dr_sources.json). This is not a new sensor-data run.
The KITTI index-time/static-init/ZUPT caveats remain explicit. Orientation filters
and Allan variance are linked separately.

## Historical retrieval rescore

[Execution](rescore_command.json) exits 0. Re-evaluating stored Scan Context,
ISC and DTD per-query outputs on KITTI 00/02/05/07/08 reproduces the committed
F1max, recall at 100% precision and unthresholded top-1 recall exactly. This
reruns the scorer, not native descriptor extraction. The
[source/metric audit](audit.json) and [its command](audit_command.json) exit 0.

## Fixed sampled candidate verification

[Protocol](matcher_protocol.json) was committed in `2d0c2eb9` before matcher
execution. Queries 500, 1500, 2500, 3500 were selected by a fixed stride in
KITTI 00, using each historical Scan Context top-1 candidate. Neither GT labels
nor registration outcomes selected them. No retrieval score threshold was
applied to this diagnostic sample. The official package, resolution and each
variant configuration are unchanged from the earlier KISS-Matcher study.
Workers never receive GT poses or an initializer.

All eight commands exit 0; **0/8** meet the existing 2° / 0.5 m registration
criterion. Exit status therefore does not imply accurate registration. Three
retrievals are outside the 4 m revisit radius, while query 3500 retrieves a
true revisit. These four correlated development queries do not estimate a
full-sequence acceptance rate or a held-out score.

| Query → candidate | GT separation [m] | Within 4 m | KISS-Matcher rot / trans error | Quatro mode rot / trans error |
|---|---:|---|---|---|
| 500 → 153 | 152.551 | no | 155.050° / 153.585 m | 2.718° / 151.983 m |
| 1500 → 1439 | 73.337 | no | 179.638° / 74.496 m | 179.328° / 73.618 m |
| 2500 → 1045 | 250.576 | no | 173.639° / 252.144 m | 163.923° / 252.778 m |
| 3500 → 507 | 0.666 | yes | 0.332° / 0.727 m | 0.178° / 0.742 m |

The true revisit 3500→507 has translation errors 0.727 m / 0.742 m, despite
rotation errors 0.332° / 0.178°. The other pairs have large errors. No threshold
or estimator setting is adjusted to hide these negatives. A GT error threshold
is an offline score, not a deployable loop-acceptance rule. Confidence gating,
repeatability on more candidates and an explicitly opt-in refinement stage
remain future work; this experiment adds neither an acceptance gate nor SLAM.

## Fixed opt-in ICP refinement: negative result

The follow-up freezes [a separate refinement protocol](refinement_protocol.json)
before execution and refines the archived coarse transforms, without rerunning
or selecting them. `small-gicp==1.0.1` uses point-to-point ICP with the documented
numeric defaults: 0.25 m downsampling, 1 m correspondence distance, 20 iterations,
one thread. This standalone diagnostic is opt-in; it changes no Zoo default or
official KISS-Matcher behavior. The [worker](refine_pairs.py) receives no GT;
the [scorer command](refinement_score_command.json) runs afterwards.

The [saved result](refinement_summary.json) still passes **0/8** cases under the
unchanged 2° / 0.5 m criterion. On true revisit 3500→507, both initializers converge
to the same ICP solution: 0.229° / 0.730 m, versus coarse 0.332° / 0.727 m and
0.178° / 0.742 m. Thus convergence does not establish accurate registration.
The false candidates remain incorrect, with translation errors 73.778–252.626 m;
the solver reports non-convergence there. No settings or thresholds are retuned.
An independent known-transform control has maximum matrix error 0.000325,
below its predeclared 0.001 check; worker and scoring receipts exit 0.

To repeat explicitly after fetching the scans below:

```bash
python3 experiments/results/imu_dr_readme_and_retrieval/refine_pairs.py \
  --protocol experiments/results/imu_dr_readme_and_retrieval/refinement_protocol.json \
  --raw-root /tmp/scan-context-pairs/dataset/sequences/00/velodyne \
  --output /tmp/scan-context-refined.json
```

This uses the committed coarse estimates and never overwrites them. GT/frame
consistency and residual/overlap diagnostics remain open; this failed experiment
does not justify adopting ICP or calibrating a loop gate on these four queries.

## Oracle reference/objective diagnostic (excluded from scores)

The [preregistered diagnostic](oracle_diagnostic_protocol.json) intentionally
uses GT initialization only to inspect reference frames and the local objective;
it changes no benchmark estimate or success count. The four original pairs,
ICP settings, point sampling and 1 m residual gate are fixed. Both residual
directions report support fractions and clipped mean squared distance, so a
low supported-point RMSE cannot hide missing overlap.

The first [oracle command](oracle_command.json) exits 1 at the raw-matrix frame
check. [All-case difference measurement](frame_difference_diagnostic.json)
records two failed pairs with translation differences 0.000258 m / 0.000255 m
between raw official transforms and the CSV representation; world positions
agree within 7e-11 m, while the published rotation matrices are slightly
nonorthogonal. This does not account for the large registration errors. The
original 1e-4 matrix tolerance is unchanged. The
[accounted rerun](oracle_accounted_command.json) still exits 1 and explicitly
skips those two oracle fits, while retaining the unaffected pair diagnostics.
No failed frame check is marked passed.

For the true revisit **3500→507**, the CSV relative transform agrees with the
independent official pose/calibration transform to 5.85e-8 per matrix element
(translation difference 1.48e-8 m). Even initialized at that reference, ICP moves
to **0.227° / 0.729 m** error. Source→target clipped MSE drops from
**0.215444 to 0.049403 m²**, and reverse MSE from **0.217413 to 0.045678 m²**.
Source support is 98.31% at the reference and 98.87% after oracle ICP. The
archived GT-free refined estimates reach almost the same residual objective.

This witnesses disagreement between the measured-cloud point-to-point objective
and the evaluation reference on this pair. It does not establish that GT is
wrong or that the estimated transform is correct. Further investigation should
check point/motion modeling and reference uncertainty; no estimator setting or
acceptance threshold is tuned from this oracle test.

The false retrieval 500→153 has **zero support** within 1 m at the GT transform.
The oracle returns its initializer there, so its zero pose error would be
meaningless as registration performance. The archived wrong alignments instead
have 34.92% / 55.98% source support. Neither a returned pose nor an oracle's
GT error is a valid deployment gate.

[Full residual data](oracle_accounted_diagnostic.json), failed commands,
[frame differences](frame_difference_diagnostic.json) and
[residual/control integrity checks](oracle_integrity_command.json) are retained.
The latter exit 0 and verify exact-match/no-overlap analytic cases, skipped-case
accounting and exclusion from benchmark counts. The normal result remains 0/8.

The cached official archives are pinned by SHA-256 in the protocol, originally
from `https://s3.eu-central-1.amazonaws.com/avg-kitti/data_odometry_poses.zip`
and `data_odometry_calib.zip`. The diagnostic CLI explicitly requires these
archives and the reference CSV; it is separate from the GT-free matcher worker.

## Reproduce

Use the pinned `kiss-matcher==1.0.2`, `numpy==2.2.6` environment described in
[the preceding study](../../../docs/kiss_matcher_validation.md). Fetch only the
selected sequence and frames; remove the scans after preserving the results:

```bash
python3 evaluation/scripts/fetch_kitti_odometry_sequences.py \
  --output-root /tmp/scan-context-pairs --sequence 00 \
  --frame 500 --frame 153 --frame 1500 --frame 1439 \
  --frame 2500 --frame 1045 --frame 3500 --frame 507
python3 evaluation/scripts/run_official_kiss_matcher.py \
  --manifest experiments/results/imu_dr_readme_and_retrieval/matcher_protocol.json \
  --raw-root /tmp/scan-context-pairs/dataset/sequences/00/velodyne \
  --reference-csv experiments/reference_data/kitti_seq_00_full_gt.csv \
  --output-dir /tmp/scan-context-matcher-rerun
```

[Actual fetch command](fetch.json), [matcher command](matcher_run.json),
and [per-case outputs](matcher_run/) retain every outcome. No earlier stored
result or previously paused InTEn process was changed.
