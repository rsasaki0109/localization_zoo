# Variant Analysis

> Generated: 2026-04-09T08:49:49+00:00

This document analyzes **why** variant performance differs across datasets and initialization modes. It complements `decisions.md` (which records **what** was chosen) with **why** the choices diverge.

## 1. GT-Seeded vs Odometry-Chain Initialization

Scan-to-map methods (LiTAMIN2, GICP, NDT) can use GT poses as per-frame initialization or rely on their own odometry chain. KISS-ICP and CT-ICP are always odometry-based.

### Drive 0009 Comparison

| Method | Mode | Default Variant | ATE [m] | FPS |
|---|---|---|---:|---:|
| aloam | GT-seeded full | fast | 6.105 | 5.8 |
| balm2 | GT-seeded full | fast | 2.366 | 13.7 |
| ct_icp | No GT seed 200f | balanced_window | 1.659 | 44.5 |
| ct_icp | GT-seeded full | balanced_window | 1.659 | 34.4 |
| dlio | GT-seeded full | fast | 5.026 | 7.3 |
| dlo | GT-seeded full | fast | 5.026 | 7.3 |
| fast_lio2 | GT-seeded full | fast | 2.328 | 12.3 |
| fast_lio_slam | GT-seeded full | fast | 2.382 | 8.8 |
| floam | GT-seeded full | fast | 5.452 | 28.6 |
| gicp | No GT seed 200f | dense_recent_map | 1.510 | 10.9 |
| gicp | GT-seeded full | fast_recent_map | 1.177 | 25.8 |
| hdl_graph_slam | GT-seeded full | fast | 2.878 | 15.4 |
| isc_loam | GT-seeded full | fast | 2.321 | 35.6 |
| kiss_icp | No GT seed 200f | fast_recent_map | 2.642 | 28.2 |
| kiss_icp | GT-seeded full | fast_recent_map | 2.642 | 24.4 |
| lego_loam | GT-seeded full | fast | 6.498 | 9.5 |
| lins | GT-seeded full | fast | 120.032 | 120.7 |
| lio_sam | GT-seeded full | fast | 2.649 | 24.9 |
| litamin2 | No GT seed 200f | paper_cov_half_threads | 122.275 | 87.3 |
| litamin2 | GT-seeded full | paper_icp_only_half_threads | 1.397 | 43.7 |
| loam_livox | GT-seeded full | fast | 98.811 | 43.8 |
| mulls | GT-seeded full | fast | 4.610 | 3.3 |
| ndt | No GT seed 200f | fast_coarse_map | 122.547 | 25.0 |
| ndt | GT-seeded full | fast_coarse_map | 0.374 | 36.1 |
| point_lio | GT-seeded full | fast | 119.890 | 117.4 |
| small_gicp | GT-seeded full | fast_recent_map | 0.437 | 92.4 |
| suma | GT-seeded full | default | 3.291 | 39.7 |
| vgicp_slam | GT-seeded full | fast | 2.085 | 23.7 |
| voxel_gicp | GT-seeded full | dense_recent_map | 0.640 | 110.1 |
| xicp | GT-seeded full | dense | 0.139 | 58.5 |

**Key finding**: Scan-to-map methods with GT seeding maintain sub-meter accuracy, but LiTAMIN2 and NDT diverge catastrophically without GT seeds. GICP is more robust to initialization. KISS-ICP and CT-ICP are unaffected (already pure odometry).

## 2. Cross-Dataset Default Stability

Does the same variant win across all datasets? Instability here is the core evidence for the variant-first thesis.

### ALOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 6.105 | 5.8 |

**Stability**: 1 unique default(s) across 1 windows.

### BALM2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 2.366 | 13.7 |
| KITTI-kitti_raw_0009_full | fast | 3.338 | 12.7 |
| KITTI-kitti_raw_0061_200 | fast | 2.926 | 13.2 |
| KITTI-kitti_raw_0061_full | fast | 15.574 | 11.4 |
| MCD-KTH | fast | 6.227 | 13.4 |
| MCD-NTU | fast | 0.172 | 12.7 |
| MCD-TUHH | fast | 1.698 | 14.6 |

**Stability**: 1 unique default(s) across 7 windows.

