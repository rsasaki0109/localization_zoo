# Fixed-setting MCD transfer and reference-frame correction

## Scope and frozen selection

Different-dataset transfer smoke check using the repository's existing MCD KTH
fixture: all three unordered pairs of its three adjacent scans, each at source
yaw 0°/90°/180°. Both official KISS-Matcher modes retain their previous 0.3 m
configuration. No settings, thresholds, pair selection or native transforms
are fitted after observing outcomes. [Protocol](protocol.json) precedes the run.
No GT initializer, deskew or sensor-data download is used.

This CI fixture has already been used in repository development. It is not an
independent held-out accuracy dataset, a loop-retrieval benchmark, full odometry
or a paper/SOTA reproduction. Reference sampling differs from cloud timestamps
by about 10 ms; the original export/topic/extrinsic lineage is not fully
preserved. Those limits remain after the frame correction below.

## Initial mismatch and source-backed correction

The original [run summary](run/summary.json) remains untouched: **0/9** cases
pass for each mode. Translation errors are small, but rotation errors are
about 3–7°. The dataset publisher explains that `pose_inW.csv` describes the
**body** frame in the world; each sensor has its own extrinsic. The repository's
legacy `mcd_gt_to_csv.py` copies body poses into columns named `lidar_pose.*`
without an extrinsic. That is not sensor-frame GT for raw Ouster point clouds.

[Source evidence](reference_sources.json) links the publisher's ground-truth
and frame definitions and the SLICT HHS Ouster configuration recommended by its
SLAM tutorial. The pinned [calibration](hhs_ouster_sensor_to_body.json) is taken
from that configuration, not estimated from these pairs. Its bottom-right zero
is a scan-end timestamp marker; the rigid transform uses homogeneous entry one.
The loader rejects a zero homogeneous entry instead of silently accepting it.

An explicit opt-in applies **T_world_sensor = T_world_body * T_body_sensor**.
The [calibrated reference protocol](calibrated_reference_protocol.json) precedes
rescoring the exact saved native estimates. No registration is rerun or selected.
The [rescore](calibrated_score_command.json) exits 0, preserves estimate hashes
and uses the same 2° / 0.5 m criterion. Both modes now meet it in **9/9** cases:

| Source/target and augmentation | KISS-Matcher rotation / translation error | Quatro mode rotation / translation error |
|---|---|---|
| mcd_0_1_yaw0 | 0.472° / 0.028 m | 0.410° / 0.022 m |
| mcd_0_1_yaw90 | 0.426° / 0.027 m | 0.419° / 0.023 m |
| mcd_0_1_yaw180 | 0.444° / 0.025 m | 0.420° / 0.021 m |
| mcd_0_2_yaw0 | 0.559° / 0.047 m | 0.802° / 0.064 m |
| mcd_0_2_yaw90 | 0.420° / 0.038 m | 0.794° / 0.067 m |
| mcd_0_2_yaw180 | 0.315° / 0.046 m | 0.807° / 0.066 m |
| mcd_1_2_yaw0 | 0.253° / 0.053 m | 0.414° / 0.065 m |
| mcd_1_2_yaw90 | 0.245° / 0.045 m | 0.427° / 0.066 m |
| mcd_1_2_yaw180 | 0.240° / 0.048 m | 0.430° / 0.066 m |

Maximum errors are **0.559° / 0.053 m** for KISS-Matcher and
**0.807° / 0.067 m** for Quatro mode. This is a source-backed reference-frame
rescore, not an estimator improvement. The documented HHS Ouster mapping is
consistent with the KTH fixture, but the fixture export lineage and remaining
timing uncertainty still need verification before broad accuracy claims.
Both the initial negative result and corrected scores are retained.

## Opt-in tools and reproduction

The matcher runner accepts `--reference-sensor-to-body-json` only when the
calibration hash is pinned in its manifest. It applies that transform in the
parent scorer; workers never receive reference/calibration paths. Existing
KITTI/default behavior and saved historical results are unchanged.

```bash
python3 evaluation/scripts/run_official_kiss_matcher.py \
  --manifest experiments/results/kiss_matcher_mcd_transfer/calibrated_run_protocol.json \
  --raw-root evaluation/fixtures/mcd_kth_smoke \
  --reference-csv evaluation/fixtures/mcd_kth_smoke_gt.csv \
  --reference-sensor-to-body-json experiments/results/kiss_matcher_mcd_transfer/hhs_ouster_sensor_to_body.json \
  --output-dir /tmp/mcd-matcher-rerun
```

This starts a new native run, so native randomness may affect detailed errors.
The reported corrected table uses the frozen initial estimates, not this rerun.
Exact [initial execution](run_command.json) and [rescore command](calibrated_score_command.json)
are retained. All committed scans, references and original results are kept.

For new MCD exports, the converter adds `--sensor-to-body-json` as an explicit
option. It preserves legacy default bytes. Do not apply an extrinsic twice if
the cloud/reference has already been transformed to the body/sensor frame.
Tests cover right multiplication/conjugation, an analytic noncommuting case,
legacy output bytes, calibrated CSV output, strict homogeneous validity, PCD
field order, and isolation of reference calibration from native workers.
