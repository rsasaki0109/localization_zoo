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
