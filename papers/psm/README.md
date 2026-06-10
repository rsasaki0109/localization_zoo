# PSM (Polar Scan Matching)

## Paper
- Dirk Hähnel, Wolfram Burgard, "Efficient SLAM with Adaptive Real-Time Loop Closure Detection"
  (ICRA 2003 workshop / widely cited PSM scan matcher)
- Classic **polar scan matching**: align scans by correlating range profiles in the
  sensor polar frame instead of Cartesian point correspondences.

## What This Repository Implements

Simplified **2D polar matching odometry**:

- reference scan stored as native polar ranges (scan-to-scan) or rebuilt from robot-frame point cache (local map)
- current scan endpoints warped by candidate SE(2) and re-binned to the reference bearing grid
- Gaussian range-profile correlation score (maximize overlap)
- coarse-to-fine brute-force search + local coordinate refinement
- motion-prior warm start from last increment
- optional **robot-frame rolling local map** (voxel merge + radius prune) in harness

## Dogfooding (2D scans)

Use `scan_dogfooding`:

```bash
python3 evaluation/scripts/generate_rf2o_corridor_fixture.py
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_corridor \
  evaluation/fixtures/rf2o_corridor/gt.csv \
  120 --methods psm,pl_icp,rf2o
```

## Benchmark (2026-06-10, local map)

| Fixture | Frames | Traj [m] | Drift | vs scan-to-scan |
|---------|--------|----------|-------|-----------------|
| `intel_val_73` | 73 | 378 | **15.3%** | 21.8% |
| `fr079_val_384` | 384 | 373 | **14.3%** | 13.9% |
| `mit_val_33` | 33 | 267 | **28.5%** | 27.9% |
| `rf2o_corridor` | 120 | 9.5 | **4.4%** | 11.6% |
| `fr079_train_1200` | 1200 | 150 | **46.7%** | 72.2% |

Full 9-method table: [`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** local map fixes Intel (21.8% → **15.3%**) and corridor (11.6% → **4.4%**).
fr079 val ~flat (13.9% → 14.3%, now mid-pack vs Karto 13.7%). Long train improves but
remains behind NDT/PL-ICP. `fr079_train_200` honest negative (~681%).

## Current Scope

- Planar motion only (x, y, yaw)
- No polar occupancy / co-occurrence matrix (direct range-profile correlation)
- Brute-force search (OK for ~360 beams and short motion priors)
