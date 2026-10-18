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
`--mesh-loam-nearest-facet`, `--mesh-loam-keep-mesh`, `--mesh-loam-marching-cubes`,
`--mesh-loam-freeze-radius`, `--mesh-loam-full-block-meshing`, `--mesh-loam-voxel-size`,
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
| paper path + correction (`paper_odometry_elevation`) | 0.565 % | **0.342 %** | 1.2 |
| + global mesh kept (`paper_odometry_elevation_keep_mesh`, ablation) | 0.568 % | 0.339 % | 1.5 |
| paper, Table I | 0.53 % | 0.31 % | |

On the historical 00/07 development comparison above, the paper path
(`--mesh-loam-paper-odometry`: 0.1 m input without a cap, normals from 10 NN
in 0.5 m cells, nearest facet) is 1.07x / 1.10x the paper
(geometric mean 1.08x, from 1.80x); its rotational error is 0.231 / 0.200
deg/100 m against 0.20 / 0.15. Most of the gain comes from the association
and the correction: the dense profile with both reaches the same ratio. The
paper path ran at 0.29 FPS before the speed work below (0.567 / 0.340 %);
the rows above are with the current code. FPS on a shared 4-core host.

On the first 400 frames of seq 07, a 120 m range (0.410 % vs 0.409 %) and a
mesh update every 2 frames (0.413 %) change nothing. Keeping the global mesh
past the 80 m prune (`--mesh-loam-keep-mesh`, Eq. 3) used to exceed 10 GB on
seq 07; with marching cubes and frozen far bins
(`--mesh-loam-marching-cubes --mesh-loam-freeze-radius 100`) the full seq 07
run peaks at 1.17 GB and gives 0.339 % (00: 0.568 %), the same as without it:
the revisits in 07 and 00 do not gain from the kept mesh here.

### KITTI 00-10 with one configuration

The coverage variant is `paper_odometry_elevation`: `--no-gt-seed
--mesh-loam-paper-odometry --input-vertical-angle-correction-deg 0.22`.
It is frozen from the prior 00/07 comparisons, including the 07 window
checks above. Both 00 and 07 are development sequences; 01-06 and 08-10
are used only for evaluation, with the same arguments on every sequence.
The correction is a KITTI preprocessing choice; the Mesh-LOAM paper does
not specify it. This evaluation does not select a new correction or a
different configuration on the added sequences.

The added manifests are
`experiments/mesh_loam_kitti_seq_NN_full_paper_comp_matrix.json`, each with
only that variant. The 00/07 comparison manifests also contain historical
ablations; to rerun just the fixed setting on any sequence, use:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full \
  experiments/reference_data/kitti_seq_NN_full_gt.csv 0 \
  --methods mesh_loam --no-gt-seed --mesh-loam-paper-odometry \
  --input-vertical-angle-correction-deg 0.22 --summary-json mesh_seq_NN.json
