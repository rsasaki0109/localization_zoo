# D²-LIO (Directional Degeneracy)

## Paper
- "D²-LIO: Enhanced Optimization for LiDAR-IMU Odometry Considering Directional
  Degeneracy", 2025
- arXiv: https://arxiv.org/abs/2508.14355
- **No public reference implementation** released by the authors as of this
  writing — this is a from-paper reimplementation.

## What This Repository Implements

A LiDAR-inertial scan-to-submap odometry built on the project's voxel-hash map
and IMU preintegration, with D²-LIO's three contributions:

1. **Adaptive per-point outlier-removal threshold** — instead of a global gate, a
   correspondence is accepted only if its point-to-map distance is within
   `ε_j = base + ‖t_rel‖ + 2·‖p_j‖·sin(½·‖θ_rel‖)`, where `t_rel` / `θ_rel` are
   the predicted inter-frame translation / rotation. Far points (large `‖p_j‖`)
   get a wider gate during rotation, matching the larger motion distortion they
   experience.
2. **Scan-to-submap registration with IMU prior** — the local voxel-hash submap
   is the registration target; gyro preintegration provides the rotation prior
   for the initial guess (with online gyro-bias correction, as in RKO-LIO).
3. **Degeneracy-aware hybrid weighting** — the rotation (0–2) and translation
   (3–5) blocks of the point-to-plane Hessian `AᵀA` are eigendecomposed
   separately. A direction is flagged ill-conditioned by a **scale-invariant
   relative criterion** `λ_k < degeneracy_ratio · λ_max` (condition-number based,
   so it fires consistently regardless of point count / scene scale). On a
   flagged direction a prior pulling the solution toward the IMU prediction is
   added, with strength scaled by the degeneracy gap, the IMU information
   projected onto that direction, and `λ_max` — an approximation of the paper's
   `W = (AᵀA·P)⁻¹` hybrid weight. On well-conditioned scenes no direction is
   flagged and the regularizer stays dormant (no bias). The prior is applied
   only when a real IMU packet was integrated for the frame; no-IMU runs still
   report the degenerate directions but do not regularize toward the
   constant-velocity fallback prediction.

## Current Scope

- self-contained core library (no ROS dependency)
- point-to-plane cost with a Cauchy robust kernel; 1-D point-to-point fallback
  where the local surface is non-planar
- rotation / translation degeneracy handled in separate 3×3 sub-blocks because
  of their scale disparity (as the paper notes)

## Deviations / Not Included Yet

- the paper leaves the regularization weight `w`, eigenvalue degeneracy
  threshold, and IMU noise covariances unspecified; these are exposed as
  parameters with repo-consistent defaults
- the translation prior uses an isotropic information matrix (the IMU
  translation covariance is not gravity/bias-corrected in this compact pipeline)
- no full tightly-coupled IEKF / sliding-window back-end — IMU enters as a
  rotation prior plus the degeneracy regularizer, not a joint state estimator
- without `imu.csv`, the KITTI dogfooding path is LiDAR-only scan-to-submap with
  degeneracy diagnostics; directional regularization is disabled because the
  constant-velocity fallback is not an inertial measurement
