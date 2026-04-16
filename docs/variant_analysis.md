# Variant Analysis

> Generated: 2026-04-17T11:59:13+00:00

This document analyzes **why** variant performance differs across datasets and initialization modes. It complements `decisions.md` (which records **what** was chosen) with **why** the choices diverge.

## 1. GT-Seeded vs Odometry-Chain Initialization

Scan-to-map methods (LiTAMIN2, GICP, NDT) can use GT poses as per-frame initialization or rely on their own odometry chain. KISS-ICP and CT-ICP are always odometry-based.

### Drive 0009 Comparison

| Method | Mode | Default Variant | ATE [m] | FPS |
|---|---|---|---:|---:|
| aloam | No GT seed 200f | fast | 3.433 | 4.8 |
| aloam | GT-seeded full | fast | 3.433 | 3.4 |
| balm2 | No GT seed 200f | fast | 2.366 | 12.6 |
| balm2 | GT-seeded full | fast | 2.366 | 13.7 |
| ct_icp | No GT seed 200f | balanced_window | 1.659 | 44.5 |
| ct_icp | GT-seeded full | balanced_window | 1.659 | 34.4 |
| dlio | No GT seed 200f | fast | 2.362 | 7.0 |
| dlio | GT-seeded full | fast | 2.362 | 5.3 |
| dlo | No GT seed 200f | fast | 2.362 | 7.1 |
| dlo | GT-seeded full | fast | 2.362 | 5.9 |
| fast_lio2 | No GT seed 200f | fast | 2.328 | 12.7 |
| fast_lio2 | GT-seeded full | fast | 2.328 | 12.3 |
| fast_lio_slam | No GT seed 200f | fast | 2.382 | 11.3 |
| fast_lio_slam | GT-seeded full | fast | 2.382 | 8.8 |
| floam | No GT seed 200f | fast | 3.486 | 28.0 |
| floam | GT-seeded full | fast | 3.486 | 24.6 |
| gicp | No GT seed 200f | dense_recent_map | 1.510 | 10.9 |
| gicp | GT-seeded full | fast_recent_map | 1.177 | 25.8 |
| hdl_graph_slam | No GT seed 200f | default | 122.141 | 1.9 |
| hdl_graph_slam | GT-seeded full | fast | 2.878 | 15.4 |
| isc_loam | No GT seed 200f | fast | 2.321 | 37.6 |
| isc_loam | GT-seeded full | fast | 2.321 | 35.6 |
| kiss_icp | No GT seed 200f | fast_recent_map | 2.642 | 28.2 |
| kiss_icp | GT-seeded full | fast_recent_map | 2.642 | 24.4 |
| lego_loam | No GT seed 200f | fast | 3.216 | 9.9 |
| lego_loam | GT-seeded full | fast | 3.216 | 8.9 |
| lins | No GT seed 200f | fast | 120.032 | 105.0 |
| lins | GT-seeded full | fast | 120.032 | 120.7 |
| lio_sam | No GT seed 200f | fast | 2.649 | 23.1 |
| lio_sam | GT-seeded full | fast | 2.649 | 24.9 |
| litamin2 | No GT seed 200f | paper_cov_half_threads | 122.275 | 87.3 |
| litamin2 | GT-seeded full | paper_icp_only_half_threads | 1.397 | 43.7 |
| loam_livox | No GT seed 200f | fast | 98.811 | 40.2 |
| loam_livox | GT-seeded full | fast | 98.811 | 43.8 |
| mulls | No GT seed 200f | fast | 2.651 | 3.4 |
| mulls | GT-seeded full | fast | 2.651 | 3.1 |
| ndt | No GT seed 200f | fast_coarse_map | 122.547 | 25.0 |
| ndt | GT-seeded full | fast_coarse_map | 0.374 | 36.1 |
| point_lio | No GT seed 200f | fast | 119.890 | 95.6 |
| point_lio | GT-seeded full | fast | 119.890 | 117.4 |
| small_gicp | No GT seed 200f | balanced_local_map | 1.624 | 49.6 |
| small_gicp | GT-seeded full | fast_recent_map | 0.471 | 83.3 |
| suma | No GT seed 200f | default | 3.291 | 82.3 |
| suma | GT-seeded full | default | 3.291 | 39.7 |
| vgicp_slam | No GT seed 200f | fast | 2.085 | 19.1 |
| vgicp_slam | GT-seeded full | fast | 2.085 | 23.7 |
| voxel_gicp | No GT seed 200f | dense_recent_map | 52.522 | 82.1 |
| voxel_gicp | GT-seeded full | dense_recent_map | 0.670 | 93.9 |
| xicp | No GT seed 200f | fast | 121.338 | 81.2 |
| xicp | GT-seeded full | dense | 0.139 | 58.5 |

