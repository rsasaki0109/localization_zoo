# PL-ICP

## Paper
- Andrea Censi, "An ICP variant using a point-to-line metric"
  (IROS 2008)
- Reference: widely reimplemented in ROS `laser_scan_matcher`, OpenSLAM, etc.

## What This Repository Implements

Simplified **2D scan-to-scan PL-ICP odometry**:

- laser scan → 2D points with beam-order line normals
- iterative point-to-line Gauss-Newton (SE2)
- brute-force nearest-neighbor correspondences

## Dogfooding (2D scans)

Use `scan_dogfooding`:

```bash
python3 evaluation/scripts/generate_rf2o_smoke_fixture.py
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_smoke \
  evaluation/fixtures/rf2o_smoke/gt.csv \
  60 --methods pl_icp,rf2o
```

## Current Scope

- Planar motion only (x, y, yaw)
- Default harness path: scan-to-scan (no rolling map)
- Optional rolling local map via `PLICPParams::use_local_map` (world-frame submap, matched in the previous pose frame)
- No Censi recursive distortion model (standard ICP loop)
- Brute-force correspondences (OK for ~360 beams)

## Smoke benchmark (2026-06-09)

| Fixture | Frames | Traj [m] | ATE [m] | Drift | vs RF2O |
|---------|--------|----------|---------|-------|---------|
| `evaluation/fixtures/rf2o_smoke` | 60 | 17.7 | 2.22 | ~12.6% | RF2O 0.20 m |
| same | 10 | 2.7 | 0.002 | ~0.1% | comparable |

Artifact: `docs/benchmarks/scan2d/rf2o_smoke_60_pl_icp_vs_rf2o.json`

Public 8-method leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).
