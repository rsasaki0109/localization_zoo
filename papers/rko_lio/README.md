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
- graceful fallback to a pure constant-velocity model when no IMU is available

## Current Scope

- designed as a small self-contained implementation for this repository
- focuses on the inertial-prior registration loop and map update path
- keeps the interface simple for benchmarking and ROS 2 wrapping

## Not Included Yet

- per-point motion compensation (de-skew) from the inertial prior
- online IMU bias / gravity / extrinsic estimation
- a feature-complete port of the upstream RKO-LIO project
