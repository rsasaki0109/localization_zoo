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

## Not Included Yet

- a feature-complete port of the upstream KISS-ICP project
- the full set of engineering details and failure handling from the reference implementation
