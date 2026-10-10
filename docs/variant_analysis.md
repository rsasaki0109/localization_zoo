# Variant Analysis

> Generated: 2026-10-10T21:33:52+00:00

This document analyzes **why** variant performance differs across datasets and initialization modes. It complements `decisions.md` (which records **what** was chosen) with **why** the choices diverge.

## 1. GT-Seeded vs Odometry-Chain Initialization

Scan-to-map methods (LiTAMIN2, GICP, NDT) can use GT poses as per-frame initialization or rely on their own odometry chain. KISS-ICP and CT-ICP are always odometry-based.

### Drive 0009 Comparison

| Method | Mode | Default Variant | ATE [m] | FPS |
|---|---|---|---:|---:|
| aloam | No GT seed 200f | fast | 3.470 | 4.2 |
| aloam | GT-seeded full | fast | 3.470 | 3.1 |
| balm2 | No GT seed 200f | fast | 2.405 | 6.3 |
| balm2 | GT-seeded full | fast | 2.405 | 4.4 |
| ct_icp | GT-seeded 200f | cluster_d_ms_chol | 2.579 | 7.2 |
| ct_icp | No GT seed 200f | balanced_window | 1.659 | 44.5 |
| ct_icp | GT-seeded full | fast_window | 2.692 | 16.9 |
| dlio | No GT seed 200f | fast | 2.362 | 7.0 |
| dlio | GT-seeded full | fast | 2.362 | 5.3 |
| dlo | No GT seed 200f | fast | 2.362 | 7.1 |
| dlo | GT-seeded full | fast | 2.362 | 5.9 |
| fast_lio2 | No GT seed 200f | fast | 2.328 | 12.7 |
| fast_lio2 | GT-seeded full | fast | 2.328 | 12.3 |
| fast_lio_slam | No GT seed 200f | fast | 2.382 | 11.3 |
| fast_lio_slam | GT-seeded full | fast | 2.382 | 8.8 |
| fast_livo2 | GT-seeded full | fast | 31.368 | 8.3 |
| floam | No GT seed 200f | fast | 3.486 | 28.0 |
| floam | GT-seeded full | fast | 3.486 | 24.6 |
| gicp | No GT seed 200f | fast_recent_map | 2.068 | 8.6 |
| gicp | GT-seeded full | fast_recent_map | 1.151 | 11.8 |
| hdl_graph_slam | No GT seed 200f | default | 122.141 | 1.9 |
| hdl_graph_slam | GT-seeded full | fast | 2.878 | 15.4 |
| imu_dead_reckoning | GT-seeded full | zupt_kitti_0009 | 210.224 | 1342750.9 |
| isc_loam | No GT seed 200f | fast | 2.321 | 37.6 |
| isc_loam | GT-seeded full | fast | 2.321 | 35.6 |
| kiss_icp | No GT seed 200f | fast_recent_map | 2.578 | 18.2 |
| kiss_icp | GT-seeded full | fast_recent_map | 2.578 | 18.7 |
| lego_loam | No GT seed 200f | fast | 3.216 | 9.9 |
| lego_loam | GT-seeded full | fast | 3.216 | 8.9 |
| lins | No GT seed 200f | fast | 120.032 | 105.0 |
| lins | GT-seeded full | fast | 120.032 | 120.7 |
| lio_sam | No GT seed 200f | fast | 2.649 | 23.1 |
| lio_sam | GT-seeded full | fast | 2.649 | 24.9 |
| litamin2 | GT-seeded 200f | cluster_t1_seeded | 0.644 | 27.9 |
| litamin2 | No GT seed 200f | fast_icp_only_half_threads | 6.290 | 30.5 |
| litamin2 | GT-seeded full | fast_icp_only_half_threads | 1.067 | 31.3 |
| loam_livox | No GT seed 200f | fast | 98.811 | 40.2 |
| loam_livox | GT-seeded full | fast | 98.811 | 43.8 |
| mulls | No GT seed 200f | fast | 2.695 | 1.2 |
| mulls | GT-seeded full | fast | 4.610 | 3.3 |
| ndt | No GT seed 200f | fast_coarse_map | 121.012 | 14.3 |
| ndt | GT-seeded full | fast_coarse_map | 0.386 | 20.4 |
| okvis | GT-seeded full | fast | 31.400 | 524.0 |
| point_lio | No GT seed 200f | fast | 119.890 | 95.6 |
| point_lio | GT-seeded full | fast | 119.890 | 117.4 |
| small_gicp | No GT seed 200f | balanced_local_map | 2.275 | 18.7 |
| small_gicp | GT-seeded full | fast_recent_map | 0.473 | 23.7 |
| suma | No GT seed 200f | dense | 2.245 | 14.6 |
| suma | GT-seeded full | default | 3.377 | 19.2 |
| vgicp_slam | No GT seed 200f | default | 1.674 | 4.4 |
| vgicp_slam | GT-seeded full | fast | 1.985 | 6.2 |
| vins_fusion | GT-seeded full | fast | 85.802 | 83.5 |
| voxel_gicp | No GT seed 200f | fast_recent_map | 22.288 | 13.3 |
| voxel_gicp | GT-seeded full | dense_recent_map | 0.644 | 23.3 |
| xicp | No GT seed 200f | default | 22.317 | 24.6 |
| xicp | GT-seeded full | default | 0.151 | 25.2 |

**Key finding**: Scan-to-map methods with GT seeding maintain sub-meter accuracy, but LiTAMIN2 and NDT diverge catastrophically without GT seeds. GICP is more robust to initialization. KISS-ICP and CT-ICP are unaffected (already pure odometry).

## 2. Cross-Dataset Default Stability

Does the same variant win across all datasets? Instability here is the core evidence for the variant-first thesis.

### ALOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.193 | 13.8 |
| KITTI-kitti_raw_0009_200 | fast | 3.470 | 3.1 |
| KITTI-kitti_raw_0009_full | fast | 6.105 | 5.8 |
| KITTI-kitti_raw_0061_200 | fast | 0.527 | 6.0 |
| KITTI-kitti_raw_0061_full | fast | 3.654 | 6.0 |
| MCD-KTH | fast | 6.100 | 6.7 |
| MCD-NTU | dense | 0.035 | 3.0 |
| MCD-TUHH | fast | 1.374 | 6.5 |
| dogfooding_results/kitti_seq_00_full | fast | 19.121 | 3.0 |
| dogfooding_results/kitti_seq_02_full | fast | 70.870 | 3.2 |
| dogfooding_results/kitti_seq_05_full | fast | 7.796 | 3.0 |
| dogfooding_results/kitti_seq_07_full | fast | 4.030 | 1.7 |
| dogfooding_results/kitti_seq_08_full | fast | 26.170 | 2.9 |

**Stability**: 2 unique default(s) across 13 windows.

### BALM2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.827 | 9.0 |
| KITTI-kitti_raw_0009_200 | fast | 2.405 | 4.4 |
| KITTI-kitti_raw_0009_full | fast | 3.338 | 12.7 |
| KITTI-kitti_raw_0061_200 | fast | 2.926 | 13.2 |
| KITTI-kitti_raw_0061_full | fast | 15.574 | 11.4 |
| MCD-KTH | fast | 6.227 | 13.4 |
| MCD-NTU | fast | 0.172 | 12.7 |
| MCD-TUHH | fast | 1.698 | 14.6 |

