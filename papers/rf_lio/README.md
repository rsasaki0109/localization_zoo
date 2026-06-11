# RF-LIO: Removal-First LiDAR-Inertial Odometry

**Paper:** Chenglong Qian, Zhaohong Xiang, Zhuoran Wu, and Hongbin Sun,
*RF-LIO: Removal-First Tightly-coupled Lidar Inertial Odometry in High Dynamic
Environments*, IROS 2021 / arXiv:2206.09463, https://arxiv.org/abs/2206.09463.
No reusable author implementation was used for this port.

## Core Idea

RF-LIO targets highly dynamic scenes. Instead of immediately scan-matching a new
LiDAR sweep against a submap, it first removes likely moving foreground objects
with adaptive multi-resolution range images, then performs tightly coupled LIO
on the cleaned scan.

This port keeps the testable mechanisms inside the existing KITTI PCD harness:

1. **Removal-first range images.** The previous static scan is projected into
   the predicted current sensor frame. Current points that are significantly
   closer than the predicted surface in fine, mid, and coarse range images are
   treated as foreground dynamic candidates before registration.
2. **Adaptive multi-resolution consensus.** A point must be foreground in at
   least two range-image resolutions before removal. A removal cap prevents
   catastrophic over-filtering when ego-motion prediction is rough.
3. **Indexed dynamic backend.** The cleaned scan is passed to the existing
   ID-LIO-style indexed map backend, preserving delayed removal, dynamic
   residual weights, and IMU gyro prior support when `imu.csv` exists.
4. **Constant-velocity fallback.** KITTI Odometry PCD exports here do not
   include IMU, so the full run uses the backend's velocity-model prediction.

## Scope / Deviations

- The full paper builds on LIO-SAM with tightly coupled IMU factors and
  submap optimization. This port is pure odometry in the repo's C++ harness.
- The adaptive range-image removal is implemented as a compact previous-scan
  foreground test, not the complete paper system.
- KITTI Odometry is not a high-dynamic benchmark; removal-first can remove
  static foreground structure when prediction is imperfect. The results below
  are therefore an honest KITTI negative for this mechanism.

## Parameters

| Param | Meaning | Dense profile |
|---|---|---:|
| `range_image_width` / `range_image_height` | fine range image | 1024 / 80 |
| `foreground_margin` | current point must be this much closer than predicted | 1.0 m |
| `min_foreground_votes` | required fine/mid/coarse foreground votes | 2 |
| `max_removal_fraction` | per-frame removal cap | 0.25 |
| `min_keep_points` | guard against over-filtering | 2200 |
| `voxel_size` | backend map voxel size | 0.8 m |
| `delayed_removal_frames` | backend dynamic map removal delay | 1 |

## Tests

`test_rf_lio` covers first-frame map creation, removal-first foreground
rejection, short translation tracking after filtering, and IMU rotation-prior
pass-through to the backend.

## Reproduce

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
  data/kitti_pcd/seq00_gt.csv --methods rf_lio --no-gt-seed \
  --rf-lio-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_rf_lio.json
```

## Result (KITTI Odometry, dense profile)

| Seq | RPE drift | ATE | FPS | removal-first/frame | backend dynamic/frame |
|---|---:|---:|---:|---:|---:|
| 00 _(4541 fr)_ | **1.351%** | 22.52 m | 6.6 | 273.0 | 430.6 |
| 07 _(1101 fr)_ | **1.272%** | 4.81 m | 11.1 | 246.2 | 404.9 |

RF-LIO is stable, but on KITTI its removal-first path is below ID-LIO
(1.111% / 0.999%) and the strongest LiDAR-only baselines. The mechanism is
active, but KITTI's mostly static geometry makes aggressive foreground removal a
trade-off rather than a win.
