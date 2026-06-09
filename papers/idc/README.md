# IDC (Iterative Dual Correspondence)

## Paper
- Feng Lu, Evangelos Milios, "Robot Pose Estimation in Unknown Environments by Matching 2D
  Range Scans" (Journal of Intelligent and Robotic Systems, 1997).

## What This Repository Implements

Simplified **2D IDC scan matching odometry**:

- **CP (closest point)** correspondences for translation (Euclidean nearest neighbor)
- **RR (matching range)** correspondences for rotation (beam-local range similarity)
- fused increment `(tx_CP, ty_CP, θ_RR)` each inner iteration (Lu & Milios IDC)
- motion-prior warm start from last increment
- optional **robot-frame rolling local map** (voxel merge + radius prune) in harness
- local-map RR uses bearing-angle window instead of beam index

## Dogfooding (2D scans)

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods idc,rf2o,ndt_2d
```

## Benchmark (2026-06-10, local map)

| Fixture | Frames | Traj [m] | Drift | vs scan-to-scan |
|---------|--------|----------|-------|-----------------|
| `intel_val_73` | 73 | 378 | **15.2%** | 15.3% |
| `fr079_val_384` | 384 | 373 | **14.3%** | 27.7% |
| `mit_val_33` | 33 | 267 | **27.8%** | 29.5% |
| `rf2o_corridor` | 120 | 9.5 | **3.6%** | 42.6% |
| `fr079_train_1200` | 1200 | 150 | **35.0%** | 46.4% |

Full 9-method table: [`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

**Honest finding:** local map closes the fr079 gap (27.7% → **14.3%**, near PSM/NDT band) and
fixes corridor (42.6% → **3.6%**). Intel ~flat. Long train `fr079_train_1200` improves but
remains behind NDT-2D (8.8%) / PL-ICP (9.2%). `fr079_train_200` honest negative (~85%).

## Current Scope

- Planar motion only (x, y, yaw)
- No visibility filter / trimmed IDC
- RR local-map matching uses bearing window (not full co-occurrence matrix)
