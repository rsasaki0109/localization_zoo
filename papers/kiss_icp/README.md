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

## Deviations From Upstream KISS-ICP 1.3.0

| Item | Upstream default | This port (`pcd_dogfooding` default) | `--kiss-upstream-profile` |
|---|---|---|---|
| KITTI elevation correction | +0.205 deg in the KITTI loader | none | none (add `--input-vertical-angle-correction-deg 0.205`) |
| Input range | 0-100 m | 1-80 m (shared loader) | 1-80 m (add `--input-max-range-m 100`) |
| Scan pre-voxel / point cap | none | 0.5 m voxel, 4500 points | none |
| Map voxel / points per voxel | 1.0 m / 20 | 1.0 m / 12 | 1.0 m / 20 |
| ICP source subsample | 1.5 × voxel | 1.0 × voxel | 1.5 × voxel |
| Initial threshold | 2.0 | 1.5 | 2.0 |
| ICP iterations / convergence | 500 / 1e-4 | 30 / 1e-3 | 500 / 1e-4 |
| Correspondence search | 27 voxels | all voxels within the threshold | 27 voxels |
| Map crop | voxels beyond 100 m, every frame | 60 m, every 4th frame | 100 m, every frame |

On the KITTI paper-number gap ([full report](../../docs/kitti_elevation_correction.md)),
official KITTI RTE in percent:

| | Seq 00 raw | Seq 00 corrected | Seq 07 raw | Seq 07 corrected |
|---|---:|---:|---:|---:|
| Official KISS-ICP 1.3.0 | 0.910 | **0.528** | 0.505 | **0.375** |
| This port, default | 0.982 | 0.832 | 0.799 | 0.682 |
| This port, `--kiss-dense-profile` (Table 6 before) | 0.954 | 0.846 | 0.905 | 0.584 |
| This port, `--kiss-upstream-profile` | 0.839 | 0.708 | 0.630 | 0.564 |

The paper reports 0.51 % on seq 00. Most of the old 1.87x gap is input
preprocessing and configuration: the official code itself scores 0.910 % on
raw scans. On raw seq 00 the upstream profile is even better than the official
code (0.839 vs 0.910; on seq 07 it trails, 0.630 vs 0.505), but the official
code gains 26-42 % from the correction while
the upstream profile gains 10-16 %, so a 1.3-1.5x gap remains on corrected scans.

## Not Included Yet

- a feature-complete port of the upstream KISS-ICP project
- the full set of engineering details and failure handling from the reference implementation
