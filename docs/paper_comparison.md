# Original-Paper Comparison

> Generated: 2026-04-17T11:59:13+00:00

This document compares paper-reported metrics with the current repository defaults across each method family. Direct comparison is limited by differences in dataset windows, hardware, and metric definitions (RPE vs ATE).

## LiTAMIN2

**Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: RPE translation [%]
**Hardware**: Intel Core i7-8700, 32GB RAM

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.65 |
| kitti_01 | 1.42 |
| kitti_02 | 0.83 |
| kitti_05 | 0.40 |
| kitti_07 | 0.42 |
| kitti_08 | 0.96 |

### Repository Defaults

| Dataset | Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | Fast local-map + ICP-only | 0.168 | 5.20 |
| HDL-400 | Paper-like 3m + covariance | 0.129 | 94.24 |
| Istanbul | Fast local-map + ICP-only | 1.222 | 20.95 |
| Istanbul | Paper-like 3m + ICP-only | 0.741 | 17.25 |
| KITTI | Fast local-map + ICP-only | 1.145 | 48.81 |
| KITTI | Paper-like 3m + ICP-only | 1.397 | 43.69 |
| KITTI | Paper-like 3m + covariance | 122.275 | 87.31 |
| KITTI | Fast local-map + ICP-only | 0.944 | 58.11 |
| KITTI | Fast local-map + covariance | 0.511 | 67.74 |
| dogfooding_results/mcd_kth_day_06_108 | Fast local-map + covariance | 0.401 | 64.22 |
| dogfooding_results/mcd_ntu_day_02_108 | Paper-like 3m + ICP-only | 0.045 | 105.65 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast local-map + covariance | 0.194 | 106.48 |
| Istanbul | Fast local-map + ICP-only | 1.213 | 23.52 |

**Notes**: Paper reports RPE (relative pose error), not ATE. Direct comparison with repo ATE requires caution.

---

## GICP

**Paper**: Segal et al., Generalized-ICP, RSS 2009

**Reported dataset**: Custom indoor/outdoor datasets
**Reported metric**: qualitative alignment quality
**Hardware**: Not specified

### Repository Defaults

| Dataset | Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | Fast recent map | 0.284 | 1.71 |
| HDL-400 | Fast recent map | 0.215 | 23.34 |
| Istanbul | Fast recent map | 1.166 | 5.68 |
| Istanbul | Fast recent map | 0.982 | 4.27 |
| KITTI | Fast recent map | 1.170 | 23.01 |
| KITTI | Fast recent map | 1.177 | 25.84 |
| KITTI | Dense recent map | 1.510 | 10.93 |
| KITTI | Fast recent map | 1.081 | 22.76 |
| KITTI | Fast recent map | 0.959 | 25.74 |
| dogfooding_results/mcd_kth_day_06_108 | Fast recent map | 0.630 | 24.67 |
| dogfooding_results/mcd_ntu_day_02_108 | Dense recent map | 0.017 | 12.99 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast recent map | 0.317 | 31.24 |
| Istanbul | Fast recent map | 1.074 | 6.27 |

**Notes**: Original GICP paper predates KITTI. No standardized numeric benchmark reported. Comparison is with the concept, not specific numbers.

---

## NDT

**Paper**: Biber & Strasser, The Normal Distributions Transform: A New Approach to Laser Scan Matching, IROS 2003; Magnusson, The Three-Dimensional Normal-Distributions Transform, PhD Thesis 2009

**Reported dataset**: Custom 2D/3D datasets
**Reported metric**: qualitative
**Hardware**: Not specified

### Repository Defaults

| Dataset | Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | Fast coarse map | 0.065 | 0.86 |
| HDL-400 | Fast coarse map | 0.052 | 32.17 |
| Istanbul | Fast coarse map | 0.007 | 2.08 |
| Istanbul | Fast coarse map | 0.005 | 1.95 |
| KITTI | Fast coarse map | 0.307 | 30.91 |
| KITTI | Fast coarse map | 0.374 | 36.15 |
| KITTI | Fast coarse map | 122.547 | 24.99 |
| KITTI | Fast coarse map | 0.247 | 23.80 |
| KITTI | Fast coarse map | 0.319 | 41.22 |
| dogfooding_results/mcd_kth_day_06_108 | Fast coarse map | 0.208 | 31.21 |
| dogfooding_results/mcd_ntu_day_02_108 | Balanced local map | 0.014 | 32.66 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast coarse map | 0.070 | 40.83 |
| Istanbul | Fast coarse map | 0.070 | 2.02 |

