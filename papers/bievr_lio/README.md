# BIEVR-LIO (Bump-Image-Enhanced Voxel Maps)

## Paper
- "BIEVR-LIO: Robust LiDAR-Inertial Odometry through Bump-Image-Enhanced Voxel
  Maps", 2026
- arXiv: https://arxiv.org/abs/2604.14421 (arXiv:2604.14421)
- The authors' implementation is now available at
  https://github.com/ethz-asl/BIEVR-LIO (BSD-3-Clause; inspected at
  `21121698f273d6fbfffca57546b940edb1de2ff0`, 2026-07-16).
- This directory remains a from-paper reimplementation. Do not treat its
  results as equivalent to upstream: it lacks upstream's map-informed
  dual-resolution sampling and inertial sliding-window backend.

## What This Repository Implements

BIEVR-LIO's headline idea — the **bump image (voxel-wise oriented height
image)** and the extra in-plane constraints it provides — as a scan-to-map
odometry front-end:

1. **Bump-image voxel map** — each voxel estimates a dominant plane (centroid
   `μ`, normal `n`) and a plane-aligned local frame (`z = n`, in-plane basis
   `ex/ey`). A `K×K` grid (`pixel_res`) stores the **height deviation** of the
   surface from the plane per cell (`I(u,v)`), weighted-averaged with
   `w = min(0.5, 1/‖p‖)`.
2. **Bump-augmented registration** — the residual is
   `r = nᵀ(p − μ) − I(u,v)` (point height minus the bilinearly-interpolated bump
   height). The Jacobian uses an **effective normal** `n − (g_u·ex + g_v·ey)`,
   where `g_u, g_v` are the height-image gradients — so beyond the usual
   point-to-plane (normal) constraint, the surface texture contributes **two
   in-plane constraint directions**. This is what lets the method stay
   observable in geometrically weak scenes.
3. **Huber-robust, variance-weighted** least squares.

## Current Scope

- self-contained core library (no ROS dependency)
- incremental voxel statistics; bump image rebuilt from recent points per voxel
- bilinear bump interpolation + gradient constraints for voxels under the query;
  neighbor voxels fall back to plain point-to-plane

## Deviations / Not Included Yet

- coarser `pixel_res` default (0.2 m) than the paper's 0.05 m for speed at the
  repo's voxel size (exposed as a parameter)
- the paper's **MID-based dual-resolution sampling** (keep the 300 most-detailed
  voxels at high resolution) is not implemented — a single downsample is used
- no IMU / sliding-window back-end — constant-velocity prediction stands in;
  the contribution reproduced here is the bump-image map + its in-plane
  constraints, not the full loosely-coupled estimator
