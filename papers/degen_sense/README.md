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
   on the degraded LiDAR estimate. Compensation is applied only when a real IMU
   packet was integrated for the frame; no-IMU runs still report degeneracy but
   do not blend toward the constant-velocity fallback prediction.

## Current Scope

- self-contained core library (no ROS dependency)
- point-to-plane registration with a constant-velocity + IMU-gyro rotation prior
  (online gyro-bias correction); falls back to constant velocity without IMU
- exposes the degeneracy factors and the per-frame degenerate flag in the result
- without `imu.csv`, the KITTI dogfooding path is LiDAR-only scan-to-map with
  degeneracy diagnostics; compensation is disabled because constant velocity is
  not an inertial reference

## Deviations / Not Included Yet

- the paper's exact **DBSCAN** (with k-distance-graph `Eps` selection) is
  approximated by a median+MAD robust-outlier test on the same 1-D
  degeneracy-factor time series; the adaptive, threshold-free intent is kept
- no tightly-coupled IEKF back-end — compensation is a pose-level LiDAR/IMU blend
  rather than a filter update
- parameters the paper leaves open (MAD multiplier, buffer length, fusion-weight
  floor) are exposed with repo-consistent defaults

## Public synchronized LiDAR-IMU validation (HDL-400 open)

120-frame public HDL-400 open window with `imu.csv` + `frame_timestamps.csv`:

```bash
python3 evaluation/scripts/run_lio_imu_public_validation.py --dataset hdl_400
```

| Variant | RPE | ATE | IMU path |
|---|---:|---:|---|
| DegenSense IMU on | 1.72% | 0.19 m | active |
| DegenSense no `imu.csv` | 1.60% | 0.19 m | fallback |

Paired summary: [`hdl_400_lio_imu_validation_summary.json`](../../docs/benchmarks/lio_imu_public/hdl_400_lio_imu_validation_summary.json).

IMU/LiDAR fusion compensation activates when `imu.csv` is present; metric deltas
vs the no-IMU fallback are small on HDL-400 open but NCLT 2013-01-10 shows ATE
0.16 m with IMU vs 0.24 m without (~45% worse off) — mechanism evidence, not a
full LIO T0 claim.

## Public synchronized LiDAR-IMU validation (NCLT 2013-01-10)

```bash
python3 evaluation/scripts/prepare_nclt_inputs.py \
  --velodyne-dir data/nclt_2013_01_10/2013-01-10/velodyne_sync \
  --ground-truth data/nclt_2013_01_10/groundtruth_2013-01-10.csv \
  --ms25 data/nclt_2013_01_10/2013-01-10/ms25.csv \
  --date 2013-01-10 --max-frames 120
python3 evaluation/scripts/run_lio_imu_public_validation.py --dataset nclt_2013_01_10_120
```

Paired summary: [`nclt_2013_01_10_120_lio_imu_validation_summary.json`](../../docs/benchmarks/lio_imu_public/nclt_2013_01_10_120_lio_imu_validation_summary.json).
