# IDC (Iterative Dual Correspondence)

## Paper
- Feng Lu, Evangelos Milios, "Robot Pose Estimation in Unknown Environments by Matching 2D
  Range Scans" (Journal of Intelligent and Robotic Systems, 1997).

## What This Repository Implements

Simplified **2D scan-to-scan IDC odometry**:

- **CP (closest point)** correspondences for translation (Euclidean nearest neighbor)
- **RR (matching range)** correspondences for rotation (beam-local range similarity)
- fused increment `(tx_CP, ty_CP, θ_RR)` each inner iteration (Lu & Milios IDC)
- motion-prior warm start from last increment

## Dogfooding (2D scans)

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods idc,pl_icp,rf2o
```

## Current Scope

- Planar motion only (x, y, yaw)
- Scan-to-scan (no visibility filter / trimmed IDC)
- No local map
