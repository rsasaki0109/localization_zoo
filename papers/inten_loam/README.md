# InTEn-LOAM: Intensity and Temporal Enhanced LOAM (from-paper reimplementation)

**Paper:** Shuaixin Li, Bin Tian, Zhu Xiaozhou, Gui Jianjian, Yao Wen, Guangyun Li,
*"InTEn-LOAM: Intensity and Temporal Enhanced LiDAR Odometry and Mapping"*，
Remote Sensing / arXiv:2209.05708 (2022/23). No public author code found.
Reconstructed from the paper.

## Core idea

InTEn-LOAM exploits **geometric + intensity + temporal** information in LiDAR sweeps:

1. **Cylindrical-image feature extraction (FEF).** Points are projected to range +
   intensity cylindrical images (paper eq. 2). Ground (column-wise line fit),
   facade, edge (range curvature), and reflector (intensity delta + absolute
   threshold) features are adaptively extracted.
2. **Intensity-based scan registration.** Reflector features are aligned by
   minimizing intensity residuals on a local **B-spline intensity surface**
   (simplified here as trilinear interpolation over 8 voxel control points; paper
   eq. 6–7).
3. **Joint scan-to-scan LO.** Geometric edge (point-to-line) and ground/facade
   (point-to-plane) factors are combined with intensity factors in Ceres.
4. **Temporal voxel filter (TVF) + dynamic object removal (DOR)** (2026-06-09):
   voxel occupancy ages features over time; DOR drops voxels with high motion
   inconsistency before mapping updates.
5. **Scan-to-map mapping** (2026-06-09): local voxel map + Ceres scan-to-map
   refinement after scan-to-scan odometry (paper §3.3 mapping thread, simplified).

## Scope / approximations for KITTI

- KITTI Odometry velodyne scans are **unordered** (not native HDL-64 ring order);
  cylindrical projection uses spherical angles from `(x,y,z)` directly.
- TVF/DOR/mapping are simplified approximations of the paper's full pipeline.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `cylindrical.width/height` | Cylindrical image size | 1024 × 64 |
| `use_intensity_registration` | B-spline intensity factors | true |
| `enable_tvf` | Temporal voxel filter | true |
| `enable_dor` | Dynamic object removal | true |
| `enable_mapping` | Scan-to-map refinement | true |
| `max_edge/surface/reflector_features` | Feature caps | 200 / 400 / 120 |
| `intensity_voxel_size` | Local intensity model voxel [m] | 0.5 |

Ablation flags: `--inten-loam-no-intensity`, `--inten-loam-no-tvf`,
`--inten-loam-no-dor`, `--inten-loam-no-mapping`.

## Paper pipeline

`--inten-loam-paper-odometry` runs `InTenLoamPaper`
(`include/inten_loam/inten_loam_paper.h`), written against the paper after an
audit of the pipeline below found why it does not track:

- it associated the **untransformed** current points once per frame, so every
  correspondence assumed zero motion (trajectory 0.83x the true length
  scan-to-scan);
- scan-to-map repeated that bias against a denser map and **overwrote** the
  scan-to-scan motion (0.60x);
- DOR ran before feature extraction with the previous pose and a fixed 0.6 m
  range test, removing ~40 % of the valid pixels, mostly ahead of and behind
  the car (0.40x overall);
- `EdgeFactor` used a wrong cross product (nonzero on inclined lines; fixed
  for both pipelines).

The paper pipeline:

- **FEF** (Sec. 3.1): 2048 x 64 range image (smallest range, largest
  intensity); segment image of the lowest z per azimuth segment (512) and
  0.5 m radial bin with per-segment ground lines (Himmelsbach, Th_g 0.2 m);
  angle-based clustering (10 deg), clusters under 15 points masked;
  curvature C = sum(D - D_nb) / (N D) along the row, edges |C| > 0.3, facades
  |C| < 0.1 (Sec. 4.1.1) per row sector with non-maximum suppression, points
  behind an occluder skipped; reflectors from the row intensity difference
  (Th_dI 80 / 255) or the 16 x 4 block median.
- **S2S** (Alg. 1): edge residual to the line through the closest point and
  its neighbour on an adjacent scan line, plane residual from the closest
  point and neighbours on the same and an adjacent line, cubic B-spline
  intensity residual (eqs. 6-7, 0.3 m knots); Gauss-Newton on se(3) with
  association redone at every iteration from the previous relative motion.
- **S2M** (Alg. 2): prediction dT_{k-1} T_w'k, local map (100 m) of keyframe
  features in a temporal voxel filter (voxel mean position, intensity
  averaged over the last 50 frames), 5-NN PCA lines and planes.
- **DOR** (Sec. 3.3): after S2M, the local map and the previous scan are
  rendered into a range image; a cluster is dynamic when half of its pixels
  differ by more than 10 % of the range; ground never is. Dynamic points are
  only kept out of the map.
- No motion compensation: KITTI scans are used as given.

Official KITTI RTE, full sequences, no GT seed, 0.2 deg vertical-angle
correction (Sec. 4.2.1), `experiments/inten_loam_kitti_seq_{00,07}_full_paper_comp_matrix.json`:

