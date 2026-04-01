# DLO

## What This Repository Implements

This is a derived DLO-style pipeline.
Dense LiDAR scans are downsampled and aligned to a local map with `GICP`, while the map is maintained as a keyframe-based sliding window.

## Current Scope

- range filtering and voxel downsampling
- dense scan-to-map registration using this repository's `GICP`
- constant-velocity initialization
- keyframe-based local map maintenance

## Important Deviation

This is not a full port of the original DLO optimizer or system stack.
It is a smaller implementation built from this repository's existing `GICP` and local-map components.
