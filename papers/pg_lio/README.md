# PG-LIO

## Paper
- Nikhil Khedekar, Kostas Alexis
- "PG-LIO: Photometric-Geometric fusion for Robust LiDAR-Inertial Odometry"
  (arXiv:2506.18583, 2025)
- Author code planned at https://github.com/ntnu-arl/mimosa (not used for this port).

## What This Repository Implements

Simplified from-paper PG-LIO-style LIO:

- **Geometric factor**: voxel-hash local map + point-to-plane ICP
- **Photometric factor**: range-image NCC (ZNSSD) patches on filtered LiDAR intensity
- **Degeneracy-aware fusion**: photometric weight increases when geometric translation
  Hessian is ill-conditioned; IMU translation prior on degenerate directions
- **IMU**: gyro preintegration rotation prior via `imu_preintegration`

## Current Scope

- Requires `imu.csv` and intensity in `cloud.pcd` (PointXYZI)
- No full factor-graph sliding window / IEKF / deskew / gravity estimation
- Spherical range-image projection (PBID-style), not Ouster-specific bias LUT
- Simplified COIN-LIO-style brightness filtering

## Dogfooding

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/nclt_2013_01_10_600 \
  experiments/reference_data/nclt_2013_01_10_600_gt.csv \
  600 --methods pg_lio --no-gt-seed --pg-lio-fast-profile
```

Profiles: `--pg-lio-fast-profile`, `--pg-lio-dense-profile`.