**Stability**: 1 unique default(s) across 8 windows.

### CLINS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense | 1.473 | 12.2 |

**Stability**: 1 unique default(s) across 1 windows.

### CT-ICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense_window | 1.254 | 18.7 |
| HDL-400 | fast_window | 1.211 | 2.4 |
| HDL-400 | fast_window | 2.582 | 54.9 |
| Istanbul | balanced_window | 6.820 | 1.6 |
| Istanbul | balanced_window | 7.539 | 1.3 |
| Istanbul | fast_window | 79.761 | 2.7 |
| KITTI-kitti_raw_0009_200 | cluster_d_ms_chol | 2.579 | 7.2 |
| KITTI-kitti_raw_0009_200 | fast_window | 2.692 | 16.9 |
| KITTI-kitti_raw_0009_full | fast_window | 3.871 | 12.7 |
| KITTI-kitti_raw_0009_full | balanced_reference | 4.105 | 7.4 |
| KITTI-kitti_raw_0061_200 | fast_reference | 1.475 | 59.4 |
| KITTI-kitti_raw_0061_200 | fast_window | 1.475 | 56.9 |
| KITTI-kitti_raw_0061_full | fast_window | 6.972 | 37.6 |
| KITTI-kitti_raw_0061_full | fast_window_reference | 6.972 | 57.8 |
| MCD-KTH | fast_window | 6.525 | 57.2 |
| MCD-KTH | dense_reference | 6.115 | 18.0 |
| MCD-KTH | dense_seeded_reference | 2.778 | 28.3 |
| MCD-NTU | dense_window | 0.325 | 18.6 |
| MCD-NTU | dense_reference | 0.325 | 21.9 |
| MCD-NTU | dense_seeded_reference | 0.451 | 28.4 |
| MCD-TUHH | fast_window | 3.553 | 51.4 |
| MCD-TUHH | dense_reference | 1.652 | 22.8 |
| MCD-TUHH | dense_seeded_reference | 1.182 | 27.4 |
| dogfooding_results/kitti_seq_00_108 | fast_window | 2.824 | 74.9 |
| dogfooding_results/kitti_seq_00_full | corr_5 | 12.931 | 9.8 |
| dogfooding_results/kitti_seq_00_full | bare_corr_8 | 17.568 | 13.8 |
| dogfooding_results/kitti_seq_00_full | cauchy_4_0 | 18.895 | 10.6 |
| dogfooding_results/kitti_seq_00_full | iter_1 | 14.099 | 9.8 |
| dogfooding_results/kitti_seq_00_full | corr_8 | 16.778 | 9.5 |
| dogfooding_results/kitti_seq_00_full | arch_tuned_all_combined | 93.156 | 16.6 |
| dogfooding_results/kitti_seq_00_full | fine_sigma_0_25 | 12.351 | 8.4 |
| dogfooding_results/kitti_seq_00_full | map_20_reference | 18.370 | 10.4 |
| dogfooding_results/kitti_seq_00_full | balanced_window | 19.413 | 13.0 |
| dogfooding_results/kitti_seq_00_full | radius_1 | 14.817 | 10.2 |
| dogfooding_results/kitti_seq_00_full | cluster_a_seeded | 5.855 | 6.9 |
| dogfooding_results/kitti_seq_00_full | minus_ms_chol | 12.931 | 11.5 |
| dogfooding_results/kitti_seq_00_full | map_50_plus_corr_5 | 14.894 | 12.5 |
| dogfooding_results/kitti_seq_00_full | bare_map_50 | 15.753 | 11.2 |
| dogfooding_results/kitti_seq_00_full | plus_corr_5 | 12.931 | 11.4 |
| dogfooding_results/kitti_seq_00_full | c2f_reference | 14.099 | 8.7 |
| dogfooding_results/kitti_seq_02_full | baseline_map_20 | 68.972 | 8.2 |
| dogfooding_results/kitti_seq_02_full | default_reference | 68.972 | 10.3 |
| dogfooding_results/kitti_seq_02_full | corr_5 | 65.305 | 10.4 |
| dogfooding_results/kitti_seq_02_full | ms_chol_corr_5 | 65.305 | 8.6 |
| dogfooding_results/kitti_seq_02_full | baseline_reference | 68.972 | 10.7 |
| dogfooding_results/kitti_seq_02_full | map_15 | 76.464 | 4.4 |
| dogfooding_results/kitti_seq_02_full | map_15 | 73.230 | 11.7 |
| dogfooding_results/kitti_seq_02_full | current_winner | 93.642 | 13.0 |
| dogfooding_results/kitti_seq_05_full | arch_tuned_map_50 | 8.844 | 10.2 |
| dogfooding_results/kitti_seq_05_full | corr_4_reference | 9.485 | 11.6 |
| dogfooding_results/kitti_seq_05_full | arch_tuned_map_50 | 8.844 | 11.3 |
| dogfooding_results/kitti_seq_05_full | bare_map_30 | 11.707 | 7.0 |
| dogfooding_results/kitti_seq_05_full | velocity_reg_005 | 11.949 | 5.1 |
| dogfooding_results/kitti_seq_05_full | map_50_c2f_plus_corr_4 | 9.305 | 9.1 |
| dogfooding_results/kitti_seq_07_108 | fast_window | 0.978 | 77.3 |
| dogfooding_results/kitti_seq_07_full | ms_chol_plus_simplified_a | 2.010 | 9.9 |
| dogfooding_results/kitti_seq_07_full | corr_8 | 2.049 | 14.1 |
| dogfooding_results/kitti_seq_07_full | velocity_reg_01 | 2.231 | 18.5 |
| dogfooding_results/kitti_seq_07_full | dense_window | 2.842 | 17.5 |
| dogfooding_results/kitti_seq_07_full | cluster_d_full_seeded | 1.603 | 11.2 |
| dogfooding_results/kitti_seq_07_full | ms_chol_map_50 | 1.472 | 12.7 |
| dogfooding_results/kitti_seq_07_full | velocity_reg_01 | 2.490 | 6.0 |
| dogfooding_results/kitti_seq_07_full | simplified_seq_00_pattern | 2.010 | 11.8 |
| dogfooding_results/kitti_seq_08_full | cholesky_c2f_no_ms | 37.043 | 12.2 |
| dogfooding_results/kitti_seq_08_full | corr_8 | 33.860 | 10.2 |
| dogfooding_results/kitti_seq_08_full | cauchy_2_5 | 31.958 | 11.9 |
| dogfooding_results/kitti_seq_08_full | iter_6 | 30.418 | 10.4 |
| dogfooding_results/kitti_seq_08_full | cluster_a_seeded | 6.813 | 6.8 |
| dogfooding_results/kitti_seq_08_full | c2f_only_map_20 | 37.043 | 8.6 |
| dogfooding_results/kitti_seq_08_full | default_reference | 28.427 | 10.9 |
| dogfooding_results/kitti_seq_08_full | velocity_reg_001 | 40.362 | 9.3 |
| dogfooding_results/kitti_seq_08_full | simplified_seq_00_pattern | 28.427 | 8.5 |
| dogfooding_results/mulran_parkinglot_120 | fast_window | 16.474 | 74.7 |
| dogfooding_results/mulran_parkinglot_120 | cluster_a_with_seed | 2.547 | 17.9 |
| dogfooding_results/mulran_parkinglot_full | cluster_a_with_seed | 9.186 | 14.6 |
| dogfooding_results/mulran_parkinglot_full | fast_window | 80.958 | 59.7 |

