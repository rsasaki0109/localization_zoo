# TC-LVGF: Tightly Coupled LiDAR-Visual Geometric Features

**Paper:** Ke Cao et al., *Tightly-Coupled LiDAR-Visual SLAM Based on Geometric
Features for Mobile Agents*, ROBIO 2023 / arXiv:2307.07763. No author code was
found; the project/publication pages point to the paper PDF, not a repository.

## Core Idea

The paper runs LiDAR and monocular visual subsystems in parallel, exchanges
geometric features in a spherical fusion frame, and uses visual line structure to
improve LiDAR linear features while using LiDAR depth/direction to reinforce
visual line landmarks.

This port keeps the testable odometry mechanisms:

1. **Spherical fusion frame.** Each scan is projected to a LiDAR range image.
2. **LiDAR linear features.** Local range-image neighborhoods are classified by
   PCA line linearity.
3. **Pseudo-visual line features.** Smooth sparse range-image segments play the
   role of visual line detections on KITTI Odometry, where RGB is absent.
4. **Tightly coupled registration.** Scan-to-map point-to-plane residuals are
   augmented with fused point-to-line and line-direction residuals.
5. **Degradation fallback.** If visual-like line support is too sparse, the
   pipeline falls back to the LiDAR line subsystem.

## Scope / Deviations

- The full paper uses RGB ORB/LSD features, semantic/depth association, two SLAM
  subsystems, and a visual back end. KITTI Odometry PCD exports in this repo have
  only Velodyne scans, so visual lines are generated from LiDAR range-image
  structure.
- The global loop-closure and semantic modules are out of scope; this is pure
  odometry for the existing KITTI full benchmark.
- A short seq07 ablation showed strong line residuals were slightly harmful on
  the pseudo-visual path, so the dense dogfooding profile keeps line and direction
  residuals light (`line_weight=0.2`, `direction_weight=0.05`).

## Parameters

| Param | Meaning | Dense profile |
|---|---|---:|
| `image_width` / `image_height` | spherical fusion image | 1024 / 80 |
| `voxel_size` | map voxel size | 0.8 m |
| `registration_voxel_size` | source point thinning | 0.7 m |
| `max_lidar_lines` / `max_visual_lines` | feature caps | 220 / 160 |
| `min_visual_lines` | fallback threshold | 8 |
| `line_weight` | point-to-line residual weight | 0.2 |
| `direction_weight` | line-direction residual weight | 0.05 |

## Tests

`test_tc_lvgf` covers spherical projection, LiDAR/visual line extraction and
fusion, short translation tracking, and fallback when visual lines are sparse.

## Reproduce

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
  data/kitti_pcd/seq00_gt.csv --methods tc_lvgf --no-gt-seed \
  --tc-lvgf-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_tc_lvgf.json
```

## Result (KITTI Odometry, dense profile)

| Seq | RPE drift | ATE | FPS | visual lines/frame | fallback |
|---|---:|---:|---:|---:|---:|
| 00 _(4541 fr)_ | **1.055%** | 11.95 m | 8.4 | 56.1 | 0 |
| 07 _(1101 fr)_ | **0.941%** | 3.74 m | 11.2 | 56.3 | 0 |

This is a mid-pack positive result for the pseudo-visual path: it is much more
stable than earlier pseudo-image PL-LOAM/VLOM ports, but the visual line residuals
do not beat the strongest KISS-like point-to-plane front ends on KITTI.