**Key finding**: Scan-to-map methods with GT seeding maintain sub-meter accuracy, but LiTAMIN2 and NDT diverge catastrophically without GT seeds. GICP is more robust to initialization. KISS-ICP and CT-ICP are unaffected (already pure odometry).

## 2. Cross-Dataset Default Stability

Does the same variant win across all datasets? Instability here is the core evidence for the variant-first thesis.

### ALOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.166 | 19.0 |
| HDL-400 | fast | 0.193 | 13.8 |
| KITTI-kitti_raw_0009_200 | fast | 3.433 | 3.4 |
| KITTI-kitti_raw_0009_full | fast | 6.105 | 5.8 |
| KITTI-kitti_raw_0061_200 | fast | 0.527 | 6.0 |
| KITTI-kitti_raw_0061_full | fast | 3.654 | 6.0 |
| MCD-KTH | fast | 6.100 | 6.7 |
| MCD-NTU | dense | 0.035 | 3.0 |
| MCD-TUHH | fast | 1.374 | 6.5 |

**Stability**: 2 unique default(s) across 9 windows.

### BALM2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.714 | 15.9 |
| HDL-400 | fast | 0.827 | 9.0 |
| KITTI-kitti_raw_0009_200 | fast | 2.366 | 13.7 |
| KITTI-kitti_raw_0009_full | fast | 3.338 | 12.7 |
| KITTI-kitti_raw_0061_200 | fast | 2.926 | 13.2 |
| KITTI-kitti_raw_0061_full | fast | 15.574 | 11.4 |
| MCD-KTH | fast | 6.227 | 13.4 |
| MCD-NTU | fast | 0.172 | 12.7 |
| MCD-TUHH | fast | 1.698 | 14.6 |

**Stability**: 1 unique default(s) across 9 windows.

### CLINS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense | 1.473 | 12.2 |

**Stability**: 1 unique default(s) across 1 windows.

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
| HDL-400 | fast | 0.239 | 12.1 |
| KITTI-kitti_raw_0009_200 | fast | 2.362 | 5.3 |
| KITTI-kitti_raw_0009_full | fast | 5.026 | 7.3 |
| KITTI-kitti_raw_0061_200 | fast | 0.882 | 8.5 |
| KITTI-kitti_raw_0061_full | fast | 7.370 | 10.7 |
| MCD-KTH | fast | 6.081 | 10.4 |
| MCD-NTU | kitti_default | 0.016 | 10.3 |
| MCD-TUHH | fast | 1.344 | 13.0 |

**Stability**: 2 unique default(s) across 8 windows.

### DLO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.118 | 15.0 |
| KITTI-kitti_raw_0009_200 | fast | 2.362 | 5.9 |
| KITTI-kitti_raw_0009_full | fast | 5.026 | 7.3 |
| KITTI-kitti_raw_0061_200 | fast | 0.882 | 8.5 |
| KITTI-kitti_raw_0061_full | fast | 7.370 | 10.6 |
| MCD-KTH | fast | 6.081 | 10.7 |
| MCD-NTU | kitti_default | 0.016 | 10.2 |
| MCD-TUHH | fast | 1.344 | 13.9 |

**Stability**: 2 unique default(s) across 8 windows.

### FAST-LIO2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.114 | 9.1 |
| KITTI-kitti_raw_0009_200 | fast | 2.328 | 12.3 |
| KITTI-kitti_raw_0009_full | fast | 5.199 | 13.5 |
| KITTI-kitti_raw_0061_200 | fast | 0.634 | 13.6 |
| KITTI-kitti_raw_0061_full | fast | 5.066 | 13.5 |
| MCD-KTH | fast | 6.072 | 12.4 |
| MCD-NTU | fast | 0.025 | 23.7 |
| MCD-TUHH | fast | 1.339 | 14.2 |

**Stability**: 1 unique default(s) across 8 windows.

### FAST-LIO-SLAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.128 | 8.4 |
| KITTI-kitti_raw_0009_200 | fast | 2.382 | 8.8 |
| KITTI-kitti_raw_0009_full | fast | 5.289 | 11.3 |
| KITTI-kitti_raw_0061_200 | fast | 0.660 | 11.2 |
| KITTI-kitti_raw_0061_full | fast | 4.945 | 9.1 |
| MCD-KTH | fast | 6.075 | 9.4 |
| MCD-NTU | fast | 0.028 | 20.4 |
| MCD-TUHH | fast | 1.332 | 12.8 |

