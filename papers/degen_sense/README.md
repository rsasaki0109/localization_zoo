# Degeneracy Sensing & Compensation (Degeneracy Factor + DBSCAN)

## Paper
- "A Real-time Degeneracy Sensing and Compensation Method for Enhanced LiDAR
  SLAM", IEEE T-ITS 2024
- arXiv: https://arxiv.org/abs/2412.07513
- **No public reference implementation** released by the authors — this is a
  from-paper reimplementation.

## What This Repository Implements

A LiDAR-(IMU) scan-to-map odometry with the paper's three contributions:

1. **Degeneracy factor** — after each registration, the converged point-to-plane
   Hessian is split into rotation (0–2) and translation (3–5) blocks; the
   degeneracy factor is the **condition number** `S = λ_max / λ_min` of each
   block. A larger `S` means poorer observability in some direction.
2. **Adaptive degeneracy perception (DBSCAN → robust-outlier approximation)** —
   the paper clusters the time-series of degeneracy factors with DBSCAN so that a
   sudden spike becomes a "noise point" outlier, detected without a manual
   threshold. This repo approximates that 1-D time-series outlier test with a
   **median + k·MAD** rule over a rolling buffer (same intent: no hand-set
   threshold, adapts to the run). A frame is flagged degenerate when its factor
   exceeds `median + k·MAD`.
3. **Degeneracy compensation by sensor fusion** — when a frame is degenerate, the
   LiDAR solution is blended with the IMU motion prediction using a weight `1/S`:
   `t = (1 − 1/S_t)·t_IMU + (1/S_t)·t_LiDAR` for translation and the
   corresponding SLERP for rotation. Higher degeneracy ⇒ more IMU, less reliance
   on the degraded LiDAR estimate.

## Current Scope

- self-contained core library (no ROS dependency)
- point-to-plane registration with a constant-velocity + IMU-gyro rotation prior
  (online gyro-bias correction); falls back to constant velocity without IMU
- exposes the degeneracy factors and the per-frame degenerate flag in the result

## Deviations / Not Included Yet

- the paper's exact **DBSCAN** (with k-distance-graph `Eps` selection) is
  approximated by a median+MAD robust-outlier test on the same 1-D
  degeneracy-factor time series; the adaptive, threshold-free intent is kept
- no tightly-coupled IEKF back-end — compensation is a pose-level LiDAR/IMU blend
  rather than a filter update
- parameters the paper leaves open (MAD multiplier, buffer length, fusion-weight
  floor) are exposed with repo-consistent defaults
