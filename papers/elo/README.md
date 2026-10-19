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

## All KITTI sequences (00-10)

The setting chosen on seq 07 windows (`dense_elevation_paper_normals_unbiased`:
0.195 deg correction, paper normal patch, the three forward-bias fixes) run
unchanged on every sequence; only 07 was used to choose values.
`experiments/elo_kitti_seq_NN_full_paper_comp_matrix.json`, official RTE [%]:

| Seq | Paper | `dense_elevation_paper_normals_unbiased` | Ratio |
|---|---:|---:|---:|
| 00 | 0.54 | 0.586 | 1.09x |
| 01 | 0.61 | 0.869 | 1.42x |
| 02 | 0.54 | 0.770 | 1.43x |
| 03 | 0.65 | 0.890 | 1.37x |
| 04 | 0.32 | 0.268 | 0.84x |
| 05 | 0.33 | 0.399 | 1.21x |
| 06 | 0.30 | 0.411 | 1.37x |
| 07 (chosen) | 0.31 | 0.391 | 1.26x |
| 08 | 0.79 | 1.079 | 1.37x |
| 09 | 0.48 | 0.678 | 1.41x |
| 10 | 0.59 | 0.504 | 0.85x |

Geometric mean over 11 sequences: 1.22x.

No sequence fails; the gap is spread over all of them (0.84-1.43x).

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
("the inlier ratio between the non-ground points and ground ones"). Three more
readings combine the raw sums as `w A_S + (1 - w) A_G`, `w = 0.7 w2` (on top of
correction + normal patch, seq 07):

| `w2` reading | flag | seq 07 |
|---|---|---:|
| none (fixed 5:1 ground weight) | — | **0.480 %** |
| `Ns_in / (Ns_in + Ng_in)` | `--elo-fusion-mode 2` | 0.488 % |
| `Ns_in / Ng_in` | `--elo-fusion-mode 3` | 0.544 % |
| ratio of inlier rates | `--elo-fusion-mode 4` | 0.564 % |

None beats the fixed weight; mode 2 lands close, with a slightly lower rotation
error (0.250 vs 0.268 deg/100 m).

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

### Forward drift: biases of the projective model

The best variant over-estimates distance on seq 07. On 400-scan windows, the
estimated path is 0.54 % (f0-399) and 0.24 % (f400-799) too long. The windows
are measured with a scratch driver that runs the dense profile, the 0.195 deg
correction and the full-size normal patch on KITTI `.bin` scans, and reports
official RTE. Each per-frame increment error was summed in the sensor frame and
compared with the official MULLS run on the same scans. Lateral (+y), yaw and,
on seq 00, pitch drift are the same for both methods to within a few
centimetres, so they come from the reference poses or the data, not from ELO.
The forward excess is ELO's own: x +1.38 m against +0.02 m for MULLS on 07
f0-399, and +0.96 m against +0.06 m on 00 f400-799.

The direction of that pull was found by taking one Gauss-Newton step from the
ground-truth increment against the model as it stood. Three causes showed up:

- **Closest-point SRI update.** The SRI model keeps the closer point on every
  pixel collision (Sec. III-D). The minimum is taken again every frame on noisy
  ranges, so model surfaces creep towards the sensor. At the ground-truth
  increment, non-ground residuals put the model surface 12-13 mm in front of
  the current points, both ahead of and behind the car. These pulls do not
  cancel, so a net forward term remains. A unit test shows the creep: a static
  sensor with 3 cm range noise moves 0.12 m in 12 scans.
- **Ground forward term.** Ground normals come from the five closest BEV
  vertices. They are noisy (rms x component about 0.15). The nearest model
  vertex is also 0.1 m away horizontally on average, and up to 3 cells
  (0.6 m). Together these give the many
  ground residuals a biased forward term: about +1.3 mm per scan once the SRI
  creep is removed. This is the analogue of SuMa's ground-normal bias. Here,
  though, the correction angle has no effect on it: with 0.15, 0.195 or
  0.22 deg the forward drift is the same, and only pitch changes.