**Stability**: 1 unique default(s) across 8 windows.

### FLOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.102 | 76.6 |
| HDL-400 | fast | 0.411 | 64.2 |
| KITTI-kitti_raw_0009_200 | fast | 3.486 | 24.6 |
| KITTI-kitti_raw_0009_full | fast | 5.452 | 28.6 |
| KITTI-kitti_raw_0061_200 | fast | 0.756 | 25.9 |
| KITTI-kitti_raw_0061_full | fast | 3.822 | 30.7 |
| MCD-KTH | fast | 6.005 | 31.1 |
| MCD-NTU | fast | 0.152 | 27.0 |
| MCD-TUHH | fast | 1.345 | 27.6 |

**Stability**: 1 unique default(s) across 9 windows.

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
| HDL-400 | fast | 15.656 | 6.7 |
| KITTI-kitti_raw_0009_200 | fast | 2.878 | 15.4 |
| KITTI-kitti_raw_0009_full | default | 185.826 | 0.2 |
| KITTI-kitti_raw_0061_200 | fast | 6.421 | 19.2 |
| MCD-KTH | fast | 9.241 | 13.9 |
| MCD-NTU | dense | 0.180 | 6.5 |
| MCD-TUHH | dense | 1.373 | 4.2 |

**Stability**: 3 unique default(s) across 7 windows.

### ISC-LOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.162 | 37.0 |
| KITTI-kitti_raw_0009_200 | fast | 2.321 | 35.6 |
| KITTI-kitti_raw_0009_full | fast | 4.323 | 30.5 |
| KITTI-kitti_raw_0061_200 | dense | 0.494 | 23.7 |
| KITTI-kitti_raw_0061_full | fast | 5.439 | 33.5 |
| MCD-KTH | fast | 6.094 | 48.6 |
| MCD-NTU | fast | 0.065 | 50.2 |
| MCD-TUHH | fast | 1.357 | 53.2 |

**Stability**: 2 unique default(s) across 8 windows.

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
| HDL-400 | fast | 0.163 | 26.1 |
| HDL-400 | fast | 0.226 | 21.8 |
| KITTI-kitti_raw_0009_200 | fast | 3.216 | 8.9 |
| KITTI-kitti_raw_0009_full | fast | 6.498 | 9.5 |
| KITTI-kitti_raw_0061_200 | fast | 0.481 | 9.7 |
| KITTI-kitti_raw_0061_full | fast | 5.248 | 11.1 |
| MCD-KTH | fast | 6.099 | 9.9 |
| MCD-NTU | fast | 0.079 | 8.4 |
| MCD-TUHH | fast | 1.401 | 10.1 |

**Stability**: 1 unique default(s) across 9 windows.

### LINS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 29.745 | 71.9 |
| KITTI-kitti_raw_0009_200 | fast | 120.032 | 120.7 |
| KITTI-kitti_raw_0009_full | fast | 183.686 | 123.3 |
| KITTI-kitti_raw_0061_200 | fast | 82.393 | 115.2 |
| KITTI-kitti_raw_0061_full | fast | 291.877 | 104.7 |
| MCD-KTH | fast | 7.120 | 166.1 |
| MCD-NTU | dense | 0.111 | 34.9 |
| MCD-TUHH | fast | 1.147 | 173.4 |

**Stability**: 2 unique default(s) across 8 windows.

### LIO-SAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.202 | 18.1 |
| KITTI-kitti_raw_0009_200 | fast | 2.649 | 24.9 |
| KITTI-kitti_raw_0009_full | fast | 5.296 | 20.2 |
| KITTI-kitti_raw_0061_200 | fast | 0.704 | 27.1 |
| KITTI-kitti_raw_0061_full | fast | 6.067 | 21.2 |
| MCD-KTH | fast | 6.074 | 29.7 |
| MCD-NTU | fast | 0.073 | 31.7 |
| MCD-TUHH | fast | 1.314 | 30.0 |

**Stability**: 1 unique default(s) across 8 windows.

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
| HDL-400 | default | 0.091 | 17.8 |
| KITTI-kitti_raw_0009_200 | fast | 98.811 | 43.8 |
| KITTI-kitti_raw_0009_full | fast | 136.097 | 33.1 |
| KITTI-kitti_raw_0061_200 | fast | 78.138 | 50.0 |
| KITTI-kitti_raw_0061_full | fast | 277.148 | 36.6 |
| MCD-KTH | fast | 4.095 | 66.3 |
| MCD-NTU | fast | 0.137 | 69.0 |
| MCD-TUHH | fast | 1.192 | 68.8 |