**Stability**: 46 unique default(s) across 76 windows.

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

### FAST-LIVO2

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 31.368 | 8.3 |
| KITTI-kitti_raw_0009_full | fast | 49.629 | 7.5 |
| KITTI-kitti_raw_0061_200 | fast | 27.581 | 7.2 |
| KITTI-kitti_raw_0061_full | fast | 96.345 | 14.0 |

**Stability**: 1 unique default(s) across 4 windows.

### FLOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.411 | 64.2 |
| KITTI-kitti_raw_0009_200 | fast | 3.486 | 24.6 |
| KITTI-kitti_raw_0009_full | fast | 5.452 | 28.6 |
| KITTI-kitti_raw_0061_200 | fast | 0.756 | 25.9 |
| KITTI-kitti_raw_0061_full | fast | 3.822 | 30.7 |
| MCD-KTH | fast | 6.005 | 31.1 |
| MCD-NTU | fast | 0.152 | 27.0 |
| MCD-TUHH | fast | 1.345 | 27.6 |
| dogfooding_results/kitti_seq_00_full | kitti_default | 19.164 | 6.5 |
| dogfooding_results/kitti_seq_02_full | kitti_default | 77.796 | 5.9 |
| dogfooding_results/kitti_seq_05_full | kitti_default | 6.780 | 2.0 |
| dogfooding_results/kitti_seq_07_full | fast | 5.026 | 8.7 |
| dogfooding_results/kitti_seq_08_full | kitti_default | 16.516 | 6.0 |

**Stability**: 2 unique default(s) across 13 windows.

### GENZ-ICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| dogfooding_results/nclt_2013_01_10_120 | default | 3.513 | 7.6 |

**Stability**: 1 unique default(s) across 1 windows.

### GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_recent_map | 0.284 | 1.7 |
| HDL-400 | fast_recent_map | 0.215 | 23.3 |
| Istanbul | fast_recent_map | 1.166 | 5.7 |
| Istanbul | fast_recent_map | 0.982 | 4.3 |
| Istanbul | fast_recent_map | 1.074 | 6.3 |
| KITTI-kitti_raw_0009_200 | fast_recent_map | 1.151 | 11.8 |
| KITTI-kitti_raw_0009_full | fast_recent_map | 1.129 | 8.4 |
| KITTI-kitti_raw_0061_200 | fast_recent_map | 0.959 | 25.7 |
| KITTI-kitti_raw_0061_full | fast_recent_map | 1.081 | 22.8 |
| MCD-KTH | fast_recent_map | 0.630 | 24.7 |
| MCD-NTU | dense_recent_map | 0.017 | 13.0 |
| MCD-TUHH | fast_recent_map | 0.317 | 31.2 |
| dogfooding_results/mulran_parkinglot_120 | fast_recent_map | 0.644 | 27.7 |
| dogfooding_results/mulran_parkinglot_full | fast_recent_map | 1.149 | 30.3 |

**Stability**: 2 unique default(s) across 14 windows.

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

### IMU-DEAD-RECKONING

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| /media/sasaki/aiueo/loc_zoo/dogfooding_results/nclt_2013_01_10_full | nhc_zupt_full | 9605.455 | 800357.3 |
| KITTI-kitti_raw_0009_200 | zupt_kitti_0009 | 210.224 | 1342750.9 |
| KITTI-kitti_raw_0009_full | nhc_zupt_kitti_0009_full | 1067.301 | 1567725.4 |
| dogfooding_results/nclt_2013_01_10_120 | zupt | 2.887 | 1054259.2 |

**Stability**: 4 unique default(s) across 4 windows.

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
| KITTI-kitti_raw_0009_200 | fast_recent_map | 2.578 | 18.7 |
| KITTI-kitti_raw_0009_full | fast_recent_map | 4.207 | 10.8 |
| KITTI-kitti_raw_0061_200 | fast_recent_map | 0.679 | 28.3 |
| KITTI-kitti_raw_0061_full | fast_recent_map | 4.623 | 11.2 |
| MCD-KTH | fast_recent_map | 5.568 | 11.3 |
| MCD-NTU | fast_recent_map | 0.026 | 66.7 |
| MCD-TUHH | fast_recent_map | 1.303 | 24.1 |
| dogfooding_results/kitti_seq_00_full | upstream_profile_elevation | 9.037 | 32.5 |
| dogfooding_results/kitti_seq_00_full | dense_profile | 12.323 | 1.2 |
| dogfooding_results/kitti_seq_02_full | balanced_reference | 71.183 | 3.1 |
| dogfooding_results/kitti_seq_05_full | dense_profile | 4.556 | 1.6 |
| dogfooding_results/kitti_seq_07_full | balanced_reference | 2.238 | 28.3 |
| dogfooding_results/kitti_seq_07_full | balanced_reference | 2.238 | 3.4 |
| dogfooding_results/kitti_seq_08_full | fast_profile | 18.085 | 2.2 |
| dogfooding_results/mulran_parkinglot_120 | fast_recent_map | 15.641 | 27.3 |
| dogfooding_results/mulran_parkinglot_full | fast_recent_map | 74.337 | 26.9 |

**Stability**: 6 unique default(s) across 21 windows.

### L-LO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| dogfooding_results/kitti_seq_00_full | seq07_tuned | 15.664 | 4.4 |
| dogfooding_results/kitti_seq_02_full | default | 58.272 | 3.2 |
| dogfooding_results/kitti_seq_05_full | default | 8.785 | 5.3 |
| dogfooding_results/kitti_seq_07_full | cluster_cell_0p15 | 2.890 | 17.9 |
| dogfooding_results/kitti_seq_07_full | sor_std_2 | 1.998 | 6.8 |
| dogfooding_results/kitti_seq_07_full | cell_0p35_gtol_0p15_range_80_min15 | 2.561 | 7.7 |
| dogfooding_results/kitti_seq_07_full | default | 2.045 | 5.1 |
| dogfooding_results/kitti_seq_07_full | pitch_band_10_40 | 1.957 | 5.1 |
| dogfooding_results/kitti_seq_07_full | voxel_0p4 | 3.178 | 22.9 |
| dogfooding_results/kitti_seq_07_full | rounds_600_tol_1e5 | 2.045 | 5.9 |
| dogfooding_results/kitti_seq_08_full | default | 32.662 | 3.7 |

**Stability**: 8 unique default(s) across 11 windows.

