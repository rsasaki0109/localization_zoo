# CSM (Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009)
- Related: Karto SLAM / OpenSLAM correlative matching

## What This Repository Implements

**Multi-resolution correlative scan matching** for 2D odometry:

- occupancy grid + chamfer **distance transform** scoring (Olson-style)
- 3-level resolution pyramid (coarse → fine via grid downsampling)
- **Olson coarse branch-and-bound** over score upper bounds
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

## Benchmark (2026-06-10, local map + BnB)

| Fixture | Frames | Drift (scan-to-scan) | Drift (local map) | Drift (+ BnB) |
|---------|--------|----------------------|-------------------|---------------|
| `intel_val_73` | 73 | 16.0% | 14.5% | **15.2%** |
| `fr079_val_384` | 384 | 20.6% | 14.5% | **14.9%** |
| `rf2o_corridor` | 120 | 73.3% | 95.8% | **102.0%** |

Artifacts: `docs/benchmarks/scan2d/*_csm_dt.json`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** robot-frame local map materially helps **real Bonn logs** (fr079 20.6%→~15%).
Adding Olson coarse BnB completes the Karto-style search stack but **does not further improve**
Bonn drift in this simplified port; synthetic corridor remains ~100% (honest negative).

## Current Scope

- Rolling local map + coarse BnB in harness (no loop closure / SLAM graph)
- Chamfer DT (not exact Euclidean Felzenszwalb)
- Coarse BnB + fine brute-force refinement (Karto-Matcher pattern)
