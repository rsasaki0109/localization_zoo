# NDT-2D (Normal Distributions Transform)

## Paper
- Peter Biber, Wolfgang Straßer, "The Normal Distributions Transform: A New Approach to
  Laser Scan Matching" (IROS 2003).

## What This Repository Implements

Simplified **2D scan-to-scan NDT odometry**:

- subdivide the reference scan into a grid of cells
- each occupied cell stores a 2D Gaussian (mean + inverse covariance)
- align the current scan by Gauss-Newton on the NDT score (no explicit correspondences)
- motion-prior warm start from the last increment

## Dogfooding (2D scans)

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods ndt_2d,rf2o,pl_icp
```

## Benchmark (2026-06-09)

| Fixture | Frames | Traj [m] | Drift | vs RF2O / PL-ICP |
|---------|--------|----------|-------|-------------------|
| `intel_val_73` | 73 | 378 | **14.8%** | RF2O 14.3% / PL-ICP 16.9% |
| `fr079_val_384` | 384 | 373 | 21.8% | RF2O 15.4% / PL-ICP 41.0% |
| `mit_val_33` | 33 | 267 | 29.2% | RF2O 27.6% / PL-ICP 30.3% |
| `rf2o_corridor` | 120 | 9.5 | 22.3% | PL-ICP 0.38% / RF2O 1.28% |

Artifacts: `docs/benchmarks/scan2d/*_ndt2d.json`. Full leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** correspondence-free NDT is **competitive on real Bonn logs** (Intel ~RF2O,
fr079 mid-pack) but **underperforms on the synthetic corridor** where PL-ICP dominates.
Scan-to-scan single-resolution port; no multi-resolution pyramid or outlier trimming.