### LEGO-LOAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.226 | 21.8 |
| KITTI-kitti_raw_0009_200 | fast | 3.216 | 8.9 |
| KITTI-kitti_raw_0009_full | fast | 6.498 | 9.5 |
| KITTI-kitti_raw_0061_200 | fast | 0.481 | 9.7 |
| KITTI-kitti_raw_0061_full | fast | 5.248 | 11.1 |
| MCD-KTH | fast | 6.099 | 9.9 |
| MCD-NTU | fast | 0.079 | 8.4 |
| MCD-TUHH | fast | 1.401 | 10.1 |
| dogfooding_results/kitti_seq_00_full | kitti_default | 12.833 | 1.4 |
| dogfooding_results/kitti_seq_02_full | kitti_default | 41.756 | 1.0 |
| dogfooding_results/kitti_seq_05_full | kitti_default | 6.449 | 0.8 |
| dogfooding_results/kitti_seq_07_full | fast | 4.196 | 2.8 |
| dogfooding_results/kitti_seq_08_full | kitti_default | 17.896 | 0.9 |

**Stability**: 2 unique default(s) across 13 windows.

### LF-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| dogfooding_results/kitti_seq_00_full | no_mitigation | 7.848 | 3.8 |
| dogfooding_results/kitti_seq_02_full | no_mitigation | 27.186 | 3.6 |
| dogfooding_results/kitti_seq_05_full | no_mitigation | 5.556 | 5.4 |
| dogfooding_results/kitti_seq_07_full | paper_default | 0.646 | 8.8 |
| dogfooding_results/kitti_seq_07_full | paper_default | 0.646 | 4.1 |
| dogfooding_results/kitti_seq_08_full | no_mitigation | 16.280 | 2.9 |

**Stability**: 2 unique default(s) across 6 windows.

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
| HDL-400 | fast_cov_half_threads | 0.111 | 80.7 |
| Istanbul | fast_icp_only_half_threads | 1.222 | 20.9 |
| Istanbul | paper_icp_only_half_threads | 0.741 | 17.2 |
| Istanbul | fast_icp_only_half_threads | 1.213 | 23.5 |
| KITTI-kitti_raw_0009_200 | cluster_t1_seeded | 0.644 | 27.9 |
| KITTI-kitti_raw_0009_200 | fast_icp_only_half_threads | 1.067 | 31.3 |
| KITTI-kitti_raw_0009_full | cluster_t1_seeded | 0.676 | 18.0 |
| KITTI-kitti_raw_0009_full | fast_cov_half_threads | 1.064 | 18.9 |
| KITTI-kitti_raw_0061_200 | cluster_t1_seeded | 0.517 | 48.1 |
| KITTI-kitti_raw_0061_200 | fast_cov_half_threads | 0.511 | 67.7 |
| KITTI-kitti_raw_0061_full | cluster_t1_seeded | 0.600 | 39.8 |
| KITTI-kitti_raw_0061_full | fast_icp_only_half_threads | 0.944 | 58.1 |
| MCD-KTH | cluster_t1_seeded | 0.192 | 20.1 |
| MCD-KTH | fast_icp_only_half_threads | 0.401 | 91.6 |
| MCD-NTU | cluster_t1_seeded | 0.021 | 43.4 |
| MCD-NTU | paper_icp_only_half_threads | 0.045 | 81.2 |
| MCD-TUHH | cluster_t1_seeded | 0.132 | 39.5 |
| MCD-TUHH | fast_icp_only_half_threads | 0.194 | 107.2 |
| dogfooding_results/kitti_seq_00_full | fast_icp_only_half_threads | 0.998 | 108.9 |
| dogfooding_results/kitti_seq_00_full | fast_cov_no_gt_seed | 110.484 | 98.9 |
| dogfooding_results/kitti_seq_00_full | tuned_voxel1_iter12 | 13.464 | 60.9 |
| dogfooding_results/kitti_seq_00_full | tuned_voxel2_iter12_seeded | 1.117 | 86.8 |
| dogfooding_results/kitti_seq_02_full | cluster_t1_seeded | 0.728 | 68.3 |
| dogfooding_results/kitti_seq_02_full | cov_floor_1e_4 | 51.895 | 91.8 |
| dogfooding_results/kitti_seq_05_full | fast_seeded_reference | 0.957 | 17.2 |
| dogfooding_results/kitti_seq_05_full | cov_floor_1e_4 | 6.565 | 93.4 |
| dogfooding_results/kitti_seq_07_full | coarse_to_fine_3_2_1_elevation | 2.086 | 47.5 |
| dogfooding_results/kitti_seq_07_full | fast_seeded_reference | 0.836 | 94.7 |
| dogfooding_results/kitti_seq_07_full | cov_floor_1e_4 | 2.202 | 106.9 |
| dogfooding_results/kitti_seq_08_full | fast_seeded_reference | 1.130 | 104.9 |
| dogfooding_results/kitti_seq_08_full | cov_floor_1e_4 | 19.672 | 94.2 |
| dogfooding_results/mulran_parkinglot_120 | cluster_t1_seeded | 0.212 | 30.6 |
| dogfooding_results/mulran_parkinglot_120 | fast_cov_half_threads | 0.498 | 90.2 |
| dogfooding_results/mulran_parkinglot_full | cluster_t1_seeded | 0.303 | 34.4 |
| dogfooding_results/mulran_parkinglot_full | fast_icp_only_half_threads | 0.711 | 118.6 |
| dogfooding_results/nclt_2012_12_01_5000 | voxel_0_5_t1 | 0.519 | 4.4 |
| dogfooding_results/nclt_2013_01_10_600 | default_voxel_2_0 | 0.380 | 14.7 |

**Stability**: 12 unique default(s) across 38 windows.

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
| KITTI-kitti_raw_0009_full | fast | 4.610 | 3.3 |
| KITTI-kitti_raw_0061_200 | fast | 0.490 | 3.3 |
| KITTI-kitti_raw_0061_full | fast | 11.390 | 3.3 |
| MCD-KTH | fast | 6.297 | 4.1 |
| MCD-NTU | kitti_default | 0.097 | 1.2 |
| MCD-TUHH | fast | 1.206 | 3.8 |
| dogfooding_results/kitti_seq_00_full | fast | 56.820 | 1.8 |
| dogfooding_results/kitti_seq_02_full | fast | 260.878 | 1.7 |
| dogfooding_results/kitti_seq_05_full | fast | 26.886 | 1.7 |
| dogfooding_results/kitti_seq_07_full | fast | 10.501 | 4.1 |
| dogfooding_results/kitti_seq_08_full | fast | 87.759 | 1.7 |

**Stability**: 2 unique default(s) across 12 windows.

### NDT

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_coarse_map | 0.065 | 0.9 |
| HDL-400 | fast_coarse_map | 0.052 | 32.2 |
| Istanbul | fast_coarse_map | 0.007 | 2.1 |
| Istanbul | fast_coarse_map | 0.005 | 1.9 |
| Istanbul | fast_coarse_map | 0.070 | 2.0 |
| KITTI-kitti_raw_0009_200 | fast_coarse_map | 0.386 | 20.4 |
| KITTI-kitti_raw_0009_full | fast_coarse_map | 0.299 | 14.1 |
| KITTI-kitti_raw_0061_200 | fast_coarse_map | 0.319 | 41.2 |
| KITTI-kitti_raw_0061_full | fast_coarse_map | 0.247 | 23.8 |
| MCD-KTH | fast_coarse_map | 0.208 | 31.2 |
| MCD-NTU | balanced_local_map | 0.014 | 32.7 |
| MCD-TUHH | fast_coarse_map | 0.070 | 40.8 |
| dogfooding_results/kitti_seq_00_full | t1_transfer_r03_i12 | 0.023 | 11.7 |
| dogfooding_results/kitti_seq_02_full | t1_transfer_r05_i12 | 0.059 | 0.2 |
| dogfooding_results/kitti_seq_05_full | t1_transfer_r05_i12 | 0.059 | 0.2 |
| dogfooding_results/kitti_seq_07_full | fast_profile | 0.279 | 43.6 |
| dogfooding_results/kitti_seq_08_full | t1_transfer_r05_i12 | 0.076 | 0.3 |