```

See [the roadmap](../../docs/roadmap.md#experiments) for one-sequence-at-a-time
fetching, result import and regeneration of the derived tables.

<!-- FIXED-KITTI-RESULTS:START -->

Full sequences, official KITTI translational RTE (100-800 m), no GT seed.
The 00/07 rows retain their existing committed results; the other rows are
the added coverage runs. Every row below uses the same fixed variant.

| Seq | Paper RTE [%] | Fixed RTE [%] | Repo / paper | FPS |
|---|---:|---:|---:|---:|
| 00 (development) | 0.53 | 0.565 | 1.07x | 1.069 |
| 01 | 0.64 | 0.857 | 1.34x | 0.195 |
| 02 | 0.52 | 0.607 | 1.17x | 0.327 |
| 03 | 0.63 | 0.928 | 1.47x | 0.260 |
| 04 | 0.41 | 0.820 | 2.00x | 0.238 |
| 05 | 0.34 | 0.345 | 1.01x | 0.340 |
| 06 | 0.35 | 0.418 | 1.19x | 0.374 |
| 07 (development) | 0.31 | 0.342 | 1.10x | 1.229 |
| 08 | 0.82 | 0.909 | 1.11x | 0.380 |
| 09 | 0.47 | 0.631 | 1.34x | 0.406 |
| 10 | 0.56 | 0.848 | 1.51x | 0.553 |

Geometric mean over all sequences with paper values: **1.28x**
with the fixed setting. Table 6 is restricted to this same variant by
`repo_variants` in `evaluation/data/paper_reported_numbers.json`.
The historical 00/07 ablations above remain available in their original
result aggregates.

FPS describes the recorded execution: the new runs use two OpenMP
threads per method and may overlap on the same sequence. The 00/07 FPS
comes from the historical runs.

Sources: `experiments/results/mesh_loam_kitti_seq_NN_full_paper_comp_matrix.json`.
New per-run summaries, host records, logs, estimated poses, evaluated GT
and step diagnostics are committed under the corresponding
`experiments/results/runs/` directory. Exact execution commands and the
isolated-output launcher used for overlapping the two methods are recorded
in `experiments/results/mesh_inten_kitti_coverage/`.

#### Sequences above twice the paper value

The strict >2x check uses the unrounded RTE. Ratios displayed as
2.00x can remain at or below that threshold:

- Seq 04: 0.819834 % <= 0.820000 % (twice the paper value).

No sequence exceeds twice its paper value with this fixed setting.

Saved-pose diagnostics are generated with
`evaluation/scripts/diagnose_kitti_steps.py --estimate <trajectory_kitti.txt>
--gt <evaluated_gt_kitti.txt> --summary <summary.json> --method "Mesh-LOAM"
--output <step_diagnostics.json>`; each JSON records the exact command.
The script checks its full-sequence RTE against the original run summary.

<!-- FIXED-KITTI-RESULTS:END -->

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
200-399 (61 k facets per frame with tetrahedra) to all 1101 frames. The full
seq 07 run with marching cubes and freeze 100 m peaked at 1.17 GB RSS. Before
this change, `keep_mesh` with tetrahedra exceeded 10 GB on seq 07.

Recommended values for the paper path (runner flags `--mesh-loam-marching-cubes`,
`--mesh-loam-full-block-meshing`, `--mesh-loam-freeze-radius`):
`mesh_dirty_cells = true` (the default). Keep `marching_cubes = false` for
accuracy runs. For `keep_mesh`, use `marching_cubes = true` and
`mesh_freeze_radius = 100`.

## Rejected acceleration experiments

Mesh-LOAM retains its original implementation. `--paper-pipeline-accelerated`
currently applies to the IMLS-SLAM and InTEn-LOAM paper paths. The Mesh tree,
packet cache and bin-normal-bound candidates are excluded after the full
normal-bound measurement was slower than its fresh control. Historical
benchmark results and the original mesh mechanism tests are preserved.

To reproduce the rejected normal-bound candidate, check out implementation
`f47555735c94d43d6f8e1554ee7bf50be77ac159`, build its recorded Release target,
and use the exact opt-in invocation in the linked result receipt. Its changed
association code was opt-in and default off. The retained Mesh C++ sources
match the frozen baseline; the experiment results below describe that recorded
candidate rather than an enabled optimization in the current implementation.

The rejected bounding-volume-tree candidate (`fe607199`) took 168128.332 ms
against 117876.891 ms for the frozen baseline on the first 120 KITTI 07 frames:
**0.70x**, slower despite identical saved poses. The changed binary's default
path took 105839.854 ms and also retained the poses. These are development-window
measurements, not a full-sequence speed claim. The spatial tree was rejected. The raw runs and exact
commands were committed in `8479040a` under
[`candidates/fe607199/`](../../experiments/results/paper_pipeline_speed/candidates/fe607199/).

The later packet-cache candidate (`fe03ed88`) took 76263.959 ms against
66420.751 ms for the frozen control and 60205.016 ms for the changed default:
**0.87x**, again with identical saved poses on the first 120 frames. Its raw
screening was committed in `0560f22e`. The per-facet packet cache is also
rejected; the full-sequence normal-bound measurement is below.

The packet cache also cost memory. In the freeze fixture, candidate `3541f773`
reported 12498398 estimated bytes with acceleration and freeze, versus 12294436
with neither. This mixed-cache comparison failed the previous memory assertion;
the experiment corrected the test to use equal cache settings. The retained
implementation uses the original uncached freeze test. The failed full-CTest receipt and memory estimates
are preserved in `6235f89c` under
[`candidates/3541f773/`](../../experiments/results/paper_pipeline_speed/candidates/3541f773/).

Repeated development windows (first 120 KITTI 07 frames, four workers,
profiler disabled; every source and exact command is in the linked evidence):

| Source | Frozen control [s] | Changed default [s] | Opt-in [s] | Control / opt-in |
|---|---:|---:|---:|---:|
| `fe607199` | 117.877 | 105.840 | 168.128 | 0.70x |
| `fe03ed88` | 66.421 | 60.205 | 76.264 | 0.87x |
| `67867d0b` | 95.673 | 92.271 | 111.466 | 0.86x |

All saved poses agree with the frozen control at the saved precision (nine
significant digits). Execution time varies substantially, including between
unchanged method implementations. These windows do not establish the full
speed target. The primary full comparison uses fresh immediately preceding
controls. Source `67867d0b` raw screening is committed in `f3c6b9f3`.

[Execution protocol and CPU profiles](../../experiments/results/paper_pipeline_speed/README.md)
and [full comparison](../../experiments/results/paper_pipeline_speed/comparisons/f4755573.json).

Full KITTI 07 at measured candidate `f4755573` and control `134ead2f`,
identical selected registration arguments, four OpenMP workers, one method at a
time on the same recorded host; the profiler is disabled. These times belong
to those revisions and are not remeasured by the later shared-component integration:

| Execution | Time [s] | FPS | Official RTE [%] | ATE [m] |
|---|---:|---:|---:|---:|
| Paired frozen control | 1165.57 | 0.945 | 0.341905 | 0.855800 |
| Rejected normal-bound candidate | 1397.59 | 0.788 | 0.341905 | 0.855800 |

Measured speedup: **0.83x**. Maximum translation difference from the
paired frozen control: 0.000000000 m.
Pose differences use the saved KITTI matrices (nine significant digits).
Official RTE agrees to the third decimal.

**Negative result:** this attempt does not meet the roadmap speed target.
The Mesh candidate is excluded; the original implementation is retained.
The roadmap item is still open.

KITTI 07 guided implementation and is not held out. This comparison uses a
frozen-source full control run immediately before acceleration on the same host
and thread count. Historical
Table 6 results retain their original evidence and are not replaced.

The rejected candidate's [final CPU profile](../../experiments/results/paper_pipeline_speed/runs/f4755573_profile_accelerated_mesh_loam/cpu_profile.txt)
was recorded after all unprofiled full measurements and committed in `82da76c0`.
The outlined facet-association body accounts for 47.5% of sampled CPU
on the first development window. This locates remaining association work; it
is early-sequence CPU attribution, not a full-sequence stage wall-time fraction.
Profiled time is excluded from the speed comparison.

<!-- SHARED_NEAR_RANGE_AUDIT -->
## Shared-loader replay: historical KITTI 04 difference

The full 04 stored-cell replay after the optional near-range filter extraction
uses the original `paper_odometry_elevation` arguments, with the cut off.
Its 100 m RPE is **0.520627%**, ATE **10.252902 m**, versus stored **0.523782%**
and **10.602073 m**, with all 271 frames complete. This improves the metrics
but remains a reproduction difference: the historical audit exits 1.

The frozen pre-extraction executable reproduces the same values. RPE, ATE,
frame count and official translation/rotation RTE differ by zero between
old and extracted implementations, and saved poses are byte-identical.
The difference therefore predates extraction; the reason the original
historical run differs remains unresolved. It is not credited as a filter
improvement or used to replace the fixed-setting paper result.

[Commands, actual exits, summaries and logs](../../experiments/results/shared_near_range/sequences/04/audit/),
[frozen replay](../../experiments/results/shared_near_range/sequences/04/frozen_baseline/)
and [comparison](../../experiments/results/shared_near_range/sequences/04/frozen_comparison.json)
were committed at `7458c24d16096cc305889ed0864a5ef8bfb6bdf4`.
The [whole-zoo audit](../../experiments/results/shared_near_range/README.md)
is incomplete. Historical aggregates and the default-off cutoff are unchanged.
<!-- /SHARED_NEAR_RANGE_AUDIT -->

<!-- SHARED_NEAR_RANGE_00_PRIMARY -->
## Shared-loader replay: KITTI 00 frozen comparison complete

The full 00 replay of the stored `paper_odometry_elevation` arguments completes all
4541 frames, with the optional near-range cut still off. The historical
100 m RPE changes from **0.773243%** to
**0.774034%**; ATE changes from **10.180105 m**
to **10.099834 m**. RPE worsens. ATE improves, so these outcomes are retained separately.
The native process exits 0, but the historical audit exits **1**.

The frozen pre-extraction replay also completes with native exit 0. RPE,
ATE, frame count and official translation/rotation RTE have **zero** recorded
differences from the extracted implementation, with byte-identical saved poses.
This historical difference therefore predates extraction; its original cause
remains unresolved. The frozen historical audit also exits 1. No setting is
reselected, and the stored aggregate and published paper comparison are preserved.

[Completed candidate commands, actual exits, logs and summaries](../../experiments/results/shared_near_range/sequences/00/audit/)
were committed at `485fa00601644731827dae601eb3b43c7543efeb`.
The full [frozen comparison](../../experiments/results/shared_near_range/sequences/00/frozen_comparison.json)
and [verified input cleanup](../../experiments/results/shared_near_range/sequences/00/cleanup.json)
were committed at `10cf31616365b41878ec9311f0b705e798716c51`.
The [committed checkpoint](../../experiments/results/shared_near_range/checkpoints/00/summary.json)
now covers cleaned 04 and 00; the whole-zoo audit remains incomplete.
Defaults remain off.
<!-- /SHARED_NEAR_RANGE_00_PRIMARY -->

<!-- SHARED_NEAR_RANGE_00_DIAGNOSTICS -->
## KITTI 00 current-replay error growth

The committed current replay has 14/4514 moving steps with length error above 20%, and 43/4514 with local translation-vector error above 20%. The existing diagnostic uses the same 100-frame blocks and GT step threshold of 0.1 m for every method.

The largest increase in pointwise position error among those blocks is poses 3800–3900: GT travels 105.799422 m, while error rises from 13.138734 m to 18.361150 m (increase 5.222416 m). These frame blocks diagnose the current trajectory; they are **not official KITTI RTE**. The tool uses first-pose normalization without fitting or rescaling.

The referenced historical per-run summary is absent at the inspected revision and was not found in this checkout's reachable history. Without its historical saved path, this does not locate the cause of the historical metric difference. [Plot, full trace, commands and limits](../../experiments/results/shared_near_range/diagnostics/00/README.md) use the committed current poses, with diagnostic evidence committed at `61c1f26e1019aa3f0b05b100404c8bb117448b41`. Native source remains `0b079dbbcb7c6f58c1d8367ade881c15f003e525`.
<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader replay: KITTI 01 frozen comparison complete

The full stored `paper_odometry_elevation` replay completes all 1101 frames with
native exit 0. Historical 100 m RPE is **0.849506%**, versus
**0.850998%** in this replay; ATE is **30.862639 m**
versus **30.940997 m**. RPE worsens; ATE
worsens. The historical audit exits **1**, preserving the difference.

The frozen pre-extraction replay also completes with native exit 0. RPE,
ATE, frames and official translation/rotation RTE have zero recorded differences
from the extracted implementation, with byte-identical saved poses. This
historical difference predates extraction; its original cause remains unresolved.
The frozen historical audit also exits 1. No setting is reselected, and the
near-range cut remains off.

[Commands, exits, hashes and candidate results](../../experiments/results/shared_near_range/sequences/01/audit/)
were committed at `9555f01fda68a0294fc7dd5a828a85fac207f68f`. The
[study and cleaned checkpoint scope](../../experiments/results/shared_near_range/README.md)
now include the full [frozen comparison](../../experiments/results/shared_near_range/sequences/01/frozen_comparison.json)
and [verified input cleanup](../../experiments/results/shared_near_range/sequences/01/cleanup.json),
committed at `6188b71d8e48933a4dffce54afdde7a01382f4b0`. The cleaned
[04/00/01 checkpoint](../../experiments/results/shared_near_range/checkpoints/01/summary.json)
remains partial whole-zoo coverage.
<!-- /SHARED_NEAR_RANGE_REPLAY -->

<!-- SHARED_NEAR_RANGE_01_DIAGNOSTICS -->
### Saved KITTI 01 path diagnosis

The current path's largest position-error increase in the existing fixed
blocks is frames **700-800**:
**33.154975 to 42.799191 m**
against evaluated GT. This locates current error growth, not the historical
metric difference.

The historical/current saved paths use byte-identical evaluated GT. Their
position gap reaches **0.906842 m** at frame
**947** and ends at **0.770865 m**.
Its largest fixed-block increase is frames **600-700**,
from **0.378576 to 0.569314 m**.
The recorded original and current OpenMP thread settings are **2 and 1**;
the method source/header match, but no controlled thread-count experiment
is performed. These observations do not establish the cause of the difference.

The [diagnostic table, plot, CSVs, source pins and exact command](../../experiments/results/shared_near_range/diagnostics/01/README.md)
come from committed evidence `1426a851c9f021cbffae239e814b6379a5788b25`. First-pose normalization
is used without fitted alignment or rescaling. No estimator is rerun and no
published accuracy result is replaced.
<!-- /SHARED_NEAR_RANGE_01_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_02_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI02 difference and saved-path diagnosis

| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| Mesh-LOAM | 02 | 0.785133 | 0.785817 | 22.288359 | 22.328196 |

The selected `paper_odometry_elevation` arguments are unchanged throughout this full
sequence. The historical audit retains exit **1**; native executions exit **0**.
Frozen pre-extraction controls have zero recorded metric differences and
byte-identical poses. This historical difference predates extraction; its
original cause remains unresolved. Input hashes were verified before owned
scan/alias removal. The near-range cut stays disabled.

The largest current position-error increase among the inherited fixed blocks
is frames **4400-4500**:
**22.548455 to 33.673883 m**.

Historical/current GT bytes match. The path gap reaches **0.252810 m**
at frame **2649**, ending at **0.092806 m**. Its largest block
increase is **1900-2000**:
**0.116048 to 0.143659 m**.
Identical method sources and differing OpenMP settings (**2 / 1**) do not isolate
runtime as the cause.

The [protocol, all steps/CSVs and plot](../../experiments/results/shared_near_range/diagnostics/02/README.md)
come from committed evidence `8f5841e3946995cedfa9c0e2fc484dbd2c02373e`. First-pose normalization uses no fitted
alignment or rescaling. These observations locate errors without assigning a
cause or replacing published accuracy. The
[cleaned checkpoint](../../experiments/results/shared_near_range/checkpoints/03/summary.json)
retains all historical differences and audit exits.
<!-- /SHARED_NEAR_RANGE_02_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_03_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI03 historical difference

| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| Mesh-LOAM | 03 | 0.877028 | 0.877431 | 3.992708 | 3.995116 |

RPE and ATE worsen, retaining historical audit exit **1**. The frozen control
has native exit **0**, zero recorded metric differences and byte-identical
poses; this difference also predates extraction. Its cause remains unresolved.
Owned-scan hashes were verified before scan/alias removal. Settings and disabled
defaults are unchanged. [Audit/control/cleanup and actual commands](../../experiments/results/shared_near_range/sequences/03/)
were committed at `eebaf3d7` before this prose.

[Saved03 trajectory diagnosis](../../experiments/results/shared_near_range/diagnostics/03/README.md),
with raw evidence committed at `bfad039e961ead539c590b07f918169bf1450afc` before this prose, locates
the historical/replay path gap. Its maximum is **0.024541 m** at frame
**782**, ending at **0.022879 m**. The largest fixed
**100**-step gap increase is frames **600-700**:
**0.013161 to 0.019651 m**.
Current GT position error in that block grows from
**4.782073 to 7.415219 m**; these are distinct quantities.
Evaluated GT bytes and the recorded Mesh core match. Original/current OpenMP
counts **2 / 1** (OpenBLAS **1**) remain a runtime confound, without cause
attribution. Diagnostic parameters transfer unchanged from 02/01; native settings,
historical audit exit **1** and published results remain unchanged.
<!-- /SHARED_NEAR_RANGE_03_DIAGNOSTICS -->

## Acceleration integration with shared components

The current-main compatibility evidence, committed in `7619f198`, compares
frozen main, merged default and merged global-flag modes on the same 3 real
MCD scans with the historical fixed arguments. Both merged modes have zero
recorded accuracy/frame differences from main and byte-identical saved poses.
The global flag is ignored by Mesh; its rejected acceleration remains excluded.
These short executions check compatibility; they do not establish long-segment
KITTI RTE or remeasure full-sequence throughput. The speed figures above remain
tied to their recorded sources. [Commands, binary/input hashes, comparison and
limits](../../experiments/results/paper_pipeline_speed/README.md#integration-with-merged-shared-components).
