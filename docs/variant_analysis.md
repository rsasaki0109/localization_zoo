# Variant Analysis

> Generated: 2026-04-07T13:50:26+00:00

This document analyzes **why** variant performance differs across datasets and initialization modes. It complements `decisions.md` (which records **what** was chosen) with **why** the choices diverge.

## 1. GT-Seeded vs Odometry-Chain Initialization

Scan-to-map methods (LiTAMIN2, GICP, NDT) can use GT poses as per-frame initialization or rely on their own odometry chain. KISS-ICP and CT-ICP are always odometry-based.

### Drive 0009 Comparison

| Method | Mode | Default Variant | ATE [m] | FPS |
|---|---|---|---:|---:|
| aloam | GT-seeded full | fast | 6.105 | 5.8 |
| ct_icp | No GT seed 200f | balanced_window | 1.659 | 44.5 |
| ct_icp | GT-seeded full | balanced_window | 1.659 | 34.4 |
| dlio | GT-seeded full | fast | 5.026 | 7.3 |
| dlo | GT-seeded full | fast | 5.026 | 7.3 |
| floam | GT-seeded full | fast | 5.452 | 28.6 |
| gicp | No GT seed 200f | dense_recent_map | 1.510 | 10.9 |
| gicp | GT-seeded full | fast_recent_map | 1.177 | 25.8 |
| kiss_icp | No GT seed 200f | fast_recent_map | 2.642 | 28.2 |
| kiss_icp | GT-seeded full | fast_recent_map | 2.642 | 24.4 |
| lego_loam | GT-seeded full | fast | 6.498 | 9.5 |
| litamin2 | No GT seed 200f | paper_cov_half_threads | 122.275 | 87.3 |
| litamin2 | GT-seeded full | paper_icp_only_half_threads | 1.397 | 43.7 |
| mulls | GT-seeded full | fast | 4.610 | 3.3 |
| ndt | No GT seed 200f | fast_coarse_map | 122.547 | 25.0 |
| ndt | GT-seeded full | fast_coarse_map | 0.374 | 36.1 |
| small_gicp | GT-seeded full | fast_recent_map | 0.437 | 92.4 |
| voxel_gicp | GT-seeded full | dense_recent_map | 0.640 | 110.1 |

**Key finding**: Scan-to-map methods with GT seeding maintain sub-meter accuracy, but LiTAMIN2 and NDT diverge catastrophically without GT seeds. GICP is more robust to initialization. KISS-ICP and CT-ICP are unaffected (already pure odometry).

## 2. Cross-Dataset Default Stability

Does the same variant win across all datasets? Instability here is the core evidence for the variant-first thesis.

### ALOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 6.105 | 5.8 |

**Stability**: 1 unique default(s) across 1 windows.

### CT-ICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_window | 1.211 | 2.4 |
| HDL-400 | fast_window | 1.513 | 2.4 |
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

**Stability**: 3 unique default(s) across 12 windows.

### CT-LIO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | seed_only_fast | 0.412 | 0.4 |

**Stability**: 1 unique default(s) across 1 windows.

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

### FLOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 5.452 | 28.6 |

**Stability**: 1 unique default(s) across 1 windows.

### GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_recent_map | 0.284 | 1.7 |
| HDL-400 | fast_recent_map | 0.193 | 1.7 |
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

### KISS-ICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_recent_map | 0.218 | 0.4 |
| HDL-400 | fast_recent_map | 1.646 | 0.5 |
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

### LiTAMIN2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_icp_only_half_threads | 0.168 | 5.2 |
| HDL-400 | paper_icp_only_half_threads | 0.121 | 6.4 |
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

**Stability**: 3 unique default(s) across 12 windows.

### MULLS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast | 4.610 | 3.3 |

**Stability**: 1 unique default(s) across 1 windows.

### NDT

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_coarse_map | 0.065 | 0.9 |
| HDL-400 | fast_coarse_map | 0.051 | 0.8 |
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

### SMALL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | fast_recent_map | 0.437 | 92.4 |
| MCD-KTH | fast_recent_map | 0.806 | 107.9 |
| MCD-NTU | dense_recent_map | 0.031 | 56.8 |
| MCD-TUHH | fast_recent_map | 0.466 | 107.2 |

**Stability**: 2 unique default(s) across 4 windows.

### VOXEL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_full | dense_recent_map | 0.640 | 110.1 |
| MCD-KTH | dense_recent_map | 0.981 | 124.2 |
| MCD-NTU | dense_recent_map | 0.121 | 117.2 |
| MCD-TUHH | dense_recent_map | 0.478 | 116.4 |

**Stability**: 1 unique default(s) across 4 windows.

## 3. Profile Impact Summary

How do profile flags (fast/balanced/dense) affect ATE and FPS? Values averaged across all datasets where the variant ran.

### ALOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 6.187 | 2.7 | 1 |
| fast | 6.105 | 5.8 | 1 |
| kitti_default | 6.463 | 2.6 | 1 |

### CT-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_window | 11.111 | 20.8 | 12 |
| dense_window | 11.927 | 10.5 | 12 |
| fast_window | 15.818 | 34.2 | 12 |

### CT-LIO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| history_smoother_dense | 1.722 | 0.5 | 1 |
| imu_preintegration_default | 2.224 | 0.5 | 1 |
| seed_only_fast | 0.412 | 0.4 | 1 |

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

### FLOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 6.094 | 2.8 | 1 |
| fast | 5.452 | 28.6 | 1 |
| kitti_default | 6.564 | 11.9 | 1 |

### GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.856 | 9.6 | 12 |
| dense_recent_map | 0.829 | 5.6 | 12 |
| fast_recent_map | 0.774 | 16.8 | 12 |

### KISS-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 40.310 | 9.5 | 12 |
| dense_local_map | 40.305 | 6.7 | 12 |
| fast_recent_map | 40.093 | 16.7 | 12 |

### LEGO-LOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 7.249 | 3.5 | 1 |
| fast | 6.498 | 9.5 | 1 |
| kitti_default | 6.066 | 2.9 | 1 |

### LiTAMIN2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| fast_cov_half_threads | 0.666 | 43.7 | 12 |
| fast_icp_only_half_threads | 0.666 | 44.9 | 12 |
| paper_cov_half_threads | 0.859 | 40.3 | 12 |
| paper_icp_only_half_threads | 0.859 | 42.9 | 12 |

### MULLS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 5.004 | 1.0 | 1 |
| fast | 4.610 | 3.3 | 1 |
| kitti_default | 4.933 | 1.2 | 1 |

### NDT

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.143 | 14.2 | 12 |
| dense_local_map | 0.160 | 8.7 | 12 |
| fast_coarse_map | 0.146 | 21.4 | 12 |

### SMALL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.650 | 58.3 | 4 |
| dense_recent_map | 0.567 | 43.9 | 4 |
| fast_recent_map | 0.455 | 105.3 | 4 |

### VOXEL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.752 | 26.9 | 4 |
| dense_recent_map | 0.555 | 117.0 | 4 |
| fast_recent_map | 0.830 | 54.3 | 4 |
