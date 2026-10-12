# Mesh-LOAM

## Paper
- Yanjin Zhu, Xin Zheng, Jianke Zhu
- "Mesh-LOAM: Real-time Mesh-Based LiDAR Odometry and Mapping"
  (IEEE Transactions on Intelligent Vehicles 2024, [arXiv:2312.15630](https://arxiv.org/abs/2312.15630))
- **No public code**: the official repository (`HelloXiaoZHU/Mesh-LOAM`) has
  contained only a README since 2024; no third-party implementation exists.

## What This Repository Implements

From-paper CPU reimplementation of the full odometry pipeline:

- **Passive-voxel IMLS-SDF map** (Sec. III-C): every scanned point *scatters*
  an IMLS-weighted signed-distance increment `n_i^T (v - p_i)` into the
  3×3×3-voxel neighborhood of its voxel-hash cell — O(N) per scan, no k-d
  tree. Hybrid weight (Eq. 8) `w = exp(-||v-p||²/h) + λ_n max(0, n_i·n_v)`,
  running weighted average update (Eqs. 9–10).
- **Incremental block-wise zero-surface extraction** (Sec. III-C4): dirty
  voxel blocks are re-meshed every `mesh_update_interval` frames from the SDF
  zero set with linear interpolation.
- **Point-to-mesh odometry** (Sec. III-B): constant-velocity prediction,
  planar-feature selection by PCA curvature (`c < 0.1`), facet candidates from
  a 0.2 m search grid, correspondence gate by normal cosine similarity
  (`|cos| > 0.98`), point-to-facet-plane Gauss-Newton (Eqs. 3–4).

## Deviations from the paper

- **Marching tetrahedra instead of marching cubes** for the zero-surface
  (6 tetrahedra per cell, table-free). Same linearly-interpolated zero
  crossing; produces more, smaller triangles.
- **CPU, single thread, `std::unordered_map`**: the paper's parallel spatial
  hashing (open addressing, distributed locks, Algorithm 1) and the voxel
  deletion scheme exist to fit GPU memory and avoid hash dropout; a chained
  hash map needs neither. Far map content is dropped by a radius prune
  instead (`local_map_radius`).
- **Cubic voxel blocks** (8³ voxels) instead of 2×2 m footprint blocks with
  variable height (Eq. 11) — the variable height exists for GPU memory
  compaction.
- The paper leaves several values unspecified (resolved here as): voxel-normal
  initialization/update (weighted running mean), negative normal agreement in
  Eq. 8 (clamped at 0), mesh extraction interval `t_s` (fixed, 5 frames),
  GN convergence (`‖δ‖ < 1e-4`, ≤30 iters), W(v) floor for extraction (0.3),
  registration point set (planar features only).
- Input is voxel-downsampled in the harness (0.25 m dense profile) instead of
  the paper's full ~130k-point scans; the scatter radius (0.3 m) still
  overlaps adjacent samples so the SDF stays connected.
  `--mesh-loam-paper-odometry` uses 0.1 m input without a point cap instead.
- Facet association: by default the gated facet with the smallest plane
  residual among the 27 search cells; `--mesh-loam-nearest-facet` (and the
  paper path) takes the gated facet nearest to the point (point-to-triangle
  distance within the 0.2 m search interval), as the paper describes.
- No deskew (none described in the paper either).

## Dogfooding

```bash
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] \
  --methods mesh_loam --no-gt-seed --mesh-loam-dense-profile
```

Flags: `--mesh-loam-{fast,dense}-profile`, `--mesh-loam-paper-odometry`,
`--mesh-loam-nearest-facet`, `--mesh-loam-keep-mesh`, `--mesh-loam-voxel-size`,
`--mesh-loam-cos-gate`, `--mesh-loam-mesh-interval`, `--mesh-loam-source-voxel`,
`--mesh-loam-max-points`, `--mesh-loam-normal-neighbors`,
`--mesh-loam-normal-radius`, `--mesh-loam-max-range`, `--mesh-loam-map-radius`.
Normals and the registration loop use OpenMP (`OMP_NUM_THREADS`).

## Results (KITTI Odometry, pure odometry, `--no-gt-seed`, dense profile)

| Window | ATE [m] | RPE [%/100 m] | FPS |
|---|---:|---:|---:|
| seq07 first 108 fr | 0.135 | 0.525 | 0.7 |
| seq07 full (1101 fr) | 0.98 | **0.616** | 0.74 |
| seq00 full (4541 fr) | 13.47 | **0.901** | 0.58 |

seq07 full matches the same-profile KISS-ICP baseline (0.618 %), while seq00
lands just behind the same-profile KISS-ICP baseline (0.872 %). The
point-to-mesh map representation holds up at full-sequence scale on CPU with
downsampled input. Artifacts:
[`docs/benchmarks/kitti_full_new_methods/seq07_mesh_loam.json`](../../docs/benchmarks/kitti_full_new_methods/seq07_mesh_loam.json),
[`docs/benchmarks/kitti_full_new_methods/seq00_mesh_loam.json`](../../docs/benchmarks/kitti_full_new_methods/seq00_mesh_loam.json).

Paper reports 0.31 % (seq07) / 0.53 % (seq00) relative translation error with
full-density scans on GPU; this port runs downsampled input on CPU, so parity
with the same-profile KISS-ICP baseline (0.618 % seq07 / 0.872 % seq00) is the
honest comparison target.

## Paper mechanics and the gap to the paper

A comparison with the paper found three differences that matter on KITTI:

- **Association.** The default keeps, among all gated facets in 27 cells of
  0.2 m, the one with the smallest plane residual. With millions of small
  triangles from marching tetrahedra, some facet plane nearly always passes
  close to the point, so residuals shrink and each Gauss-Newton step
  under-corrects. The paper takes the nearest facet within the 0.2 m search
  interval.
- **Vertical-angle correction.** KITTI scans need the 0.22 deg HDL-64
  elevation correction that KISS-ICP, CT-ICP and IMLS-SLAM apply; the paper
  does not mention its loader (no code).
- **Input density.** The dense profile keeps about 6.5 % of the raw points
  (0.25 m voxels, 8000-point cap); the paper uses full scans with a 0.1 m map.

Official KITTI RTE on full sequences (`experiments/mesh_loam_kitti_seq_{00,07}_full_paper_comp_matrix.json`):

| variant | seq 00 | seq 07 | FPS (07) |
|---|---:|---:|---:|
| `--mesh-loam-dense-profile` | 1.011 % | 0.528 % | 0.7 |
| + 0.22 deg correction (`dense_elevation`) | 0.598 % | 0.419 % | 0.36 |
| + nearest facet (`dense_elevation_nearest_facet`) | **0.556 %** | 0.346 % | 0.63 |
| paper path + correction (`paper_odometry_elevation`) | 0.567 % | **0.340 %** | 0.29 |
| paper, Table I | 0.53 % | 0.31 % | |

The paper path (`--mesh-loam-paper-odometry`: 0.1 m input without a cap,
normals from 10 NN in 0.5 m cells, nearest facet) is 1.07x / 1.10x the paper
(geometric mean 1.08x, from 1.80x); its rotational error is 0.232 / 0.198
deg/100 m against 0.20 / 0.15. Most of the gain comes from the association
and the correction: the dense profile with both reaches the same ratio at
twice the speed.

On the first 400 frames of seq 07, a 120 m range (0.410 % vs 0.409 %) and a
mesh update every 2 frames (0.413 %) change nothing. Keeping the global mesh
past the 80 m prune (`--mesh-loam-keep-mesh`, Eq. 3) exceeds 10 GB on seq 07
with marching-tetrahedra facets and was not evaluated.

