# CSM (Correlative Scan Matching)

## Paper
- Edwin Olson, "Real-Time Correlative Scan Matching" (ICRA 2009)
- Related: Karto SLAM / OpenSLAM correlative matching

## What This Repository Implements

Simplified **multi-resolution correlative scan matching** for 2D odometry:

- occupancy grid from reference scan
- coarse-to-fine grid resolution pyramid
- brute-force SE(2) search with motion prior warm-start

## Dogfooding

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_smoke \
  evaluation/fixtures/rf2o_smoke/gt.csv \
  60 --methods csm,rf2o
```

## Current Scope

- Scan-to-scan only (no loop closure / SLAM graph)
- Binary occupancy grid (no probability smearing from Karto)
- Brute-force pose search (small windows only)