**Stability**: 2 unique default(s) across 8 windows.

### MULLS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.876 | 9.3 |
| KITTI-kitti_raw_0009_200 | fast | 2.651 | 3.1 |
| KITTI-kitti_raw_0009_full | fast | 4.610 | 3.3 |
| KITTI-kitti_raw_0061_200 | fast | 0.490 | 3.3 |
| KITTI-kitti_raw_0061_full | fast | 11.390 | 3.3 |
| MCD-KTH | fast | 6.297 | 4.1 |
| MCD-NTU | kitti_default | 0.097 | 1.2 |
| MCD-TUHH | fast | 1.206 | 3.8 |

**Stability**: 2 unique default(s) across 8 windows.

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
| HDL-400 | fast | 165.820 | 69.9 |
| KITTI-kitti_raw_0009_200 | fast | 119.890 | 117.4 |
| KITTI-kitti_raw_0009_full | fast | 183.384 | 113.1 |
| KITTI-kitti_raw_0061_200 | fast | 82.450 | 92.8 |
| KITTI-kitti_raw_0061_full | fast | 292.011 | 89.5 |
| MCD-KTH | fast | 7.325 | 112.7 |
| MCD-NTU | fast | 0.083 | 77.3 |
| MCD-TUHH | fast | 1.158 | 88.7 |

**Stability**: 1 unique default(s) across 8 windows.

### SMALL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_recent_map | 0.251 | 110.3 |
| KITTI-kitti_raw_0009_200 | fast_recent_map | 0.471 | 83.3 |
| KITTI-kitti_raw_0009_full | fast_recent_map | 0.437 | 92.4 |
| KITTI-kitti_raw_0061_200 | fast_recent_map | 0.639 | 78.1 |
| KITTI-kitti_raw_0061_full | fast_recent_map | 0.959 | 82.2 |
| MCD-KTH | fast_recent_map | 0.806 | 107.9 |
| MCD-NTU | dense_recent_map | 0.031 | 56.8 |
| MCD-TUHH | fast_recent_map | 0.466 | 107.2 |

**Stability**: 2 unique default(s) across 8 windows.

### SUMA

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | default | 0.248 | 74.6 |
| KITTI-kitti_raw_0009_200 | default | 3.291 | 39.7 |
| KITTI-kitti_raw_0009_full | dense | 5.487 | 25.0 |
| KITTI-kitti_raw_0061_200 | dense | 1.496 | 33.5 |
| KITTI-kitti_raw_0061_full | fast | 32.429 | 110.9 |
| MCD-KTH | fast | 7.419 | 150.2 |
| MCD-NTU | dense | 0.036 | 33.9 |
| MCD-TUHH | default | 1.414 | 59.1 |

**Stability**: 3 unique default(s) across 8 windows.

### VGICP-SLAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.145 | 16.8 |
| KITTI-kitti_raw_0009_200 | fast | 2.085 | 23.7 |
| KITTI-kitti_raw_0009_full | fast | 4.544 | 22.2 |
| KITTI-kitti_raw_0061_200 | fast | 0.903 | 31.0 |
| KITTI-kitti_raw_0061_full | fast | 5.230 | 22.8 |
| MCD-KTH | fast | 6.096 | 20.8 |
| MCD-NTU | fast | 0.026 | 40.3 |
| MCD-TUHH | fast | 1.322 | 21.8 |

**Stability**: 1 unique default(s) across 8 windows.

### VOXEL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense_recent_map | 0.268 | 141.1 |
| KITTI-kitti_raw_0009_200 | dense_recent_map | 0.670 | 93.9 |
| KITTI-kitti_raw_0009_full | dense_recent_map | 0.640 | 110.1 |
| KITTI-kitti_raw_0061_200 | dense_recent_map | 1.041 | 73.7 |
| KITTI-kitti_raw_0061_full | dense_recent_map | 1.062 | 75.4 |
| MCD-KTH | dense_recent_map | 0.981 | 124.2 |
| MCD-NTU | dense_recent_map | 0.121 | 117.2 |
| MCD-TUHH | dense_recent_map | 0.478 | 116.4 |

**Stability**: 1 unique default(s) across 8 windows.

### XICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense | 0.168 | 71.8 |
| KITTI-kitti_raw_0009_200 | dense | 0.139 | 58.5 |
| KITTI-kitti_raw_0009_full | dense | 0.130 | 59.9 |
| KITTI-kitti_raw_0061_200 | fast | 0.171 | 102.0 |
| KITTI-kitti_raw_0061_full | fast | 0.202 | 104.6 |
| MCD-KTH | fast | 0.401 | 84.8 |
| MCD-NTU | dense | 0.095 | 69.0 |
| MCD-TUHH | dense | 0.081 | 72.0 |

