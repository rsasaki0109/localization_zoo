# KISS-ICP

## Paper
- Ignacio Vizzo, Tiziano Guadagnino, Benedikt Mersch, Louis Wiesmann, Jens Behley, Cyrill Stachniss
- RA-L 2023
- Reference implementation: https://github.com/PRBonn/kiss-icp

## What This Repository Implements

This is a compact KISS-ICP-style pipeline that keeps the main idea:

- voxel-hash local map
- adaptive correspondence threshold
- robust point-to-point ICP
- voxel subsampling

## Current Scope

- designed as a small self-contained implementation for this repository
- focuses on the registration loop and map update path
- keeps the interface simple for benchmarking and ROS 2 wrapping

## Correspondence Search Neighborhood

Upstream KISS-ICP searches only the 27 voxels around each query
(`voxel_shifts` in `VoxelHashMap.cpp`), and so did this implementation until
2026-08-02. Since #64 the default searches every voxel within the
correspondence distance, so a match up to `max_dist` away is never missed.

That change alters results: KITTI Odometry 00 with `--kiss-fast-profile`
moved from 0.857 % to 1.069 % 100 m RPE (official KITTI RTE 1.045 % to
1.211 %). KISS-ICP aggregates recorded before 2026-08-02 therefore do not
reproduce with the default. Pass `--kiss-legacy-27-neighborhood` to
`pcd_dogfooding` (or set `KISSICPParams::neighbor_voxel_radius = 1`) to use
the upstream / pre-change search; it reproduces those aggregates bit for bit.
The default is unchanged so the v6/v10/v14 LiDAR odometry evidence, which was
produced after the change, stays reproducible.

## Not Included Yet

- a feature-complete port of the upstream KISS-ICP project
- the full set of engineering details and failure handling from the reference implementation
