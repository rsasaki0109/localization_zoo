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
feature detection and keeps the range image as the LiDAR depth prior. Visual
bootstrap is disabled by default on LiDAR-only pseudo-images because it injects
a bad motion prior into A-LOAM; use `--vlom-enable-bootstrap` to reproduce the
old paper-style bootstrap path. The old depth-gradient pseudo-image path is
still available with `--vlom-depth-pseudo-image`.

**KITTI Raw RGB** (2026-06-09): pass `--vlom-rgb-root --vlom-enable-bootstrap`
to use real `image_02` PNG features with visual bootstrapping (see PL-LOAM
README reproduce block; swap `--methods vlom`).

## Tests

`test_vlom` — scale corrector outlier rejection, triangulation, cam/velo transform,
state reset.

## Reproduce

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods vlom
```

## Result (KITTI Odometry, default profile, intensity pseudo-image)

| Seq | Drift | ATE | FPS | mean scale | bootstrap frames |
|-----|-------|-----|-----|------------|------------------|
| seq00 | **0.91%** | 9.52 m | 2.8 | 1.000 | 0/4541 |
| seq07 | **0.61%** | 2.51 m | 3.2 | 1.000 | 0/1101 |

**Caveat**: this KITTI Odometry row is not using visual bootstrap, because the
available "visual" signal is only a LiDAR-intensity pseudo-image. The previous
bootstrap-on run diverged badly (seq00 **89.2%**, seq07 **81.6%** drift). With
bootstrap off, the A-LOAM mapping core and scale correction remain stable, but
the monocular front-end is still not a substitute for the paper's full
ORB-SLAM2-style VO stack.

## Result (KITTI Raw RGB, `--vlom-dense-profile`, 200-frame windows)

| Drive | Drift | ATE | mean scale | bootstrap |
|-------|------:|----:|-----------:|----------:|
| raw_0009 | **99.7%** | 122 m | 1.000 | 199/200 |
| raw_0061 | **99.3%** | 81 m | 1.000 | 199/200 |

Real RGB does not rescue drift (~99%, same as pseudo-image path). Scale correction
remains stable; visual bootstrap fires on almost every frame but the simplified
monocular front-end still diverges. Paper seq00 1.18% assumes full ORB-SLAM2-style
VO on KITTI Raw.
