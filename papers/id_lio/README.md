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
