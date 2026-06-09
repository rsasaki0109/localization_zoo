# RF2O

## Paper
- Miguel Jaimez, Jorge G. Monroy, Javier G. Monroy, Luis Riazuelo, and others
- "Planar Odometry from a Radial Laser Scanner. A Range Flow-based Approach"
  (ICRA 2016)
- Reference ROS implementation: https://github.com/MAPIRlab/rf2o_laser_odometry (GPL-3.0)

## What This Repository Implements

Simplified from-paper RF2O-style **2D laser odometry**:

- dense range-flow constraints (no explicit point correspondences)
- Gaussian-weighted range preprocessing + image pyramid
- coarse-to-fine Gauss-Newton with Cauchy-style IRLS iterations
- covariance eigenvalue smoothing on under-constrained axes (corridor-friendly)

Coarse-to-fine loop and pose composition follow the MAPIRlab reference structure
(coarse level skips initial warp; `filterLevelSolution` + `PoseUpdate`-style increment).

## Dogfooding (2D scans)

Use `scan_dogfooding` (not `pcd_dogfooding`):

```bash
python3 evaluation/scripts/generate_rf2o_smoke_fixture.py
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_smoke \
  evaluation/fixtures/rf2o_smoke/gt.csv \
  60 --methods rf2o
```

Scan tree layout: `scan_meta.json` + `NNNNNNNN/scan.csv` (comma-separated ranges).

## Current Scope

- Planar motion only (x, y, yaw)
- No ROS `LaserScan` bag reader (CSV fixture / future prep script)
- Simplified port; not a line-by-line copy of MAPIRlab RF2O
- Cauchy M-estimator reweighting inside IRLS not yet ported (linear solve only)

## Smoke benchmark (2026-06-09)

| Fixture | Frames | Traj [m] | ATE [m] | Drift |
|---------|--------|----------|---------|-------|
| `evaluation/fixtures/rf2o_smoke` | 60 | 17.7 | 0.20 | ~1.1% |

Artifact: `docs/benchmarks/scan2d/rf2o_smoke_60.json`

Public 7-method leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).
