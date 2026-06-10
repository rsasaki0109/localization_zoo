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
- optional **outlier trimming** — range-jump filter on match points only; Mahalanobis trim on finest pyramid level

## Dogfooding (2D scans)

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods ndt_2d,rf2o,pl_icp
```

## Benchmark (2026-06-10, outlier trim P17)

| Fixture | Frames | Traj [m] | Drift | vs P14 pyramid |
|---------|--------|----------|-------|----------------|
| `intel_val_73` | 73 | 378 | **14.8%** | 14.6% (similar) |
| `fr079_val_384` | 384 | 373 | **14.5%** | 14.8% (improved) |
| `mit_val_33` | 33 | 267 | **27.3%** | 28.1% (improved) |
| `rf2o_corridor` | 120 | 9.5 | **0.3%** | 1.0% (improved) |
| `fr079_train_1200` | 1200 | 150 | **7.5%** | **8.8%** (improved) |
| `intel_train_150` | 150 | 154 | **16.5%** | 18.0% (improved) |
| `mit_train_120` | 120 | 150 | **31.6%** | 29.1% (slight regression) |

Full leaderboard: [`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** naive trimming (map + all pyramid levels) regressed `fr079_train_1200` to ~23%.
Match-only range-jump filter + finest-level Mahalanobis trim (90%) fixes this and improves long train
(**7.5%**) and corridor (**0.3%**). Intel val ~flat.
