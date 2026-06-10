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
- Optional robot-frame rolling local map (`use_local_map`, default **off** — see below)

## Local map: honest negative as a default (2026-06-10, P18)

`RF2OParams::use_local_map` adds a robot-frame rolling local map (voxel merge +
radius prune + optional per-point age expiry) and rebuilds the range-flow
*reference* as a min-range polar profile rendered from the accumulated points
(`setReferenceFromRanges`). The mechanism is unit-tested
(`RobotFrameLocalMapTracksTranslation`, `LocalMapImprovesLongTranslationVsScanToScan`,
`LocalMapMaxAgeExpiresStalePoints`) and helps on short windows — but it is
**not** the benchmark default, because a full sweep showed it structurally
degrades long Bonn train windows:

| Fixture | scan-to-scan (default) | local map (r=15 m, voxel=0.15 m) |
|---|---:|---:|
| `fr079_val_384` | 15.4% | **13.9%** |
| `rf2o_corridor` | 1.3% | **0.6%** |
| `intel_val_73` | **14.3%** | 14.2% (~flat) |
| `mit_val_33` | 27.6% | 27.6% (flat) |
| `intel_train_150` | **17.5%** | 21.8% |
| `fr079_train_200` | **30.3%** | 99.4% |
| `fr079_train_1200` | **10.6%** | 25.8% |

A sweep over radius {8, 10, 15, 20} m, voxel {0.05–0.25} m, and point max-age
{2–50} frames rescued no long window: `fr079_train_200` never drops below
~88% under any config, and `fr079_train_1200` bottoms out at ~20% vs 10.6%
scan-to-scan. Age expiry in particular does *not* help (fr079_train_200 stays
~93% even with a 2-frame age cap), which rules out stale/dynamic points as the
root cause: the min-range polar projection itself quantizes the reference at
bin resolution and biases it toward closer returns, and the range-flow
gradients degrade on those segments. IDC and PSM — the other
projection-style local maps in the 2D suite — show the same long-window
blow-up (85% / 681% on `fr079_train_200`), so this is a family-level finding,
not an RF2O bug. Scan-to-scan RF2O remains the Intel val leader and the
`fr079_train_1200` RF2O default.

## Smoke benchmark (2026-06-09)

| Fixture | Frames | Traj [m] | ATE [m] | Drift |
|---------|--------|----------|---------|-------|
| `evaluation/fixtures/rf2o_smoke` | 60 | 17.7 | 0.20 | ~1.1% |

Artifact: `docs/benchmarks/scan2d/rf2o_smoke_60.json`

Public 8-method leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).
