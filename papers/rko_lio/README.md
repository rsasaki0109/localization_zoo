# RKO-LIO

## Paper
- Meher V. R. Malladi, Tiziano Guadagnino, Luca Lobefaro, Cyrill Stachniss
- "RKO-LIO: Robust LiDAR-Inertial Odometry" (University of Bonn / PRBonn), 2025
- Reference implementation: https://github.com/PRBonn/rko_lio

## What This Repository Implements

This is a compact RKO-LIO-style pipeline that keeps the central idea of a
KISS-ICP-style scan-to-map loop driven by an inertial motion prior:

- voxel-hash local map and robust point-to-point ICP
- adaptive correspondence threshold
- **IMU rotation prior**: the gyro stream is preintegrated (via the shared
  `imu_preintegration` library) into a relative rotation that initializes the
  ICP guess; translation uses a constant-velocity prior
- **online gyro-bias correction**: the discrepancy between the IMU-predicted
  rotation and the ICP-solved rotation feeds back a running gyro-bias estimate
  (`--rko-lio-gyro-bias-gain`, default 0.3), so raw-gyro bias does not steer the
  prior into a bad initial guess
- graceful fallback to a pure constant-velocity model when no IMU is available

## Current Scope

- designed as a small self-contained implementation for this repository
- focuses on the inertial-prior registration loop and map update path
- keeps the interface simple for benchmarking and ROS 2 wrapping

On the HDL-400 (real IMU) window the rotation prior + bias feedback lands between
plain KISS-ICP and full LIO: it beats KISS-ICP (ATE 2.03 vs 3.08 m) but does not
reach iterated-filter LIO such as FAST-LIO2 / CT-LIO, which also model inertial
translation.

## Not Included Yet

- per-point motion compensation (de-skew) from the inertial prior
- inertial translation prediction (only rotation is taken from the IMU)
- gravity / IMU-LiDAR extrinsic estimation
- a feature-complete port of the upstream RKO-LIO project