### CT-ICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense_window | 1.254 | 16.6 |
| HDL-400 | fast_window | 1.211 | 2.4 |
| HDL-400 | fast_window | 2.582 | 72.9 |
| Istanbul | balanced_window | 6.820 | 1.6 |
| Istanbul | balanced_window | 7.539 | 1.3 |
| Istanbul | fast_window | 79.761 | 2.7 |
| KITTI-kitti_raw_0009_200 | balanced_window | 1.659 | 34.4 |
| KITTI-kitti_raw_0009_full | balanced_window | 4.673 | 40.6 |
| KITTI-kitti_raw_0061_200 | fast_window | 1.475 | 56.9 |
| KITTI-kitti_raw_0061_full | fast_window | 6.972 | 37.6 |
| MCD-KTH | fast_window | 6.525 | 59.8 |
| MCD-NTU | dense_window | 0.325 | 22.0 |
| MCD-TUHH | fast_window | 3.553 | 71.6 |

**Stability**: 3 unique default(s) across 13 windows.

### CT-LIO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | seed_only_fast | 0.479 | 19.6 |
| HDL-400 | seed_only_fast | 0.488 | 17.5 |

**Stability**: 1 unique default(s) across 2 windows.

### DLIO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 5.026 | 7.3 |

**Stability**: 1 unique default(s) across 1 windows.

### DLO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 5.026 | 7.3 |

**Stability**: 1 unique default(s) across 1 windows.

### FAST-LIO2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 2.328 | 12.3 |
| KITTI-kitti_raw_0009_full | fast | 5.199 | 13.5 |
| KITTI-kitti_raw_0061_200 | fast | 0.634 | 13.6 |
| KITTI-kitti_raw_0061_full | fast | 5.066 | 13.5 |
| MCD-KTH | fast | 6.072 | 12.4 |
| MCD-NTU | fast | 0.025 | 23.7 |
| MCD-TUHH | fast | 1.339 | 14.2 |

**Stability**: 1 unique default(s) across 7 windows.

### FAST-LIO-SLAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 2.382 | 8.8 |
| KITTI-kitti_raw_0009_full | fast | 5.289 | 11.3 |
| KITTI-kitti_raw_0061_200 | fast | 0.660 | 11.2 |
| KITTI-kitti_raw_0061_full | fast | 4.945 | 9.1 |
| MCD-KTH | fast | 6.075 | 9.4 |
| MCD-NTU | fast | 0.028 | 20.4 |
| MCD-TUHH | fast | 1.332 | 12.8 |

**Stability**: 1 unique default(s) across 7 windows.

### FLOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 5.452 | 28.6 |

**Stability**: 1 unique default(s) across 1 windows.

### GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_recent_map | 0.284 | 1.7 |
| HDL-400 | fast_recent_map | 0.215 | 23.3 |
| Istanbul | fast_recent_map | 1.166 | 5.7 |
| Istanbul | fast_recent_map | 0.982 | 4.3 |
| Istanbul | fast_recent_map | 1.074 | 6.3 |
| KITTI-kitti_raw_0009_200 | fast_recent_map | 1.177 | 25.8 |
| KITTI-kitti_raw_0009_full | fast_recent_map | 1.170 | 23.0 |
| KITTI-kitti_raw_0061_200 | fast_recent_map | 0.959 | 25.7 |
| KITTI-kitti_raw_0061_full | fast_recent_map | 1.081 | 22.8 |
| MCD-KTH | fast_recent_map | 0.630 | 24.7 |
| MCD-NTU | dense_recent_map | 0.017 | 13.0 |
| MCD-TUHH | fast_recent_map | 0.317 | 31.2 |

**Stability**: 2 unique default(s) across 12 windows.

### HDL-GRAPH-SLAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 2.878 | 15.4 |
| KITTI-kitti_raw_0009_full | default | 185.826 | 0.2 |
| KITTI-kitti_raw_0061_200 | fast | 6.421 | 19.2 |
| MCD-KTH | fast | 9.241 | 13.9 |
| MCD-NTU | dense | 0.180 | 6.5 |
| MCD-TUHH | dense | 1.373 | 4.2 |

**Stability**: 3 unique default(s) across 6 windows.

### ISC-LOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 2.321 | 35.6 |
| KITTI-kitti_raw_0009_full | fast | 4.323 | 30.5 |
| KITTI-kitti_raw_0061_200 | dense | 0.494 | 23.7 |
| KITTI-kitti_raw_0061_full | fast | 5.439 | 33.5 |
| MCD-KTH | fast | 6.094 | 48.6 |
| MCD-NTU | fast | 0.065 | 50.2 |
| MCD-TUHH | fast | 1.357 | 53.2 |