**Stability**: 5 unique default(s) across 17 windows.

### OKVIS

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 31.400 | 524.0 |
| KITTI-kitti_raw_0009_full | fast | 108.629 | 1146.6 |
| KITTI-kitti_raw_0061_200 | fast | 29.001 | 894.7 |
| KITTI-kitti_raw_0061_full | fast | 225.666 | 1717.1 |

**Stability**: 1 unique default(s) across 4 windows.

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

### RKO-LIO

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| dogfooding_results/nclt_2013_01_10_120 | no_bias_feedback | 0.141 | 6.9 |

**Stability**: 1 unique default(s) across 1 windows.

### SMALL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast_recent_map | 0.251 | 110.3 |
| KITTI-kitti_raw_0009_200 | fast_recent_map | 0.473 | 23.7 |
| KITTI-kitti_raw_0009_full | fast_recent_map | 0.462 | 17.3 |
| KITTI-kitti_raw_0061_200 | fast_recent_map | 0.639 | 78.1 |
| KITTI-kitti_raw_0061_full | fast_recent_map | 0.959 | 82.2 |
| MCD-KTH | fast_recent_map | 0.806 | 107.9 |
| MCD-NTU | dense_recent_map | 0.031 | 56.8 |
| MCD-TUHH | fast_recent_map | 0.466 | 107.2 |
| dogfooding_results/kitti_seq_00_full | fast_seeded_reference | 0.890 | 63.3 |
| dogfooding_results/kitti_seq_00_full | fast_profile | 0.890 | 106.4 |
| dogfooding_results/kitti_seq_02_full | fast_profile | 0.909 | 96.5 |
| dogfooding_results/kitti_seq_05_full | fast_profile | 0.984 | 104.5 |
| dogfooding_results/kitti_seq_07_full | fast_profile | 0.682 | 107.7 |
| dogfooding_results/kitti_seq_08_full | fast_profile | 0.858 | 74.6 |
| dogfooding_results/nclt_2013_01_10_full | gate_0_5 | 0.348 | 9.1 |

**Stability**: 5 unique default(s) across 15 windows.

### SUMA

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | default | 0.248 | 74.6 |
| KITTI-kitti_raw_0009_200 | default | 3.377 | 19.2 |
| KITTI-kitti_raw_0009_full | dense | 4.073 | 15.6 |
| KITTI-kitti_raw_0061_200 | dense | 1.496 | 33.5 |
| KITTI-kitti_raw_0061_full | fast | 32.429 | 110.9 |
| MCD-KTH | fast | 7.419 | 150.2 |
| MCD-NTU | dense | 0.036 | 33.9 |
| MCD-TUHH | default | 1.414 | 59.1 |
| dogfooding_results/kitti_seq_00_full | dense_profile | 18.961 | 24.2 |
| dogfooding_results/kitti_seq_02_full | dense_profile | 51.911 | 24.2 |
| dogfooding_results/kitti_seq_05_full | default | 10.983 | 40.3 |
| dogfooding_results/kitti_seq_07_full | dense_profile_elevation | 3.694 | 34.5 |
| dogfooding_results/kitti_seq_08_full | dense_profile | 19.290 | 26.6 |

**Stability**: 5 unique default(s) across 13 windows.

### VGICP-SLAM

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | fast | 0.145 | 16.8 |
| KITTI-kitti_raw_0009_200 | fast | 1.985 | 6.2 |
| KITTI-kitti_raw_0009_full | fast | 2.901 | 8.4 |
| KITTI-kitti_raw_0061_200 | fast | 0.903 | 31.0 |
| KITTI-kitti_raw_0061_full | fast | 5.230 | 22.8 |
| MCD-KTH | fast | 6.096 | 20.8 |
| MCD-NTU | fast | 0.026 | 40.3 |
| MCD-TUHH | fast | 1.322 | 21.8 |

**Stability**: 1 unique default(s) across 8 windows.

### VINS-FUSION

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| KITTI-kitti_raw_0009_200 | fast | 85.802 | 83.5 |
| KITTI-kitti_raw_0009_full | fast | 136.572 | 93.7 |
| KITTI-kitti_raw_0061_200 | fast | 61.904 | 255.5 |
| KITTI-kitti_raw_0061_full | fast | 257.641 | 236.7 |

**Stability**: 1 unique default(s) across 4 windows.

### VOXEL-GICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense_recent_map | 0.268 | 141.1 |
| KITTI-kitti_raw_0009_200 | dense_recent_map | 0.644 | 23.3 |
| KITTI-kitti_raw_0009_full | dense_recent_map | 0.614 | 27.5 |
| KITTI-kitti_raw_0061_200 | dense_recent_map | 1.041 | 73.7 |
| KITTI-kitti_raw_0061_full | dense_recent_map | 1.062 | 75.4 |
| MCD-KTH | dense_recent_map | 0.981 | 124.2 |
| MCD-NTU | dense_recent_map | 0.121 | 117.2 |
| MCD-TUHH | dense_recent_map | 0.478 | 116.4 |
| dogfooding_results/kitti_seq_00_full | dense_seeded_reference | 1.047 | 90.8 |
| dogfooding_results/kitti_seq_00_full | dense_profile | 1.047 | 97.9 |
| dogfooding_results/kitti_seq_02_full | dense_profile | 0.944 | 84.8 |
| dogfooding_results/kitti_seq_05_full | dense_profile | 1.031 | 111.1 |
| dogfooding_results/kitti_seq_07_full | dense_profile | 1.027 | 101.4 |
| dogfooding_results/kitti_seq_08_full | dense_profile | 0.955 | 91.1 |

**Stability**: 3 unique default(s) across 14 windows.

### XICP

| Dataset | Default Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | dense | 0.168 | 71.8 |
| KITTI-kitti_raw_0009_200 | default | 0.151 | 25.2 |
| KITTI-kitti_raw_0009_full | dense | 0.129 | 14.3 |
| KITTI-kitti_raw_0061_200 | fast | 0.171 | 102.0 |
| KITTI-kitti_raw_0061_full | fast | 0.202 | 104.6 |
| MCD-KTH | fast | 0.401 | 84.8 |
| MCD-NTU | dense | 0.095 | 69.0 |
| MCD-TUHH | dense | 0.081 | 72.0 |

**Stability**: 3 unique default(s) across 8 windows.

## 3. Profile Impact Summary

How do profile flags (fast/balanced/dense) affect ATE and FPS? Values averaged across all datasets where the variant ran.

### ALOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 3.007 | 3.2 | 8 |
| fast | 11.499 | 5.2 | 13 |
| kitti_default | 8.998 | 2.1 | 12 |

