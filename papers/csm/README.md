# CSM (Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009)
- Related: Karto SLAM / OpenSLAM correlative matching

## What This Repository Implements

**Multi-resolution correlative scan matching** for 2D odometry:

- occupancy grid + chamfer **distance transform** scoring (Olson-style)
- 3-level resolution pyramid (coarse → fine)
- relative SE(2) search around motion prior warm-start
- bilinear distance lookup
- **robot-frame rolling local map** (voxel merge + radius prune) in harness
- adaptive search window from last increment

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods csm,rf2o
```

## Benchmark (2026-06-10, robot-frame local map)

| Fixture | Frames | Drift (scan-to-scan) | Drift (local map) |
|---------|--------|----------------------|-------------------|
| `intel_val_73` | 73 | 16.0% | **14.5%** |
| `fr079_val_384` | 384 | 20.6% | **14.5%** |
| `rf2o_corridor` | 120 | 73.3% | 95.8% |

Artifacts: `docs/benchmarks/scan2d/*_csm_dt.json`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** robot-frame local map materially helps **real Bonn logs** (fr079 20.6%→14.5%,
Intel 16.0%→14.5%) and now matches Karto-Matcher on fr079, but **hurts** the synthetic slow-motion
corridor (73%→96%) where scan-to-scan was already ambiguous. Brute-force search windows remain the
bottleneck versus branch-and-bound Karto.

## Current Scope

- Rolling local map in harness (no loop closure / SLAM graph)
- Chamfer DT (not exact Euclidean Felzenszwalb)
- Brute-force pose search (small windows only)
