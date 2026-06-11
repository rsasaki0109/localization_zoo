# TC-MVLO: Tightly Coupled Monocular Visual-LiDAR Odometry

**Paper:** Lingbo Meng, Chao Ye, and Weiyang Lin, *A Tightly Coupled Monocular
Visual Lidar Odometry with Loop Closure*, Intelligent Service Robotics 2022,
https://link.springer.com/article/10.1007/s11370-021-00406-2. No author
implementation was used for this port.

## Core Idea

The paper tightly couples monocular visual measurements with LiDAR constraints
and adds loop closure for long-term consistency. The odometry part uses both
visual landmarks and LiDAR geometric residuals in the state estimate.

This port keeps the odometry mechanisms:

1. **Monocular visual proxies.** High-curvature range-image cells stand in for
   visual point landmarks, with metric LiDAR depth attached.
2. **Line support.** Pseudo-visual line segments and LiDAR line structure are
   fused to add point-to-line and direction residuals.
3. **Helmert-style residual balancing.** Point and line scales are adapted from
   residual variance with tighter clamps than the generic backend.
4. **LiDAR scan-to-map anchor.** The shared point-to-plane map remains the
   primary metric constraint.

## Scope / Deviations

- The full method uses camera features and loop closure. KITTI Odometry PCD
  exports in this repo are Velodyne-only, so visual features are range-image
  proxies and loop closure is out of scope.
- This adapter intentionally emphasizes coupled point+line residuals more than
  TC-VLO (`visual_line_bonus=1.8`) while keeping Helmert scales clamped to
  `0.35..3.0`.
- The dense profile uses `visual_point_weight=0.065`, `line_weight=0.12`, and
  `direction_weight=0.035`.

## Parameters

| Param | Meaning | Dense profile |
|---|---|---:|
| `image_width` / `image_height` | spherical range/visual proxy image | 1024 / 80 |
| `voxel_size` | map voxel size | 0.8 m |
| `registration_voxel_size` | source point thinning | 0.7 m |
| `visual_point_weight` | visual point residual weight | 0.065 |
| `line_weight` / `direction_weight` | line residual weights | 0.12 / 0.035 |
| `visual_line_bonus` | pseudo-visual line emphasis | 1.8 |
| `helmert_min_scale` / `helmert_max_scale` | adaptive weight clamp | 0.35 / 3.0 |

## Tests

`test_tc_mvlo` covers profile selection, point/line feature extraction, and
short translation tracking with the monocular visual-LiDAR profile.

## Reproduce

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
  data/kitti_pcd/seq00_gt.csv --methods tc_mvlo --no-gt-seed \
  --tc-mvlo-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_tc_mvlo.json
```

## Result (KITTI Odometry, dense profile)

| Seq | RPE drift | ATE | FPS | visual points/frame | visual lines/frame | fallback |
|---|---:|---:|---:|---:|---:|---:|
| 00 _(4541 fr)_ | **1.054%** | 10.82 m | 8.0 | 94.8 | 56.1 | 0 |
| 07 _(1101 fr)_ | **0.939%** | 3.00 m | 10.8 | 98.8 | 56.3 | 0 |

TC-MVLO has the best seq00 and seq07 ATE among the four new adapters. Its drift
is close to OPL-LVIO/TC-LVGF, reinforcing the same finding: pseudo-visual
features help stabilize the backend, but the point-to-plane LiDAR map still does
most of the work on KITTI PCD.
