# CSM (Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009)
- Related: Karto SLAM / OpenSLAM correlative matching

## What This Repository Implements

**Multi-resolution correlative scan matching** for 2D odometry:

- occupancy grid + chamfer **distance transform** scoring (Olson-style)
- 3-level resolution pyramid (coarse → fine via grid downsampling)
- **Olson coarse branch-and-bound** over score upper bounds
- **Tuned BnB path**: 64-node budget, 2×2 leaf grid, finest-level-only refine
- precomputed **score-grid bilinear lookup** (avoids per-eval `exp()`)
- relative SE(2) search around motion prior warm-start
- **robot-frame rolling local map** (voxel merge + radius prune) in harness
- adaptive search window from last increment

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods csm,rf2o
```

## Benchmark (2026-06-10, local map + tuned BnB)

| Fixture | Frames | Drift (scan-to-scan) | Drift (local map) | Drift (+ BnB) | Drift (tuned BnB) | FPS (tuned) |
|---------|--------|----------------------|-------------------|---------------|-------------------|-------------|
| `intel_val_73` | 73 | 16.0% | 14.5% | 15.2% | **14.7%** | **79** |
| `fr079_val_384` | 384 | 20.6% | 14.5% | 14.9% | **14.3%** | **58** |
| `rf2o_corridor` | 120 | 73.3% | 95.8% | 102.0% | **41.3%** | 28 |

Artifacts: `docs/benchmarks/scan2d/*_csm_dt.json`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Finding:** speed tuning (64-node BnB budget, skip intermediate refine, score-grid lookup)
cuts runtime ~**5×** on Bonn val fixtures while **improving** drift on Intel/fr079 and
materially improving the synthetic corridor (102%→41%). Corridor remains an honest negative
vs PL-ICP/RF2O but is no longer ~100% drift.

## Current Scope

- Rolling local map + tuned BnB in harness (no loop closure / SLAM graph)
- Chamfer DT (not exact Euclidean Felzenszwalb)
- Coarse BnB + fine brute-force refinement at finest pyramid level only