**Stability**: 2 unique default(s) across 7 windows.

### KISS-ICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_recent_map | 0.218 | 0.4 |
| HDL-400 | fast_recent_map | 1.281 | 11.3 |
| Istanbul | dense_local_map | 144.086 | 3.6 |
| Istanbul | fast_recent_map | 131.692 | 3.7 |
| Istanbul | fast_recent_map | 182.960 | 4.0 |
| KITTI-kitti_raw_0009_200 | fast_recent_map | 2.642 | 24.4 |
| KITTI-kitti_raw_0009_full | fast_recent_map | 5.839 | 21.9 |
| KITTI-kitti_raw_0061_200 | fast_recent_map | 0.679 | 28.3 |
| KITTI-kitti_raw_0061_full | fast_recent_map | 4.623 | 11.2 |
| MCD-KTH | fast_recent_map | 5.568 | 11.3 |
| MCD-NTU | fast_recent_map | 0.026 | 66.7 |
| MCD-TUHH | fast_recent_map | 1.303 | 24.1 |

**Stability**: 2 unique default(s) across 12 windows.

### LEGO-LOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 6.498 | 9.5 |

**Stability**: 1 unique default(s) across 1 windows.

### LINS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 120.032 | 120.7 |
| KITTI-kitti_raw_0009_full | fast | 183.686 | 123.3 |
| KITTI-kitti_raw_0061_200 | fast | 82.393 | 115.2 |
| KITTI-kitti_raw_0061_full | fast | 291.877 | 104.7 |
| MCD-KTH | fast | 7.120 | 166.1 |
| MCD-NTU | dense | 0.111 | 34.9 |
| MCD-TUHH | fast | 1.147 | 173.4 |

**Stability**: 2 unique default(s) across 7 windows.

### LIO-SAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 2.649 | 24.9 |
| KITTI-kitti_raw_0009_full | fast | 5.296 | 20.2 |
| KITTI-kitti_raw_0061_200 | fast | 0.704 | 27.1 |
| KITTI-kitti_raw_0061_full | fast | 6.067 | 21.2 |
| MCD-KTH | fast | 6.074 | 29.7 |
| MCD-NTU | fast | 0.073 | 31.7 |
| MCD-TUHH | fast | 1.314 | 30.0 |

**Stability**: 1 unique default(s) across 7 windows.

### LiTAMIN2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_icp_only_half_threads | 0.168 | 5.2 |
| HDL-400 | paper_cov_half_threads | 0.129 | 94.2 |
| Istanbul | fast_icp_only_half_threads | 1.222 | 20.9 |
| Istanbul | paper_icp_only_half_threads | 0.741 | 17.2 |
| Istanbul | fast_icp_only_half_threads | 1.213 | 23.5 |
| KITTI-kitti_raw_0009_200 | paper_icp_only_half_threads | 1.397 | 43.7 |
| KITTI-kitti_raw_0009_full | fast_icp_only_half_threads | 1.145 | 48.8 |
| KITTI-kitti_raw_0061_200 | fast_cov_half_threads | 0.511 | 67.7 |
| KITTI-kitti_raw_0061_full | fast_icp_only_half_threads | 0.944 | 58.1 |
| MCD-KTH | fast_cov_half_threads | 0.401 | 64.2 |
| MCD-NTU | paper_icp_only_half_threads | 0.045 | 105.6 |
| MCD-TUHH | fast_cov_half_threads | 0.194 | 106.5 |

**Stability**: 4 unique default(s) across 12 windows.

### LOAM-LIVOX

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 98.811 | 43.8 |
| KITTI-kitti_raw_0009_full | fast | 136.097 | 33.1 |
| KITTI-kitti_raw_0061_200 | fast | 78.138 | 50.0 |
| KITTI-kitti_raw_0061_full | fast | 277.148 | 36.6 |
| MCD-KTH | fast | 4.095 | 66.3 |
| MCD-NTU | fast | 0.137 | 69.0 |
| MCD-TUHH | fast | 1.192 | 68.8 |

**Stability**: 1 unique default(s) across 7 windows.

### MULLS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 4.610 | 3.3 |

**Stability**: 1 unique default(s) across 1 windows.

