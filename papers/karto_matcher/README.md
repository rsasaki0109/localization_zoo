# Karto-Matcher (Map-Based Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009) — branch-and-bound search
- Related: Karto SLAM / OpenSLAM map-based correlative matching (Konolige et al.)

## What This Repository Implements

**Map-based multi-resolution correlative scan matching** for 2D odometry:

- rolling local occupancy map (world-frame point cache + radius pruning)
- nested score pyramid with max-pooled Olson upper-bound grids
- branch-and-bound pose search at the coarsest level, then coarse-to-fine refinement
- motion-prior warm start

Unlike scan-to-scan CSM (`papers/csm/`), each frame is matched against an accumulated local map.

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods karto_matcher,csm
```

Full leaderboard: [`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

## Benchmark (2026-06-10, 9-method refresh)

| Fixture | Frames | Drift |
|---------|--------|------:|
| `intel_val_73` | 73 | **15.1%** |
| `fr079_val_384` | 384 | **14.7%** |
| `mit_val_33` | 33 | 29.1% |
| `rf2o_corridor` | 120 | 123.8% |

**Honest finding:** rolling local map + Olson coarse BnB helps **real Bonn logs** versus scan-to-scan CSM
(Intel 16.0% → 15.1%, fr079 20.6% → **14.7%**). Synthetic slow-motion corridor remains an honest negative
(~124% drift) — map-based correlative matching drifts on repetitive box geometry.

## Current Scope

- Nested bound pyramid from finest DT (max-pool upper bounds)
- BnB at coarsest level only; finer levels use brute-force refinement
- Rolling point map (not full log-odds occupancy grid)
- No loop closure / SLAM graph