### BALM2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 3.630 | 1.7 | 7 |
| dense | 3.633 | 0.7 | 7 |
| fast | 4.146 | 11.4 | 8 |

### CLINS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 484.064 | 61.3 | 1 |
| dense | 1.473 | 12.2 | 1 |
| fast | 350.052 | 100.3 | 1 |

### CT-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| arch_tuned_all_combined | 93.156 | 16.6 | 1 |
| arch_tuned_all_combined_elevation | 170.163 | 16.3 | 1 |
| arch_tuned_map_30 | 9.485 | 12.0 | 1 |
| arch_tuned_map_50 | 8.844 | 10.7 | 2 |
| balanced_reference | 3.342 | 7.3 | 2 |
| balanced_window | 13.353 | 22.2 | 19 |
| bare_c2f | 37.043 | 10.2 | 1 |
| bare_corr_4 | 14.403 | 11.3 | 1 |
| bare_corr_5 | 18.281 | 13.7 | 1 |
| bare_corr_8 | 17.568 | 13.8 | 1 |
| bare_corr_8_reference | 93.642 | 10.8 | 1 |
| bare_corr_default | 19.413 | 13.6 | 1 |
| bare_map_20 | 16.205 | 10.4 | 2 |
| bare_map_30 | 14.163 | 9.1 | 2 |
| bare_map_50 | 14.305 | 9.1 | 2 |
| bare_map_50_corr_4 | 10.292 | 11.6 | 1 |
| bare_map_50_iter_8 | 11.158 | 9.8 | 1 |
| bare_map_50_reference | 14.305 | 10.4 | 2 |
| bare_map_50_voxel_05 | 11.337 | 9.6 | 1 |
| baseline_map_20 | 68.972 | 8.2 | 1 |
| baseline_map_50 | 103.301 | 8.4 | 1 |
| baseline_reference | 68.972 | 10.7 | 1 |
| c2f_only_full | 85.979 | 9.4 | 1 |
| c2f_only_map_20 | 37.043 | 8.6 | 1 |
| c2f_only_map_50 | 30.497 | 6.9 | 1 |
| c2f_only_reference | 37.043 | 8.9 | 1 |
| c2f_only_sigma | 66.553 | 10.2 | 1 |
| c2f_reference | 14.099 | 8.7 | 1 |
| cauchy_1_5 | 26.327 | 10.7 | 2 |
| cauchy_2_0_reference | 27.707 | 10.1 | 2 |
| cauchy_2_5 | 25.166 | 10.9 | 2 |
| cauchy_3_0 | 28.507 | 10.5 | 2 |
| cauchy_4_0 | 26.034 | 10.4 | 2 |
| cholesky_c2f_no_ms | 37.043 | 12.2 | 1 |
| cluster_a | 3.329 | 14.3 | 7 |
| cluster_a_no_seed | 30.774 | 13.8 | 3 |
| cluster_a_no_seed_reference | 22.557 | 6.7 | 2 |
| cluster_a_seeded | 2.995 | 13.0 | 6 |
| cluster_a_with_ms_chol_seed | 16.399 | 10.4 | 1 |
| cluster_a_with_seed | 5.866 | 16.2 | 2 |
| cluster_d_full_no_seed_reference | 2.842 | 12.5 | 1 |
| cluster_d_full_seeded | 1.603 | 11.2 | 1 |
| cluster_d_ms_chol | 2.924 | 18.4 | 7 |
| cluster_d_seeded | 1.847 | 24.5 | 3 |
| cluster_d_with_seed | 4.473 | 25.5 | 1 |
| corr_16 | 15.056 | 9.7 | 2 |
| corr_2 | 144.290 | 11.5 | 2 |
| corr_4 | 75.821 | 11.5 | 4 |
| corr_4_reference | 9.485 | 11.6 | 1 |
| corr_5 | 39.118 | 10.1 | 2 |
| corr_6 | 42.565 | 9.4 | 2 |
| corr_7 | 47.117 | 9.8 | 2 |
| corr_8 | 31.415 | 11.1 | 5 |
| corr_8_reference | 55.210 | 9.6 | 2 |
| corr_default | 13.698 | 10.7 | 1 |
| corr_default_reference | 16.687 | 9.0 | 1 |
| corr_dist_reference | 38.901 | 8.0 | 1 |
| current_winner | 35.955 | 10.3 | 4 |
| default_reference | 30.794 | 10.2 | 5 |
| dense_map50_reference | 11.158 | 4.7 | 1 |
| dense_reference | 5.991 | 22.4 | 4 |
| dense_seeded_reference | 1.470 | 28.0 | 3 |
| dense_window | 13.558 | 13.8 | 19 |
| fast_reference | 1.475 | 59.4 | 1 |
| fast_window | 26.057 | 42.6 | 19 |
| fast_window_reference | 6.972 | 57.8 | 1 |
| fine_sigma_0_25 | 12.351 | 8.4 | 1 |
| fine_sigma_0_375 | 16.726 | 8.3 | 1 |
| fine_sigma_0_75 | 17.269 | 10.9 | 1 |
| fine_sigma_1_0 | 17.425 | 11.4 | 1 |
| fine_sigma_default | 16.687 | 10.8 | 1 |
| full_recipe_reference | 12.931 | 11.0 | 1 |
| iter_1 | 25.516 | 10.1 | 2 |
| iter_2 | 27.512 | 9.5 | 2 |
| iter_3_reference | 27.831 | 9.7 | 2 |
| iter_4 | 24.470 | 9.3 | 2 |
| iter_6 | 22.669 | 9.5 | 2 |
| map_10 | 93.790 | 5.2 | 1 |
| map_15 | 56.437 | 8.5 | 3 |
| map_20_reference | 60.328 | 7.9 | 3 |
| map_30 | 44.096 | 9.5 | 2 |
| map_5 | 113.336 | 5.7 | 1 |
| map_50 | 76.824 | 9.0 | 2 |
| map_50_c2f | 12.226 | 9.0 | 1 |
| map_50_c2f_plus_corr_4 | 9.305 | 9.1 | 1 |
| map_50_plus_c2f | 16.687 | 11.5 | 1 |
| map_50_plus_corr_5 | 14.894 | 12.5 | 1 |
| map_50_plus_ms_chol | 15.753 | 9.3 | 1 |
| minus_c2f | 14.894 | 10.5 | 1 |
| minus_corr_5 | 16.687 | 8.9 | 1 |
| minus_map_50 | 17.009 | 11.2 | 1 |
| minus_ms_chol | 12.931 | 11.5 | 1 |
| ms_chol_c2f_no_cholesky | 37.043 | 12.1 | 1 |
| ms_chol_corr_5 | 65.305 | 8.6 | 1 |
| ms_chol_corr_8 | 93.642 | 9.2 | 1 |
| ms_chol_flat_reference | 2.715 | 5.1 | 1 |
| ms_chol_map_20 | 2.842 | 14.2 | 1 |
| ms_chol_map_50 | 1.472 | 12.7 | 1 |
| ms_chol_plus_c2f | 2.460 | 10.6 | 1 |
| ms_chol_plus_simplified_a | 2.010 | 9.9 | 1 |
| ms_chol_reference | 2.842 | 10.5 | 1 |
| plus_corr_16 | 18.250 | 11.3 | 1 |
| plus_corr_4 | 40.759 | 11.3 | 1 |
| plus_corr_5 | 12.931 | 11.4 | 1 |
| plus_corr_8 | 23.053 | 10.3 | 2 |
| radius_1 | 14.817 | 10.2 | 1 |
| radius_2_reference | 16.687 | 9.1 | 1 |
| radius_3 | 15.119 | 8.3 | 1 |
| radius_4 | 15.004 | 7.2 | 1 |
| simplified_plus_corr_8 | 194.204 | 9.3 | 1 |
| simplified_seq_00_pattern | 40.851 | 9.9 | 3 |
| velocity_reg_0005 | 14.346 | 8.2 | 1 |
| velocity_reg_001 | 27.694 | 9.0 | 2 |
| velocity_reg_0015 | 98.563 | 9.4 | 1 |
| velocity_reg_005 | 7.555 | 5.2 | 2 |
| velocity_reg_01 | 2.361 | 12.2 | 2 |
| velocity_reg_01_elevation | 3.388 | 18.3 | 1 |

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