| variant | seq 00 | seq 07 | FPS |
|---|---:|---:|---:|
| previous pipeline (`--inten-loam-dense-profile`, edge factor fixed) | 47.783 % | 42.158 % | 9 |
| paper pipeline (`paper_odometry`) | **0.816 %** | **0.368 %** | 4.1 |
| without the correction | 1.462 % | 0.855 % | 4.1 |
| without the intensity residual | 0.830 % | 0.379 % | 4.7 |
| S2S only (no S2M, map, DOR) | 3.457 % | 2.395 % | 9-11 |
| paper, Table 1 | 0.52 % | 0.34 % | |

The plane map voxel, which the paper does not give, was chosen on seq 07
(0.8 m: 0.803 %, 0.4 m: 0.444 %, 0.2 m: 0.368 %, 0.1 m: 0.358 %); seq 00 is
held out. That gives 1.08x on 07 and 1.57x on 00 (geometric mean 1.30x).
Mapping and the correction matter most; the intensity residual is worth
1-3 %. Free parameters can be changed with `--inten-loam-paper-set key=value`
(`map_plane_voxel`, `max_ground`, `huber`, ...).

## Result of the simplified pipeline (KITTI Odometry full, `--no-gt-seed`)

Post-fix ablation artifacts (2026-07-03, quaternion-layout fix applied):
`docs/benchmarks/kitti_full_new_methods/seq{00,07}_inten_loam_ablation.json`

Canonical default-pipeline artifacts (TVF+DOR+mapping, dense profile):
`docs/benchmarks/kitti_full_new_methods/seq{00,07}_inten_loam.json`

The 2026-06-19 evaluator build fixes the Ceres/Eigen quaternion layout used by
the geometric and intensity residuals. Earlier artifacts interpreted Eigen
`{x,y,z,w}` parameter blocks as Ceres `{w,x,y,z}` inside
`QuaternionRotatePoint`, so the identity initial guess was not actually
identity inside the residuals.

### Post-fix ablation (7 variants, `--inten-loam-dense-profile`)

| Sequence | Variant | Drift | ATE [m] | FPS | Notes |
|----------|---------|------:|--------:|----:|-------|
| 07 (1101 fr) | **no_mapping** | **29.597%** | 129 | 39.7 | best seq07 RPE |
| 07 | mapping_only | 50.131% | 143 | 14.1 | TVF/DOR off |
| 07 | no_dor | 52.318% | 109 | 13.3 | |
| 07 | no_intensity | 63.511% | 124 | 10.4 | intensity near-redundant |
| 07 | **full** (default) | 64.599% | 126 | 10.3 | TVF+DOR+mapping |
| 07 | no_tvf | 69.874% | 96 | 10.9 | |
| 00 (4541 fr) | **full** (default) | **63.008%** | 455 | 10.6 | best stable seq00 row |
| 00 | no_intensity | 68.624% | 423 | 11.1 | |
| 00 | no_tvf | 71.336% | 349 | 11.0 | |
| 00 | no_mapping | 218.170% | 13265 | 47.5 | severe divergence |
| 00 | no_dor | 299.979% | 11569 | 15.5 | severe divergence |
| 00 | mapping_only | 4362.094% | 223579 | 21.3 | severe divergence |
| 00 | no_tvf_no_dor | 4362.094% | 223579 | 21.6 | severe divergence |

Per-variant raw JSON:
`docs/benchmarks/kitti_full_new_methods/seq{00,07}_inten_loam_{variant}.json`.

**Honest negative (T3).** Post-fix, the simplified pipeline remains far from the paper's
reported ~0.54% drift on KITTI. seq07 scan-to-scan without mapping is the
least-bad local row (~30% RPE), but seq00 scan-to-scan diverges catastrophically
while the default full pipeline stays merely high-drift (~63% RPE). Mapping +
TVF/DOR without each other is unstable on seq00. Intensity on/off is
near-redundant on both sequences. **Not promoted to the README from-paper
leaderboard** (high-drift policy); raw artifacts and ablation bundles are kept
for reproducibility.

## Cross-dataset (`--no-gt-seed`)

| Dataset | Drift | ATE | Notes |
|---------|------:|----:|---|
| MulRan 120 | **185.4%** | 32 m | TVF/DOR/mapping ON |
| NCLT 600 | **46.5%** | 40 m | ICPSC/MCC ~0.5% drift on same data |

See `docs/benchmarks/cross_dataset/`.

## Reproduce

Full ablation (7 variants × seq00 + seq07):

```sh
python3 evaluation/scripts/run_inten_loam_ablation.py --sequence seq07 --sequence seq00
```

Single run (default = TVF+DOR+mapping ON, dense profile):

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods inten_loam --no-gt-seed --inten-loam-dense-profile
```

Scan-to-scan ablation (seq07 least-bad RPE; seq00 diverges on long runs):

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods inten_loam --no-gt-seed --inten-loam-dense-profile \
  --inten-loam-no-mapping
```

## Tests

`test_inten_loam`: cylindrical projection validity; reflector labeling pipeline;
intensity residual scaling; Eigen/Ceres quaternion layout in residual factors;
edge factor on an inclined line; short-sequence pipeline stability; `clear()`
reset.

`test_inten_loam_paper`: ground segmentation and feature classes on a
ray-cast street; the B-spline intensity field on a linear ramp; 1 m per scan
tracked at full scale.
