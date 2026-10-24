# Official KISS-Matcher validation

This evaluates the official MIT-SPARK **KISS-Matcher 1.0.2** Python package,
including its `use_quatro` variant. The repository's existing C++
`lz::KISSMatcher` helper is a local, initial-guess-dependent ICP matcher; it is
not the MIT-SPARK global registration algorithm. No existing method or default
is changed by this external evaluation.

## Frozen protocol and provenance

[Protocol](../experiments/results/kiss_matcher_validation/protocol.json) was
committed in `19bba1aa` before registration. Raw results were committed in
`f9a242eb` before interpretation. Each method uses one configuration in every
case: voxel size 0.3 m, selected from the upstream Python quickstart, native
defaults otherwise. `use_quatro` is the only method toggle. Wheels, raw scan hashes, settings, worker commands, output transforms,
inliers, runtimes and exit codes are retained in the
[result directory](../experiments/results/kiss_matcher_validation/).

The synthetic full-rotation construction follows the upstream MIT-licensed
[quickstart](https://github.com/MIT-SPARK/KISS-Matcher/blob/main/python/examples/quickstart.py).
Real data are three KITTI 00 scan pairs already cached by a separate paused
experiment: source 3072 and targets 3074, 3082, 3112. Each is tested with
0°, 90° and 180° source-yaw augmentation. These are correlated cases from one
segment, not nine independent scenes. KITTI is development data; the pairs were
chosen by availability, before registration outcomes. Raw XYZ is passed without
range crop, elevation correction, GT initialization or an ICP refinement.

Registration workers receive only cloud paths, input hashes, augmentations and
settings. Reference poses are read by the parent after all workers exit. Scoring
compares the estimated augmented-source-to-target transform with
`inv(T_world_target) * T_world_source * inv(A_source)`.
Success is the predeclared engineering criterion: rotation error at most 2° and
translation error at most 0.5 m. These are not a paper benchmark's thresholds.
Failed and timed-out commands remain in the result denominator.

## Results and negative cases

All registration commands exit 0. On the initial real-data execution, KISS-Matcher passes
4/9 and its Quatro mode passes 6/9. The complete case table includes failures:

| Case | KISS-Matcher rotation / translation error | Official package Quatro mode |
|---|---|---|
| synthetic_full_rotation | 0.020° / 0.001 m (pass) | 168.522° / 8.366 m (fail) |
| 00_003072_003074_yaw0 | 0.077° / 0.020 m (pass) | 0.352° / 0.031 m (pass) |
| 00_003072_003074_yaw90 | 0.026° / 0.031 m (pass) | 0.351° / 0.034 m (pass) |
| 00_003072_003074_yaw180 | 0.038° / 0.031 m (pass) | 0.355° / 0.035 m (pass) |
| 00_003072_003082_yaw0 | 2.373° / 0.177 m (fail) | 0.645° / 0.120 m (pass) |
| 00_003072_003082_yaw90 | 0.891° / 0.168 m (pass) | 0.574° / 0.094 m (pass) |
| 00_003072_003082_yaw180 | 2.039° / 0.183 m (fail) | 0.524° / 0.150 m (pass) |
| 00_003072_003112_yaw0 | 32.062° / 21.790 m (fail) | 91.372° / 28.196 m (fail) |
| 00_003072_003112_yaw90 | 175.023° / 29.110 m (fail) | 10.119° / 24.020 m (fail) |
| 00_003072_003112_yaw180 | 174.387° / 29.163 m (fail) | 175.039° / 29.097 m (fail) |

Quatro constrains rotation to yaw. Its failure on the arbitrary full-rotation
synthetic case is outside that assumption, and remains reported. On the
3072→3112 real pair both variants fail at every tested yaw; translation errors
exceed 20 m. KISS-Matcher also misses the rotation threshold for 3072→3082 at
0° and 180°. No setting is changed after these observations.

This initial diagnostic verifies package execution and exposes failure cases.
It does not establish a full odometry result, loop-closure retrieval quality,
paper-number reproduction, held-out accuracy, or SOTA. The Quatro mode here is
not an evaluation of the separate Quatro++ ground-segmentation pipeline.
KISS-SLAM and broader revisit/low-overlap datasets remain unevaluated.

The [fixed-setting MCD transfer check](../experiments/results/kiss_matcher_mcd_transfer/README.md)
uses a separate existing fixture. Its initial body/sensor reference mismatch
gives 0/9 per mode; a documented HHS Ouster frame conversion applied only to
scoring the same estimates gives 9/9. This is a limited calibrated-reference
smoke check, not an independent held-out benchmark or estimator improvement.

The [Scan Context candidate follow-up](../experiments/results/imu_dr_readme_and_retrieval/README.md)
now tests four fixed historical retrievals with these unchanged configurations.
All eight registrations miss the existing accuracy criterion. The true revisit
3500→507 retains 0.727 m / 0.742 m translation error; the remaining retrievals
are false candidates. These negative results do not provide a loop-acceptance
rule or a full SLAM score.

## Same-protocol provenance rerun and variability

The initial audit failed because the wheel installs its native extension at the
site-packages top level, so the initial `backend_files` map was empty. The
original failed audit and all initial results remain committed. The worker now
hashes the extension actually defining `KISSMatcher`, and the complete protocol
was rerun without changing the algorithm, package, inputs or settings. Compiled
backend hashes are present in every rerun estimate, and
[the source/config/GT-separation and score audit](../experiments/results/kiss_matcher_validation/commands/audit_with_backend_provenance.json)
exits 0.

The rerun passes **6/9** real cases for both variants, versus **4/9** and **6/9**
initially. In the full solver, the 3072→3082 cases at 0° and 180° cross the
rotation threshold on the rerun. Neither native randomness nor other native
thread pools are controlled by this harness; no single best run is selected.
Both runs remain visible. This variability and the repeated hard-pair failures
limit conclusions from this small diagnostic.

| Provenance rerun case | Rotation error [deg] | Translation error [m] | Outcome |
|---|---:|---:|---|
| kiss_matcher / synthetic_full_rotation | 0.020 | 0.001 | pass |
| kiss_matcher / 00_003072_003074_yaw0 | 0.154 | 0.033 | pass |
| kiss_matcher / 00_003072_003074_yaw90 | 0.163 | 0.035 | pass |
| kiss_matcher / 00_003072_003074_yaw180 | 0.122 | 0.030 | pass |
| kiss_matcher / 00_003072_003082_yaw0 | 1.307 | 0.150 | pass |
| kiss_matcher / 00_003072_003082_yaw90 | 1.405 | 0.190 | pass |
| kiss_matcher / 00_003072_003082_yaw180 | 1.885 | 0.180 | pass |
| kiss_matcher / 00_003072_003112_yaw0 | 170.177 | 27.727 | fail |
| kiss_matcher / 00_003072_003112_yaw90 | 90.994 | 28.120 | fail |
| kiss_matcher / 00_003072_003112_yaw180 | 174.019 | 29.334 | fail |
| kiss_matcher_quatro / synthetic_full_rotation | 168.523 | 8.366 | fail |
| kiss_matcher_quatro / 00_003072_003074_yaw0 | 0.354 | 0.027 | pass |
| kiss_matcher_quatro / 00_003072_003074_yaw90 | 0.351 | 0.054 | pass |
| kiss_matcher_quatro / 00_003072_003074_yaw180 | 0.352 | 0.034 | pass |
| kiss_matcher_quatro / 00_003072_003082_yaw0 | 0.524 | 0.140 | pass |
| kiss_matcher_quatro / 00_003072_003082_yaw90 | 0.522 | 0.164 | pass |
| kiss_matcher_quatro / 00_003072_003082_yaw180 | 0.532 | 0.124 | pass |
| kiss_matcher_quatro / 00_003072_003112_yaw0 | 175.122 | 29.076 | fail |
| kiss_matcher_quatro / 00_003072_003112_yaw90 | 10.081 | 24.951 | fail |
| kiss_matcher_quatro / 00_003072_003112_yaw180 | 90.660 | 28.165 | fail |

## Reproduce

Use an isolated environment; the recorded run uses Python 3.12 and pins both
`kiss-matcher==1.0.2` and `numpy==2.2.6`. Wheel hashes are in the protocol.
Obtain the original scans with the repository's range extractor:

```bash
python3 evaluation/scripts/fetch_kitti_odometry_sequences.py \
  --output-root /tmp/kiss-matcher-data --sequence 00 \
  --frame 3072 --frame 3074 --frame 3082 --frame 3112
python3 evaluation/scripts/run_official_kiss_matcher.py \
  --manifest experiments/results/kiss_matcher_validation/protocol.json \
  --raw-root /tmp/kiss-matcher-data/dataset/sequences/00/velodyne \
  --reference-csv experiments/reference_data/kitti_seq_00_full_gt.csv \
  --output-dir /tmp/kiss-matcher-rerun
```

Use a new output directory; retained runs are never overwritten. Native parallel
sampling/solver behavior can vary across executions; the stored table describes
the committed run. The worker threads are capped with `OMP_NUM_THREADS=2` and
`OPENBLAS_NUM_THREADS=1`; other native thread pools are not claimed controlled.
See [execution receipt](../experiments/results/kiss_matcher_validation/commands/run_pairs.json)
and [per-case commands](../experiments/results/kiss_matcher_validation/run/commands.json).