### FAST-LIVO2

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 51.390 | 2.9 | 4 |
| dense | 51.260 | 2.4 | 4 |
| fast | 51.231 | 9.2 | 4 |

### FLOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 8.776 | 2.6 | 13 |
| fast | 2.939 | 29.8 | 9 |
| kitti_default | 11.379 | 10.6 | 13 |

### GENZ-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 3.513 | 7.6 | 1 |
| dense_profile | 5.093 | 7.9 | 1 |
| fast_profile | 10.076 | 11.0 | 1 |

### GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.860 | 10.6 | 14 |
| dense_recent_map | 0.851 | 6.3 | 14 |
| fast_recent_map | 0.788 | 18.0 | 14 |

### HDL-GRAPH-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 69.223 | 2.4 | 7 |
| dense | 8.390 | 3.3 | 6 |
| fast | 12.179 | 15.3 | 6 |

### IMU-DEAD-RECKONING

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| accel_bias | 9.075 | 1525979.8 | 1 |
| accel_bias_full | 288751.377 | 1354057.2 | 1 |
| accel_bias_kitti_0009 | 9059.896 | 1068993.7 | 1 |
| accel_bias_kitti_0009_full | 90451.005 | 2584793.5 | 1 |
| default_dr | 9.071 | 1360852.8 | 1 |
| default_dr_full | 288700.449 | 1095010.7 | 1 |
| default_dr_kitti_0009 | 9195.203 | 25723.2 | 1 |
| default_dr_kitti_0009_full | 91851.969 | 376448.4 | 1 |
| euler | 10.280 | 1405827.2 | 1 |
| euler_full | 291892.627 | 1116254.9 | 1 |
| euler_kitti_0009 | 9349.895 | 1687110.0 | 1 |
| euler_kitti_0009_full | 92489.724 | 2692976.4 | 1 |
| nhc | 9.156 | 899732.3 | 1 |
| nhc_full | 46003.188 | 932269.4 | 1 |
| nhc_kitti_0009 | 7468.054 | 1154591.6 | 1 |
| nhc_kitti_0009_full | 31926.115 | 1503157.3 | 1 |
| nhc_zupt | 3.717 | 888178.3 | 1 |
| nhc_zupt_full | 9605.455 | 800357.3 | 1 |
| nhc_zupt_kitti_0009 | 201.185 | 1035661.6 | 1 |
| nhc_zupt_kitti_0009_full | 1067.301 | 1567725.4 | 1 |
| no_gyro_bias | 24.676 | 1258838.1 | 1 |
| no_gyro_bias_full | 672302.751 | 1112120.0 | 1 |
| no_gyro_bias_kitti_0009 | 1170.370 | 1988293.4 | 1 |
| no_gyro_bias_kitti_0009_full | 11184.985 | 1740955.3 | 1 |
| rk4 | 9.072 | 177532.7 | 1 |
| rk4_full | 290555.090 | 174601.0 | 1 |
| rk4_kitti_0009 | 9195.273 | 524466.6 | 1 |
| rk4_kitti_0009_full | 91855.575 | 55899.7 | 1 |
| zupt | 2.887 | 1054259.2 | 1 |
| zupt_full | 14531.743 | 798152.2 | 1 |
| zupt_kitti_0009 | 210.224 | 1342750.9 | 1 |
| zupt_kitti_0009_full | 5994.497 | 2617012.4 | 1 |

### ISC-LOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.692 | 12.0 | 8 |
| dense | 2.755 | 22.1 | 8 |
| fast | 2.595 | 41.5 | 8 |

### KISS-ICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 40.765 | 9.9 | 14 |
| balanced_reference | 19.109 | 7.0 | 7 |
| balanced_reference_elevation | 7.403 | 16.0 | 2 |
| dense_local_map | 40.108 | 6.8 | 12 |
| dense_profile | 15.434 | 3.8 | 7 |
| dense_profile_elevation | 5.370 | 11.2 | 2 |
| dense_recent_map | 44.631 | 6.8 | 2 |
| fast_profile | 29.017 | 2.5 | 5 |
| fast_recent_map | 40.645 | 17.7 | 14 |
| t1_transfer_v03_i12 | 2.681 | 1.5 | 1 |
| t1_transfer_v05_i12 | 23.054 | 0.9 | 5 |
| upstream_profile | 7.826 | 18.2 | 2 |
| upstream_profile_elevation | 5.326 | 19.4 | 2 |
| upstream_profile_range100 | 7.479 | 18.8 | 2 |
| upstream_profile_range100_elevation | 5.921 | 19.2 | 2 |

### L-LO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| cell_0p25_gtol_0p15_range_80 | 2.693 | 8.5 | 1 |
| cell_0p35_gtol_0p15 | 2.879 | 7.8 | 1 |
| cell_0p35_gtol_0p15_range_80 | 3.294 | 7.7 | 1 |
| cell_0p35_gtol_0p15_range_80_min15 | 2.561 | 7.7 | 1 |
| cell_0p35_gtol_0p15_range_80_shape_pitch | 3.014 | 7.9 | 1 |
| cell_0p3_gtol_0p15_range_80 | 5.960 | 7.9 | 1 |
| cluster_cell_0p15 | 2.890 | 17.9 | 1 |
| cluster_cell_0p2 | 1.686 | 8.7 | 1 |
| cluster_cell_0p25 | 3.720 | 10.1 | 1 |
| cluster_cell_0p3 | 5.107 | 9.0 | 1 |
| cluster_cell_0p35 | 3.031 | 6.5 | 1 |
| cluster_cell_0p7 | 2.898 | 4.0 | 1 |
| cluster_cell_1p0 | 6.039 | 3.2 | 1 |
| default | 24.159 | 4.3 | 5 |
| ground_tol_0p15 | 2.210 | 7.0 | 1 |
| ground_tol_0p4 | 2.037 | 8.5 | 1 |
| max_range_40 | 2.568 | 6.6 | 1 |
| max_range_80 | 4.065 | 6.8 | 1 |
| min_cluster_15 | 2.280 | 5.8 | 1 |
| min_cluster_60 | 2.073 | 6.5 | 1 |
| pitch_band_10_40 | 1.957 | 5.1 | 1 |
| pitch_band_3_15 | 2.346 | 5.2 | 1 |
| pitch_band_5_40 | 1.989 | 5.1 | 1 |
| rounds_600_tol_1e5 | 2.045 | 5.9 | 1 |
| seq07_tuned | 42.442 | 4.2 | 5 |
| shape_0p2 | 2.189 | 5.9 | 1 |
| shape_0p8 | 3.239 | 5.1 | 1 |
| sor_std_2 | 1.998 | 6.8 | 1 |
| step_double | 3.069 | 5.1 | 1 |
| step_half | 2.348 | 5.2 | 1 |
| voxel_0p1 | 2.013 | 1.2 | 1 |
| voxel_0p4 | 3.178 | 22.9 | 1 |

