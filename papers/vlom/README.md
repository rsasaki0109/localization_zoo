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

Same as PL-LOAM: no RGB in the Odometry benchmark → features on a LiDAR-projected
depth-gradient pseudo-image (KITTI half-res camera model).

## Tests

`test_vlom` — scale corrector outlier rejection, triangulation, cam/velo transform,
state reset.

## Reproduce

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods vlom --no-gt-seed --vlom-dense-profile
```

## Result (KITTI Odometry, `--vlom-dense-profile`, pseudo-image)

| Seq | Drift | ATE | FPS | mean scale | bootstrap frames |
|-----|-------|-----|-----|------------|------------------|
| seq07 | **153.9%** | 439 m | 2.4 | 1.003 | 172/1100 |
| seq00 | (ベンチ実行中) | — | — | — | — |

**Honest negative** (PL-LOAM と同様、RGB 無し疑似画像制約)。スケール補正は安定
(mean≈1.0) だが visual bootstrap が後半で A-LOAM を攪乱し PL-LOAM (117%) より悪化。
論文の seq00 1.18% は KITTI Raw + 実 RGB が前提。
