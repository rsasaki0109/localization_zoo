# R-VoxelMap (Recursive Plane Fitting Voxel Map)

## Paper
- "R-VoxelMap: Accurate Voxel Mapping with Recursive Plane Fitting for Online
  LiDAR Odometry", 2026
- arXiv: https://arxiv.org/abs/2601.12377 (arXiv:2601.12377)
- The paper states the code "will be available" but **no implementation is
  released yet** — this is a from-paper reimplementation.

## What This Repository Implements

R-VoxelMap's geometry-driven recursive voxel mapping, as a self-contained
scan-to-map odometry front-end (built on the project's voxel loop):

1. **Recursive plane fitting with outlier detect-and-reuse** — each root voxel
   least-squares fits a plane. Points farther than `inlier_dist` from the plane
   are **outliers**; instead of discarding them, they are propagated to deeper
   octree children and recursively re-fit. This recovers multiple physical
   surfaces that share one root voxel (e.g. a ground/wall corner) without
   over-segmenting large planes.
2. **Point-distribution validity check** — a node is accepted as a plane only
   when `λ_min < planarity_ratio · λ_max` of its point covariance, preventing
   the incorrect merging of points from different physical planes.
3. **Octree organization** — a hash of root voxels, each holding an octree up to
   `max_depth`; query descends to the **finest valid plane** containing the
   query point.
4. **Registration** — point-to-plane against the recursively-fit planes, each
   weighted by `1/plane_variance` (thin, well-fit planes count more).

## Current Scope

- self-contained core library (no ROS dependency)
- incremental: root voxels re-fit when touched, recent points capped to bound
  per-voxel re-fit cost
- least-squares plane fit with an inlier-distance gate as a light substitute for
  the paper's RANSAC fit (keeps it deterministic and fast)
- dense KITTI profile enables a scan-to-scan recovery path only when recursive
  map correspondences fall below the configured support threshold

## Result (KITTI Odometry, strict exact-frame association)

Command profile: `--no-gt-seed --r-voxelmap-dense-profile`.

| Seq | Drift | ATE | FPS | map match | recovery |
|-----|------:|----:|----:|----------:|---------:|
| seq00 | **58.3%** | 1872 m | 21.1 | 0.805 | 0.02% |
| seq07 | **35.8%** | 103 m | 7.7 | 0.856 | 0.09% |

**Honest negative.** Low-correspondence recovery prevents the seq07 runaway
failure seen with strict frame association, but this compact front-end remains
far from the paper claim and from the scan-to-map baselines. The result should
be read as "no longer diverges", not as a reproduced R-VoxelMap score.

## Deviations / Not Included Yet

- the paper's exact **RANSAC** plane estimator is approximated by least-squares
  + inlier-distance outlier detection (the recursive detect-and-reuse pipeline
  and validity check are kept)
- no IMU / continuous-time back-end — this is the mapping + registration
  front-end only; constant-velocity prediction stands in for motion
- parameters the paper does not pin down (recursion depth, inlier distance,
  planarity ratio) are exposed with repo-consistent defaults