### LEGO-LOAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 3.448 | 4.0 | 9 |
| fast | 3.049 | 10.2 | 9 |
| kitti_default | 8.047 | 2.7 | 13 |

### LF-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| no_mitigation | 11.568 | 4.0 | 5 |
| paper_default | 10.938 | 4.3 | 6 |
| paper_default_elevation | 1.859 | 8.3 | 1 |

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
| cluster_t1_seeded | 0.471 | 37.7 | 13 |
| coarse_to_fine_3_2_1 | 30.823 | 62.3 | 5 |
| coarse_to_fine_3_2_1_elevation | 2.086 | 47.5 | 1 |
| cov_floor_1e_4 | 20.084 | 96.6 | 4 |
| covariance_gradient | 104.515 | 79.8 | 4 |
| covariance_gradient_w0_1_linesearch | 29.073 | 79.9 | 4 |
| default_voxel_2_0 | 0.624 | 10.8 | 2 |
| fast_cov_half_threads | 0.674 | 53.1 | 15 |
| fast_cov_no_gt_seed | 110.484 | 98.9 | 1 |
| fast_icp_only_half_threads | 0.674 | 59.0 | 15 |
| fast_seeded_reference | 0.725 | 47.8 | 13 |
| paper_cov_half_threads | 0.888 | 58.5 | 15 |
| paper_icp_only_half_threads | 0.888 | 54.7 | 15 |
| radius1_distance_gate_1_5m | 17.977 | 41.3 | 4 |
| radius1_no_distance_gate | 18.373 | 41.9 | 4 |
| tuned_voxel0_5_iter12_seeded | 0.731 | 54.4 | 1 |
| tuned_voxel1_iter12 | 19.255 | 67.0 | 5 |
| tuned_voxel1_iter12_seeded | 1.099 | 55.2 | 1 |
| tuned_voxel1_iter20_seeded | 1.096 | 56.0 | 1 |
| tuned_voxel2_iter12_seeded | 1.117 | 86.8 | 1 |
| voxel_0_5_t1 | 0.484 | 8.7 | 2 |
| voxel_1_0 | 0.690 | 9.0 | 2 |

### LOAM-LIVOX

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 59.700 | 8.9 | 8 |
| dense | 59.699 | 11.4 | 8 |
| fast | 74.817 | 52.4 | 8 |

### MULLS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| dense | 36.323 | 1.1 | 12 |
| fast | 38.992 | 3.4 | 12 |
| kitti_default | 4.115 | 1.6 | 8 |

### NDT

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.141 | 14.3 | 12 |
| balanced_reference | 0.257 | 24.9 | 2 |
| dense_local_map | 0.160 | 8.5 | 12 |
| dense_profile | 0.270 | 16.6 | 2 |
| fast_coarse_map | 0.146 | 21.3 | 12 |
| fast_profile | 0.263 | 40.6 | 2 |
| t1_transfer_r03_i12 | 0.023 | 11.7 | 1 |
| t1_transfer_r05_i12 | 0.068 | 4.3 | 5 |
| t1_transfer_r10_i12 | 0.269 | 10.7 | 1 |

### OKVIS

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 96.788 | 476.9 | 4 |
| dense | 92.416 | 215.8 | 4 |
| fast | 98.674 | 1070.6 | 4 |

### POINT-LIO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 99.415 | 5.8 | 8 |
| dense | 88.212 | 14.4 | 8 |
| fast | 106.515 | 95.2 | 8 |

### RKO-LIO

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| bias_gain_010 | 0.550 | 6.8 | 1 |
| bias_gain_030_default | 0.815 | 8.1 | 1 |
| bias_gain_060 | 1.920 | 7.6 | 1 |
| no_bias_feedback | 0.141 | 6.9 | 1 |

### SMALL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.797 | 47.4 | 8 |
| balanced_no_seed | 584.185 | 46.3 | 1 |
| balanced_reference | 1.099 | 56.7 | 5 |
| default_gate_2_0 | 1.086 | 11.8 | 1 |
| dense_no_seed | 141.664 | 42.6 | 1 |
| dense_profile | 1.096 | 46.7 | 5 |
| dense_recent_map | 0.723 | 36.5 | 8 |
| fast_no_seed | 202.722 | 62.2 | 1 |
| fast_profile | 0.864 | 97.9 | 5 |
| fast_recent_map | 0.521 | 80.1 | 8 |
| fast_seeded_reference | 0.890 | 63.3 | 1 |
| gate_0_5 | 0.348 | 9.1 | 1 |
| gate_1_0 | 0.675 | 10.9 | 1 |
| gate_1_5 | 0.882 | 11.7 | 1 |
| t1_transfer_v03_i12 | 0.925 | 38.7 | 1 |
| t1_transfer_v05_i12 | 1.081 | 40.4 | 5 |

### SUMA

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 26.934 | 35.4 | 12 |
| dense | 3.849 | 27.6 | 8 |
| dense_profile | 20.729 | 27.7 | 5 |
| dense_profile_elevation | 3.694 | 34.5 | 1 |
| fast | 376.693 | 109.8 | 8 |

### VGICP-SLAM

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 2.282 | 6.9 | 8 |
| dense | 2.441 | 9.6 | 8 |
| fast | 2.326 | 21.0 | 8 |

### VINS-FUSION

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 76.709 | 11.2 | 4 |
| dense | 51.770 | 8.3 | 4 |
| fast | 135.480 | 167.4 | 4 |

### VOXEL-GICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| balanced_local_map | 0.776 | 20.4 | 8 |
| balanced_no_seed | 259.978 | 22.4 | 1 |
| balanced_reference | 1.072 | 21.4 | 5 |
| dense_no_seed | 87.915 | 95.6 | 1 |
| dense_profile | 1.001 | 97.3 | 5 |
| dense_recent_map | 0.651 | 87.4 | 8 |
| dense_seeded_reference | 1.047 | 90.8 | 1 |
| fast_profile | 1.094 | 50.6 | 5 |
| fast_recent_map | 0.941 | 42.2 | 8 |
| t1_transfer_v03_i12 | 1.128 | 27.6 | 1 |
| t1_transfer_v05_i12 | 1.168 | 25.5 | 5 |

### XICP

| Variant | Avg ATE [m] | Avg FPS | N |
|---|---:|---:|---:|
| default | 0.197 | 58.0 | 8 |
| dense | 0.180 | 53.2 | 8 |
| fast | 0.325 | 80.4 | 8 |
| no_gt_seed | 60.191 | 59.5 | 8 |
