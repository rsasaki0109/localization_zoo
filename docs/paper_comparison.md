# Original-Paper Comparison

> Generated: 2026-10-18T23:03:44+00:00

This document compares paper-reported metrics with the current repository defaults across each method family. Direct comparison is limited by differences in dataset windows, hardware, and metric definitions (ATE and RPE availability still differs by family).

For an explicit statement of implementation scope and what the repo currently claims to reproduce, see [`docs/reproduction_status.md`](reproduction_status.md).

## LiTAMIN2

**Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Intel Core i9-9900K, 32 GB RAM, NVIDIA GPU (Sec. IV)
**Repo scope**: Paper-oriented front-end reimplementation with the paper odometry pipeline — The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
**Current claim**: Near paper across KITTI Odometry 00-10 (paper pipeline, no GT seed) — --litamin2-paper-odometry follows Sec. III of the paper: raw scans voted into 3 m normal distributions, a voxel map of distributions, nearest-distribution pairing, and the ICP+Cov cost (shape term linear in the cost, Newton steps). Run unchanged on all 11 sequences it scores official KITTI RTE 1.01/2.06/1.15/0.76/0.90/0.62/0.58/0.48/0.98/0.78/0.99 % on 00-10 versus the paper's ICP+Cov row 0.78/2.10/0.95/0.96/1.05/0.55/0.55/0.48/1.01/0.69/0.80 % (Table III, no loop closure): geometric mean 1.05x. The ICP-only variant against the same map gives 1.01x versus the paper's ICP row. Table 6 picks the best variant per sequence and reaches 1.00x.
**Numeric comparison**: Same metric, ~1.05x with one fixed setting on 00-10 — Official KITTI RTE of paper_icp_cov (one setting, all sequences) vs the paper's ICP+Cov row: 00 1.009 vs 0.78, 01 2.062 vs 2.10, 02 1.148 vs 0.95, 03 0.756 vs 0.96, 04 0.902 vs 1.05, 05 0.624 vs 0.55, 06 0.578 vs 0.55, 07 0.477 vs 0.48, 08 0.984 vs 1.01, 09 0.777 vs 0.69, 10 0.986 vs 0.80; geometric mean 1.05x. paper_icp_first (ICP term only) vs the paper's ICP row: 1.01x. The earlier harness path (0.5 m pre-downsampling, at most 2500 source points, a point map rebuilt into voxels, same-voxel pairing) stayed at 1.22x on 00/02/05/07/08. Table 6 (docs/assets/paper/paper_ratio_table.csv) selects the best 100 m RPE variant per sequence; see papers/litamin2/README.md.
**Main blocker**: None for the KITTI numbers. The shape term only helps when the map holds single-scan distributions (the first one per voxel); against aggregated map voxels it hurts (2.0x, diverges on 04 and 08), which the paper does not discuss. Loop closure is not implemented.
**Next step**: Loop closure (Table III rows with loop closure) is out of scope; the paper odometry could become the method default once other datasets are checked.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.78 |
| kitti_01 | 2.10 |
| kitti_02 | 0.95 |
| kitti_03 | 0.96 |
| kitti_04 | 1.05 |
| kitti_05 | 0.55 |
| kitti_06 | 0.55 |
| kitti_07 | 0.48 |
| kitti_08 | 1.01 |
| kitti_09 | 0.69 |
| kitti_10 | 0.80 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Fast local-map + ICP-only | 0.168 | - | - | 5.20 |
| HDL-400 | Fast local-map + covariance | 0.111 | 1.313 | 0.050393 | 80.71 |
| Istanbul | Fast local-map + ICP-only | 1.222 | - | - | 20.95 |
| Istanbul | Paper-like 3m + ICP-only | 0.741 | - | - | 17.25 |
| KITTI | cluster T1 + seed | 0.644 | 0.792 | 0.008077 | 27.87 |
| KITTI | cluster T1 + seed | 0.676 | 0.758 | 0.007913 | 18.02 |
| KITTI | Fast local-map + covariance | 1.064 | 0.793 | 0.006576 | 18.85 |
| KITTI | Fast local-map + ICP-only | 1.067 | 0.742 | 0.003888 | 31.26 |
| KITTI | Fast local-map + ICP-only | 6.290 | 2.287 | 0.004462 | 30.54 |
| KITTI | cluster T1 + seed | 0.517 | 0.445 | 0.007128 | 48.13 |
| KITTI | cluster T1 + seed | 0.600 | 1.040 | 0.010314 | 39.77 |
| KITTI | Fast local-map + ICP-only | 0.944 | - | - | 58.11 |
| KITTI | Fast local-map + covariance | 0.511 | - | - | 67.74 |
| KITTI | Fast local-map + ICP-only | 0.998 | 1.914 | 0.018601 | 108.86 |
| KITTI | Fast local-map + covariance (pure odometry) | 110.484 | 3.081 | 0.018176 | 98.95 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 15.440 | 0.880 | 0.009885 | 82.33 |
| KITTI | Tuned: voxel 1.0 + iter 12 (no GT seed) | 13.464 | 0.920 | 0.008111 | 60.95 |
| KITTI | voxel=2.0 + iter=12 + GT seed | 1.117 | 2.024 | 0.018026 | 86.83 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 96.992 | 0.930 | 0.004868 | 67.82 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 37.095 | 1.034 | 0.008464 | 85.90 |
| KITTI | cluster T1: voxel=0.5 + iter=12 + seed | 0.728 | 0.952 | 0.010395 | 68.31 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 2.666 | 0.747 | 0.005545 | 72.40 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 2.573 | 0.784 | 0.005020 | 61.97 |
| KITTI | LiTAMIN2 paper odometry, ICP only, aggregated map | 6.342 | 0.778 | 0.009976 | 55.10 |
| KITTI | fast + seed (baseline) | 0.957 | 1.419 | 0.013525 | 17.17 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 3.161 | 0.514 | 0.003921 | 68.85 |
| KITTI | coarse_to_fine_3_2_1 (Table 6 variant on seq 07, raw scans) + KITTI 0.205 deg elevation correction | 2.086 | 0.478 | 0.006129 | 47.52 |
| KITTI | LiTAMIN2 paper odometry, ICP+Cov, scan-distribution map | 1.434 | 0.542 | 0.007820 | 58.76 |
| KITTI | fast + seed (baseline) | 0.836 | 1.039 | 0.011014 | 94.73 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 20.825 | 1.371 | 0.008129 | 73.59 |
| KITTI | fast + seed (baseline) | 1.130 | 2.117 | 0.018472 | 104.92 |
| KITTI | LiTAMIN2 paper odometry, ICP only, aggregated map | 5.688 | 0.759 | 0.009896 | 58.20 |
| KITTI | LiTAMIN2 paper odometry, ICP only, scan-distribution map | 5.615 | 0.893 | 0.012083 | 67.74 |
| dogfooding_results/mcd_kth_day_06_108 | cluster T1 + seed | 0.192 | 3.297 | 0.181810 | 20.06 |
| dogfooding_results/mcd_kth_day_06_108 | Fast local-map + ICP-only | 0.401 | 7.357 | 0.537257 | 91.64 |
| dogfooding_results/mcd_ntu_day_02_108 | cluster T1 + seed | 0.021 | 16.626 | 2.360907 | 43.40 |
| dogfooding_results/mcd_ntu_day_02_108 | Paper-like 3m + ICP-only | 0.045 | 13.029 | 16.949197 | 81.24 |
| dogfooding_results/mcd_tuhh_night_09_108 | cluster T1 + seed | 0.132 | 5.561 | 0.646547 | 39.45 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast local-map + ICP-only | 0.194 | 12.688 | 1.119389 | 107.21 |
| dogfooding_results/mulran_parkinglot_120 | cluster T1: voxel=0.5 + iter=12 + seed | 0.212 | 2.153 | 0.099764 | 30.60 |
| dogfooding_results/mulran_parkinglot_120 | Fast local-map + covariance | 0.498 | 4.253 | 0.104025 | 90.23 |
| dogfooding_results/mulran_parkinglot_full | cluster T1: voxel=0.5 + iter=12 + seed | 0.303 | 0.527 | 0.009781 | 34.43 |
| dogfooding_results/mulran_parkinglot_full | Fast local-map + ICP-only | 0.711 | 1.263 | 0.024128 | 118.63 |
| dogfooding_results/nclt_2012_12_01_5000 | Voxel 0.5 + iter 12 (KITTI T1 transfer) | 0.519 | 1.357 | 0.015294 | 4.39 |
| dogfooding_results/nclt_2013_01_10_600 | Default voxel 2.0 | 0.380 | 0.580 | 0.017993 | 14.69 |
| Istanbul | Fast local-map + ICP-only | 1.213 | - | - | 23.52 |
| KITTI | Covariance floor: 1e-4 | 51.895 | 0.967 | 0.007174 | 91.83 |
| KITTI | Covariance floor: 1e-4 | 6.565 | 0.612 | 0.005857 | 93.44 |
| KITTI | Covariance floor: 1e-4 | 2.202 | 0.548 | 0.006877 | 106.88 |
| KITTI | Covariance floor: 1e-4 | 19.672 | 1.307 | 0.006429 | 94.18 |

**Notes**: Paper values were re-verified against the arXiv PDF on 2026-10-03 (Table III, ICP+Cov without loop closure; the ICP-only row is 0.75/1.88/0.99/0.84/0.90/0.74/0.45/0.55/1.25/0.74/1.36 %). Results: experiments/results/litamin2_kitti_seq_NN_full_paper_matrix.json. The older harness-path sweeps (voxel resolution, correspondence radius and gate, covariance floor, covariance gradient, coarse-to-fine) remain in the other litamin2 aggregates; they used GT-free constant-velocity seeds but not the paper's map or input. Policy A GT-seeded numbers (0.74 % on Raw 0009 short window, 1.26 % on MulRan parkinglot full) remain non-comparable to the paper.

