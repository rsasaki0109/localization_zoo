# ID-LIO

## Paper

- Weizhuang Wu, Wanliang Wang
- "LiDAR Inertial Odometry Based on Indexed Point and Delayed Removal Strategy
  in Highly Dynamic Environments"
- Sensors 2023, 23(11):5188, DOI: `10.3390/s23115188`
- Public author code: none found in the web survey for this port.

## What This Repository Implements

This is a compact ID-LIO-style front-end for dynamic LiDAR odometry:

1. **Pseudo-occupancy dynamic detection**: the current scan and predicted local
   map are projected into a spherical range image. Points that appear as a new
   closer foreground surface are down-weighted as likely dynamic points.
2. **Indexed point map**: every stored map point carries a stable id,
   observation count, confidence, missing age, and dynamic age.
3. **Delayed removal**: disappearing map points are not deleted immediately.
   They are first kept with a low dynamic weight; only points that remain
   inconsistent for several frames are removed.
4. **Dynamic-weighted scan-to-map**: point-to-plane residuals are weighted by
   both current-source dynamic likelihood and indexed-map confidence.
5. **IMU rotation prior when available**: synchronized `imu.csv` samples are
   preintegrated as the rotation part of the prediction. KITTI odometry PCD
   exports used here have timestamps but no `imu.csv`, so the full KITTI numbers
   below use constant-velocity fallback.

## Deviations

- The paper builds on LIO-SAM with feature extraction, keyframes, GTSAM
  smoothing, loop closure, and full inertial factors. This port keeps the
  dynamic indexed-point / delayed-removal mechanisms inside this repository's
  scan-to-map odometry harness.
- Dynamic detection is a compact range-image pseudo-occupancy test rather than
  the complete paper pipeline.
- The benchmark below is KITTI full seq00/seq07 without IMU, so it evaluates the
  LiDAR dynamic-map mechanism, not the complete tightly-coupled LIO stack.

## Dogfooding

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq07_full data/kitti_pcd/seq07_gt.csv \
  --methods id_lio --no-gt-seed --id-lio-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq07_id_lio.json
```

## Results

KITTI Odometry full sequences, first-pose anchor, `--no-gt-seed`,
`--id-lio-dense-profile`.

| Sequence | Frames | ATE [m] | RPE trans [%/100m] | RPE rot [deg/m] | FPS | Artifact |
|---|---:|---:|---:|---:|---:|---|
| seq00 | 4541 | 15.446 | 1.111 | 0.014 | 7.82 | `docs/benchmarks/kitti_full_new_methods/seq00_id_lio.json` |
| seq07 | 1101 | 4.611 | 0.999 | 0.013 | 11.52 | `docs/benchmarks/kitti_full_new_methods/seq07_id_lio.json` |

On KITTI, the dynamic-map machinery is active (`dynamic/frame` about 430-460 in
the JSON notes) and does not diverge, but without the paper's real dynamic
urban scenes and IMU/GTSAM back-end it is not competitive with the top
KISS-like LiDAR-only front ends. This is an honest port of the front-end
mechanism, not a claim of reproducing the original paper's dynamic-dataset
numbers.

## Synthetic Dynamic-Object Stress

Dataset-free mechanism stress:

```bash
python3 evaluation/scripts/run_dynamic_object_stress.py
```

The runner generates clean and crossing-object fixtures under
`evaluation/fixtures/dynamic_object_stress*` and writes
`docs/benchmarks/dynamic_object_stress/rf_id_lio_dynamic_object_stress_summary.json`.
On the 30-frame stress, ID-LIO goes from **0.676 m** clean ATE to
**130.549 m** with moving foreground boxes (`dynamic/frame=78.6`). This confirms
the pseudo-occupancy path is strongly active, but also records a hard failure
boundary for this compact LiDAR-only port.

Public urban KITTI seq05 validation (paired with RF-LIO):
[`rf_id_lio_kitti_seq05_validation_summary.json`](../../docs/benchmarks/kitti_seq05_public/rf_id_lio_kitti_seq05_validation_summary.json).
On seq05 full, ID-LIO reaches **0.702%** RPE vs RF-LIO default **1.005%** and
KISS-ICP **0.617%**; both dynamic paths remain active but this is still a
public KITTI proxy, not a dedicated high-dynamic benchmark.

## Public synchronized LiDAR-IMU validation (HDL-400 open)

120-frame public HDL-400 open window with `imu.csv`:

```bash
python3 evaluation/scripts/run_lio_imu_public_validation.py --dataset hdl_400
```

IMU gyro prior activates when `imu.csv` is present (1.43% vs 1.35% RPE without).
Paired summary: [`hdl_400_lio_imu_validation_summary.json`](../../docs/benchmarks/lio_imu_public/hdl_400_lio_imu_validation_summary.json).
