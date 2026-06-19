# Scale-correction VLOM

**Visual-LiDAR Odometry and Mapping with Monocular Scale Correction and Visual
Bootstrapping** (arXiv:2304.08978, 2023). From-paper reimplementation; no author
code.

## Core idea

Loosely coupled fusion of monocular visual odometry (ORB-SLAM2-style, simplified)
and A-LOAM LiDAR odometry + mapping:

1. **Monocular scale correction** — triangulated keypoint depth vs LiDAR depth
   ratio with MAD outlier rejection; periodically rescales visual translation.
2. **Visual bootstrapping** — visual frame-to-frame motion initializes A-LOAM
   scan-to-scan optimization (`aloam::LaserOdometry::setMotionPrior`).

## KITTI Odometry approximations

Same as PL-LOAM on KITTI Odometry: no RGB camera stream is present, so the
default evaluator renders LiDAR intensity into a pseudo-image for point/line
feature detection and keeps the range image as the LiDAR depth prior. The old
depth-gradient pseudo-image path is still available with
`--vlom-depth-pseudo-image`.

**KITTI Raw RGB** (2026-06-09): pass `--vlom-rgb-root` to use real `image_02`
PNG features (see PL-LOAM README reproduce block; swap `--methods vlom`).

## Tests

`test_vlom` — scale corrector outlier rejection, triangulation, cam/velo transform,
state reset.

## Reproduce

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods vlom --no-gt-seed --vlom-dense-profile
```

## Result (KITTI Odometry, `--vlom-dense-profile`, intensity pseudo-image)

| Seq | Drift | ATE | FPS | mean scale | bootstrap frames |
|-----|-------|-----|-----|------------|------------------|
| seq00 | **89.2%** | 315 m | 1.0 | 1.000 | 4539/4541 |
| seq07 | **81.6%** | 141 m | 1.7 | 1.000 | 1099/1101 |

**Honest negative** (PL-LOAM と同様、RGB 無し疑似画像制約)。LiDAR intensity
pseudo-image makes the visual bootstrap far less sparse, fixing the seq07
catastrophe (153.9% -> 81.6%) and slightly improving seq00 drift
(91.5% -> 89.2%). It does not solve the method: seq00 ATE worsens (249 m ->
315 m), and both sequences remain far from scan-to-map LiDAR odometry. Scale
correction stays stable (mean≈1.0), while the simplified monocular front-end is
still not a substitute for the paper's full ORB-SLAM2-style VO stack.

## Result (KITTI Raw RGB, `--vlom-dense-profile`, 200-frame windows)

| Drive | Drift | ATE | mean scale | bootstrap |
|-------|------:|----:|-----------:|----------:|
| raw_0009 | **99.7%** | 122 m | 1.000 | 199/200 |
| raw_0061 | **99.3%** | 81 m | 1.000 | 199/200 |

Real RGB does not rescue drift (~99%, same as pseudo-image path). Scale correction
remains stable; visual bootstrap fires on almost every frame but the simplified
monocular front-end still diverges. Paper seq00 1.18% assumes full ORB-SLAM2-style
VO on KITTI Raw.
