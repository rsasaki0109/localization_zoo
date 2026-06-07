# PL-LOAM: LiDAR-Monocular Visual Odometry (from-paper reimplementation)

**Paper:** Shi-Sheng Huang, Ze-Yu Ma, Tai-Jiang Mu, Hongbo Fu, Shi-Min Hu,
*"Lidar-Monocular Visual Odometry using Point and Line Features"*, ICRA 2020.
No author code released (verified: no PL-LOAM repo on authors' GitHub). Reconstructed
from the paper.

## Core idea

PL-LOAM fuses **monocular point + line features** with **LiDAR depth priors** in a
point-line bundle adjustment (PL-BA):

1. **Depth extraction from LiDAR.** For each 2D point or line endpoint, a robust
   patch median depth is read from the LiDAR point cloud projected into the camera.
2. **Point-line tracking.** Harris corners and gradient line segments are matched
   frame-to-frame on the image plane.
3. **Depth-prior PL-BA.** Ceres minimizes reprojection residuals plus LiDAR depth
   prior terms on the optimized pose (Huber loss).
4. **Scale correction.** Monocular translation scale is corrected by the median ratio
   of LiDAR vs. visual depth estimates (paper scheme).

## Scope / approximations for KITTI Odometry

- **KITTI Odometry benchmark has no RGB images** (velodyne + poses only). Visual
  features are extracted on a **LiDAR-rendered pseudo-image**: Velodyne points are
  projected to the standard KITTI camera model (half resolution for throughput),
  and corners/lines are detected on the **depth gradient map**. This preserves the
  paper's depth-extraction + depth-prior PL-BA mechanism while allowing seq00/07
  evaluation on the repo's existing dogfooding trees.
- ORB/LSD are approximated by Harris corners + gradient line segments (no OpenCV
  dependency; same role in the pipeline).
- **Odometry only** (frame-to-frame PL-BA); no global mapping or loop closure.
- Full LiDAR-visual evaluation on KITTI Raw (actual camera images) is future work.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `input_stride` | LiDAR point subsampling | 2 |
| `max_point_features` | Harris corner cap | 280 |
| `max_line_features` | Line segment cap | 64 |
| `patch_radius` | Depth extraction patch [px] | 4 |
| `use_depth_prior` | LiDAR depth prior factors in BA | true |
| `use_line_features` | Line reprojection factors | true |
| `use_scale_correction` | Median depth ratio scale fix | true |
| `depth_prior_weight` | Depth prior residual weight | 1.0 |

Ablation flags: `--pl-loam-no-depth-prior`, `--pl-loam-no-lines`, `--pl-loam-no-scale`.

## Tests

`test_pl_loam` covers: patch median depth extraction; scale-correction median ratio;
depth-prior PL-BA convergence on synthetic correspondences; short-sequence tracking on
LiDAR-rendered features; and state reset on `clear()`.

## Reproduce

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods pl_loam --no-gt-seed --pl-loam-dense-profile
```

## Result (KITTI Odometry, `--no-gt-seed`, pseudo-image eval)

| Sequence | RPE drift | ATE |
|---|---:|---:|
| Seq 00 _(4541 fr)_ | **143.211%** | 3016 m |
| Seq 07 _(1101 fr)_ | **116.899%** | 271 m |

**Honest negative on this benchmark path.** PL-LOAM's paper pipeline expects real
monocular RGB + synchronized LiDAR (KITTI Raw). The Odometry benchmark here has
velodyne only, so visual features come from a LiDAR-projected depth-gradient
pseudo-image — the depth-extraction + depth-prior PL-BA mechanism is implemented
and unit-tested, but without true photometric texture the tracker diverges badly
(~117–143% drift vs paper ~0.6–1.0% on full KITTI Odom with RGB). Scale
correction stays near 1.0 (mean ≈ 1.005) while depth-prior residuals remain
large (≈3.7–4.1 m), indicating the visual front-end — not the LiDAR depth stage
alone — is the bottleneck on pseudo-images. Full RGB evaluation on KITTI Raw is
future work.
