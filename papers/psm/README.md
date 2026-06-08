# PSM (Polar Scan Matching)

## Paper
- Dirk Hähnel, Wolfram Burgard, "Efficient SLAM with Adaptive Real-Time Loop Closure Detection"
  (ICRA 2003 workshop / widely cited PSM scan matcher)
- Classic **polar scan matching**: align scans by correlating range profiles in the
  sensor polar frame instead of Cartesian point correspondences.

## What This Repository Implements

Simplified **2D scan-to-scan polar matching odometry**:

- reference scan stored as native polar ranges
- current scan endpoints warped by candidate SE(2) and re-binned to the reference bearing grid
- Gaussian range-profile correlation score (maximize overlap)
- coarse-to-fine brute-force search + local coordinate refinement
- motion-prior warm start from last increment

## Dogfooding (2D scans)

Use `scan_dogfooding`:

```bash
python3 evaluation/scripts/generate_rf2o_corridor_fixture.py
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_corridor \
  evaluation/fixtures/rf2o_corridor/gt.csv \
  120 --methods psm,pl_icp,rf2o
```

## Current Scope

- Planar motion only (x, y, yaw)
- Scan-to-scan (no local map)
- No polar occupancy / co-occurrence matrix (direct range-profile correlation)
- Brute-force search (OK for ~360 beams and short motion priors)

## Smoke benchmark (2026-06-09)

| Fixture | Frames | Traj [m] | ATE [m] | Drift | vs PL-ICP / RF2O |
|---------|--------|----------|---------|-------|------------------|
| `evaluation/fixtures/rf2o_corridor` | 120 | 9.5 | 0.30 | ~11.6% | PL-ICP 0.02 m / RF2O 0.09 m |
| `evaluation/fixtures/rf2o_smoke` | 60 | 17.7 | 3.01 | ~38.6% | PL-ICP 2.22 m / RF2O 0.20 m |

Artifact: `docs/benchmarks/scan2d/rf2o_corridor_120_psm.json`

**Honest finding:** this simplified polar range-profile port underperforms PL-ICP and RF2O on
both slow-motion corridor and fast mixed-motion smoke fixtures. Brute-force polar correlation
without occupancy/co-occurrence matrix is competitive on unit tests but drifts on long tracks.