---

## GICP

**Paper**: Segal et al., Generalized-ICP, RSS 2009

**Reported dataset**: Custom indoor/outdoor datasets
**Reported metric**: qualitative alignment quality
**Hardware**: Not specified
**Repo scope**: Compact shared implementation — The repo keeps a self-contained GICP-style registration loop with local covariance modeling under the stable evaluator.
**Current claim**: Concept-only comparison — The implementation is useful as benchmark evidence for the GICP idea, but not as a paper-result reproduction of a specific historical codebase.
**Numeric comparison**: Not direct — The original paper predates the repo's modern public benchmark suite and does not provide a standardized numeric target that can be matched one-to-one here.
**Main blocker**: There is no single paper benchmark and hardware protocol to reproduce against.
**Next step**: Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence.

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Fast recent map | 0.284 | - | - | 1.71 |
| HDL-400 | Fast recent map | 0.215 | - | - | 23.34 |
| Istanbul | Fast recent map | 1.166 | - | - | 5.68 |
| Istanbul | Fast recent map | 0.982 | - | - | 4.27 |
| KITTI | Fast recent map | 1.129 | 1.361 | 0.012012 | 8.43 |
| KITTI | Fast recent map | 1.151 | 1.186 | 0.010974 | 11.76 |
| KITTI | Fast recent map | 2.068 | 1.311 | 0.008609 | 8.58 |
| KITTI | Fast recent map | 1.081 | - | - | 22.76 |
| KITTI | Fast recent map | 0.959 | - | - | 25.74 |
| dogfooding_results/mcd_kth_day_06_108 | Fast recent map | 0.630 | - | - | 24.67 |
| dogfooding_results/mcd_ntu_day_02_108 | Dense recent map | 0.017 | - | - | 12.99 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast recent map | 0.317 | - | - | 31.24 |
| dogfooding_results/mulran_parkinglot_120 | Fast recent map | 0.644 | 4.988 | 0.124007 | 27.74 |
| dogfooding_results/mulran_parkinglot_full | Fast recent map | 1.149 | 2.443 | 0.047401 | 30.28 |
| Istanbul | Fast recent map | 1.074 | - | - | 6.27 |

