# Karto-Matcher (Map-Based Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009) — branch-and-bound search
- Related: Karto SLAM / OpenSLAM map-based correlative matching (Konolige et al.)

## What This Repository Implements

**Map-based multi-resolution correlative scan matching** for 2D odometry:

- **robot-frame** rolling local map (voxel merge + radius prune; updated via `inv(increment)`)
- nested score pyramid with max-pooled Olson upper-bound grids
- motion-prior **adaptive search window** (tight on slow synthetic corridor motion)
- branch-and-bound pose search at the coarsest level, then coarse-to-fine refinement

Unlike scan-to-scan CSM (`papers/csm/`), each frame is matched against an accumulated local map.

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods karto_matcher,csm
```

Full leaderboard: [`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

## Benchmark (2026-06-10, robot-frame map refresh)

| Fixture | Frames | Drift |
|---------|--------|------:|
| `intel_val_73` | 73 | **14.2%** |
| `fr079_val_384` | 384 | **14.5%** |
| `mit_val_33` | 33 | 29.1% |
| `rf2o_corridor` | 120 | 102.0% |

**Honest finding:** robot-frame map + adaptive search improves **real Bonn logs** (Intel 15.1% → **14.2%**,
fr079 14.7% → **14.5%**). Synthetic slow-motion corridor improves 124% → **~102%** but remains an honest
negative — repetitive box geometry still fools correlative map matching.

## Current Scope

- Nested bound pyramid from finest DT (max-pool upper bounds)
- BnB at coarsest level only; finer levels use brute-force refinement
- Robot-frame point map (not full log-odds occupancy grid)
- No loop closure / SLAM graph
