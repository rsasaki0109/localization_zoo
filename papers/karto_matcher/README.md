# Karto-Matcher (Map-Based Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009) — branch-and-bound search
- Related: Karto SLAM / OpenSLAM map-based correlative matching (Konolige et al.)

## What This Repository Implements

**Map-based multi-resolution correlative scan matching** for 2D odometry:

- **robot-frame** rolling local map (voxel merge + radius prune; updated via `inv(increment)`)
- nested score pyramid with max-pooled Olson upper-bound grids (Felzenszwalb EDT at finest level)
- motion-prior **adaptive search window** (tight on slow synthetic corridor motion)
- branch-and-bound pose search at the coarsest level, then **finest-level-only** refinement (tuned BnB path)
- precomputed score grid with bilinear lookup (avoids per-pose `exp()`)

Unlike scan-to-scan CSM (`papers/csm/`), each frame is matched against an accumulated local map.

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods karto_matcher,csm
```

Full leaderboard: [`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

## Benchmark (2026-06-10, Felzenszwalb EDT + tuned BnB)

| Fixture | Frames | Drift (chamfer) | Drift (Felzenszwalb EDT) | FPS (approx.) |
|---------|--------|-----------------|--------------------------|--------------:|
| `intel_val_73` | 73 | 14.7% | **14.0%** | ~61 |
| `fr079_val_384` | 384 | 14.3% | **13.7%** | ~48 |
| `mit_val_33` | 33 | 28.1% | 28.1% | ~44 |
| `rf2o_corridor` | 120 | 41.3% | **30.5%** | ~26 |

**Honest finding:** Felzenszwalb EDT (shared `common/felzenszwalb_edt`) improves Intel/fr079/corridor
vs chamfer DT. Short `fr079_train_200` regressed (12%→40%, indicative). Synthetic corridor
still an honest negative vs PL-ICP/RF2O.

## Current Scope

- Nested bound pyramid from finest DT (max-pool upper bounds)
- BnB at coarsest level only; finer levels use brute-force refinement
- Robot-frame point map (not full log-odds occupancy grid)
- No loop closure / SLAM graph