### NDT

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_coarse_map | 0.065 | 0.9 |
| HDL-400 | fast_coarse_map | 0.052 | 32.2 |
| Istanbul | fast_coarse_map | 0.007 | 2.1 |
| Istanbul | fast_coarse_map | 0.005 | 1.9 |
| Istanbul | fast_coarse_map | 0.070 | 2.0 |
| KITTI-kitti_raw_0009_200 | fast_coarse_map | 0.374 | 36.1 |
| KITTI-kitti_raw_0009_full | fast_coarse_map | 0.307 | 30.9 |
| KITTI-kitti_raw_0061_200 | fast_coarse_map | 0.319 | 41.2 |
| KITTI-kitti_raw_0061_full | fast_coarse_map | 0.247 | 23.8 |
| MCD-KTH | fast_coarse_map | 0.208 | 31.2 |
| MCD-NTU | balanced_local_map | 0.014 | 32.7 |
| MCD-TUHH | fast_coarse_map | 0.070 | 40.8 |

**Stability**: 2 unique default(s) across 12 windows.

### POINT-LIO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 119.890 | 117.4 |
| KITTI-kitti_raw_0009_full | fast | 183.384 | 113.1 |
| KITTI-kitti_raw_0061_200 | fast | 82.450 | 92.8 |
| KITTI-kitti_raw_0061_full | fast | 292.011 | 89.5 |
| MCD-KTH | fast | 7.325 | 112.7 |
| MCD-NTU | fast | 0.083 | 77.3 |
| MCD-TUHH | fast | 1.158 | 88.7 |

**Stability**: 1 unique default(s) across 7 windows.

### SMALL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast_recent_map | 0.437 | 92.4 |
| MCD-KTH | fast_recent_map | 0.806 | 107.9 |
| MCD-NTU | dense_recent_map | 0.031 | 56.8 |
| MCD-TUHH | fast_recent_map | 0.466 | 107.2 |

**Stability**: 2 unique default(s) across 4 windows.

### SUMA

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | default | 3.291 | 39.7 |
| KITTI-kitti_raw_0009_full | dense | 5.487 | 25.0 |
| KITTI-kitti_raw_0061_200 | dense | 1.496 | 33.5 |
| KITTI-kitti_raw_0061_full | fast | 32.429 | 110.9 |
| MCD-KTH | fast | 7.419 | 150.2 |
| MCD-NTU | dense | 0.036 | 33.9 |
| MCD-TUHH | default | 1.414 | 59.1 |

**Stability**: 3 unique default(s) across 7 windows.

### VGICP-SLAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 2.085 | 23.7 |
| KITTI-kitti_raw_0009_full | fast | 4.544 | 22.2 |
| KITTI-kitti_raw_0061_200 | fast | 0.903 | 31.0 |
| KITTI-kitti_raw_0061_full | fast | 5.230 | 22.8 |
| MCD-KTH | fast | 6.096 | 20.8 |
| MCD-NTU | fast | 0.026 | 40.3 |
| MCD-TUHH | fast | 1.322 | 21.8 |

**Stability**: 1 unique default(s) across 7 windows.

### VOXEL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | dense_recent_map | 0.640 | 110.1 |
| MCD-KTH | dense_recent_map | 0.981 | 124.2 |
| MCD-NTU | dense_recent_map | 0.121 | 117.2 |
| MCD-TUHH | dense_recent_map | 0.478 | 116.4 |

**Stability**: 1 unique default(s) across 4 windows.

### XICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | dense | 0.139 | 58.5 |
| KITTI-kitti_raw_0009_full | dense | 0.130 | 59.9 |
| KITTI-kitti_raw_0061_200 | fast | 0.171 | 102.0 |
| KITTI-kitti_raw_0061_full | fast | 0.202 | 104.6 |
| MCD-KTH | fast | 0.401 | 84.8 |
| MCD-NTU | dense | 0.095 | 69.0 |
| MCD-TUHH | dense | 0.081 | 72.0 |

**Stability**: 2 unique default(s) across 7 windows.

## 3. Profile Impact Summary

How do profile flags (fast/balanced/dense) affect ATE and FPS? Values averaged across all datasets where the variant ran.

### ALOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 6.187 | 2.7 | 1 |
| fast | 6.105 | 5.8 | 1 |
| kitti_default | 6.463 | 2.6 | 1 |

### BALM2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 4.039 | 1.8 | 7 |
| dense | 4.081 | 0.7 | 7 |
| fast | 4.614 | 13.1 | 7 |

