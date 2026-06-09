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

## Benchmark (2026-06-09)

| Fixture | Frames | Traj [m] | ATE [m] | Drift | Leader on fixture |
|---------|--------|----------|---------|-------|-------------------|
| `intel_val_73` | 73 | 378 | 28.5 | 21.8% | RF2O 14.3% |
| `fr079_val_384` | 384 | 373 | 28.5 | **13.9%** | **PSM (this port)** |
| `mit_val_33` | 33 | 267 | 17.2 | 27.9% | Kinematic-ICP 23.4% |
| `rf2o_corridor` | 120 | 9.5 | 0.30 | 11.6% | PL-ICP 0.38% |

Artifacts: `docs/benchmarks/scan2d/{intel_val_73,fr079_val_384,mit_val_33,rf2o_corridor}.json`.
Full 8-method leaderboard:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** polar range-profile matching is **dataset-dependent** — best drift on
fr079 val (13.9%) but behind RF2O on Intel and behind PL-ICP on the synthetic corridor.
Brute-force correlation without occupancy/co-occurrence matrix is competitive on unit tests
but not uniformly strong across environments.