**Notes**: Original NDT papers predate KITTI. Modern NDT implementations (e.g., Autoware) use NDT on KITTI but those numbers are from different codebases.

---

## KISS-ICP

**Paper**: Vizzo et al., KISS-ICP: In Defense of Point-to-Point ICP, RAL 2023

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: ATE [m] (mean over full sequence)
**Reported FPS**: ~100
**Hardware**: Intel Core i9-12900K

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 3.12 |
| kitti_01 | 10.37 |
| kitti_02 | 7.28 |
| kitti_05 | 1.63 |
| kitti_07 | 0.55 |
| kitti_08 | 3.07 |

### Repository Defaults

| Dataset | Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | Fast recent map | 0.218 | 0.45 |
| HDL-400 | Fast recent map | 1.281 | 11.28 |
| Istanbul | Dense local map | 144.086 | 3.59 |
| Istanbul | Fast recent map | 131.692 | 3.74 |
| KITTI | Fast recent map | 5.839 | 21.88 |
| KITTI | Fast recent map | 2.642 | 24.43 |
| KITTI | Fast recent map | 2.642 | 28.21 |
| KITTI | Fast recent map | 4.623 | 11.23 |
| KITTI | Fast recent map | 0.679 | 28.26 |
| dogfooding_results/mcd_kth_day_06_108 | Fast recent map | 5.568 | 11.29 |
| dogfooding_results/mcd_ntu_day_02_108 | Fast recent map | 0.026 | 66.68 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast recent map | 1.303 | 24.10 |
| Istanbul | Fast recent map | 182.960 | 4.05 |

**Notes**: KISS-ICP reports near real-time performance (~100 FPS) on KITTI. ATE values are for full sequences; our 108-frame windows will differ.

---

## CT-ICP

**Paper**: Dellenbach et al., CT-ICP: Real-time Elastic LiDAR Odometry with Loop Closure, ICRA 2022

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: RPE translation [%]
**Reported FPS**: ~13
**Hardware**: Intel Core i7-9700K, 64GB RAM

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.52 |
| kitti_01 | 0.72 |
| kitti_02 | 0.58 |
| kitti_05 | 0.31 |
| kitti_07 | 0.30 |
| kitti_08 | 0.75 |

### Repository Defaults

| Dataset | Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | Dense window | 1.254 | 16.61 |
| HDL-400 | Fast window | 1.211 | 2.36 |
| HDL-400 | Fast window | 2.582 | 72.91 |
| Istanbul | Balanced window | 6.820 | 1.58 |
| Istanbul | Balanced window | 7.539 | 1.31 |
| KITTI | Balanced window | 4.673 | 40.58 |
| KITTI | Balanced window | 1.659 | 34.38 |
| KITTI | Balanced window | 1.659 | 44.51 |
| KITTI | Fast window | 6.972 | 37.60 |
| KITTI | Fast window | 1.475 | 56.88 |
| dogfooding_results/mcd_kth_day_06_108 | Fast window | 6.525 | 59.77 |
| dogfooding_results/mcd_ntu_day_02_108 | Dense window | 0.325 | 22.02 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast window | 3.553 | 71.57 |
| Istanbul | Fast window | 79.761 | 2.75 |

**Notes**: CT-ICP reports RPE, not ATE. Continuous-time formulation. Our repo measures ATE on short windows.

---

## CT-LIO

**Paper**: Zheng et al., Continuous-Time Fixed-Lag Smoothing for LiDAR-Inertial-Odometry (CT-LIO), IROS 2023 / derived from CT-ICP + IMU integration

**Reported dataset**: KITTI raw, NTU VIRAL, custom
**Reported metric**: ATE [m]
**Hardware**: Not standardized

### Repository Defaults

| Dataset | Variant | ATE [m] | FPS |
|---|---|---:|---:|
| HDL-400 | Seed-only fast | 0.479 | 19.56 |
| HDL-400 | Seed-only fast | 0.488 | 17.48 |

**Notes**: CT-LIO in this repo is a custom integration. Not directly comparable to any single published paper. Reference-based comparison only.

---
