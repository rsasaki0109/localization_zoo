# DAMM-LOAM (Degeneracy-Aware Multi-Metric LiDAR Odometry)

## Paper
- "DAMM-LOAM: Degeneracy Aware Multi-Metric LiDAR Odometry and Mapping",
  arXiv:2510.13287 (2025)
- **No public reference implementation** — this is a from-paper reimplementation.

## What This Repository Implements

A **pure-LiDAR** scan-to-map odometry with two contributions:

1. **Range-image normal-consensus point classification** — each point is labelled
   `ground` / `roof` / `wall` / `edge` / `non-planar` from its local PCA normal:
   planar points with a near-vertical normal (`|n_z| ≥ vertical_cos`) are ground
   (below the sensor) or roof (above); planar points with a near-horizontal
   normal (`|n_z| ≤ horizontal_cos`) are walls; high-linearity points are edges.
   `classifyScan()` exposes this for inspection.
2. **Degeneracy-aware multi-metric ICP** — planar classes use a point-to-plane
   residual, edges/non-planar use point-to-point. Each iteration builds the
   geometric translational Hessian block `H_tt = Σ nnᵀ` from the planar normals
   and eigen-decomposes it; a translation axis `k` is **degenerate** when
   `λ_k < degeneracy_ratio · λ_max`. Points whose normal aligns with a degenerate
   eigenvector are up-weighted (boost ∝ `|n·u_k|`), and the edge metric weight
   `α` is amplified when the scene is degenerate — so the rare informative
   constraints that fill the weak direction dominate the update.

## API

```cpp
DammLoamPipeline pipe(params);
pipe.setInitialPose(T0);
for (auto& scan : scans) {
  auto r = pipe.registerFrame(scan);   // r.pose, r.degenerate, r.counts
}
```

## Benchmark (`pcd_dogfooding`)

```
./pcd_dogfooding <pcd_dir> <gt_csv> --methods damm_loam [--damm-loam-dense-profile]
```

## Current Scope / Deviations

- the paper's spherical range-image projection is approximated by local PCA
  normals on the voxel-downsampled scan (same ground/wall/roof/edge taxonomy,
  no explicit image rasterization)
- the Scan-Context loop-closure back-end is intentionally dropped (loop closure
  lives in the separate `scan_context` / `dtd` modules); this module is
  front-end odometry only
- no IMU is used (pure LiDAR, constant-velocity prediction)
- thresholds (`vertical_cos`, `horizontal_cos`, `degeneracy_ratio`,
  `degeneracy_boost`, `edge_weight`) are exposed as parameters
