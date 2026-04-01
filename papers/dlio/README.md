# DLIO

## What This Repository Implements

This is a compact DLIO-style LiDAR-inertial odometry pipeline.
It builds on the repository's `DLO`-style scan-to-map registration and uses `IMU preintegration` for pose prediction and velocity updates.

## Current Scope

- voxel-downsampled dense LiDAR scans
- direct scan-to-map alignment through this repository's `GICP`
- IMU preintegration for motion prediction
- keyframe-based sliding-window local map

## Important Deviation

This is not a full port of the original DLIO stack.
It does not yet reproduce the full continuous-time deskewing or the broader trajectory-optimization behavior of the reference system.
