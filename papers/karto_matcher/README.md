# Karto-Matcher (Map-Based Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009) — branch-and-bound search
- Related: Karto SLAM / OpenSLAM map-based correlative matching (Konolige et al.)

## What This Repository Implements

**Map-based multi-resolution correlative scan matching** for 2D odometry:

- rolling local occupancy map (world-frame point cache + radius pruning)
- occupancy grid + chamfer distance transform scoring (Olson-style)
- 3-level resolution pyramid with coarse-to-fine pose search
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
| `intel_val_73` | 73 | **15.2%** |
| `fr079_val_384` | 384 | **14.8%** |
| `mit_val_33` | 33 | 28.2% |
| `rf2o_corridor` | 120 | 96.9% |

**Honest finding:** rolling local map materially helps **real Bonn logs** versus scan-to-scan CSM
(Intel 16.0% → 15.2%, fr079 20.6% → **14.8%**, near PSM/MbICP). Synthetic slow-motion corridor
remains an honest negative (~97% drift) — map-based correlative matching drifts on repetitive box geometry.

## Current Scope

- Rolling point map (not full log-odds occupancy grid)
- Chamfer DT (not exact Euclidean Felzenszwalb)
- Simplified coarse-to-fine search (same family as CSM; branch-and-bound not yet ported)
- No loop closure / SLAM graph
