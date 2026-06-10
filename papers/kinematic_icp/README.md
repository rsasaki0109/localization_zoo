# Kinematic-ICP (2D)

## Paper
- Tiziano Guadagnino et al., "Kinematic-ICP: Enhancing LiDAR Odometry with Kinematic
  Constraints for Wheeled Mobile Robots Moving on Planar Surfaces" (ICRA 2025)
- Reference: https://github.com/PRBonn/kinematic-icp (MIT)

## What This Repository Implements

Simplified **2D Kinematic-ICP** scan-to-scan odometry:

- PL-ICP-style point-to-line matching
- **Unicycle** motion parameterization (forward arc + yaw, no lateral slip)
- Wheel odometry prior fused in the linearized least-squares solve

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_smoke \
  evaluation/fixtures/rf2o_smoke/gt.csv \
  60 --methods kinematic_icp,rf2o
```

Optional `odom.csv` in the scan tree: `timestamp,forward_m,yaw_rad` (GT-derived deltas
used on smoke fixture when present).

## Current Scope

- 2D `LaserScan` only
- No ROS / TF / extrinsic calibration
- Simplified port; not a line-by-line copy of PRBonn kinematic-icp

## Smoke benchmark (2026-06-09)

| Fixture | Frames | Kinematic-ICP | PL-ICP | RF2O |
|---------|--------|---------------|--------|------|
| `rf2o_smoke` + GT wheel odom | 60 | 4.21 m | 2.22 m | 0.20 m |

Artifact: `docs/benchmarks/scan2d/rf2o_smoke_60_kinematic_icp_vs_rf2o.json`

Public 8-method leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

## Local map: opt-in only, no safe default (2026-06-11, P19)

`KinematicICPParams::use_local_map` adds a robot-frame rolling local map of
point-to-line references (voxel merge + radius prune + grid-indexed
correspondence search, same structure as the PL-ICP/MbICP local maps). The
mechanism is unit-tested (`LocalMapTracksTranslationWithWheelOdom`,
`LocalMapMatchesScanToScanOnShortRun`) but it is **not** the benchmark
default, because no swept configuration is safe across fixtures:

| Fixture | scan-to-scan (default) | local map (r=15 m, voxel=0.10 m) |
|---|---:|---:|
| `mit_train_120` | 46.4% | **12.8%** |
| `fr079_train_200` | 11.0% | **5.9%** |
| `fr079_val_384` | 18.9% | **16.0%** |
| `intel_val_73` | **18.3%** | 18.2% (~flat) |
| `mit_val_33` | **23.4%** | 30.5% |
| `rf2o_corridor` | **83.8%** | 93.5% |
| `intel_train_150` | **25.4%** | 39.0% |
| `fr079_train_1200` | **10.7%** | 18.7% |

The sweep covered radius {10, 15, 20} m, voxel {0.10–0.25} m, and wheel-prior
weight {8–64}. Gains and losses move together: configs that rescue
`mit_train_120` (46.4% → 12.8%) break `mit_val_33` — Kinematic-ICP's only
leaderboard win — and `fr079_train_1200`; raising the unicycle prior weight to
compensate for the larger correspondence set destabilizes `fr079_train_200`
instead (5.9% → 81.8% at w=32). This mirrors the suite-wide finding that the
best 2D odometry configuration is fixture-dependent; unlike RF2O's polar
reprojection (which loses everywhere on long windows), the point-style map
here is a genuine trade-off with no dominating choice, so the paper-faithful
scan-to-scan reference stays the default.