- **Curved surfaces.** With `curvature_threshold = 0.1`, foliage and convex
  objects enter the non-ground term. Their tangent planes leave current points
  behind the model: a residual bias of about 5 mm remains after the fusion
  fix. A threshold of 0.03 lowers it to about 3 mm (the paper gives no
  value).

The fixes are opt-in `EloParams` fields, with defaults unchanged:

- `sri_fuse_same_surface` (`sri_same_surface_tolerance = 0.2` m,
  `sri_max_fuse_count = 10`): points within the tolerance in range are
  averaged. The closer point still wins across larger gaps (occlusion).
- `ground_constrains_forward = false`: zeroes the x column of the ground
  Jacobian.
- `curvature_threshold = 0.03`.

Results on top of `dense_elevation_paper_normals` (official RTE, 400-scan
windows). The variant was chosen on 07, and 00 is held out:

| variant | 07 f0-399 | 07 f400-799 | 07 f800-1100 | 00 f0-399 | 00 f400-799 |
|---|---:|---:|---:|---:|---:|
| `dense_elevation_paper_normals` | 0.622 % | 0.544 % | 0.328 % | 1.254 % | 0.573 % |
| + same-surface fusion | 0.451 % | 0.484 % | | 1.090 % | |
| + curvature 0.03 | 0.563 % | 0.383 % | | | |
| + ground without x | 0.593 % | 0.526 % | | | |
| + fusion + curvature 0.03 | 0.451 % | 0.415 % | | | |
| + fusion + ground without x | 0.397 % | 0.463 % | | 1.116 % | |
| + ground without x + curvature 0.03 | 0.483 % | 0.318 % | | | |
| **+ all three** | **0.383 %** | **0.345 %** | **0.292 %** | **1.098 %** | **0.573 %** |
| all three, curvature 0.02 / 0.05 | 0.399 / 0.395 % | 0.325 / 0.384 % | | | |
| all three, ground without yaw, x, y | 0.378 % (c 0.1) | 0.627 % (c 0.1) | | | |
| all three, 0.22 deg correction | 0.378 % | 0.407 % | | | |
| official MULLS, same scans | 0.326 % | 0.411 % | 0.304 % | 1.013 % | 0.627 % |

With all three changes, the summed forward error falls to +0.21 / +0.23 /
+0.07 m on the 07 windows and +0.07 m on 00 f400-799. The remaining
per-axis errors match MULLS. On 00 f400-799 the RTE does not change: the
lateral error (+2.1 m for both methods) dominates there, and the old forward
excess had partly cancelled it.

Two observations from the same windows:

- Dropping all three planar ground directions (yaw, x and y) loses yaw on
  07 f400-799.
- With the fixes in place, the 0.1 m single-vertex BEV (0.373 / 0.332 %) and
  fusion mode 2 (0.374 / 0.359 %) no longer hurt. They do not clearly help
  either.

Full-sequence results are below.

Full sequences with the three fixes (`dense_elevation_paper_normals_unbiased`:
`--elo-sri-fuse-same-surface --elo-ground-no-forward --elo-curvature-threshold
0.03` on top of `dense_elevation_paper_normals`), official KITTI RTE: seq 00
0.656 -> **0.586 %** (paper 0.54 %, held out), seq 07 0.480 -> **0.391 %**
(paper 0.31 %); 1.09x / 1.26x, geometric mean 1.17x (from 1.37x).

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 11 selected stored ELO full-sequence cells complete with native exits 0. 11 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

The runner's sensor-frame near-range cut is **non-paper preprocessing**, still default zero/off. It is shared by ELO/DiLO and intensity-preserving loaders, after sensor-frame conversion and before elevation correction/downsampling; standalone cores do not apply it automatically.

Historical NCLT full-sequence RTE without/with the existing 5 m cut is 18.132951% / 3.361084%; ATE is 126.612474 m / 51.639158 m. The cut still leaves substantial absolute position error. The cutoff was selected on that NCLT sequence and restart window, with no new held-out claim. These are historical numbers, not a fresh NCLT rerun. [Committed summaries](../../docs/benchmarks/nclt_paper_paths/) and [recorded commands and limits](../../docs/nclt_paper_paths.md) retain their original missing evaluator-commit/argv provenance. Fresh NCLT validation remains open.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