**Stability**: 2 unique default(s) across 8 windows.

## 3. Profile Impact Summary

How do profile flags (fast/balanced/dense) affect ATE and FPS? Values averaged across all datasets where the variant ran.

### ALOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 2.514 | 3.9 | 9 |
| fast | 2.404 | 8.1 | 9 |
| kitti_default | 2.638 | 3.3 | 9 |

### BALM2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 3.254 | 1.7 | 9 |
| dense | 3.275 | 0.7 | 9 |
| fast | 3.760 | 12.9 | 9 |

### CLINS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 484.064 | 61.3 | 1 |
| dense | 1.473 | 12.2 | 1 |
| fast | 350.052 | 100.3 | 1 |

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
| dense | 3.134 | 2.5 | 8 |
| fast | 2.917 | 10.5 | 8 |
| kitti_default | 3.036 | 4.7 | 8 |

### DLO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 3.065 | 2.9 | 8 |
| fast | 2.902 | 11.1 | 8 |
| kitti_default | 2.985 | 5.0 | 8 |

### FAST-LIO2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.699 | 5.1 | 8 |
| dense | 2.823 | 3.3 | 8 |
| fast | 2.597 | 14.0 | 8 |

### FAST-LIO-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.699 | 4.2 | 8 |
| dense | 2.796 | 2.6 | 8 |
| fast | 2.605 | 11.4 | 8 |

### FLOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 2.541 | 3.9 | 9 |
| fast | 2.392 | 37.4 | 9 |
| kitti_default | 2.724 | 16.6 | 9 |

### GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.855 | 10.9 | 12 |
| dense_recent_map | 0.830 | 6.3 | 12 |
| fast_recent_map | 0.776 | 18.6 | 12 |

### HDL-GRAPH-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 69.223 | 2.4 | 7 |
| dense | 8.390 | 3.3 | 6 |
| fast | 12.179 | 15.3 | 6 |

### ISC-LOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.692 | 12.0 | 8 |
| dense | 2.755 | 22.1 | 8 |
| fast | 2.595 | 41.5 | 8 |

### KISS-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 40.209 | 9.8 | 12 |
| dense_local_map | 40.251 | 6.8 | 12 |
| fast_recent_map | 40.063 | 17.6 | 12 |

### LEGO-LOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 3.031 | 5.0 | 9 |
| fast | 2.601 | 12.8 | 9 |
| kitti_default | 2.582 | 4.3 | 9 |

### LINS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 119.795 | 40.5 | 8 |
| dense | 105.839 | 33.6 | 8 |
| fast | 89.560 | 127.8 | 8 |

### LIO-SAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.668 | 7.8 | 8 |
| dense | 2.884 | 11.3 | 8 |
| fast | 2.797 | 25.4 | 8 |

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
| default | 59.700 | 8.9 | 8 |
| dense | 59.699 | 11.4 | 8 |
| fast | 74.817 | 52.4 | 8 |

### MULLS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 3.356 | 1.3 | 8 |
| fast | 3.464 | 4.1 | 8 |
| kitti_default | 3.401 | 1.5 | 8 |

### NDT

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.142 | 15.9 | 12 |
| dense_local_map | 0.159 | 9.5 | 12 |
| fast_coarse_map | 0.146 | 24.0 | 12 |

### POINT-LIO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 99.415 | 5.8 | 8 |
| dense | 88.212 | 14.4 | 8 |
| fast | 106.515 | 95.2 | 8 |

### SMALL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.754 | 56.1 | 8 |
| dense_recent_map | 0.698 | 43.0 | 8 |
| fast_recent_map | 0.517 | 96.9 | 8 |

### SUMA

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 6.073 | 39.5 | 8 |
| dense | 4.094 | 31.1 | 8 |
| fast | 41.453 | 123.9 | 8 |

### VGICP-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.506 | 7.2 | 8 |
| dense | 2.667 | 11.1 | 8 |
| fast | 2.544 | 24.9 | 8 |

### VOXEL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.779 | 23.0 | 8 |
| dense_recent_map | 0.658 | 106.5 | 8 |
| fast_recent_map | 0.944 | 50.5 | 8 |

### XICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 0.197 | 67.2 | 8 |
| dense | 0.180 | 63.5 | 8 |
| fast | 0.361 | 99.2 | 8 |
| no_gt_seed | 84.182 | 70.7 | 8 |
