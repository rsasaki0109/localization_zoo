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
