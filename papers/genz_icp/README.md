# GenZ-ICP

## Paper
- Daehan Lee, Hyungtae Lim, Soohee Han
- "GenZ-ICP: Generalizable and Degeneracy-Robust LiDAR Odometry Using an Adaptive Weighting", RA-L 2024
- Reference implementation: https://github.com/cocel-postech/genz-icp

## What This Repository Implements

This is a compact GenZ-ICP-style pipeline built on the same scan-to-map loop as
the KISS-ICP port, with the key GenZ idea added:

- voxel-hash local map with per-correspondence normal / planarity estimation
- adaptive correspondence threshold and constant-velocity motion model
- **hybrid residual**: point-to-plane for planar correspondences,
  point-to-point for unstructured ones
- **adaptive weighting** that boosts the point-to-point term in planar-dominated
  (degeneracy-prone) scenes so unconstrained directions stay observable

## Current Scope

- designed as a small self-contained implementation for this repository
- focuses on the registration loop, normal/planarity estimation, and the
  adaptive plane/point blend
- keeps the interface simple for benchmarking and ROS 2 wrapping

## Not Included Yet

- a feature-complete port of the upstream GenZ-ICP project
- the full set of engineering details and failure handling from the reference
  implementation
