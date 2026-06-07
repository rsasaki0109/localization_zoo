# InTEn-LOAM: Intensity and Temporal Enhanced LOAM (from-paper reimplementation)

**Paper:** Shuaixin Li, Bin Tian, Zhu Xiaozhou, Gui Jianjun, Yao Wen, Guangyun Li,
*"InTEn-LOAM: Intensity and Temporal Enhanced LiDAR Odometry and Mapping"*，
Remote Sensing / arXiv:2209.05708 (2022/23). No public author code found.
Reconstructed from the paper.

## Core idea

InTEn-LOAM exploits **geometric + intensity + temporal** information in LiDAR sweeps:

1. **Cylindrical-image feature extraction (FEF).** Points are projected to range +
   intensity cylindrical images (paper eq. 2). Ground (column-wise line fit),
   facade, edge (range curvature), and reflector (intensity delta + absolute
   threshold) features are adaptively extracted.
2. **Intensity-based scan registration.** Reflector features are aligned by
   minimizing intensity residuals on a local **B-spline intensity surface**
   (simplified here as trilinear interpolation over 8 voxel control points; paper
   eq. 6–7).
3. **Joint scan-to-scan LO.** Geometric edge (point-to-line) and ground/facade
   (point-to-plane) factors are combined with intensity factors in Ceres.

## Scope / approximations for KITTI

- **Scan-to-scan odometry only**; mapping thread + temporal voxel filter (TVF) and
  **dynamic object removal (DOR)** are deferred (paper §3.3–3.4).
- KITTI Odometry velodyne scans are **unordered** (not native HDL-64 ring order);
  cylindrical projection uses spherical angles from `(x,y,z)` directly.
- ORB/LSD-style front-ends are N/A — this is pure LiDAR.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `cylindrical.width/height` | Cylindrical image size | 1024 × 64 |
| `use_intensity_registration` | B-spline intensity factors | true |
| `max_edge/surface/reflector_features` | Feature caps | 200 / 400 / 120 |
| `intensity_voxel_size` | Local intensity model voxel [m] | 0.5 |

Ablation: `--inten-loam-no-intensity` disables intensity registration.

## Result (KITTI Odometry, `--no-gt-seed`, scan-to-scan)

| Sequence | RPE drift (intensity ON) | ATE | RPE (intensity OFF) |
|---|---:|---:|---:|
| Seq 00 _(4541 fr)_ | **52.695%** | 1459 m | (not run full) |
| Seq 07 _(1101 fr)_ | **67.497%** | 448 m | **66.896%** |

**Honest negative.** Paper reports ~0.54% on native HDL-64 with full mapping +
temporal/DOR pipeline. Our scan-to-scan reimplementation without ring structure,
DOR, or mapping diverges (~53–67% drift). On seq07, intensity registration is
**near-neutral** (67.5% vs 66.9% drift OFF) — the cylindrical feature + geometric
front-end dominates; intensity path is implemented and unit-tested but does not
rescue KITTI odometry here. Temporal/DOR omission is the largest fidelity gap vs
the paper.

## Reproduce

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods inten_loam --no-gt-seed --inten-loam-dense-profile
```

## Tests

`test_inten_loam`: cylindrical projection validity; reflector labeling pipeline;
intensity residual scaling; short-sequence pipeline stability; `clear()` reset.
