# OPL-LVIO: Optimized Point-Line LiDAR-Visual-Inertial Odometry

**Paper:** Xuan He et al., *LiDAR-Visual-Inertial Odometry Based on Optimized
Visual Point-Line Features*, Remote Sensing 2022, 14(3):622,
https://doi.org/10.3390/rs14030622. No author implementation was found in the
web/GitHub survey for this port.

## Core Idea

The paper builds an LVIO system around optimized visual point-line features:
improved line extraction/matching, multi-frame LiDAR depth association for visual
features, VIO-assisted LiDAR scan matching, and a Bayesian/factor-graph fusion
back end with variance-component weighting.

This port keeps the testable odometry mechanisms inside the existing KITTI PCD
harness:

1. **Range-image visual feature proxy.** KITTI Odometry PCD exports do not
   include RGB frames here, so high-curvature spherical range-image cells stand
   in for visual point features and smooth range segments stand in for visual
   line detections.
2. **LiDAR depth correlation.** Each visual proxy already carries metric LiDAR
   depth, matching the paper's LiDAR-to-visual depth association role.
3. **Point-line feature maps.** Visual point landmarks and line landmarks are
   maintained separately from the scan-to-plane map.
4. **Tightly coupled registration.** Scan-to-map point-to-plane residuals are
   augmented with visual point residuals; line residuals are available as an
   opt-in weight.
5. **Helmert-style weighting.** Point and line residual scales are adapted from
   per-iteration residual variance and clamped to avoid over-trusting the
   pseudo-visual channel.

## Scope / Deviations

- The full paper uses camera features, IMU/VIO initialization, GNSS/loop factors,
  and a graph optimizer. This port is an odometry front-end for the repo's
  Velodyne-only KITTI PCD harness.
- The dense profile uses `visual_point_weight=0.08` and disables line/direction
  residuals (`line_weight=0`, `direction_weight=0`). A seq07 108-frame ablation
  showed pseudo-line residuals slightly hurt RPE, while visual point residuals
  slightly improved the plane-only core.
- IMU is represented only by the constant-velocity prediction already used by the
  harness when `imu.csv` is absent.

## Parameters

| Param | Meaning | Dense profile |
|---|---|---:|
| `image_width` / `image_height` | spherical range/visual proxy image | 1024 / 80 |
| `voxel_size` | map voxel size | 0.8 m |
| `registration_voxel_size` | source point thinning | 0.7 m |
| `max_visual_points` | high-curvature visual point cap | 420 |
| `max_lidar_lines` / `max_visual_lines` | extracted line caps | 220 / 160 |
| `visual_point_weight` | visual point residual weight | 0.08 |
| `line_weight` | visual/LiDAR line residual weight | 0.0 |
| `helmert_min_scale` / `helmert_max_scale` | adaptive weight clamp | 0.25 / 4.0 |

## Tests

`test_opl_lvio` covers spherical projection, point/line feature extraction,
short translation tracking with point correspondences, and fallback when visual
support is sparse.

## Reproduce

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
  data/kitti_pcd/seq00_gt.csv --methods opl_lvio --no-gt-seed \
  --opl-lvio-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_opl_lvio.json
```

## Result (KITTI Odometry, dense profile)

| Seq | RPE drift | ATE | FPS | visual points/frame | visual lines/frame | fallback |
|---|---:|---:|---:|---:|---:|---:|
| 00 _(4541 fr)_ | **1.050%** | 15.24 m | 7.7 | 94.8 | 56.1 | 0 |
| 07 _(1101 fr)_ | **0.902%** | 3.65 m | 11.2 | 98.8 | 56.3 | 0 |

This is a stable pseudo-visual result: it slightly improves TC-LVGF's RPE on
seq00/07, but TC-LVGF keeps better seq00 ATE. The useful signal is the weak
range-image visual point residual; pseudo-line residuals remain secondary to the
point-to-plane core on KITTI PCD.
