# CT-VoxelMap (Probabilistic Adaptive Voxel Mapping)

## Paper
- "CT-VoxelMap: Efficient Continuous-Time LiDAR-Inertial Odometry with
  Probabilistic Adaptive Voxel Mapping", 2026
- arXiv: https://arxiv.org/abs/2604.03747 (arXiv:2604.03747)
- The paper states the code "will be open-sourced" but **no implementation is
  available yet** — this is a from-paper reimplementation of its core idea.

## What This Repository Implements

CT-VoxelMap's headline contribution — the **probabilistic adaptive voxel map with
uncertainty-propagated hybrid registration** — implemented as a self-contained
scan-to-map odometry front-end:

1. **Probabilistic adaptive voxel map** — every voxel incrementally accumulates a
   Gaussian `(μ, Σ)` of the points it contains. A voxel is classified as a
   **plane feature** when `λ_min < planarity_ratio · λ_max` (storing normal `n`,
   point `q = μ`, and plane thickness `σ_plane² = λ_min`); otherwise it is kept
   as a **distribution feature** `(μ, Σ)`. This is the paper's hybrid
   plane/voxel feature set.
2. **Point uncertainty propagation** — each measured point gets a range/bearing
   covariance `Σ_p = σ_tan²·I + (σ_depth² − σ_tan²)·d̂d̂ᵀ` with
   `σ_tan = d·σ_bearing`, propagated through the pose as `R Σ_p Rᵀ`.
3. **Uncertainty-weighted registration** —
   - plane voxels: Mahalanobis point-to-plane, residual `r = nᵀ(p − μ)` with
     variance `nᵀ R Σ_p Rᵀ n + σ_plane²`, weight `1/var`;
   - distribution voxels: GICP-style point-to-distribution with information
     `(R Σ_p Rᵀ + Σ_voxel)⁻¹`.

## Current Scope

- self-contained core library (no ROS dependency)
- incremental voxel statistics (running `sum` and `sum_outer`), so the map mean /
  covariance / plane fit update online as points stream in
- 27-neighborhood feature lookup so points near voxel borders still find a
  surface

## Deviations / Not Included Yet

This is intentionally the **registration + probabilistic-mapping front-end**, not
the full paper system. Not implemented here:

- the **continuous-time B-spline** trajectory with control-point increments on
  matrix Lie groups, and the **spline fitting-error modeling** via IMU
  forward-propagation — the harness feeds one merged cloud per frame without
  per-point timestamps, so intra-scan continuous-time deskew is out of scope;
  continuous-time motion is approximated by a constant-velocity prediction
- the **IEKF** tightly-coupled IMU fusion back-end
- the **re-estimation policy** that decomposes the m-dimensional problem into
  k sub-problems
- true adaptive octree voxel subdivision (a single voxel size with the
  plane/distribution split approximates the adaptivity)

These are tracked as future work; the probabilistic voxel map and
uncertainty-weighted registration are the transferable core that this repo adds.
