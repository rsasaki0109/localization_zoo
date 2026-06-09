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

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods csm,rf2o
```

## Benchmark (2026-06-09, DT + pyramid refresh)

| Fixture | Frames | Drift (before) | Drift (after) |
|---------|--------|----------------|---------------|
| `intel_val_73` | 73 | 17.6% | **16.0%** |
| `fr079_val_384` | 384 | 38.9% | **20.6%** |
| `rf2o_corridor` | 120 | 69.6% | 73.3% |

Artifacts: `docs/benchmarks/scan2d/*_csm_dt.json`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** distance-transform scoring materially helps **real Bonn logs** (especially
fr079) but remains weak on the synthetic slow-motion corridor versus PL-ICP / RF2O. Brute-force
search windows and scan-to-scan-only scope limit Karto-level performance.

## Current Scope

- Scan-to-scan only (no loop closure / SLAM graph)
- Chamfer DT (not exact Euclidean Felzenszwalb)
- Brute-force pose search (small windows only)
