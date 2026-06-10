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
- No deskew (none described in the paper either).

## Dogfooding

```bash
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] \
  --methods mesh_loam --no-gt-seed --mesh-loam-dense-profile
```

Flags: `--mesh-loam-{fast,dense}-profile`, `--mesh-loam-voxel-size`,
`--mesh-loam-cos-gate`, `--mesh-loam-mesh-interval`.

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
