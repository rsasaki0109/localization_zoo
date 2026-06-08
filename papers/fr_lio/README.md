# FR-LIO

## Paper
- "FR-LIO: Fast and Robust LiDAR-Inertial Odometry by Tightly-Coupled Iterated
  Kalman Smoother and Robocentric Voxel Map" (arXiv:2302.04031)
- No public author code at time of port (2026-06).

## What This Repository Implements

Compact from-paper FR-LIO-style LiDAR-inertial odometry:

- **RC-Vox**: robocentric two-layer voxel map with toroidal top-level indexing
  and bottom-level voxels (simplified from eq. 19–24 in the paper)
- **Adaptive sub-frames**: IMU variance drives the number of scan sub-frames per
  LiDAR sweep (paper eq. 15–18, simplified)
- **ESKS**: backward error-state Kalman smoother over sub-frame poses (simplified
  gain-based correction, not a full iterated ESKF)
- **IMU preintegration**: rotation prior via shared `imu_preintegration` library
- **Point-to-plane ICP** against RC-Vox planes fitted from local kNN

## Current Scope

- Requires synchronized `imu.csv` in dogfooding (skips when absent, unlike RKO-LIO)
- No per-point motion compensation (deskew)
- No full IEKF / gravity / extrinsic estimation
- Map update gated by minimum plane-match count

## Dogfooding

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/nclt_2013_01_10_600 \
  experiments/reference_data/nclt_2013_01_10_600_gt.csv \
  600 --methods fr_lio --no-gt-seed --fr-lio-fast-profile
```

Profiles: `--fr-lio-fast-profile`, `--fr-lio-dense-profile`.
