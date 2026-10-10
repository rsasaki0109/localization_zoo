# DTD (Density Triangle Descriptor)

## Paper
- "DTD: Density Triangle Descriptor for 3D LiDAR Loop Closure Detection", 2025
  (PMC12788053)
- **No public reference implementation** — this is a from-paper reimplementation.

## What This Repository Implements

A **loop-closure / place-recognition** descriptor (like `scan_context` and
`isc_loam`, this is a place-recognition module, not odometry):

1. **BEV density image + keypoints** — the cloud is projected to an `M×M` BEV
   grid over `±L` m; per-cell point counts form a normalized density image, and
   cells below `density_threshold·max` are suppressed (removes ground/noise). A
   Good-Features-To-Track-style local-contrast score selects the strongest cells
   as keypoints, back-projected to 3D via the cell centroid.
2. **Triangle descriptor** — triangles are formed from each keypoint's nearest
   neighbours (side lengths constrained to `[min_side, max_side]`). Each
   descriptor stores the **three side lengths sorted ascending** (rotation- and
   translation-invariant canonical form) plus a per-vertex **entropy**
   `H = ½·log((2πe)³·|Σ|)` of the cell's point distribution (a local cue).
3. **Two-stage loop detection** —
   - candidate retrieval by **hashing the sorted side lengths**, with an entropy
     cosine-similarity gate (`entropy_sim_threshold`), voting per past frame;
   - **geometric verification** of the best-voted frame: SVD on the matched
     vertex correspondences gives `R, t`; a loop is confirmed when the
     reprojection-inlier count exceeds `min_inliers`.

## API

```cpp
DTDDetector det;
det.addToDatabase(det.describe(cloud_k, k));      // build DB over time
auto loop = det.detectLoop(det.describe(cloud_n, n), n, min_frame_gap);
if (loop.detected) { /* loop.matched_frame, loop.rotation, loop.translation */ }
```

## Current Scope / Deviations

- self-contained core library (no ROS dependency); not wired into the odometry
  `pcd_dogfooding` benchmark because it solves a different task (loop closure)
- the paper's **GFTT** keypoint detector is approximated by a density
  local-contrast score on the BEV grid
- neighbour search uses a brute-force distance query instead of a kd-tree
- rotation/translation invariance comes from the sorted-side canonical form +
  entropy, matching the paper; thresholds (`σ_k, σ_s, σ_d, σ_n`) are exposed as
  parameters

## KITTI Result: Retrieval Fails

On the [KITTI place-recognition benchmark](../../docs/place_recognition_benchmark.md)
this port finds almost no correct loops (F1max 0.035 on KITTI 07, 0.000 on
KITTI 00), while Scan Context reaches 0.588 and 0.935 under the same
protocol. Do not use it as a loop detector until this is fixed.

A direct check on KITTI 07 points at the verification step. A scan queried
against itself verifies with 216 inliers, but the next scan, about 1 m
further along, gets 5 inliers and the scan after that fewer than 4. Verification
solves one SVD over every hash-matched triangle of the best-voted frame, with
no outlier rejection, so a few wrong triangle matches corrupt the transform
and true revisits score as low as false ones. A per-match RANSAC (or a
transform from each triangle pair, as in STD) is the likely fix.
