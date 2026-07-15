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
  LiDAR intensity is rendered and lightly dilated for corner/line detection, and
  depths are still read from the projected LiDAR depth map. This preserves the
  paper's depth-extraction + depth-prior PL-BA mechanism while avoiding the
  near-empty feature set produced by raw depth gradients on sparse KITTI scans.
- ORB/LSD are approximated by Harris corners + gradient line segments (no OpenCV
  dependency; same role in the pipeline).
- **Odometry only** (frame-to-frame PL-BA); no global mapping or loop closure.
- **KITTI Raw RGB path** (2026-06-09): real `image_02` PNGs loaded via
  `--pl-loam-rgb-root` + `--pl-loam-rgb-camera kitti_color02`. Harris/line
  detection runs on grayscale RGB instead of the depth-gradient pseudo-image.

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
| `use_intensity_pseudo_image` | Detect KITTI pseudo-image features on LiDAR intensity, not depth gradients | true |
| `intensity_dilation_radius` | Pixel radius used to densify sparse projected intensity returns | 2 |
| `depth_prior_weight` | Depth prior residual weight | 1.0 |

Ablation flags: `--pl-loam-no-depth-prior`, `--pl-loam-no-lines`,
`--pl-loam-no-scale`, `--pl-loam-depth-pseudo-image`,
`--pl-loam-intensity-dilation N`.

## Tests

`test_pl_loam` covers: patch median depth extraction; intensity pseudo-image
densification; scale-correction median ratio; Eigen/Ceres quaternion layout in
PL-BA residuals; depth-prior PL-BA convergence on synthetic correspondences;
short-sequence tracking on LiDAR-rendered features; and state reset on
`clear()`.

## Reproduce

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_00_full \
  experiments/reference_data/kitti_seq_00_full_gt.csv \
  --methods pl_loam --no-gt-seed --pl-loam-intensity-dilation 2 \
  --pl-loam-no-lines

./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods pl_loam --no-gt-seed --pl-loam-intensity-dilation 2
```

## Result (KITTI Odometry, `--no-gt-seed`, intensity pseudo-image eval)

| Sequence | Profile | RPE drift | ATE |
|---|---|---:|---:|
| Seq 00 _(4541 fr)_ | intensity pseudo-image, line factors off | **89.731%** | 275 m |
| Seq 07 _(1101 fr)_ | intensity pseudo-image, line factors on | **84.782%** | 142 m |

**Honest negative on this benchmark path.** PL-LOAM's paper pipeline expects real
monocular RGB + synchronized LiDAR (KITTI Raw). The Odometry benchmark here has
velodyne only, so visual features come from a LiDAR-intensity pseudo-image while
depth priors still come from projected LiDAR depth. This fixes the older
depth-gradient feature starvation (previously ~117–143% drift), and the
2026-06-20 pass also fixes the PL-BA residual quaternion layout
(`Eigen::Quaterniond::coeffs()` is `{x,y,z,w}`, while Ceres rotation helpers
expect `{w,x,y,z}`). RPE improves from 90.100% / 87.377% to 89.731% / 84.782%,
but this is still far from the paper's ~0.6–1.0% KITTI drift. Seq00's best
stable artifact disables line factors; with the corrected rotation convention,
line endpoint residuals are still too brittle on the long pseudo-image run.
Scale correction stays near 1.0 (mean ≈ 0.985–0.988) while depth-prior
residuals remain large (≈1.8–2.1 m), indicating the visual front-end — not the
LiDAR depth stage alone — is the bottleneck on pseudo-images.

## Result (KITTI Raw RGB, `--no-gt-seed`, 200-frame windows)

| Drive | RPE drift | ATE | rgb_frames | mean scale |
|---|---:|---:|---:|---:|
| raw_0009 | **99.6%** | 120 m | 200 | 0.977 |
| raw_0061 | **99.3%** | 84 m | 200 | 0.974 |

**Still honest negative with real camera images.** Drift remains ~99% on both
windows (paper ~0.6–1% on full KITTI Odom with RGB). The simplified Harris/line
front-end (no ORB/LSD, no full PL-BA loop closure) does not close the gap vs the
published pipeline. Depth-prior residuals stay large (~2.5–3.2 m).

### Reproduce (KITTI Raw RGB)

```sh
python3 evaluation/scripts/download_kitti_raw_color_window.py \
  --sequence-dir dogfooding_results/kitti_raw_0009_200 \
  --kitti-root /path/to/kitti_raw

./build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 \
  experiments/reference_data/kitti_raw_0009_200_gt.csv \
  --methods pl_loam --no-gt-seed \
  --pl-loam-rgb-root /path/to/2011_09_26_drive_0009_sync \
  --pl-loam-rgb-camera kitti_color02 \
  --summary-json docs/benchmarks/kitti_full_new_methods/kitti_raw_0009_200_rgb.json
```
