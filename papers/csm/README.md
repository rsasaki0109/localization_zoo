# CSM (Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009)
- Related: Karto SLAM / OpenSLAM correlative matching

## What This Repository Implements

**Multi-resolution correlative scan matching** for 2D odometry:

- occupancy grid + **Felzenszwalb Euclidean distance transform** scoring (Olson-style)
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

## Benchmark (2026-06-10, Felzenszwalb EDT + tuned BnB)

| Fixture | Frames | Drift (chamfer) | Drift (Felzenszwalb EDT) | FPS |
|---------|--------|-----------------|--------------------------|-----|
| `intel_val_73` | 73 | 14.7% | **14.0%** | ~59 |
| `fr079_val_384` | 384 | 14.3% | **13.7%** | ~46 |
| `rf2o_corridor` | 120 | 41.3% | **30.5%** | ~25 |

Artifacts: `docs/benchmarks/scan2d/*.json`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Finding:** replacing chamfer DT with exact **Felzenszwalb EDT** (`common/felzenszwalb_edt`)
improves Bonn Intel/fr079 drift and corridor (41%→**30%**). Short `fr079_train_200` window
regressed (12%→40%, indicative only). Corridor remains an honest negative vs PL-ICP/RF2O.

## Current Scope

- Rolling local map + tuned BnB in harness (no loop closure / SLAM graph)
- Felzenszwalb EDT (shared with Karto-Matcher)
- Coarse BnB + fine brute-force refinement at finest pyramid level only
