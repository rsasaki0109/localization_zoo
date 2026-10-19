# NDT

## Paper
- Peter Biber and Wolfgang Strasser, IROS 2003
- Martin Magnusson, PhD thesis 2009 for the 3D formulation

## What This Repository Implements

This is a compact NDT-style registration module:

- the target cloud is converted into voxel Gaussians
- the source cloud is aligned against those Gaussians
- the optimizer is intentionally lightweight and does not aim to be a full Magnusson-style production NDT port

## Current Scope

- voxel Gaussian map construction
- analytic gradient and Gauss-Newton Hessian approximation
- line search on the update step

## Not Included Yet

- full production-grade NDT optimizer behavior as found in mature libraries
- neighbor-cell interpolation and other accuracy-focused refinements
- an exact port of Autoware or PCL NDT internals

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 2 selected stored NDT full-sequence cells complete with native exits 0. 2 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
