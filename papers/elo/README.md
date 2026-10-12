# ELO

## Paper
- Xin Zheng, Jianke Zhu
- "Efficient LiDAR Odometry for Autonomous Driving"
  (IEEE Robotics and Automation Letters 2021, [arXiv:2104.10879](https://arxiv.org/abs/2104.10879))
- **No public author implementation found** in the current web/code search.

## What This Repository Implements

From-paper CPU reimplementation of the LiDAR-only odometry core:

- **Non-ground spherical range-image model**: point clouds are projected into a
  spherical range image. Non-ground model cells store a vertex and normal, and
  current points use projective association instead of a tree search.
- **Range-adaptive normal estimation**: local PCA normals use a search window
  that shrinks with range, plus range and plane-distance outlier rejection.
- **Ground BEV model**: ground candidates are segmented by LiDAR-height and
  vertical-angle tests, then projected into a bird's-eye-view grid where ground
  correspondences are found and normals are estimated online from neighboring
  model cells.
- **Frame-to-model update**: after each registration, the previous local model
  is transformed into the current LiDAR frame, stale cells are dropped, and
  current scan cells are fused into the SRI/BEV maps.

## Deviations from the Paper

- CPU arrays and loops replace the paper's CUDA implementation.
- The full KITTI dogfooding runs use `--elo-fast-profile` for tractable CPU
  runtime: 1024 x 80 SRI and 0.3 m BEV resolution. The paper/KITTI detail page
  reports a 120 m x 60 m BEV map and 2048 x 80 SRI.
- Timestamps are approximated with frame indices (`model_window_frames`) because
  the PCD dogfooding input has no per-point time for this method.
- Non-ground normal estimation is PCA based with the paper's range-adaptive
  window and outlier criteria; exact GPU patch scheduling is not reproduced.
- Ground segmentation keeps the paper's height and 5 degree slope gates, but
  uses a conservative fallback when adjacent vertical SRI pixels are missing.

## Dogfooding

```bash
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] \
  --methods elo --no-gt-seed --elo-fast-profile
```

Flags: `--elo-{fast,dense}-profile`, `--elo-sri-width`,
`--elo-bev-resolution`, `--elo-ground-weight`.

## Results

KITTI Odometry full sequences, first-pose anchored, no GT seed, `--elo-fast-profile`:

| Sequence | Frames | ATE [m] | RPE [%/100 m] | RPE [deg/m] | FPS | Artifact |
|---|---:|---:|---:|---:|---:|---|
| seq00 | 4541 | 22.504 | 1.124 | 0.010 | 5.15 | `docs/benchmarks/kitti_full_new_methods/seq00_elo.json` |
| seq07 | 1101 | 3.536 | 0.981 | 0.010 | 5.55 | `docs/benchmarks/kitti_full_new_methods/seq07_elo.json` |

The paper reports 0.54 % (seq00) / 0.31 % (seq07) relative translation error
for its fusion model on KITTI training sequences, and the KITTI benchmark page
lists ELO at 0.68 % on the odometry test set. This CPU port preserves the
projective SRI + ground BEV mechanism, but the reduced CPU profile and simplified
model update do not reproduce the paper-level drift.

## Paper mechanics and the gap to the paper

A line-by-line comparison with the paper found these differences in the harness
profiles: no 0.195 deg vertical-angle correction (the paper applies it, Sec. IV-A);
the range-adaptive normal patch of eq. 12 used as the half-width, so the window is
twice the paper's; a 0.2-0.3 m BEV with a running mean per cell instead of 0.1 m
and one vertex; a fixed 5:1 ground weight instead of the adaptive fusion
`w = 0.7 w2` of eqs. 1-2; a one-sided ground test instead of eq. 13; voxelised,
capped input; a Cauchy kernel and 3x3 search instead of plain Gauss-Newton.

Each is now a flag (`--elo-normal-full-patch`, `--elo-bev-resolution 0.1
--elo-bev-keep-closest`, `--elo-fusion-mode 1`, `--elo-ground-both-sides`,
`--elo-raw-input`, `--elo-robust-scale 0 --elo-sri-search-radius 0`), and
`--elo-paper-profile` turns them all on. Added one at a time to the dense profile
with the correction (official KITTI RTE, seq 07):

| change | seq 07 |
|---|---:|
| dense profile | 0.893 % |
| + 0.195 deg correction | 0.706 % |
| + full-size normal patch (eq. 12) | **0.480 %** |
| + 0.1 m single-vertex BEV | 0.662 % |
| + adaptive fusion w = 0.7 w2 | 0.700 % |
| + two-sided ground test (eq. 13) | 0.781 % |
| + plain GN, one-pixel association | 0.908 % |
| + raw input (no voxel grid, no caps) | 0.925 % |
| everything (`--elo-paper-profile`) | 1.111 % |

Only the correction and the normal patch help; the rest, as implemented here,
hurt, also in combination (correction + normal patch + BEV 0.517 %, + fusion
0.581 %, + raw input 0.632 %). The fusion weight is the least certain reading
("the inlier ratio between the non-ground points and ground ones"), so a
different reading of `w2` may still matter.

Full sequences (`experiments/elo_kitti_seq_{00,07}_full_paper_comp_matrix.json`):

| variant | seq 00 | seq 07 |
|---|---:|---:|
| `--elo-dense-profile` | 1.272 % | 0.893 % |
| + correction + full-size normal patch (`dense_elevation_paper_normals`) | **0.656 %** | **0.480 %** |
| + correction only (ablation) | 0.755 % | 0.706 % |
| `--elo-paper-profile` + correction (ablation) | 1.638 % | 1.111 % |
| paper, Table II | 0.54 % | 0.31 % |

The selective variant is 1.21x / 1.55x the paper (geometric mean 1.37x, from
2.60x before).
