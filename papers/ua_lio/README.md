# UA-LIO (Uncertainty-Aware, Distribution-to-Distribution)

## Paper
- "UA-LIO: An Uncertainty-Aware LiDAR-Inertial Odometry for Autonomous Driving in
  Urban Environments", IEEE T-IM 2025 (DOI 10.1109/TIM.2025.3544286)
- Project page: https://hfut-liuliu.com/
- The authors state the code will be open-sourced, but **no public repository is
  available yet** — this is a from-paper reimplementation.

## What This Repository Implements

A LiDAR-inertial scan-to-map odometry with UA-LIO's three ideas:

1. **Gaussian voxel map (distribution representation)** — each voxel
   incrementally accumulates a Gaussian `(μ, Σ)` of its points; the smallest
   eigenvector gives the surface normal and plane thickness.
2. **Distribution-to-distribution, uncertainty-aware update** — each measured
   point carries a range/bearing covariance `Σ_p`, propagated through the pose as
   `R Σ_p Rᵀ`. The point-to-plane residual `r = nᵀ(p − μ)` is weighted by the
   inverse of the combined distribution variance along the normal,
   `1 / (nᵀ(R Σ_p Rᵀ + Σ_voxel)·n)` — uncertain points and thick surfaces are
   down-weighted.
3. **Ground-plane constraint** — voxels whose normal is near vertical
   (`|n_z| ≥ ground_normal_z_min`) add an extra z-direction residual
   `r_z = e_zᵀ(p − μ)`, which suppresses the z-drift that plagues urban driving.

## Current Scope

- self-contained core library (no ROS dependency)
- gyro-preintegration rotation prior with online bias correction; constant
  velocity fallback without IMU
- reports the number of ground-constrained cells per frame

## Deviations / Not Included Yet

- a full **IESKF** tightly-coupled state update is approximated by the
  uncertainty-weighted Gauss-Newton solve (the D2D uncertainty weighting and the
  ground constraint — the paper's contributions — are kept)
- the exact urban ground-segmentation pipeline is replaced by a normal-direction
  test on the Gaussian voxels
- range/bearing noise constants and the ground weight are exposed as parameters