### CT-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_window | 10.377 | 23.9 | 13 |
| dense_window | 11.098 | 12.5 | 13 |
| fast_window | 14.873 | 42.3 | 13 |

### CT-LIO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| history_smoother_dense | 2.112 | 25.6 | 2 |
| imu_preintegration_default | 2.668 | 17.2 | 2 |
| seed_only_fast | 0.484 | 18.5 | 2 |

### DLIO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 5.864 | 1.2 | 1 |
| fast | 5.026 | 7.3 | 1 |
| kitti_default | 5.543 | 2.8 | 1 |

### DLO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 5.864 | 1.2 | 1 |
| fast | 5.026 | 7.3 | 1 |
| kitti_default | 5.543 | 2.6 | 1 |

### FAST-LIO2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 3.068 | 5.3 | 7 |
| dense | 3.212 | 3.5 | 7 |
| fast | 2.952 | 14.7 | 7 |

### FAST-LIO-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 3.068 | 4.4 | 7 |
| dense | 3.180 | 2.8 | 7 |
| fast | 2.959 | 11.9 | 7 |

### FLOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 6.094 | 2.8 | 1 |
| fast | 5.452 | 28.6 | 1 |
| kitti_default | 6.564 | 11.9 | 1 |

### GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.855 | 10.9 | 12 |
| dense_recent_map | 0.830 | 6.3 | 12 |
| fast_recent_map | 0.776 | 18.6 | 12 |

### HDL-GRAPH-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 78.351 | 2.3 | 6 |
| dense | 7.304 | 3.5 | 5 |
| fast | 11.484 | 17.0 | 5 |

### ISC-LOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 3.049 | 9.9 | 7 |
| dense | 3.125 | 22.4 | 7 |
| fast | 2.943 | 42.2 | 7 |

### KISS-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 40.209 | 9.8 | 12 |
| dense_local_map | 40.251 | 6.8 | 12 |
| fast_recent_map | 40.063 | 17.6 | 12 |

### LEGO-LOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 7.249 | 3.5 | 1 |
| fast | 6.498 | 9.5 | 1 |
| kitti_default | 6.066 | 2.9 | 1 |

### LINS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 98.195 | 40.7 | 7 |
| dense | 98.141 | 36.3 | 7 |
| fast | 98.105 | 135.8 | 7 |

### LIO-SAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 3.010 | 8.2 | 7 |
| dense | 3.259 | 12.3 | 7 |
| fast | 3.168 | 26.4 | 7 |

### LiTAMIN2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| fast_cov_half_threads | 0.664 | 48.4 | 12 |
| fast_icp_only_half_threads | 0.664 | 49.3 | 12 |
| paper_cov_half_threads | 0.860 | 47.6 | 12 |
| paper_icp_only_half_threads | 0.860 | 49.9 | 12 |

### LOAM-LIVOX

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 68.215 | 7.6 | 7 |
| dense | 68.216 | 11.5 | 7 |
| fast | 85.088 | 52.5 | 7 |

### MULLS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 5.004 | 1.0 | 1 |
| fast | 4.610 | 3.3 | 1 |
| kitti_default | 4.933 | 1.2 | 1 |

### NDT

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.142 | 15.9 | 12 |
| dense_local_map | 0.159 | 9.5 | 12 |
| fast_coarse_map | 0.146 | 24.0 | 12 |

### POINT-LIO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 98.162 | 2.6 | 7 |
| dense | 98.252 | 16.0 | 7 |
| fast | 98.043 | 98.8 | 7 |

### SMALL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.650 | 58.3 | 4 |
| dense_recent_map | 0.567 | 43.9 | 4 |
| fast_recent_map | 0.455 | 105.3 | 4 |

### SUMA

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 6.906 | 34.5 | 7 |
| dense | 4.652 | 31.7 | 7 |
| fast | 47.109 | 117.6 | 7 |

### VGICP-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.839 | 6.7 | 7 |
| dense | 3.023 | 11.6 | 7 |
| fast | 2.887 | 26.1 | 7 |

### VOXEL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.752 | 26.9 | 4 |
| dense_recent_map | 0.555 | 117.0 | 4 |
| fast_recent_map | 0.830 | 54.3 | 4 |

### XICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 0.183 | 63.6 | 7 |
| dense | 0.182 | 62.3 | 7 |
| fast | 0.319 | 95.6 | 7 |
| no_gt_seed | 95.382 | 67.7 | 7 |
