# InTEn-LOAM: Intensity and Temporal Enhanced LOAM (from-paper reimplementation)

**Paper:** Shuaixin Li, Bin Tian, Zhu Xiaozhou, Gui Jianjian, Yao Wen, Guangyun Li,
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
4. **Temporal voxel filter (TVF) + dynamic object removal (DOR)** (2026-06-09):
   voxel occupancy ages features over time; DOR drops voxels with high motion
   inconsistency before mapping updates.
5. **Scan-to-map mapping** (2026-06-09): local voxel map + Ceres scan-to-map
   refinement after scan-to-scan odometry (paper §3.3 mapping thread, simplified).

## Scope / approximations for KITTI

- KITTI Odometry velodyne scans are **unordered** (not native HDL-64 ring order);
  cylindrical projection uses spherical angles from `(x,y,z)` directly.
- TVF/DOR/mapping are simplified approximations of the paper's full pipeline.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `cylindrical.width/height` | Cylindrical image size | 1024 × 64 |
| `use_intensity_registration` | B-spline intensity factors | true |
| `enable_tvf` | Temporal voxel filter | true |
| `enable_dor` | Dynamic object removal | true |
| `enable_mapping` | Scan-to-map refinement | true |
| `max_edge/surface/reflector_features` | Feature caps | 200 / 400 / 120 |
| `intensity_voxel_size` | Local intensity model voxel [m] | 0.5 |

Ablation flags: `--inten-loam-no-intensity`, `--inten-loam-no-tvf`,
`--inten-loam-no-dor`, `--inten-loam-no-mapping`.

## Result (KITTI Odometry full, `--no-gt-seed`, `--inten-loam-dense-profile`)

Ablation script: `evaluation/scripts/run_inten_loam_ablation.py`
Bundles: `docs/benchmarks/kitti_full_new_methods/seq{00,07}_inten_loam_ablation.json`

### Seq 00 (4541 frames)

| Variant | Drift | ATE [m] | Notes |
|---------|------:|--------:|-------|
| **no_mapping** (scan-to-scan) | **52.5%** | 1461 | baseline main artifact |
| mapping_only (no TVF/DOR) | 60.6% | 826 | mapping **hurts** drift on long seq |
| no_dor | 60.6% | 864 | |
| full (TVF+DOR+mapping) | 68.4% | 531 | best ATE but worse drift |
| no_intensity | 68.3% | 512 | intensity neutral with mapping |
| no_tvf | 68.2% | 573 | |

### Seq 07 (1101 frames)

| Variant | Drift | ATE [m] | Notes |
|---------|------:|--------:|-------|
| **no_mapping** (scan-to-scan) | **67.2%** | 432 | baseline main artifact |
| mapping_only / no_tvf_no_dor | 67.9% | 308 | mapping ≈ neutral on drift |
| no_dor | 67.9% | 301 | |
| full / no_tvf / no_intensity | 71.7% | 200–218 | TVF+mapping **hurts** drift |

**Honest negative — ablation findings.** Scan-to-scan (`--inten-loam-no-mapping`)
remains the best **drift** config on both full sequences. Scan-to-map mapping
lowers ATE on seq07 (432→~300 m) but does not improve RPE drift; on long seq00
mapping **increases** drift (52.5%→60.6%). TVF/DOR/intensity with mapping do not
rescue KITTI odometry (~53–72% drift vs paper ~0.54%). The 200-frame prefix
(56.5% with mapping) was **not representative** of full-sequence behavior.

## Cross-dataset (`--no-gt-seed`)

| Dataset | Drift | ATE | Notes |
|---------|------:|----:|---|
| MulRan 120 | **185.4%** | 32 m | TVF/DOR/mapping ON |
| NCLT 600 | **46.5%** | 40 m | ICPSC/MCC ~0.5% drift on same data |

See `docs/benchmarks/cross_dataset/`.

## Reproduce

Full ablation (7 variants × seq00 + seq07):

```sh
python3 evaluation/scripts/run_inten_loam_ablation.py --sequence seq07 --sequence seq00
```

Single run (default = TVF+DOR+mapping ON):

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods inten_loam --no-gt-seed --inten-loam-dense-profile
```

Scan-to-scan baseline:

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods inten_loam --no-gt-seed --inten-loam-dense-profile --inten-loam-no-mapping
```

## Tests

`test_inten_loam`: cylindrical projection validity; reflector labeling pipeline;
intensity residual scaling; short-sequence pipeline stability; `clear()` reset.
