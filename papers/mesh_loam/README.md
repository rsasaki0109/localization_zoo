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
- **Incremental zero-surface extraction** (Sec. III-C4): every
  `mesh_update_interval` frames, the cells with a corner voxel updated since
  the last extraction are re-meshed from the SDF zero set with linear
  interpolation (`mesh_dirty_cells`; `false` re-meshes every cell of each
  touched 8³ block, the earlier behaviour).
- **Point-to-mesh odometry** (Sec. III-B): constant-velocity prediction,
  planar-feature selection by PCA curvature (`c < 0.1`), facet candidates from
  a 0.2 m search grid, correspondence gate by normal cosine similarity
  (`|cos| > 0.98`), point-to-facet-plane Gauss-Newton (Eqs. 3–4).

## Deviations from the paper

- **Marching tetrahedra by default** for the zero-surface (6 tetrahedra per
  cell, table-free): same linearly-interpolated zero crossing, about 3x more
  and smaller triangles. `marching_cubes = true` uses the standard
  Lorensen-Cline table as in the paper; on KITTI 07 it matches tetrahedra on
  frames 400-799 and is 0.002 points behind on frames 0-399 (see below), so
  tetrahedra stay the default.
- **CPU, `std::unordered_map` of dense 8³ voxel blocks**: the paper's
  parallel spatial hashing (open addressing, distributed locks, Algorithm 1)
  and the voxel deletion scheme exist to fit GPU memory and avoid hash
  dropout; a chained hash map needs neither. SDF integration, mesh
  extraction, normals and registration run on OpenMP threads and give the
  same result for any thread count except the registration sums. Far map
  content is dropped by a radius prune instead (`local_map_radius`).
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
Normals, SDF integration, mesh extraction and the registration loop use
OpenMP (`OMP_NUM_THREADS`). `MeshLoamParams` also has `mesh_dirty_cells`,
`marching_cubes` and `mesh_freeze_radius`, which have no runner flags yet
(see "Speed and memory").

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
past the 80 m prune (`--mesh-loam-keep-mesh`, Eq. 3) used to exceed 10 GB on
seq 07. The section below covers its current memory use.

## Speed and memory

Measured with a scratch driver that loads KITTI 07 `.bin` scans like the
harness (0.22 deg correction, PCL voxel grid, 1-80 m) on a shared 4-core
machine. The driver computes the same official KITTI RTE as the harness, and
it reproduces the reference 400-frame numbers (dense + nearest facet
0.430 %, paper path 0.409 %).

Per-frame time on the paper path. "1 thread" is frames 200-239, with the old
and new code run side by side. "2 threads" is frames 0-399 on a loaded
machine (load about 6 on 4 cores):

| stage [ms/frame] | before, 1 thread | after, 1 thread | before, 2 threads | after, 2 threads |
|---|---:|---:|---:|---:|
| normals (kNN PCA) | 130 | 85 | 135 | 106 |
| point-to-mesh GN | 1421 | 465 | 2037 | 868 |
| SDF integration | 251 | 45 | 295 | 61 |
| mesh extraction | 378 | 191 | 549 | 239 |
| radius prune | 36 | 5 | 105 | 16 |
| **total** | **2217** | **793** | **3126** | **1294** |
| peak RSS (400 frames) | | | 4.13 GB | 1.36 GB |

What changed:

- **Voxel map** in dense 8³ blocks (float SDF, weight and normal) instead of
  one hash node per voxel. SDF integration is parallel and deterministic:
  each thread applies every point's updates, in scan order, to the blocks
  it owns.
- **Incremental meshing** (`mesh_dirty_cells`, default on): only cells with
  an updated corner are re-meshed. On frames 0-399 that is 70 M cell
  extractions instead of 222 M for whole blocks, with the same RTE
  (0.4113 vs 0.4112 %). A unit test checks that the incremental mesh equals
  a full re-extraction.
- **Compact facets**, stored in their 0.2 m search bin: float vertices
  relative to the bin origin, float normal and a 3-byte owner cell, 52 B
  per facet. The centroid is derived from the vertices.
- **Nearest-facet search**: per-bin vertex AABB and largest triangle radius,
  bins visited nearest first, and float lower bounds (plane distance,
  centroid distance) with a margin before the exact double test. A
  brute-force check over the 27 bins finds the same nearest distance for
  every query.
- **Normals**: exact kNN within the same 27-cell window, with cells visited
  nearest first and an early stop.

Accuracy on KITTI 07 frames 0-399 (official RTE %, rotation in deg/100 m):

| variant | before | after |
|---|---:|---:|
| paper path | 0.409 / 0.263 | 0.412 / 0.265 |
| paper path, `marching_cubes` | | 0.414 / 0.265 |
| dense profile + nearest facet | 0.430 / 0.220 | 0.425 / 0.218 |
| dense profile (default association) | 0.532 / 0.389 | 0.507 / 0.393 |

On frames 400-799 the paper path gives 0.469 % with tetrahedra and 0.469 %
with marching cubes. Marching cubes produces 3x fewer facets (3.7 M instead
of 11.1 M in the local map) and peaks at 0.72 GB instead of 1.36 GB.

The default association (no `--mesh-loam-nearest-facet`) runs about 25
Gauss-Newton iterations per frame and is sensitive to rounding. On frames
400-799 it gives 0.508 % before and 0.507 % after.

The differences come from float storage: voxels, facets and the float
prefilter, which only reorders exact ties between facets that share an
edge. The pruning and the per-cell meshing do not change results.

**Global mesh (`keep_mesh`)**. Facets beyond the 80 m voxel prune are kept.
`mesh_freeze_radius` moves bins farther than that radius (plus 11 m of
hysteresis) to a compressed store and restores them when the pose comes
back within it. The store uses
16-bit vertices relative to the bin, with the normal recovered from the
winding, about 22 B per facet instead of about 70 B. The radius has to exceed
`max_range` plus the distance travelled between mesh updates. With 100 m,
frozen bins are never searched, so the trajectory is identical to keeping
everything searchable. Paper path, frames 0-399:

| keep_mesh variant | facets (frozen) | peak RSS | RTE % | full seq 07 (estimate) |
|---|---:|---:|---:|---:|
| tetrahedra | 22.9 M | 1.90 GB | 0.413 | about 5 GB |
| tetrahedra, freeze 100 m | 22.9 M (7.6 M) | 1.62 GB | 0.413 | about 2.8 GB |
| marching cubes, freeze 100 m | 7.5 M (2.4 M) | 0.83 GB | 0.415 | about 1.2 GB |

The full-sequence estimates extrapolate the kept-mesh growth of frames
200-399 (61 k facets per frame with tetrahedra) to all 1101 frames. They are
estimates, not runs. Before this change, `keep_mesh` with tetrahedra
exceeded 10 GB on seq 07.

Recommended values for the paper path (runner flags still to be added):
`mesh_dirty_cells = true` (the default). Keep `marching_cubes = false` for
accuracy runs. For `keep_mesh`, use `marching_cubes = true` and
`mesh_freeze_radius = 100`.

