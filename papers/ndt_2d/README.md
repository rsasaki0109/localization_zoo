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
- optional **3-level multi-resolution pyramid** (cell scale 1.5× per level, coarse → fine)
- optional **robot-frame rolling local map** (voxel merge + radius prune) in harness

## Dogfooding (2D scans)

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods ndt_2d,rf2o,pl_icp
```

## Benchmark (2026-06-10, pyramid P14)

| Fixture | Frames | Traj [m] | Drift | vs P13 local map |
|---------|--------|----------|-------|------------------|
| `intel_val_73` | 73 | 378 | **14.6%** | 14.9% (similar) |
| `fr079_val_384` | 384 | 373 | **14.8%** | 14.4% (slight regression) |
| `mit_val_33` | 33 | 267 | **28.1%** | 27.8% (similar) |
| `rf2o_corridor` | 120 | 9.5 | **1.0%** | 0.8% (similar) |
| `fr079_train_1200` | 1200 | 150 | **8.8%** | **10.3%** (improved) |
| `intel_train_150` | 150 | 154 | **18.0%** | 20.5% (improved) |
| `mit_train_120` | 120 | 150 | **29.1%** | 30.1% (improved) |

Artifacts: `docs/benchmarks/scan2d/*`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** mild pyramid (scale **1.5**, 3 levels) recovers long-window drift on
`fr079_train_1200` (**8.8%**, now best on that fixture) without undoing P13 val/corridor gains.
Aggressive 2× scaling regressed `fr079_train_1200` to ~14%. Outlier trimming not implemented yet.
