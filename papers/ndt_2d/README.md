# NDT-2D (Normal Distributions Transform)

## Paper
- Peter Biber, Wolfgang Straßer, "The Normal Distributions Transform: A New Approach to
  Laser Scan Matching" (IROS 2003).

## What This Repository Implements

Simplified **2D NDT odometry**:

- subdivide the reference scan into a grid of cells
- each occupied cell stores a 2D Gaussian (mean + inverse covariance)
- align the current scan by Gauss-Newton on the NDT score (no explicit correspondences)
- motion-prior warm start from the last increment
- optional **robot-frame rolling local map** (voxel merge + radius prune) in harness

## Dogfooding (2D scans)

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods ndt_2d,rf2o,pl_icp
```

## Benchmark (2026-06-10, local map refresh)

| Fixture | Frames | Traj [m] | Drift | vs scan-to-scan |
|---------|--------|----------|-------|-----------------|
| `intel_val_73` | 73 | 378 | **14.9%** | 14.8% (similar) |
| `fr079_val_384` | 384 | 373 | **14.4%** | 21.8% (improved) |
| `mit_val_33` | 33 | 267 | **27.8%** | 29.2% (improved) |
| `rf2o_corridor` | 120 | 9.5 | **0.8%** | 22.3% (improved) |

Artifacts: `docs/benchmarks/scan2d/*`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** robot-frame local map makes NDT competitive on **fr079 val** and **synthetic corridor**
(where scan-to-scan NDT was mid-pack / weak). Intel drift is unchanged. Long `fr079_train_1200` is
**10.3%** (scan-to-scan was 7.4% — slight regression on that window). No multi-resolution pyramid
or outlier trimming yet.
