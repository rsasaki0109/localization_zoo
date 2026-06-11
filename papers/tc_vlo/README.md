# TC-VLO: Tightly Coupled Vision-LiDAR Odometry

**Paper:** Young-Woo Seo and Chih-Chung Chou, *A Tight Coupling of Vision-Lidar
Measurements for an Effective Odometry*, IEEE IV 2019, pp. 1118-1123,
https://dblp.dagstuhl.de/rec/conf/ivs/SeoC19.html. No author implementation was
used for this port.

## Core Idea

The paper couples visual and LiDAR measurements during odometry optimization
instead of running one modality only as a loose initializer. Visual landmarks
and LiDAR map constraints both contribute residuals to the pose solve.

This port keeps the testable mechanisms:

1. **Separate visual point support.** Range-image high-curvature cells provide
   sparse visual point proxies with LiDAR depth.
2. **LiDAR voxel map.** The shared backend maintains the scan-to-map plane
   structure that anchors the metric odometry.
3. **Joint residuals.** Point-to-plane, visual point, point-to-line, and line
   direction residuals are active together.
4. **Sparse-feature fallback.** If visual support is too thin, the solve keeps
   the LiDAR scan-to-map path valid.

## Scope / Deviations

- The full paper uses real camera features and a visual SLAM front-end. KITTI
  Odometry PCD exports here are Velodyne-only, so visual measurements are
  range-image proxies.
- Loop closure and full visual bundle adjustment are out of scope. The adapter
  focuses on the tightly coupled residual layout inside the existing C++ API.
- The dense profile uses `visual_point_weight=0.055`, `line_weight=0.10`,
  `direction_weight=0.025`, and `visual_line_bonus=1.3`.

## Parameters

| Param | Meaning | Dense profile |
|---|---|---:|
| `image_width` / `image_height` | spherical range/visual proxy image | 1024 / 80 |
| `voxel_size` | map voxel size | 0.8 m |
| `registration_voxel_size` | source point thinning | 0.7 m |
| `max_visual_points` | visual point cap | 420 |
| `max_visual_lines` | visual line cap | 160 |
| `visual_point_weight` | visual point residual weight | 0.055 |
| `line_weight` / `direction_weight` | line residual weights | 0.10 / 0.025 |

## Tests

`test_tc_vlo` covers profile selection, point/line feature extraction, and short
translation tracking with coupled residuals enabled.

## Reproduce

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
  data/kitti_pcd/seq00_gt.csv --methods tc_vlo --no-gt-seed \
  --tc-vlo-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_tc_vlo.json
```

## Result (KITTI Odometry, dense profile)

| Seq | RPE drift | ATE | FPS | visual points/frame | visual lines/frame | fallback |
|---|---:|---:|---:|---:|---:|---:|
| 00 _(4541 fr)_ | **1.060%** | 12.01 m | 7.6 | 94.8 | 56.1 | 0 |
| 07 _(1101 fr)_ | **0.925%** | 4.22 m | 10.9 | 98.8 | 56.3 | 0 |

The coupled residuals are stable and improve substantially over pseudo-image
visual-only front ends, but the useful contribution remains auxiliary to the
point-to-plane LiDAR core.
