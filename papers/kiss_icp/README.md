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

`--kiss-upstream-profile` matches the upstream configuration values; three
algorithmic differences need their own flags. "Upstream mechanics" below means
`--kiss-upstream-profile --kiss-model-deviation-threshold
--kiss-geman-mcclure-kernel --kiss-map-min-point-distance 0.2236068`.

| Item | Upstream default | This port (`pcd_dogfooding` default) | Upstream mechanics |
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
| Adaptive threshold σ | RMS of prediction-vs-estimate deviation (rotation scaled by max range) | 3 × median frame translation over the last 100 frames | upstream (`--kiss-model-deviation-threshold`) |
| Correspondence distance / kernel | 3σ / Geman-McClure k²/(k+r²)², k = σ | σ / Welsch, k = σ | upstream (`--kiss-geman-mcclure-kernel`) |
| Map insertion | skip points within sqrt(voxel² / 20) of a point in the voxel | first 12 points per voxel | upstream (`--kiss-map-min-point-distance 0.2236068`) |

On the KITTI paper-number gap ([full report](../../docs/kitti_elevation_correction.md)),
official KITTI RTE in percent:

| | Seq 00 raw | Seq 00 corrected | Seq 07 raw | Seq 07 corrected |
|---|---:|---:|---:|---:|
| Official KISS-ICP 1.3.0 | 0.910 | **0.528** | 0.505 | **0.375** |
| This port, default | 0.982 | 0.832 | 0.799 | 0.682 |
| This port, `--kiss-dense-profile` (Table 6 before 2026-10-10) | 0.954 | 0.846 | 0.905 | 0.584 |
| This port, `--kiss-upstream-profile` | 0.839 | 0.708 | 0.630 | 0.564 |
| This port, upstream mechanics | **0.901** | **0.535** | **0.481** | **0.363** |

With the upstream mechanics and the correction the port reproduces the official
code (0.535 vs 0.528 on seq 00, 0.363 vs 0.375 on seq 07) and the paper's
0.51 % on seq 00 (1.05x), and it gains as much from the correction (−41 %
and −25 %, against −42 % and −26 %). Adding one mechanism at a time on
corrected scans shows where the earlier gap came from:

| Corrected scans | Seq 00 | Seq 07 |
|---|---:|---:|
| `--kiss-upstream-profile` | 0.708 | 0.564 |
| + model-deviation threshold | 0.544 | 0.346 |
| + Geman-McClure kernel | 0.536 | 0.343 |
| + map insertion gate (upstream mechanics) | 0.535 | 0.363 |

The adaptive threshold is the difference that matters. The default σ follows
the vehicle's speed (mean 2.0 m on seq 07 and 2.5 m on seq 00) and is used as
both the correspondence distance and the kernel scale. The upstream σ tracks
the registration's own prediction error (mean 0.43 and 0.50 m), so
correspondences end at 3σ ≈ 1.3-1.5 m and the kernel is about five times
tighter. The defaults are unchanged so stored results and the CI golden stay
reproducible.

## Not Included Yet

- a feature-complete port of the upstream KISS-ICP project
- the full set of engineering details and failure handling from the reference implementation
