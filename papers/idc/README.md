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
  --methods idc,rf2o,ndt_2d
```

## Benchmark (2026-06-09)

| Fixture | Frames | Traj [m] | Drift | vs RF2O / PSM |
|---------|--------|----------|-------|----------------|
| `intel_val_73` | 73 | 378 | **15.3%** | RF2O 14.3% / NDT-2D 14.8% |
| `fr079_val_384` | 384 | 373 | 27.7% | PSM 13.9% / RF2O 15.4% |

Artifacts: `docs/benchmarks/scan2d/*_idc.json`. Full 8-method table:
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** dual CP+RR fusion is **mid-pack on Intel** (between NDT-2D and CSM) but
**behind RF2O/PSM on fr079**. Scan-to-scan only; no visibility filter or trimmed IDC.

## Current Scope

- Planar motion only (x, y, yaw)
- Scan-to-scan (no visibility filter / trimmed IDC)
- No local map