**Notes**: Original GICP paper predates KITTI. No standardized numeric benchmark reported. Comparison is with the concept, not specific numbers. As supporting evidence under the dogfooding-fixed --no-gt-seed pure-odometry path (build 2026-05-18, velocity-model prior), GICP fast on KITTI Raw 0009 full reports ATE 4.82 m / RPE 4.70 % (best in the Policy A family on this sequence, edging LiTAMIN2's 7.45 m). This is honest pure-odometry behavior but not a paper-reproduction claim; it is an internal calibration of where GICP sits versus other Policy A methods when the GT-seed asymmetry is removed.

---

## NDT

**Paper**: Biber & Strasser, The Normal Distributions Transform: A New Approach to Laser Scan Matching, IROS 2003; Magnusson, The Three-Dimensional Normal-Distributions Transform, PhD Thesis 2009

**Reported dataset**: Custom 2D/3D datasets
**Reported metric**: qualitative
**Hardware**: Not specified
**Repo scope**: Compact baseline — The repo implements an NDT-style registration baseline against voxel Gaussian models with a compact optimizer.
**Current claim**: Concept-only comparison — This path is benchmark-useful, but it is not a direct reproduction of the original NDT papers or of a single modern NDT codebase.
**Numeric comparison**: Not direct — The original papers predate KITTI-style public odometry evaluation, and modern NDT implementations use materially different code paths.
**Main blocker**: There is no agreed modern NDT reference setup wired into the repo as the reproduction target.
**Next step**: Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required.

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Fast coarse map | 0.065 | - | - | 0.86 |
| HDL-400 | Fast coarse map | 0.052 | - | - | 32.17 |
| Istanbul | Fast coarse map | 0.007 | - | - | 2.08 |
| Istanbul | Fast coarse map | 0.005 | - | - | 1.95 |
| KITTI | Fast coarse map | 0.299 | 0.508 | 0.005771 | 14.13 |
| KITTI | Fast coarse map | 0.386 | 0.562 | 0.005239 | 20.40 |
| KITTI | Fast coarse map | 121.012 | 99.982 | 0.014306 | 14.30 |
| KITTI | Fast coarse map | 0.247 | - | - | 23.80 |
| KITTI | Fast coarse map | 0.319 | - | - | 41.22 |
| KITTI | T1 (res=0.3+i12) | 0.023 | 0.073 | 0.000910 | 11.66 |
| KITTI | T1 transfer (res=0.5 + iter=12) | 0.059 | - | - | 0.23 |
| KITTI | T1 transfer (res=0.5 + iter=12) | 0.059 | - | - | 0.24 |
| KITTI | fast profile | 0.279 | 0.492 | 0.003748 | 43.64 |
| KITTI | T1 transfer (res=0.5 + iter=12) | 0.076 | - | - | 0.25 |
| dogfooding_results/mcd_kth_day_06_108 | Fast coarse map | 0.208 | - | - | 31.21 |
| dogfooding_results/mcd_ntu_day_02_108 | Balanced local map | 0.014 | - | - | 32.66 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast coarse map | 0.070 | - | - | 40.83 |
| Istanbul | Fast coarse map | 0.070 | - | - | 2.02 |

**Notes**: Original NDT papers predate KITTI. Modern NDT implementations (e.g., Autoware) use NDT on KITTI but those numbers are from different codebases.

---

## KISS-ICP

**Paper**: Vizzo et al., KISS-ICP: In Defense of Point-to-Point ICP, RAL 2023

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Not stated in paper
**Repo scope**: Compact baseline — The repo keeps a small KISS-ICP-style local-map pipeline that preserves the main idea while simplifying the full upstream engineering stack.
**Current claim**: Matches the official code on KITTI 00/07 with upstream mechanics — With --kiss-upstream-profile --kiss-model-deviation-threshold --kiss-geman-mcclure-kernel --kiss-map-min-point-distance 0.2236068 and the KITTI elevation correction the port scores 0.535 % (seq 00) and 0.363 % (seq 07) official KITTI RTE against 0.528 % and 0.375 % for the official KISS-ICP 1.3.0 code. The repository default keeps a simpler speed-based adaptive threshold and is not a faithful rerun.
**Numeric comparison**: KITTI 00/04 per sequence plus the 00-10 average — The paper reports per-sequence KITTI RTE only for seq 00 (0.51 %) and 04 (0.36 %), plus the 00-10 average (0.50 %). Upstream KISS-ICP corrects the KITTI elevation angle by +0.205 deg in its KITTI loader. With the same correction and the upstream mechanics (model-deviation threshold, Geman-McClure kernel, map insertion gate) the port scores 0.535 % on seq 00 (1.05x) and 0.373 % on seq 04 (1.04x), and 0.564 % pooled over 00-10, identical to the official KISS-ICP 1.3.0 code on the same pipeline (0.564 %, 1.13x the paper's average). Before matching the adaptive threshold the best seq 00 value was 0.708 % (1.39x), and 0.954 % on raw scans (1.87x). Seq 00/04 values are the best variant among the sweeps on that sequence (selected on the evaluated sequence, so optimistic); see docs/kitti_elevation_correction.md, papers/kiss_icp/README.md, and docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The remaining 1.13x on the 00-10 average is shared by the official 1.3.0 code, so it lies between the paper and the released code (for example an older release or a different deskewing setup), not in the port.
**Next step**: Optionally rerun an older official KISS-ICP release on 00-10 to locate the paper-vs-release difference; the port itself needs no further change for KITTI.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.51 |
| kitti_04 | 0.36 |
| kitti_avg_00_10 | 0.50 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Fast recent map | 0.218 | - | - | 0.45 |
| HDL-400 | Fast recent map | 1.281 | - | - | 11.28 |
| Istanbul | Dense local map | 144.086 | - | - | 3.59 |
| Istanbul | Fast recent map | 131.692 | - | - | 3.74 |
| KITTI | Fast recent map | 4.207 | 1.403 | 0.007330 | 10.84 |
| KITTI | Fast recent map | 2.578 | 1.675 | 0.004303 | 18.71 |
| KITTI | Fast recent map | 2.578 | 1.675 | 0.004303 | 18.20 |
| KITTI | Fast recent map | 4.623 | - | - | 11.23 |
| KITTI | Fast recent map | 0.679 | - | - | 28.26 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 7.633 | 0.797 | 0.004292 | 36.56 |
| KITTI | dense profile | 12.323 | 0.963 | 0.007617 | 1.25 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 7.947 | 0.826 | 0.001348 | 20.32 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 18.507 | 0.789 | 0.002933 | 37.36 |
| KITTI | balanced | 71.183 | 1.178 | 0.008933 | 3.06 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 2.185 | 0.845 | 0.002308 | 25.56 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 0.661 | 0.411 | 0.001442 | 24.60 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 3.739 | 0.473 | 0.002622 | 34.51 |
| KITTI | dense | 4.556 | 0.715 | 0.005066 | 1.64 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 1.802 | 0.465 | 0.001403 | 23.85 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 0.537 | 0.541 | 0.002913 | 39.53 |
| KITTI | balanced (default) | 2.238 | 0.664 | 0.007003 | 3.38 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 15.965 | 1.332 | 0.003594 | 26.35 |
| KITTI | fast | 18.085 | 1.549 | 0.007241 | 2.23 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 3.681 | 0.607 | 0.002472 | 31.02 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 2.835 | 0.681 | 0.003854 | 39.68 |
| dogfooding_results/mcd_kth_day_06_108 | Fast recent map | 5.568 | - | - | 11.29 |
| dogfooding_results/mcd_ntu_day_02_108 | Fast recent map | 0.026 | - | - | 66.68 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast recent map | 1.303 | - | - | 24.10 |
| dogfooding_results/mulran_parkinglot_120 | Fast recent map | 15.641 | 224.647 | 0.103093 | 27.26 |
| dogfooding_results/mulran_parkinglot_full | Fast recent map | 74.337 | 103.248 | 0.035060 | 26.85 |
| Istanbul | Fast recent map | 182.960 | - | - | 4.05 |

**Notes**: KISS-ICP reports the KITTI average relative translational error (Table II: 0.50 % train / 0.61 % test) and per-sequence values only in the Table VI threshold ablation. The paper does not state hardware or a KITTI frame rate.

---

## CT-ICP

**Paper**: Dellenbach et al., CT-ICP: Real-time Elastic LiDAR Odometry with Loop Closure, ICRA 2022

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Reported FPS**: ~16.7
**Hardware**: Not stated; single-thread CPU
**Repo scope**: Paper-oriented core reimplementation with an upstream-mechanics pipeline — The repo implements the continuous-time two-pose-per-scan core, interpolation, and point-to-plane optimization described by CT-ICP. --ct-icp-upstream-mechanics runs a dedicated pipeline that follows upstream jedeschaud/ct_icp with its driving profile (papers/ct_icp/README.md).
**Current claim**: Matches the official code on KITTI 00-10 with upstream mechanics — With --ct-icp-upstream-mechanics and the KITTI elevation correction the port scores 0.587 % official KITTI RTE pooled over sequences 00-10 against 0.585 % for the official jedeschaud/ct_icp code on the same scans (run_official_ct_icp_baseline.py), and stays within 0.05 points of it on every sequence (seq 07: 0.315 % vs 0.308 %, paper 0.31 %). The default runCTICP pipeline is not a faithful rerun: it used the point index as time, a 20-frame window and zero-motion initialisation, scored 1.03-1.18 % on seq 07, and is numerically unstable (about +/-7-11 % under 1e-6 deg input perturbations), which explains the earlier host dependence.
**Numeric comparison**: Same metric, KITTI 00-10 per sequence — Official KITTI RTE on KITTI-corrected scans, port with --ct-icp-upstream-mechanics vs paper Table I: 00 0.525/0.49, 01 0.716/0.76, 02 0.549/0.52, 03 0.586/0.72, 04 0.717/0.39, 05 0.315/0.25, 06 0.284/0.27, 07 0.315/0.31, 08 0.902/0.81, 09 0.629/0.49, 10 0.742/0.48; pooled 00-10 0.587 % vs 0.531 % (the paper's per-sequence values weighted by segment count), 1.10x. The official code gives 0.585 % pooled on the same scans, so the remaining difference lies between the paper and the released code. Before the upstream mechanics the best of ~313 sweep variants was 4.03x (geometric mean over 00/02/05/07/08). See papers/ct_icp/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The remaining 1.10x on the 00-10 average (largest on 04/05/09/10) is shared by the official code on the same scans, so it lies between the paper and the released code (for example the paper's KITTI PLY timestamps or an older release), not in the port.
**Next step**: Optionally obtain the official KITTI PLY files (password-protected download) to check whether their per-point timestamps explain the paper-vs-release difference; the port itself needs no further change for KITTI.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.49 |
| kitti_01 | 0.76 |
| kitti_02 | 0.52 |
| kitti_03 | 0.72 |
| kitti_04 | 0.39 |
| kitti_05 | 0.25 |
| kitti_06 | 0.27 |
| kitti_07 | 0.31 |
| kitti_08 | 0.81 |
| kitti_09 | 0.49 |
| kitti_10 | 0.48 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Dense window | 1.254 | 6.506 | 0.505498 | 18.70 |
| HDL-400 | Fast window | 1.211 | - | - | 2.36 |
| HDL-400 | Fast window | 2.582 | 20.008 | 0.745586 | 54.94 |
| Istanbul | Balanced window | 6.820 | - | - | 1.58 |
| Istanbul | Balanced window | 7.539 | - | - | 1.31 |
| KITTI | corr=5 | 12.931 | 2.092 | 0.026054 | 9.80 |
| KITTI | baseline (dense + iter=6 + map=20) | 68.972 | 2.977 | 0.031920 | 8.22 |
| KITTI | cluster D (ms_chol) | 2.579 | 1.606 | 0.015756 | 7.20 |
| KITTI | Fast window | 3.871 | 2.230 | 0.032283 | 12.69 |
| KITTI | balanced_window (current best 4.67 m) | 4.105 | 1.750 | 0.020566 | 7.41 |
| KITTI | Fast window | 2.692 | 2.159 | 0.023410 | 16.89 |
| KITTI | Balanced window | 1.659 | - | - | 44.51 |
| KITTI | fast_window (current best 1.48 m) | 1.475 | 1.361 | 0.035598 | 59.36 |
| KITTI | Fast window | 6.972 | - | - | 37.60 |
| KITTI | fast_window (current best 6.97 m) | 6.972 | 1.722 | 0.039063 | 57.76 |
| KITTI | Fast window | 1.475 | - | - | 56.88 |
| KITTI | bare + corr=8 | 17.568 | 2.134 | 0.027303 | 13.76 |
| KITTI | cauchy=4.0 | 18.895 | 2.066 | 0.027959 | 10.58 |
| KITTI | coarse_iter=1 | 14.099 | 2.057 | 0.027515 | 9.79 |
| KITTI | corr_dist=8 m² (2.8 m linear) | 16.778 | 2.053 | 0.026043 | 9.51 |
| KITTI | arch_tuned_all_combined (Table 6 best on seq 00, raw scans) | 93.156 | 2.565 | 0.031724 | 16.64 |
| KITTI | fine σ=0.25 (tighter) | 12.351 | 2.040 | 0.025852 | 8.44 |
| KITTI | c2f_sigma_only map=20 (existing winner) | 18.370 | 2.137 | 0.027809 | 10.39 |
| KITTI | Balanced window | 19.413 | 2.111 | 0.027532 | 12.99 |
| KITTI | coarse search radius=1 (3x3x3, same as fine) | 14.817 | 2.087 | 0.027970 | 10.24 |
| KITTI | cluster A + GT seed | 5.855 | 5.998 | 0.076457 | 6.87 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 9.445 | 0.831 | 0.004205 | 8.85 |
| KITTI | full minus ms_chol (no multi-scale, no cholesky) | 12.931 | 2.092 | 0.026054 | 11.55 |
| KITTI | map=50 + corr=5 | 14.894 | 2.149 | 0.027041 | 12.48 |
| KITTI | bare map=50 | 15.753 | 2.043 | 0.027532 | 11.23 |
| KITTI | Fast window | 2.824 | 3.773 | 0.047622 | 74.88 |
| KITTI | + corr=5 | 12.931 | 2.092 | 0.026054 | 11.40 |
| KITTI | dense + c2f cluster-A (existing best, RPE 2.059%) | 14.099 | 2.057 | 0.027515 | 8.66 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 58.191 | 0.827 | 0.003356 | 5.57 |
| KITTI | baseline (dense + iter=6 + map=20) — existing winner | 68.972 | 2.977 | 0.031920 | 10.31 |
| KITTI | corr=5 | 65.305 | 3.165 | 0.033007 | 10.40 |
| KITTI | ms_chol + corr=5 | 65.305 | 3.165 | 0.033007 | 8.65 |
| KITTI | dense + iter=6 + map=20 (existing winner) | 68.972 | 2.977 | 0.031920 | 10.70 |
| KITTI | baseline map=15 | 76.464 | 2.651 | 0.033446 | 4.44 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 20.607 | 0.810 | 0.002946 | 8.06 |
| KITTI | corr=8 + map=15 | 73.230 | 2.678 | 0.032340 | 11.73 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 2.035 | 0.818 | 0.003750 | 6.11 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 2.185 | 0.713 | 0.001453 | 5.24 |
| KITTI | arch_tuned + map=50 (full + bigger) | 8.844 | 1.175 | 0.023420 | 10.16 |
| KITTI | arch_tuned corr=4 (existing winner) | 9.485 | 1.185 | 0.023558 | 11.60 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 4.264 | 0.470 | 0.002347 | 7.53 |
| KITTI | arch_tuned map=50 | 8.844 | 1.175 | 0.023420 | 11.31 |
| KITTI | bare map=30 | 11.707 | 1.227 | 0.023648 | 7.02 |
| KITTI | + constant-velocity-weight 0.05 (winner) | 11.949 | 1.122 | 0.023083 | 5.11 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 3.963 | 0.389 | 0.002206 | 5.16 |
| KITTI | ms_chol + map=50 + c2f σ×2 (full combo) | 2.010 | 1.772 | 0.017504 | 9.86 |
| KITTI | ms_chol + corr_dist=8 m² | 2.049 | 2.147 | 0.020803 | 14.06 |
| KITTI | velocity_reg_01 (Table 6 best on seq 07, raw scans) | 2.231 | 1.302 | 0.018304 | 18.51 |
| KITTI | Dense window | 2.842 | 2.103 | 0.021053 | 17.52 |
| KITTI | cluster D full + GT seed | 1.603 | 2.082 | 0.020798 | 11.20 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 1.257 | 0.455 | 0.002867 | 8.86 |
| KITTI | ms_chol map=50 | 1.472 | 1.730 | 0.018054 | 12.67 |
| KITTI | Fast window | 0.978 | 1.359 | 0.082217 | 77.32 |
| KITTI | + constant-velocity-weight 0.1 | 2.490 | 1.253 | 0.018813 | 5.97 |
| KITTI | cholesky + c2f (no multi-scale) | 37.043 | 2.113 | 0.025832 | 12.18 |
| KITTI | c2f_only + corr_dist=8 m² | 33.860 | 2.113 | 0.026506 | 10.18 |
| KITTI | cauchy=2.5 | 31.958 | 2.112 | 0.026070 | 11.93 |
| KITTI | coarse_iter=6 | 30.418 | 2.105 | 0.026321 | 10.39 |
| KITTI | cluster A + GT seed | 6.813 | 8.142 | 0.078035 | 6.85 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 26.321 | 1.306 | 0.003959 | 6.21 |
| KITTI | c2f_only map=20 (existing winner) | 37.043 | 2.113 | 0.025832 | 8.63 |
| KITTI | simplified (map=50 + c2f σ×2) — existing 27.85 m | 28.427 | 2.017 | 0.024197 | 10.95 |
| KITTI | + constant-velocity-weight 0.01 (RPE 1.935%) | 40.362 | 1.949 | 0.023975 | 9.30 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 9.703 | 0.737 | 0.003053 | 6.84 |
| KITTI | upstream mechanics + KITTI 0.205 deg elevation correction | 5.317 | 0.781 | 0.003878 | 9.05 |
| KITTI | bare + corr=8 (existing seq 02 winner) | 93.642 | 3.717 | 0.033521 | 12.98 |
| KITTI | map=50 + c2f σ×2 + corr=4 (combine both) | 9.305 | 1.214 | 0.023269 | 9.09 |
| KITTI | map=50 + c2f σ×2 (no ms_chol) | 2.010 | 1.772 | 0.017504 | 11.82 |
| KITTI | map=50 + c2f σ×2 (no ms_chol) | 28.427 | 2.017 | 0.024197 | 8.53 |
| dogfooding_results/mcd_kth_day_06_108 | Fast window | 6.525 | 31.125 | 2.631934 | 57.24 |
| dogfooding_results/mcd_kth_day_06_108 | dense_window only (current best 6.12 m) | 6.115 | 29.891 | 2.381092 | 17.96 |
| dogfooding_results/mcd_kth_day_06_108 | dense_window + GT seed | 2.778 | 17.676 | 1.581335 | 28.26 |
| dogfooding_results/mcd_ntu_day_02_108 | Dense window | 0.325 | 155.350 | 16.620838 | 18.62 |
| dogfooding_results/mcd_ntu_day_02_108 | dense_window (current best 0.325 m) | 0.325 | 155.350 | 16.620838 | 21.88 |
| dogfooding_results/mcd_ntu_day_02_108 | dense_window + GT seed | 0.451 | 133.296 | 18.851173 | 28.37 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast window | 3.553 | 149.906 | 22.063407 | 51.38 |
| dogfooding_results/mcd_tuhh_night_09_108 | dense_window (current best 1.65 m) | 1.652 | 97.868 | 21.936664 | 22.80 |
| dogfooding_results/mcd_tuhh_night_09_108 | dense_window + GT seed | 1.182 | 40.665 | 2.768656 | 27.42 |
| dogfooding_results/mulran_parkinglot_120 | Fast window | 16.474 | 230.558 | 0.296730 | 74.72 |
| dogfooding_results/mulran_parkinglot_120 | cluster A + GT seed | 2.547 | 32.885 | 0.529313 | 17.87 |
| dogfooding_results/mulran_parkinglot_full | cluster A (map=50 + c2f σ×2) + GT seed | 9.186 | 9.969 | 0.090951 | 14.58 |
| dogfooding_results/mulran_parkinglot_full | Fast window | 80.958 | 107.256 | 0.155683 | 59.75 |
| Istanbul | Fast window | 79.761 | - | - | 2.75 |

**Notes**: Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6). The --ct-icp-gt-seed toggle remains available for fair-prior dogfooding-style cross-method comparison; GT-seeded rows are excluded from Table 6.

---

## CT-LIO

**Paper**: Zheng et al., Continuous-Time Fixed-Lag Smoothing for LiDAR-Inertial-Odometry (CT-LIO), IROS 2023 / derived from CT-ICP + IMU integration

**Reported dataset**: KITTI raw, NTU VIRAL, custom
**Reported metric**: ATE [m]
**Hardware**: Not standardized
**Repo scope**: Custom integration — This repo combines CT-ICP-style continuous-time interpolation with IMU preintegration as a compact local prototype built from shared components.
**Current claim**: Not comparable to a single paper — The code path is intentionally a custom integration, so its numbers should not be presented as reproduction of one upstream CT-LIO or CLINS paper result.
**Numeric comparison**: Reference-based only — The current results are useful inside the repo benchmark, but they are not direct paper-result comparisons.
**Main blocker**: There is no single paper-faithful target implementation and protocol behind this path.
**Next step**: Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol.

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Seed-only fast | 0.479 | - | - | 19.56 |
| HDL-400 | Seed-only fast | 0.488 | - | - | 17.48 |

**Notes**: CT-LIO in this repo is a custom integration. Not directly comparable to any single published paper. Reference-based comparison only.

---

## MULLS

**Paper**: Pan et al., MULLS: Versatile LiDAR SLAM via Multi-metric Linear Least Square, ICRA 2021

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Reported FPS**: ~12.5
**Hardware**: Intel Core i7-7700HQ @ 2.80 GHz
**Repo scope**: Lightweight multi-metric variant plus a MULLS-LO reimplementation — The default MULLS path is a lightweight A-LOAM-feature multi-metric variant. --mulls-lo runs MULLS-LO written from the paper and the authors' KITTI profiles: dual-threshold grid ground filter, PCA pillar/beam/facade classification, multi-metric linear least-squares ICP, and a local map (papers/mulls/README.md).
**Current claim**: MULLS-LO matches the official code on KITTI 00-10 — With --mulls-lo (highway profile on seq 01) and the profile's 0.195 deg vertical-angle correction the reimplementation scores 0.572 % official KITTI RTE pooled over sequences 00-10 against 0.576 % for the official YuePanEdward/MULLS code on the same scans (run_official_mulls_baseline.py), within about 0.02 points on every sequence (seq 07: 0.319 % vs 0.311-0.332 %, paper 0.29 %). The default lightweight variant is not a faithful rerun (2.6 % on seq 07).
**Numeric comparison**: Same metric, KITTI 00-10 per sequence — Official KITTI RTE with the 0.195 deg correction, --mulls-lo vs paper Table II MULLS-LO(mc): 00 0.536/0.51, 01 0.674/0.62 (highway profile), 02 0.585/0.55, 03 0.609/0.61, 04 0.492/0.35, 05 0.315/0.28, 06 0.297/0.24, 07 0.319/0.29, 08 0.812/0.80, 09 0.548/0.49, 10 0.691/0.61; pooled 00-10 0.572 % vs 0.539 % (the paper's per-sequence values weighted by segment count), 1.06x. The official code gives 0.576 % pooled on the same scans, so the remaining difference lies between the paper and the released code. The lightweight variant was 7.62x (00/02/05/07/08). See papers/mulls/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The remaining 1.06x on the 00-10 average is shared by the official code on the same scans, so it lies between the paper and the released code, not in the reimplementation.
**Next step**: None for KITTI odometry; loop closure (MULLS-SLAM) and the other datasets of the paper are not covered.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.51 |
| kitti_01 | 0.62 |
| kitti_02 | 0.55 |
| kitti_03 | 0.61 |
| kitti_04 | 0.35 |
| kitti_05 | 0.28 |
| kitti_06 | 0.24 |
| kitti_07 | 0.29 |
| kitti_08 | 0.80 |
| kitti_09 | 0.49 |
| kitti_10 | 0.61 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Fast | 0.876 | - | - | 9.34 |
| KITTI | Fast | 4.610 | - | - | 3.34 |
| KITTI | Fast | 2.695 | 1.558 | 0.009244 | 1.20 |
| KITTI | Fast | 11.390 | - | - | 3.28 |
| KITTI | Fast | 0.490 | - | - | 3.31 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 10.786 | 0.757 | 0.004080 | 7.39 |
| KITTI | Fast | 56.820 | 2.691 | 0.028323 | 1.79 |
| KITTI | MULLS-LO highway profile + 0.195 deg vertical-angle correction | 3.658 | 0.815 | 0.001557 | 5.68 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 31.006 | 0.770 | 0.002710 | 7.80 |
| KITTI | Fast | 260.878 | 2.272 | 0.026005 | 1.72 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 2.360 | 0.770 | 0.003460 | 8.61 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 1.030 | 0.501 | 0.001290 | 8.17 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 4.828 | 0.393 | 0.002175 | 8.04 |
| KITTI | Fast | 26.886 | 1.939 | 0.021480 | 1.72 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 2.245 | 0.425 | 0.001790 | 7.86 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 0.939 | 0.439 | 0.002297 | 7.89 |
| KITTI | Fast | 10.501 | 2.994 | 0.028779 | 4.13 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 23.593 | 1.275 | 0.003850 | 8.10 |
| KITTI | Fast | 87.759 | 3.317 | 0.030065 | 1.69 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 9.245 | 0.609 | 0.002895 | 8.59 |
| KITTI | MULLS-LO + 0.195 deg vertical-angle correction | 4.709 | 0.696 | 0.003343 | 8.10 |
| dogfooding_results/mcd_kth_day_06_108 | Fast | 6.297 | - | - | 4.12 |
| dogfooding_results/mcd_ntu_day_02_108 | KITTI default | 0.097 | - | - | 1.17 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast | 1.206 | - | - | 3.81 |

**Notes**: Paper Table II reports 0.08 s/frame for MULLS-LO(mc). Transfer runs on 00/02/05/08 ran four sequences concurrently on one host, so their FPS is depressed.

---

## SUMA

**Paper**: Behley and Stachniss, Efficient Surfel-Based SLAM using 3D Laser Range Data in Urban Environments, RSS 2018

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Intel i7-6700 @ 3.4 GHz, 16 GB RAM, Nvidia GeForce GTX 960 (GPU)
**Repo scope**: Paper reimplementation with the paper's frame-to-model pipeline (SuMaPaper) — Dense surfel odometry from range-image maps inside the shared benchmark harness.
**Current claim**: Near paper on KITTI Odometry 00-10 (paper pipeline) — --suma-paper (rendered surfel model, projective frame-to-model ICP, surfel stability and fusion, official default.xml values, no loop closure). Official KITTI RTE 00-10, official defaults on raw scans: 0.697 / 13.793 / 3.512 / 0.651 / 0.417 / 0.379 / 0.286 / 0.331 / 1.060 / 0.510 / 0.698 %; normalised model lookup + 0.22 deg correction: 0.884 / 3.550 / 0.784 / 1.186 / 0.802 / - / 0.530 / 0.521 / 1.179 / 0.962 / 0.897 %; paper 0.7 / 1.7 / 1.1 / 0.7 / 0.4 / 0.5 / 0.4 / 0.4 / 1.0 / 0.5 / 0.7 %. Best variant per sequence 0.97x (01 2.09x); one setting for all 1.25x (official) or 1.41x (normalised).
**Numeric comparison**: Same metric, 0.97x on 00-10 (best of two variants per sequence) — Best variant per sequence: 00 1.00x, 01 2.09x, 02 0.71x, 03 0.93x, 04 1.04x, 05 0.76x, 06 0.71x, 07 0.83x, 08 1.06x, 09 1.02x, 10 1.00x. See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: No single setting wins everywhere: the official model lookup on raw scans drifts forward on degenerate stretches (02 corridors 3.2x, 01 highway 8.1x); the normalised lookup with the correction fixes 02 and halves 01 (still 2.09x) but costs elsewhere. Official SuMa (GPU/OpenGL) cannot run here for a direct check.
**Next step**: Look at the seq 01 highway (forward degeneracy); check the official code on a GPU host.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.70 |
| kitti_01 | 1.70 |
| kitti_02 | 1.10 |
| kitti_03 | 0.70 |
| kitti_04 | 0.40 |
| kitti_05 | 0.50 |
| kitti_06 | 0.40 |
| kitti_07 | 0.40 |
| kitti_08 | 1.00 |
| kitti_09 | 0.50 |
| kitti_10 | 0.70 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Default | 0.248 | - | - | 74.60 |
| KITTI | Dense | 4.073 | 1.690 | 0.011810 | 15.62 |
| KITTI | Default | 3.377 | 1.837 | 0.027786 | 19.19 |
| KITTI | Dense | 2.245 | 1.471 | 0.006882 | 14.60 |
| KITTI | Fast | 32.429 | - | - | 110.93 |
| KITTI | Dense | 1.496 | - | - | 33.54 |
| KITTI | default | 50.852 | 2.582 | 0.028282 | 33.78 |
| KITTI | SuMa paper pipeline, normalised model lookup + 0.22 deg correction | 59.043 | 3.508 | 0.003011 | 6.38 |
| KITTI | SuMa paper pipeline, normalised model lookup + 0.22 deg correction | 16.837 | 0.905 | 0.003883 | 4.43 |
| KITTI | SuMa paper pipeline (rendered surfel model, projective frame-to-model ICP) | 1.449 | 0.910 | 0.005307 | 4.70 |
| KITTI | SuMa paper pipeline (rendered surfel model, projective frame-to-model ICP) | 0.974 | 0.392 | 0.002635 | 5.03 |
| KITTI | default | 10.983 | 2.013 | 0.021825 | 40.33 |
| KITTI | SuMa paper pipeline, normalised model lookup + 0.22 deg correction | 2.873 | 0.470 | 0.004529 | 4.76 |
| KITTI | dense_profile (Table 6 variant on seq 07, raw scans) | 3.971 | 0.944 | 0.008807 | 35.60 |
| KITTI | dense profile | 19.290 | 1.909 | 0.012225 | 26.63 |
| KITTI | SuMa paper pipeline (rendered surfel model, projective frame-to-model ICP) | 6.469 | 0.634 | 0.003811 | 4.44 |
| KITTI | SuMa paper pipeline (rendered surfel model, projective frame-to-model ICP) | 5.253 | 0.782 | 0.004395 | 4.56 |
| dogfooding_results/mcd_kth_day_06_108 | Fast | 7.419 | - | - | 150.20 |
| dogfooding_results/mcd_ntu_day_02_108 | Dense | 0.036 | - | - | 33.95 |
| dogfooding_results/mcd_tuhh_night_09_108 | Default | 1.414 | - | - | 59.10 |

**Notes**: Paper values are rounded to one decimal, so ratios carry up to ~25 % rounding uncertainty on short-error sequences.

---

## ALOAM

**Paper**: Zhang and Singh, LOAM: Lidar Odometry and Mapping in Real-time, RSS 2014 (A-LOAM is the HKUST re-implementation)

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Not stated in the citing papers
**Repo scope**: Paper reimplementation — A-LOAM port in the shared harness, no per-frame GT seed. Fixed kitti_default on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. kitti_default was the accuracy candidate in the existing sequence 07 profile comparison (lowest stored 100 m RPE and official RTE there). Its arguments are frozen; the added sequences do not select a profile.
**Current claim**: Full KITTI Odometry coverage with one fixed setting — Full-sequence official KITTI RTE on 00-10, variant kitti_default, geometric mean 1.07x. kitti_default was the accuracy candidate in the existing sequence 07 profile comparison (lowest stored 100 m RPE and official RTE there). Its arguments are frozen; the added sequences do not select a profile. No sequence exceeds twice its paper value with the fixed setting. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
**Numeric comparison**: Same metric, 1.07x on 00-10 with one fixed setting — kitti_default, official KITTI RTE [%], repo vs paper: 00 0.855 vs 0.780 (1.10x); 01 2.189 vs 1.430 (1.53x); 02 1.029 vs 0.920 (1.12x); 03 0.964 vs 0.860 (1.12x); 04 0.880 vs 0.710 (1.24x); 05 0.535 vs 0.570 (0.94x); 06 0.602 vs 0.650 (0.93x); 07 0.681 vs 0.630 (1.08x); 08 0.975 vs 1.120 (0.87x); 09 0.673 vs 0.770 (0.87x); 10 0.873 vs 0.790 (1.11x); geometric mean 1.07x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
**Main blocker**: No sequence exceeds twice its paper value with the fixed setting. The reference is LOAM values cited by later papers, not an A-LOAM per-sequence author table; the repo implements A-LOAM rather than original LOAM.
**Next step**: Evaluate the fixed configuration on independent sensor conditions while preserving the full-KITTI baseline and the documented paper/implementation differences.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.78 |
| kitti_01 | 1.43 |
| kitti_02 | 0.92 |
| kitti_03 | 0.86 |
| kitti_04 | 0.71 |
| kitti_05 | 0.57 |
| kitti_06 | 0.65 |
| kitti_07 | 0.63 |
| kitti_08 | 1.12 |
| kitti_09 | 0.77 |
| kitti_10 | 0.79 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| HDL-400 | Fast | 0.193 | - | - | 13.77 |
| KITTI | Fast | 6.105 | - | - | 5.78 |
| KITTI | Fast | 3.470 | 1.715 | 0.005369 | 3.12 |
| KITTI | Fast | 3.470 | 1.715 | 0.005369 | 4.15 |
| KITTI | Fast | 3.654 | - | - | 6.01 |
| KITTI | Fast | 0.527 | - | - | 6.00 |
| KITTI | Fast | 19.121 | 0.893 | 0.007859 | 3.05 |
| KITTI | KITTI default | 172.741 | 0.984 | 0.005507 | 1.10 |
| KITTI | Fast | 70.870 | 0.961 | 0.006722 | 3.18 |
| KITTI | KITTI default | 3.155 | 0.818 | 0.005256 | 1.15 |
| KITTI | KITTI default | 4.164 | 0.650 | 0.005396 | 0.81 |
| KITTI | Fast | 7.796 | 0.577 | 0.005857 | 3.02 |
| KITTI | KITTI default | 2.587 | 0.612 | 0.004341 | 0.95 |
| KITTI | Fast | 4.030 | 0.691 | 0.006795 | 1.75 |
| KITTI | Fast | 26.170 | 1.436 | 0.006181 | 2.89 |
| KITTI | KITTI default | 8.242 | 0.650 | 0.004430 | 0.76 |
| KITTI | KITTI default | 5.619 | 0.729 | 0.006344 | 0.83 |
| dogfooding_results/mcd_kth_day_06_108 | Fast | 6.100 | - | - | 6.66 |
| dogfooding_results/mcd_ntu_day_02_108 | Dense | 0.035 | - | - | 2.96 |
| dogfooding_results/mcd_tuhh_night_09_108 | Fast | 1.374 | - | - | 6.47 |

**Notes**: Secondary-source values: LOAM (RSS 2014) does not tabulate per-sequence KITTI RTE. The repo implementation follows A-LOAM, not the original LOAM code.

---

## LF-GICP

**Paper**: Im, LF-GICP: Parameter-Free Degeneracy Handling for LiDAR Odometry via a Voxel-Normal Localizability Field, arXiv 2026

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Not stated
**Repo scope**: Paper reimplementation (no author code) — From-paper reimplementation in the shared harness, no per-frame GT seed. Fixed paper_default on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. field beta 0.25 was calibrated on sequence 00 first 500 frames against the paper dilution range, after earlier 02/08 runs exposed excessive gating. Those sequences influenced the decision to recalibrate; no added sequence influences the frozen setting.
**Current claim**: Full KITTI Odometry coverage with one fixed setting — Full-sequence official KITTI RTE on 00-10, variant paper_default, geometric mean 1.12x. field beta 0.25 was calibrated on sequence 00 first 500 frames against the paper dilution range, after earlier 02/08 runs exposed excessive gating. Those sequences influenced the decision to recalibrate; no added sequence influences the frozen setting. Seq 10: 5.616 % vs 0.837 % (6.71x), 26 moving steps with length error >20 % and 34 with local vector error >20 %. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
**Numeric comparison**: Same metric, 1.12x on 00-10 with one fixed setting — paper_default, official KITTI RTE [%], repo vs paper: 00 0.664 vs 0.692 (0.96x); 01 1.816 vs 1.822 (1.00x); 02 1.075 vs 1.084 (0.99x); 03 0.785 vs 1.207 (0.65x); 04 0.629 vs 0.814 (0.77x); 05 0.512 vs 0.577 (0.89x); 06 0.543 vs 0.546 (0.99x); 07 0.411 vs 0.496 (0.83x); 08 1.005 vs 0.878 (1.14x); 09 0.725 vs 0.566 (1.28x); 10 5.616 vs 0.837 (6.71x); geometric mean 1.12x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
**Main blocker**: Seq 10: 5.616 % vs 0.837 % (6.71x), 26 moving steps with length error >20 % and 34 with local vector error >20 %. The field regularization was calibrated on KITTI 00 after earlier 02/08 observations. Genuine absence scenes in GEODE remain unavailable, so full KITTI coverage does not validate that paper claim.
**Next step**: Trace association and optimization through the measured failure intervals; keep this negative fixed-setting baseline and any non-paper fix opt-in.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.69 |
| kitti_01 | 1.82 |
| kitti_02 | 1.08 |
| kitti_03 | 1.21 |
| kitti_04 | 0.81 |
| kitti_05 | 0.58 |
| kitti_06 | 0.55 |
| kitti_07 | 0.50 |
| kitti_08 | 0.88 |
| kitti_09 | 0.57 |
| kitti_10 | 0.84 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | Vanilla VGICP backend | 7.848 | 0.831 | 0.006775 | 3.75 |
| KITTI | Paper default | 94.094 | 0.920 | 0.004873 | 2.00 |
| KITTI | Vanilla VGICP backend | 27.186 | 0.884 | 0.005697 | 3.58 |
| KITTI | Paper default | 5.074 | 0.645 | 0.005395 | 3.39 |
| KITTI | Paper default | 1.719 | 0.493 | 0.005192 | 2.32 |
| KITTI | Vanilla VGICP backend | 5.556 | 0.512 | 0.004895 | 5.39 |
| KITTI | Paper default | 2.133 | 0.572 | 0.003767 | 2.27 |
| KITTI | paper_default (Table 6 variant on seq 07, raw scans) | 0.646 | 0.540 | 0.003946 | 8.77 |
| KITTI | Paper default | 0.646 | 0.540 | 0.003946 | 4.12 |
| KITTI | Vanilla VGICP backend | 16.280 | 1.309 | 0.005269 | 2.93 |
| KITTI | Paper default | 7.784 | 0.682 | 0.006319 | 1.63 |
| KITTI | Paper default | 50.982 | 2.455 | 0.018687 | 2.03 |

**Notes**: Paper average 0.865 % over 00-10; same raw-scan protocol as this repository's KITTI data.

---

## L-LO

**Paper**: Li, Fu, Sun, L-LO: Enhancing Pose Estimation Precision via a Landmark-Based LiDAR Odometry, arXiv 2023

**Reported dataset**: KITTI Odometry (sequences 00, 02-10; 01 reported as NA)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Intel Xeon Gold 6128, 64 GB RAM, Nvidia Quadro P5000
**Repo scope**: Paper reimplementation (no author code) — From-paper reimplementation in the shared harness, no per-frame GT seed. Fixed seq07_tuned on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. cluster_cell=0.25, ground_tolerance=0.15 and max_range=80 were selected by the existing 100 m RPE sweeps on sequence 07 and previously transferred unchanged to 00/02/05/08. Sequence 01 is NA in the paper and excluded.
**Current claim**: Full KITTI Odometry coverage with one fixed setting — Full-sequence official KITTI RTE on 00/02-10 (01 is NA in the paper), variant seq07_tuned, geometric mean 1.57x. cluster_cell=0.25, ground_tolerance=0.15 and max_range=80 were selected by the existing 100 m RPE sweeps on sequence 07 and previously transferred unchanged to 00/02/05/08. Sequence 01 is NA in the paper and excluded. Seq 09: 4.122 % vs 1.890 % (2.18x), 4 moving steps with length error >20 % and 9 with local vector error >20 %. Seq 10: 4.105 % vs 1.900 % (2.16x), 7 moving steps with length error >20 % and 17 with local vector error >20 %. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
**Numeric comparison**: Same metric, 1.57x on 00/02-10 (01 is NA in the paper) with one fixed setting — seq07_tuned, official KITTI RTE [%], repo vs paper: 00 1.479 vs 0.910 (1.63x); 02 4.478 vs 2.350 (1.91x); 03 3.066 vs 2.920 (1.05x); 04 2.135 vs 2.280 (0.94x); 05 1.473 vs 0.820 (1.80x); 06 1.315 vs 0.660 (1.99x); 07 1.004 vs 0.690 (1.46x); 08 1.661 vs 1.370 (1.21x); 09 4.122 vs 1.890 (2.18x); 10 4.105 vs 1.900 (2.16x); geometric mean 1.57x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
**Main blocker**: Seq 09: 4.122 % vs 1.890 % (2.18x), 4 moving steps with length error >20 % and 9 with local vector error >20 %. Seq 10: 4.105 % vs 1.900 % (2.16x), 7 moving steps with length error >20 % and 17 with local vector error >20 %. The paper leaves numeric parameters unspecified and the tested GT-derived roll-free baselines stay above the 00/07 paper values. Those tested trajectories are not a proof of a lower bound over all possible estimates. Existing vertical-pose experiments remain negative.
**Next step**: Trace association and optimization through the measured failure intervals; keep this negative fixed-setting baseline and any non-paper fix opt-in.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.91 |
| kitti_02 | 2.35 |
| kitti_03 | 2.92 |
| kitti_04 | 2.28 |
| kitti_05 | 0.82 |
| kitti_06 | 0.66 |
| kitti_07 | 0.69 |
| kitti_08 | 1.37 |
| kitti_09 | 1.89 |
| kitti_10 | 1.90 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | Seq 07 tuned | 15.664 | 1.539 | 0.020500 | 4.37 |
| KITTI | Default | 58.272 | 4.283 | 0.037143 | 3.18 |
| KITTI | Seq 07 tuned | 24.086 | 1.826 | 0.023715 | 2.82 |
| KITTI | Seq 07 tuned | 5.259 | 1.819 | 0.011828 | 0.88 |
| KITTI | Default | 8.785 | 1.719 | 0.015879 | 5.30 |
| KITTI | Seq 07 tuned | 5.791 | 1.649 | 0.012480 | 3.26 |
| KITTI | cluster cell 0p15 | 2.890 | 1.319 | 0.017045 | 17.90 |
| KITTI | sor std 2 | 1.998 | 1.376 | 0.015596 | 6.79 |
| KITTI | cell 0p35 gtol 0p15 range 80 min15 | 2.561 | 1.222 | 0.015045 | 7.72 |
| KITTI | Default | 2.045 | 1.335 | 0.015446 | 5.10 |
| KITTI | pitch band 10 40 | 1.957 | 1.349 | 0.015678 | 5.13 |
| KITTI | voxel 0p4 | 3.178 | 1.399 | 0.015898 | 22.94 |
| KITTI | rounds 600 tol 1e5 | 2.045 | 1.335 | 0.015446 | 5.87 |
| KITTI | Default | 32.662 | 2.193 | 0.018837 | 3.75 |
| KITTI | Seq 07 tuned | 36.396 | 2.483 | 0.038126 | 1.16 |
| KITTI | Seq 07 tuned | 8.632 | 3.430 | 0.037561 | 1.31 |

**Notes**: Paper average 1.59 % over the reported sequences.

---

## ELO

**Paper**: Zheng and Zhu, Efficient LiDAR Odometry for Autonomous Driving, IEEE RA-L 2021

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official KITTI t_rel (100-800 m))
**Hardware**: laptop CPU (paper Sec. V)
**Repo scope**: Paper reimplementation (no author code) — From-paper reimplementation in the shared harness; see the method README.
**Current claim**: KITTI Odometry 00-10 with the correction and the paper normal patch — The dense profile with the 0.195 deg correction the paper applies and the eq. 12 normal patch read as the full patch size scores official KITTI RTE 0.656 / 0.480 % on 00 / 07. Three forward-bias fixes of the projective model (same-surface fusion in the closest-point SRI update, no forward term from the noisy BEV ground normals, curvature threshold 0.03), chosen on 07 windows with 00 held out, bring it to 0.586 / 0.391 % vs the paper's 0.54 / 0.31 % (1.09x / 1.26x, geometric mean 1.17x). See papers/elo/README.md.
**Numeric comparison**: Same metric, 1.22x on 00-10 (one setting, chosen on 07) — dense_elevation_paper_normals_unbiased on 00-10: 0.84-1.43x, geometric mean 1.22x; 00 0.586 vs 0.54, 07 0.391 vs 0.31. Repo values: experiments/results/elo_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
**Main blocker**: The paper fusion weight w2 ("inlier ratio between the non-ground points and ground ones") is ambiguous; the reading implemented here does not help.
**Next step**: Try other readings of w2 and the ground test; run the remaining KITTI sequences.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.54 |
| kitti_01 | 0.61 |
| kitti_02 | 0.54 |
| kitti_03 | 0.65 |
| kitti_04 | 0.32 |
| kitti_05 | 0.33 |
| kitti_06 | 0.30 |
| kitti_07 | 0.31 |
| kitti_08 | 0.79 |
| kitti_09 | 0.48 |
| kitti_10 | 0.59 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | ELO dense profile + 0.195 deg correction + the paper's normal patch size | 12.632 | 0.862 | 0.004886 | 3.11 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 17.997 | 0.934 | 0.002267 | 1.00 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 29.828 | 0.827 | 0.003609 | 2.37 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 5.238 | 0.747 | 0.005862 | 2.49 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 0.557 | 0.316 | 0.001548 | 1.67 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 3.802 | 0.447 | 0.003400 | 2.42 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 3.915 | 0.409 | 0.002782 | 1.19 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 1.521 | 0.437 | 0.003413 | 3.64 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 23.811 | 1.381 | 0.005285 | 2.33 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 3.109 | 0.691 | 0.003436 | 1.65 |
| KITTI | ELO dense profile + 0.195 deg correction + paper normal patch + forward-bias fixes | 1.533 | 0.736 | 0.003874 | 2.00 |

**Notes**: Paper average 0.50 % (online test 0.68 %). The frame-to-frame variant scores 1.14/0.83/0.91/1.25/0.83/0.75/0.76/0.60/1.34/1.18/2.57 %. No loop closure. Values read from the paper PDF on 2026-10-12.

---

## IMLS-SLAM

**Paper**: Deschaud, IMLS-SLAM: scan-to-model matching based on 3D data, ICRA 2018

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (KITTI odometry drift (%))
**Hardware**: None
**Repo scope**: Paper reimplementation (no author code) with the paper odometry pipeline — From-paper reimplementation in the shared harness; see the method README.
**Current claim**: Near paper on KITTI Odometry 00-10 (paper pipeline) — --imls-slam-paper-odometry follows the paper: raw scans with the 0.22 deg beam correction, the last 100 raw scans as the model, h = 0.06 m / r = 0.20 m with a per-query normal, nine planarity-weighted lists of 100 samples, 20 iterations. Official KITTI RTE 0.523 / 0.339 % on 00 / 07 vs the paper's 0.50 / 0.33 % (1.05x / 1.03x, geometric mean 1.04x). The harness profile scores 1.378 / 0.846 %; without the beam correction the paper pipeline scores 1.327 / 0.706 %. See papers/imls_slam/README.md.
**Numeric comparison**: Same metric, 1.05x on 00-10 (one setting, paper pipeline) — paper_odometry_w100_coarse_start on 00-10: 0.91-1.29x, geometric mean 1.05x; 00 0.523 vs 0.50, 07 0.339 vs 0.33; 04 11.9 % without the coarse first registration (it starts in motion). Repo values: experiments/results/imls_slam_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
**Main blocker**: Small-object removal (paper Table II, 0.58 -> 0.55 %) is not implemented; the coarse first registration is not in the paper.
**Next step**: Run the remaining KITTI sequences with paper_odometry_w10 (1.7 FPS) or w100 (1.1 FPS).

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.50 |
| kitti_01 | 0.82 |
| kitti_02 | 0.53 |
| kitti_03 | 0.68 |
| kitti_04 | 0.33 |
| kitti_05 | 0.32 |
| kitti_06 | 0.33 |
| kitti_07 | 0.33 |
| kitti_08 | 0.80 |
| kitti_09 | 0.55 |
| kitti_10 | 0.53 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | IMLS-SLAM, dense profile | 17.571 | 1.000 | 0.009952 | 5.31 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 17.876 | 0.855 | 0.001524 | 1.47 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 12.780 | 0.803 | 0.002417 | 0.88 |
| KITTI | IMLS-SLAM paper odometry, window of 100 raw scans, 0.22 deg beam correction | 2.086 | 0.874 | 0.002632 | 1.08 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 0.921 | 0.432 | 0.001478 | 1.32 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 4.176 | 0.427 | 0.002220 | 0.89 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 1.166 | 0.502 | 0.001866 | 0.99 |
| KITTI | IMLS-SLAM, dense profile | 3.038 | 0.700 | 0.007423 | 6.25 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 19.599 | 1.319 | 0.003472 | 0.87 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 3.050 | 0.618 | 0.002089 | 0.95 |
| KITTI | IMLS-SLAM paper odometry, window 100, coarse start on the first registration | 2.970 | 0.678 | 0.002889 | 0.90 |

**Notes**: Paper overall 0.55 % on the training set (0.69 % on the test set). Scans were de-skewed with an external odometry before matching; scan-to-model, no loop closure. Values read from the paper PDF on 2026-10-12.

---

## INTEN-LOAM

**Paper**: Li et al., InTEn-LOAM: Intensity and Temporal Enhanced LiDAR Odometry and Mapping, Remote Sensing 2022 (arXiv 2209.05708)

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (KITTI RTE (%))
**Hardware**: None
**Repo scope**: Paper reimplementation (no author code) with the paper pipeline — From-paper reimplementation in the shared harness, no GT seed. One fixed variant on KITTI Odometry 00-10; existing 00/07 results are reused unchanged, and 01-06 / 08-10 are new evaluation-only full runs. Table 6 is restricted to this variant; historical ablations remain stored.
**Current claim**: Gap remains on KITTI Odometry 00-10 with one fixed setting — --inten-loam-paper-odometry uses FEF, S2S re-association, S2M on a temporal-voxel-filtered keyframe map, DOR map-insertion gating and a B-spline intensity residual. The fixed paper_odometry_plane_check variant explicitly uses s2m_plane_max_dist=0.05 and a 0.17 deg correction on all 00-10 sequences: geometric mean 2.19x. These KITTI choices were selected on 07 windows, with diagnosis of 00 frames 4000-4399 suggesting the plane check; 00 is not fully held out. The added sequences are evaluation only. Defaults and stored ablations are unchanged.
**Numeric comparison**: Same metric, 2.19x on 00-10 with one fixed setting — paper_odometry_plane_check, official KITTI RTE [%], repo vs paper: 00 0.697 vs 0.52 (1.34x); 01 16.614 vs 0.64 (25.96x); 02 3.221 vs 0.54 (5.97x); 03 0.891 vs 0.63 (1.41x); 04 0.676 vs 0.37 (1.83x); 05 0.497 vs 0.36 (1.38x); 06 0.359 vs 0.24 (1.49x); 07 0.364 vs 0.34 (1.07x); 08 0.938 vs 0.71 (1.32x); 09 0.661 vs 0.48 (1.38x); 10 1.156 vs 0.45 (2.57x); geometric mean 2.19x. Table 6 pins this variant for every sequence. Sources: experiments/results/inten_loam_kitti_seq_NN_full_paper_comp_matrix.json and experiments/results/mesh_inten_kitti_coverage/summary.json.
**Main blocker**: 01: 16.614 % vs 0.64 % (25.96x), with motion-length deficits at destination frames 217-253 and 420-518; 136 moving steps differ in length by >20 %. 02: 3.221 % vs 0.54 % (5.97x), with 133 such steps, principally 1878-1943 and 4460-4524. 10: 1.156 % vs 0.45 % (2.57x), but no moving step exceeds 20 % length or vector error; the 600-700 block grows from 6.7 to 9.2 m position error. Saved-pose diagnostics locate drift, without identifying its solver-level cause.
**Next step**: Trace association and optimization through the measured 01/02 failure intervals and the smaller accumulated errors on 10. Keep this negative fixed-setting baseline; any non-paper correction must be opt-in and validated on all sequences.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.52 |
| kitti_01 | 0.64 |
| kitti_02 | 0.54 |
| kitti_03 | 0.63 |
| kitti_04 | 0.37 |
| kitti_05 | 0.36 |
| kitti_06 | 0.24 |
| kitti_07 | 0.34 |
| kitti_08 | 0.71 |
| kitti_09 | 0.48 |
| kitti_10 | 0.45 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | InTEn-LOAM paper pipeline without the intensity residual | 16.112 | 0.820 | 0.005641 | 2.62 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 275.593 | 13.720 | 0.004111 | 1.65 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 122.845 | 3.607 | 0.010788 | 1.82 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 6.262 | 0.587 | 0.006389 | 1.55 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 1.540 | 0.702 | 0.003353 | 1.71 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 9.976 | 0.414 | 0.003794 | 1.21 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 1.918 | 0.511 | 0.002331 | 1.79 |
| KITTI | InTEn-LOAM paper pipeline without the intensity residual | 1.193 | 0.465 | 0.003995 | 3.20 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 27.673 | 1.310 | 0.004499 | 1.26 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 11.000 | 0.700 | 0.003665 | 2.11 |
| KITTI | InTEn-LOAM paper pipeline, 0.05 m S2M plane check + 0.17 deg correction | 9.670 | 0.843 | 0.004653 | 1.83 |

**Notes**: Paper average 0.54 % (frame-weighted). Includes scan-to-local-map refinement, no loop closure. Values taken from arXiv v1, not checked against the published version. Values read from the paper PDF on 2026-10-12.

---

## MESH-LOAM

**Paper**: Zhu et al., Mesh-LOAM: Real-time Mesh-Based LiDAR Odometry and Mapping, IEEE T-IV 2024

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (KITTI relative translational error (%))
**Hardware**: None
**Repo scope**: Paper reimplementation (no author code) with the paper association and input — From-paper reimplementation in the shared harness, no GT seed. One fixed variant on KITTI Odometry 00-10; existing 00/07 results are reused unchanged, and 01-06 / 08-10 are new evaluation-only full runs. Table 6 is restricted to this variant; historical ablations remain stored.
**Current claim**: Gap remains on KITTI Odometry 00-10 with one fixed setting — --mesh-loam-paper-odometry uses the paper input and nearest-facet association with an explicit 0.22 deg KITTI elevation correction, not specified by the paper. paper_odometry_elevation is unchanged across 00-10: geometric mean 1.28x, rather than the historical 1.08x on 00/07 alone. Both 00/07 influenced development; the added sequences did not influence the setting. No sequence exceeds twice its paper value. The correction remains opt-in and defaults are unchanged.
**Numeric comparison**: Same metric, 1.28x on 00-10 with one fixed setting — paper_odometry_elevation, official KITTI RTE [%], repo vs paper: 00 0.565 vs 0.53 (1.07x); 01 0.857 vs 0.64 (1.34x); 02 0.607 vs 0.52 (1.17x); 03 0.928 vs 0.63 (1.47x); 04 0.820 vs 0.41 (2.00x); 05 0.345 vs 0.34 (1.01x); 06 0.418 vs 0.35 (1.19x); 07 0.342 vs 0.31 (1.10x); 08 0.909 vs 0.82 (1.11x); 09 0.631 vs 0.47 (1.34x); 10 0.848 vs 0.56 (1.51x); geometric mean 1.28x. Table 6 pins this variant for every sequence. Sources: experiments/results/mesh_loam_kitti_seq_NN_full_paper_comp_matrix.json and experiments/results/mesh_inten_kitti_coverage/summary.json.
**Main blocker**: No author code. Full coverage shows a distributed gap (1.01-2.00x after rounding). Seq 04 is 0.819834 % vs 0.41 %, strictly below twice the paper value despite rounding to 2.00x. The earlier association and elevation fixes improved 00/07 but do not establish full reproduction.
**Next step**: Explain the remaining full-sequence gap, particularly 04 and 10; profile speed before attempting more coverage runs. Preserve the fixed-setting baseline and keep any non-paper fixes opt-in.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.53 |
| kitti_01 | 0.64 |
| kitti_02 | 0.52 |
| kitti_03 | 0.63 |
| kitti_04 | 0.41 |
| kitti_05 | 0.34 |
| kitti_06 | 0.35 |
| kitti_07 | 0.31 |
| kitti_08 | 0.82 |
| kitti_09 | 0.47 |
| kitti_10 | 0.56 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | Mesh-LOAM paper path + 0.22 deg correction + global mesh (marching cubes, far bins frozen) | 10.170 | 0.773 | 0.004332 | 1.34 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 30.863 | 0.850 | 0.002215 | 0.19 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 22.288 | 0.785 | 0.002906 | 0.33 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 3.993 | 0.877 | 0.004800 | 0.26 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 10.602 | 0.524 | 0.003036 | 0.24 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 5.758 | 0.397 | 0.002625 | 0.34 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 2.019 | 0.505 | 0.002763 | 0.37 |
| KITTI | Mesh-LOAM paper path + 0.22 deg correction + global mesh (marching cubes, far bins frozen) | 0.853 | 0.470 | 0.002749 | 1.49 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 22.402 | 1.332 | 0.004109 | 0.38 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 6.347 | 0.629 | 0.002841 | 0.41 |
| KITTI | Mesh-LOAM paper path (0.1 m input, nearest facet) + 0.22 deg correction | 7.086 | 0.725 | 0.003802 | 0.55 |

**Notes**: Paper mean 0.51 %. The point-to-plane ablation row averages 0.71 %. Scan-to-mesh-map odometry, no loop closure. Values read from the paper PDF on 2026-10-12.

---

## QUADRIC-LO

**Paper**: Xia et al., Quadric Representations for LiDAR Odometry, Mapping and Localization, IEEE RA-L 2023

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (KITTI average translation error (%))
**Hardware**: None
**Repo scope**: Paper reimplementation (no author code) — From-paper reimplementation in the shared harness, no per-frame GT seed. Fixed dense_profile on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. The existing dense profile is frozen from the 00/07 paper comparisons; both are treated as development data. No correction or parameter is selected on the added sequences.
**Current claim**: Full KITTI Odometry coverage with one fixed setting — Full-sequence official KITTI RTE on 00-10, variant dense_profile, geometric mean 0.85x. The existing dense profile is frozen from the 00/07 paper comparisons; both are treated as development data. No correction or parameter is selected on the added sequences. No sequence exceeds twice its paper value with the fixed setting. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
**Numeric comparison**: Same metric, 0.85x on 00-10 with one fixed setting — dense_profile, official KITTI RTE [%], repo vs paper: 00 0.966 vs 0.920 (1.05x); 01 2.060 vs 2.610 (0.79x); 02 1.176 vs 1.160 (1.01x); 03 1.236 vs 1.450 (0.85x); 04 0.680 vs 0.870 (0.78x); 05 0.588 vs 0.760 (0.77x); 06 0.560 vs 0.550 (1.02x); 07 0.662 vs 1.090 (0.61x); 08 1.128 vs 1.680 (0.67x); 09 0.933 vs 0.850 (1.10x); 10 1.556 vs 1.760 (0.88x); geometric mean 0.85x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
**Main blocker**: No sequence exceeds twice its paper value with the fixed setting. No author code. The local-map implementation is compared with the scan-to-submap table; separate earlier 02 fallback-validation evidence exists and 02 is not claimed as previously unseen.
**Next step**: Evaluate the fixed configuration on independent sensor conditions while preserving the full-KITTI baseline and the documented paper/implementation differences.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.92 |
| kitti_01 | 2.61 |
| kitti_02 | 1.16 |
| kitti_03 | 1.45 |
| kitti_04 | 0.87 |
| kitti_05 | 0.76 |
| kitti_06 | 0.55 |
| kitti_07 | 1.09 |
| kitti_08 | 1.68 |
| kitti_09 | 0.85 |
| kitti_10 | 1.76 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | Quadric-LO, dense profile | 14.736 | 0.867 | 0.007640 | 0.81 |
| KITTI | Quadric-LO, dense profile | 98.016 | 1.103 | 0.004921 | 0.32 |
| KITTI | Quadric-LO, dense profile | 30.983 | 0.933 | 0.006145 | 0.39 |
| KITTI | Quadric-LO, dense profile | 4.680 | 0.908 | 0.006539 | 0.40 |
| KITTI | Quadric-LO, dense profile | 1.875 | 0.516 | 0.004466 | 0.43 |
| KITTI | Quadric-LO, dense profile | 3.083 | 0.601 | 0.005693 | 0.56 |
| KITTI | Quadric-LO, dense profile | 2.425 | 0.587 | 0.003939 | 0.41 |
| KITTI | Quadric-LO, dense profile | 2.130 | 0.598 | 0.006249 | 0.88 |
| KITTI | Quadric-LO, dense profile | 23.845 | 1.330 | 0.007331 | 0.29 |
| KITTI | Quadric-LO, dense profile | 10.301 | 0.720 | 0.006676 | 0.26 |
| KITTI | Quadric-LO, dense profile | 13.264 | 0.830 | 0.008151 | 0.29 |

**Notes**: Table II (scan-to-scan plus scan-to-submap correction, average 1.25 %) is compared because the repository implementation matches against a local map. Table I (scan-to-scan only) gives 2.64/5.05/3.31/1.34/1.66/1.97/1.30/2.56/2.57/3.32/2.21 %. No loop closure. Values read from the paper PDF on 2026-10-12.

---

## DILO

**Paper**: Han et al., DiLO: Direct light detection and ranging odometry based on spherical range images for autonomous driving, ETRI Journal 2021

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (KITTI average translation error (%), 100-800 m)
**Hardware**: 4 cores, 2.6 GHz (paper)
**Repo scope**: Paper reimplementation (no author code) with a paper odometry path (plane key points only) — From-paper reimplementation in the shared harness; see the method README.
**Current claim**: Near paper on KITTI Odometry 00-10 (plane-only paper pipeline) — --dilo-paper-odometry (spherical range image, eq. 11 normals, IMLS-style key-point lists, Algorithm 1 window, plane residuals) with planarity > 0.98, at most one key point per 8 x 8 px cell per list, a slope-aware vertical fill (fill_slope 0.1) and the 0.18 deg correction: official KITTI RTE 0.549 / 0.372 % on 00 / 07 vs the paper's full DiLO 0.64 / 0.42 % (0.86x / 0.89x, geometric mean 0.87x) and plane-only ablation 0.84 / 0.58 %. Edge key points (Sec. 3.5) were implemented and do not help on KITTI. The settings were chosen on 07 windows with some 00 windows in view. See papers/dilo/README.md.
**Numeric comparison**: Same metric, 0.96x on 00-10 (one setting; 01 1.42x with a degenerate-direction remap chosen there) — paper_odometry_degenerate on 00-10: 0.75-1.42x, geometric mean 0.96x; 00 0.549 vs 0.64, 07 0.372 vs 0.42 (unchanged by the remap); 01 8.70 % without the remap. See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: Below the paper's full DiLO without its edge term. Open highway (01) needs the remap, which is not in the paper and was chosen on 01.
**Next step**: Run the remaining KITTI sequences.

### Paper-Reported Values

| Sequence | Value |
|---|---:|
| kitti_00 | 0.64 |
| kitti_01 | 1.07 |
| kitti_02 | 0.67 |
| kitti_03 | 0.74 |
| kitti_04 | 0.45 |
| kitti_05 | 0.46 |
| kitti_06 | 0.41 |
| kitti_07 | 0.42 |
| kitti_08 | 0.93 |
| kitti_09 | 0.64 |
| kitti_10 | 0.85 |

### Repository Defaults

| Dataset | Variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|---|---|---:|---:|---:|---:|
| KITTI | DiLO, default parameters | 38.742 | 1.200 | 0.022193 | 42.11 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 45.102 | 1.017 | 0.004993 | 4.84 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 40.767 | 0.788 | 0.004386 | 2.43 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 3.037 | 0.846 | 0.004714 | 3.97 |
| KITTI | DiLO paper pipeline + spread planar key points + slope-aware vertical fill + 0.18 deg correction | 0.696 | 0.372 | 0.002520 | 4.22 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 6.007 | 0.415 | 0.002889 | 2.36 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 4.029 | 0.440 | 0.002235 | 1.84 |
| KITTI | DiLO, default parameters | 7.161 | 1.533 | 0.023380 | 39.12 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 24.254 | 1.292 | 0.005352 | 2.76 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 5.353 | 0.623 | 0.003245 | 2.79 |
| KITTI | DiLO paper pipeline, spread planar key points, degenerate translation directions kept at the prediction | 6.515 | 0.713 | 0.006286 | 2.75 |

**Notes**: Paper average 0.69 % (frame-weighted). The table is printed rotated; values were rebuilt from word positions and match the stated average. No loop closure. Values read from the paper PDF on 2026-10-12.

---
