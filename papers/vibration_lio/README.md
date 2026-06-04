# Vibration-aware LIO (Post-Undistortion Uncertainty)

## Paper
- "Vibration-aware Lidar-Inertial Odometry based on Point-wise Post-Undistortion
  Uncertainty", IEEE 2025
- arXiv: https://arxiv.org/abs/2507.04311
- **No public reference implementation** released by the authors — this is a
  from-paper reimplementation.

## What This Repository Implements

A LiDAR-inertial scan-to-map odometry that models the residual distortion left
after motion-undistortion (caused by high-frequency vibration), as the paper
does:

1. **Point-wise post-undistortion covariance** — each point gets
   `Σ_p = ⌊p⌋× Σ_r ⌊p⌋×ᵀ + Σ_T + R·Σ_meas·Rᵀ` where
   - `Σ_r = σ_r²·I` is the **angular vibration** term (rotated through the point's
     skew, so far points get larger lateral uncertainty),
   - `Σ_T = σ_T²·I` is the **linear vibration** term,
   - `Σ_meas` is the usual range/bearing measurement covariance.
2. **Vibration-intensity estimation from IMU** — `σ_r = γ·δt·k_ω` and
   `σ_T = γ·δt·k_v`, where `k_ω`, `k_v` are the **mean absolute deviation** of the
   per-scan IMU angular velocity / acceleration (a proxy for how violently the
   platform is vibrating). `γ = 0.1` as in the paper for 100–200 Hz IMU.
3. **Uncertainty-weighted point-to-plane** — the point covariance is propagated
   through the pose and projected onto the plane normal,
   `R_j = nᵀ·R·Σ_p·Rᵀ·n + σ_plane²`, and the residual is weighted by `1/R_j`, so
   points that are uncertain under the current vibration level are down-weighted.

## Current Scope

- self-contained core library (no ROS dependency)
- gyro-preintegration rotation prior with online bias correction; falls back to
  constant velocity (and a vibration floor) without IMU
- reports the estimated angular / linear vibration intensity per frame

## Deviations / Not Included Yet

- a full **iterated Kalman filter** state update is approximated by the
  uncertainty-weighted Gauss-Newton solve (the Mahalanobis weighting — the core
  of the contribution — is faithful)
- the two-stage (Euclidean → Mahalanobis) candidate search is reduced to a single
  nearest-plane lookup with Mahalanobis weighting
- `k_v` uses raw acceleration MAD as the linear-vibration proxy (the paper
  derives it from linear velocity); `γ`, `scan_period` exposed as parameters
