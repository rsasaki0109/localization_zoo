# Original-Paper Comparison

> Generated: 2026-10-04T23:20:39+00:00

This document compares paper-reported metrics with the current repository defaults across each method family. Direct comparison is limited by differences in dataset windows, hardware, and metric definitions (ATE and RPE availability still differs by family).

For an explicit statement of implementation scope and what the repo currently claims to reproduce, see [`docs/reproduction_status.md`](reproduction_status.md).

## LiTAMIN2

**Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021

**Reported dataset**: KITTI Odometry (sequences 00-10)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Intel Core i9-9900K, 32 GB RAM, NVIDIA GPU (Sec. IV)
**Repo scope**: Paper-oriented front-end reimplementation — The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
**Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08 — Official KITTI RTE of the best swept variant per sequence: 0.92/1.32/0.73/0.61/0.98 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.78/0.95/0.55/0.48/1.01 % (LiTAMIN2 ICP+Cov without loop closure, Table III): 0.97-1.39x, geometric mean 1.22x.
**Numeric comparison**: Same metric, ~1.2x ratio (best-of-sweep) — Official KITTI RTE, repo vs paper: seq 00 0.920 vs 0.78 (1.18x), seq 02 1.324 vs 0.95 (1.39x), seq 05 0.734 vs 0.55 (1.33x), seq 07 0.613 vs 0.48 (1.28x), seq 08 0.977 vs 1.01 (0.97x). Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: Remaining ~1.2x best-of-sweep ratio vs paper (official KITTI RTE); no single variant is best on every sequence. Correspondence pruning, covariance floor, covariance-shape-gradient, line-search, and coarse-to-fine voxel controls are now sweepable. Radius-1 plus 1.5 m gate improves seq02/05 full ATE/RPE, but seq07 regresses and seq08 trades lower ATE for worse RPE. A weaker covariance floor (1e-4) improves speed and some sequences but leaves seq02/05/07/08 geometric-mean RPE flat (0.806 -> 0.807). The raw opt-in covariance gradient is not stable at full sequence scale: geometric-mean RPE worsens to 1.451 %. A damped covariance-gradient variant (0.1x + line search) avoids that collapse and reaches 0.806 % geometric-mean RPE, but it trades seq02 ATE from 50.622 m to 81.961 m. A 3.0 -> 2.0 -> 1.0 coarse-to-fine voxel schedule helps short seq02 smoke but full-sequence geometric-mean RPE worsens to 1.185 % because seq02/08 regress. Remaining gaps likely involve adaptive stage acceptance, map refresh, and side-by-side comparison against the upstream implementation. Also sequences 01/03/04/06/09/10 not yet exported.
**Next step**: Export and run KITTI Odometry 01/03/04/06/09/10 for full paper coverage. Investigate adaptive stage acceptance / map-refresh policies for the coarse-to-fine path, or compare side-by-side with the upstream LiTAMIN2 reference implementation to identify which internal step introduces the ceiling.

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
| KITTI | Tuned: voxel 1.0 + iter 12 (no GT seed) | 13.464 | 0.920 | 0.008111 | 60.95 |
| KITTI | voxel=2.0 + iter=12 + GT seed | 1.117 | 2.024 | 0.018026 | 86.83 |
| KITTI | cluster T1: voxel=0.5 + iter=12 + seed | 0.728 | 0.952 | 0.010395 | 68.31 |
| KITTI | fast + seed (baseline) | 0.957 | 1.419 | 0.013525 | 17.17 |
| KITTI | fast + seed (baseline) | 0.836 | 1.039 | 0.011014 | 94.73 |
| KITTI | fast + seed (baseline) | 1.130 | 2.117 | 0.018472 | 104.92 |
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

**Notes**: Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6). Moving the voxel resolution from 2.0 m to 1.0 m delivered most of the gap closure. A new correspondence sweep knob is promising but selective: on KITTI seq02 200-frame no-GT smoke, radius 1 plus a 1.5 m gate improves ATE 22.522 -> 1.042 m and drift 2.330 -> 0.730 m/100m. On full sequences, the same gate improves seq02 ATE/RPE 50.622 m / 0.975 % -> 44.005 m / 0.936 % and seq05 7.350 m / 0.626 % -> 6.069 m / 0.611 %, but worsens seq07 1.964 m / 0.535 % -> 2.315 m / 0.575 % and seq08 RPE 1.295 % -> 1.311 % despite lower ATE. Across seq02/05/07/08, geometric-mean RPE is 0.806 % baseline vs 0.810 % with the gate. A weaker covariance floor (`--litamin2-min-cov-eigenvalue 1e-4`) is faster and improves seq05, but cross-sequence geometric-mean RPE is also flat at 0.807 %. The opt-in covariance-gradient path (`--litamin2-covariance-gradient`) wires the covariance-shape cost into the rotation update, but full-sequence results regress: seq02/05/07/08 produce RPE 5.679/0.684/0.845/1.351 %, or 1.451 % geometric mean. Adding `--litamin2-covariance-gradient-weight 0.1 --litamin2-line-search` avoids the raw collapse and gives RPE 0.980/0.609/0.552/1.282 %, or 0.806 % geometric mean, but seq02 ATE worsens to 81.961 m. The `--litamin2-coarse-to-fine-voxels 3.0,2.0,1.0` path strongly improves seq02 108-frame smoke, but full sequences produce RPE 2.348/0.613/0.530/2.582 %, or 1.185 % geometric mean, so it is not a paper default. The paper-oriented default remains unchanged. Policy A GT-seeded numbers (0.74 % on Raw 0009 short window, 1.26 % on MulRan parkinglot full) remain in the file but are explicitly non-comparable to paper - they measure local scan-registration quality on top of a per-frame GT prior, not pure odometry drift.

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
**Current claim**: Benchmark-comparable only — The current implementation is close enough for same-contract comparisons, but it should not be presented as a faithful rerun of the upstream KISS-ICP project.
**Numeric comparison**: Partial only (one sequence) — The paper reports per-sequence KITTI RTE only for seq 00 (0.51 %) and 04 (0.36 %), plus the 00-10 average (0.50 %). With the current default correspondence search the best re-run seq 00 variant reaches 0.954 % official KITTI RTE (1.87x); --kiss-legacy-27-neighborhood (the upstream search) reproduces the pre-2026-08 aggregates. Values are the official KITTI RTE of the best variant among sweeps re-run with the current code on 2026-10-04 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation.
**Next step**: Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation.

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
| KITTI | dense profile | 12.323 | 0.963 | 0.007617 | 1.25 |
| KITTI | balanced | 71.183 | 1.178 | 0.008933 | 3.06 |
| KITTI | dense | 4.556 | 0.715 | 0.005066 | 1.64 |
| KITTI | balanced (default) | 2.238 | 0.664 | 0.007003 | 3.38 |
| KITTI | fast | 18.085 | 1.549 | 0.007241 | 2.23 |
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
**Repo scope**: Paper-oriented core reimplementation — The repo implements the continuous-time two-pose-per-scan core, interpolation, and point-to-plane optimization described by CT-ICP.
**Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08 — Official KITTI RTE: 1.66/3.47/1.58/1.07/1.75 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.49/0.52/0.25/0.31/0.81 % (Table I, KITTI-corrected): 2.16-6.68x, geometric mean 4.03x. Parameter tuning has not closed the gap. Values are the official KITTI RTE of the best variant among all ~313 CT-ICP KITTI sweep variants re-run with the current code on 2026-10-05 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
**Numeric comparison**: Same metric, ~4.0x ratio (parameter-tuning floor) — Official KITTI RTE, repo vs paper: seq 00 1.664 vs 0.49 (3.40x), seq 02 3.472 vs 0.52 (6.68x), seq 05 1.577 vs 0.25 (6.31x), seq 07 1.067 vs 0.31 (3.44x), seq 08 1.749 vs 0.81 (2.16x). Values are the official KITTI RTE of the best variant among all ~313 CT-ICP KITTI sweep variants re-run with the current code on 2026-10-05 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: Parameter-only ceiling at ~4.0x best-of-sweep ratio (official KITTI RTE) on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations.
**Next step**: Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.0x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage.

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
| KITTI | fine σ=0.25 (tighter) | 12.351 | 2.040 | 0.025852 | 8.44 |
| KITTI | c2f_sigma_only map=20 (existing winner) | 18.370 | 2.137 | 0.027809 | 10.39 |
| KITTI | Balanced window | 19.413 | 2.111 | 0.027532 | 12.99 |
| KITTI | coarse search radius=1 (3x3x3, same as fine) | 14.817 | 2.087 | 0.027970 | 10.24 |
| KITTI | cluster A + GT seed | 5.855 | 5.998 | 0.076457 | 6.87 |
| KITTI | full minus ms_chol (no multi-scale, no cholesky) | 12.931 | 2.092 | 0.026054 | 11.55 |
| KITTI | map=50 + corr=5 | 14.894 | 2.149 | 0.027041 | 12.48 |
| KITTI | bare map=50 | 15.753 | 2.043 | 0.027532 | 11.23 |
| KITTI | + corr=5 | 12.931 | 2.092 | 0.026054 | 11.40 |
| KITTI | dense + c2f cluster-A (existing best, RPE 2.059%) | 14.099 | 2.057 | 0.027515 | 8.66 |
| KITTI | baseline (dense + iter=6 + map=20) — existing winner | 68.972 | 2.977 | 0.031920 | 10.31 |
| KITTI | corr=5 | 65.305 | 3.165 | 0.033007 | 10.40 |
| KITTI | ms_chol + corr=5 | 65.305 | 3.165 | 0.033007 | 8.65 |
| KITTI | dense + iter=6 + map=20 (existing winner) | 68.972 | 2.977 | 0.031920 | 10.70 |
| KITTI | baseline map=15 | 76.464 | 2.651 | 0.033446 | 4.44 |
| KITTI | corr=8 + map=15 | 73.230 | 2.678 | 0.032340 | 11.73 |
| KITTI | arch_tuned + map=50 (full + bigger) | 8.844 | 1.175 | 0.023420 | 10.16 |
| KITTI | arch_tuned corr=4 (existing winner) | 9.485 | 1.185 | 0.023558 | 11.60 |
| KITTI | arch_tuned map=50 | 8.844 | 1.175 | 0.023420 | 11.31 |
| KITTI | bare map=30 | 11.707 | 1.227 | 0.023648 | 7.02 |
| KITTI | + constant-velocity-weight 0.05 (winner) | 11.949 | 1.122 | 0.023083 | 5.11 |
| KITTI | ms_chol + map=50 + c2f σ×2 (full combo) | 2.010 | 1.772 | 0.017504 | 9.86 |
| KITTI | ms_chol + corr_dist=8 m² | 2.049 | 2.147 | 0.020803 | 14.06 |
| KITTI | Dense window | 2.842 | 2.103 | 0.021053 | 17.52 |
| KITTI | cluster D full + GT seed | 1.603 | 2.082 | 0.020798 | 11.20 |
| KITTI | ms_chol map=50 | 1.472 | 1.730 | 0.018054 | 12.67 |
| KITTI | + constant-velocity-weight 0.1 | 2.490 | 1.253 | 0.018813 | 5.97 |
| KITTI | cholesky + c2f (no multi-scale) | 37.043 | 2.113 | 0.025832 | 12.18 |
| KITTI | c2f_only + corr_dist=8 m² | 33.860 | 2.113 | 0.026506 | 10.18 |
| KITTI | cauchy=2.5 | 31.958 | 2.112 | 0.026070 | 11.93 |
| KITTI | coarse_iter=6 | 30.418 | 2.105 | 0.026321 | 10.39 |
| KITTI | cluster A + GT seed | 6.813 | 8.142 | 0.078035 | 6.85 |
| KITTI | c2f_only map=20 (existing winner) | 37.043 | 2.113 | 0.025832 | 8.63 |
| KITTI | simplified (map=50 + c2f σ×2) — existing 27.85 m | 28.427 | 2.017 | 0.024197 | 10.95 |
| KITTI | + constant-velocity-weight 0.01 (RPE 1.935%) | 40.362 | 1.949 | 0.023975 | 9.30 |
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
| KITTI | Fast window | 2.824 | 3.773 | 0.047622 | 74.88 |
| KITTI | Fast window | 0.978 | 1.359 | 0.082217 | 77.32 |

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
**Repo scope**: Derived variant — Multi-metric scan-to-map alignment with edge, plane, and point residuals inside the shared benchmark harness.
**Current claim**: KITTI Odometry 00/02/05/07/08 — Official KITTI RTE of the dense variant: 3.79/4.11/2.68/2.63/4.27 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.51/0.55/0.28/0.29/0.80 % (MULLS-LO(mc)): 5.34-9.56x, geometric mean 7.62x. The repo variant is a derived multi-metric scan-to-map registration, not the full MULLS pipeline. Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Numeric comparison**: Same metric, ~7.6x ratio — Official KITTI RTE, repo vs paper: seq 00 3.793 vs 0.51 (7.44x), seq 02 4.107 vs 0.55 (7.47x), seq 05 2.678 vs 0.28 (9.56x), seq 07 2.627 vs 0.29 (9.06x), seq 08 4.271 vs 0.80 (5.34x). Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The derived variant implements only the multi-metric registration core; the gap to MULLS-LO is ~7.6x on the official KITTI RTE.
**Next step**: Compare step-by-step with the upstream MULLS implementation to locate the missing components.

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
| KITTI | Fast | 56.820 | 2.691 | 0.028323 | 1.79 |
| KITTI | Fast | 260.878 | 2.272 | 0.026005 | 1.72 |
| KITTI | Fast | 26.886 | 1.939 | 0.021480 | 1.72 |
| KITTI | Fast | 10.501 | 2.994 | 0.028779 | 4.13 |
| KITTI | Fast | 87.759 | 3.317 | 0.030065 | 1.69 |
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
**Repo scope**: Paper reimplementation — Dense surfel odometry from range-image maps inside the shared benchmark harness.
**Current claim**: KITTI Odometry 00/02/05/07/08 — Official KITTI RTE of the dense profile: 1.60/1.78/1.28/1.10/2.03 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.7/1.1/0.5/0.4/1.0 % (Frame-to-Model, no loop closure): 1.62-2.74x, geometric mean 2.21x. Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Numeric comparison**: Same metric, ~2.2x ratio — Official KITTI RTE, repo vs paper: seq 00 1.602 vs 0.7 (2.29x), seq 02 1.785 vs 1.1 (1.62x), seq 05 1.279 vs 0.5 (2.56x), seq 07 1.095 vs 0.4 (2.74x), seq 08 2.035 vs 1.0 (2.03x). The paper rounds to one decimal. Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The CPU reimplementation reaches ~2.2x the paper's Frame-to-Model RTE on the official metric.
**Next step**: Compare surfel stability and map fusion against the upstream SuMa implementation.

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
| KITTI | dense profile | 18.961 | 1.254 | 0.012130 | 24.17 |
| KITTI | dense profile | 51.911 | 1.276 | 0.009604 | 24.22 |
| KITTI | default | 10.983 | 2.013 | 0.021825 | 40.33 |
| KITTI | dense profile | 19.290 | 1.909 | 0.012225 | 26.63 |
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
**Repo scope**: Paper reimplementation — Curvature features with a three-stage odometry-to-map LOAM pipeline (A-LOAM structure).
**Current claim**: KITTI Odometry 00/02/05/07/08 — Official KITTI RTE: 0.86/1.01/0.54/0.68/0.98 % on KITTI Odometry 00/02/05/07/08 versus LOAM's 0.78/0.92/0.57/0.63/1.12 % (secondary citation): 0.87-1.10x, geometric mean 1.01x. Values are the official KITTI RTE of the best variant among sweeps re-run with the current code on 2026-10-04 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
**Numeric comparison**: Same metric, secondary-source paper values — Official KITTI RTE, repo vs LOAM: seq 00 0.855 vs 0.78 (1.10x), seq 02 1.012 vs 0.92 (1.10x), seq 05 0.535 vs 0.57 (0.94x), seq 07 0.681 vs 0.63 (1.08x), seq 08 0.975 vs 1.12 (0.87x). Values are the official KITTI RTE of the best variant among sweeps re-run with the current code on 2026-10-04 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The LOAM paper itself does not report per-sequence KITTI RTE; the values are LOAM's KITTI numbers as cited by later papers.
**Next step**: Compare against the official A-LOAM implementation on the same full sequences.

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
| KITTI | Fast | 70.870 | 0.961 | 0.006722 | 3.18 |
| KITTI | Fast | 7.796 | 0.577 | 0.005857 | 3.02 |
| KITTI | Fast | 4.030 | 0.691 | 0.006795 | 1.75 |
| KITTI | Fast | 26.170 | 1.436 | 0.006181 | 2.89 |
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
**Repo scope**: Paper reimplementation (no author code) — GICP scan-to-map backend with the voxel-normal localizability field, median/hysteresis gate, and soft Fisher-information weighting, following the paper's Table SI constants.
**Current claim**: KITTI Odometry 00/02/05/07/08 — Official KITTI RTE of the paper-default configuration: 0.66/1.08/0.51/0.41/1.01 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.692/1.084/0.577/0.496/0.878 % (Table I): 0.83-1.14x, geometric mean 0.96x. The gate fires on 0-19 % of frames (paper: <= 8 % on open roads). Repo values are the paper_default variant only (the vanilla-backend ablation is excluded); see docs/assets/paper/paper_ratio_table.csv (Table 6) and papers/lf_gicp/README.md for the planarity/lambda0 calibration note.
**Numeric comparison**: Same metric, ~0.96x ratio (single configuration) — Official KITTI RTE, repo vs paper: seq 00 0.664 vs 0.692 (0.96x), seq 02 1.075 vs 1.084 (0.99x), seq 05 0.512 vs 0.577 (0.89x), seq 07 0.411 vs 0.496 (0.83x), seq 08 1.005 vs 0.878 (1.14x). Disabling the gate gives 0.734/1.074/0.512/0.483/0.918 %: the gate helps on 00/07, is neutral on 02/05, and hurts on 08, where it fires on 19 % of frames. Repo values are the paper_default variant only (the vanilla-backend ablation is excluded); see docs/assets/paper/paper_ratio_table.csv (Table 6) and papers/lf_gicp/README.md for the planarity/lambda0 calibration note.
**Main blocker**: The lambda0 scale differs from the paper (planarity definition is ambiguous) and GEODE tunnels for gate calibration are not available.
**Next step**: Obtain GEODE tunnel data to check the gate on genuine absence and recalibrate tau2 with the paper's rule.

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
| KITTI | Vanilla VGICP backend | 27.186 | 0.884 | 0.005697 | 3.58 |
| KITTI | Vanilla VGICP backend | 5.556 | 0.512 | 0.004895 | 5.39 |
| KITTI | Paper default | 0.646 | 0.540 | 0.003946 | 4.12 |
| KITTI | Vanilla VGICP backend | 16.280 | 1.309 | 0.005269 | 2.93 |

**Notes**: Paper average 0.865 % over 00-10; same raw-scan protocol as this repository's KITTI data.

---

## L-LO

**Paper**: Li, Fu, Sun, L-LO: Enhancing Pose Estimation Precision via a Landmark-Based LiDAR Odometry, arXiv 2023

**Reported dataset**: KITTI Odometry (sequences 00, 02-10; 01 reported as NA)
**Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
**Hardware**: Intel Xeon Gold 6128, 64 GB RAM, Nvidia Quadro P5000
**Repo scope**: Paper reimplementation (no author code) — Frame-to-frame landmark odometry with convex-hull similarity and overlap maximisation; the paper gives no numeric parameters.
**Current claim**: KITTI Odometry 00/02/05/07/08 — Official KITTI RTE: 1.48/4.48/1.47/1.00/1.66 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.91/2.35/0.82/0.69/1.37 % (Table I, t_rel): 1.21-1.91x, geometric mean 1.58x. Seq 07 is the best 100 m RPE of the seq 07 sweeps (l_lo_kitti_seq_07_*_sweep_matrix.json, 30 settings); 00/02/05/08 use that choice unchanged, so those rows are held out. The untuned default gives 1.62x, so parameter choice moves the ratio by only ~3 %; see papers/l_lo/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
**Numeric comparison**: Same metric, ~1.6x ratio (parameters chosen on seq 07) — Official KITTI RTE, repo vs paper: seq 00 1.479 vs 0.91 (1.63x), seq 02 4.478 vs 2.35 (1.91x), seq 05 1.473 vs 0.82 (1.80x), seq 07 1.004 vs 0.69 (1.46x), seq 08 1.661 vs 1.37 (1.21x). At least three landmark matches on every frame except 6 of 4660 on seq 02. Seq 07 is the best 100 m RPE of the seq 07 sweeps (l_lo_kitti_seq_07_*_sweep_matrix.json, 30 settings); 00/02/05/08 use that choice unchanged, so those rows are held out. The untuned default gives 1.62x, so parameter choice moves the ratio by only ~3 %; see papers/l_lo/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: The paper specifies no thresholds and its pitch estimate is not physically consistent as written; a seq 07 parameter sweep closes only ~3 % of the gap, so the remaining difference is likely in the method details. See papers/l_lo/README.md.
**Next step**: Ask the authors for code, or compare per-frame landmark matches against a reference implementation.

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
| KITTI | Default | 8.785 | 1.719 | 0.015879 | 5.30 |
| KITTI | cluster cell 0p15 | 2.890 | 1.319 | 0.017045 | 17.90 |
| KITTI | sor std 2 | 1.998 | 1.376 | 0.015596 | 6.79 |
| KITTI | cell 0p35 gtol 0p15 range 80 min15 | 2.561 | 1.222 | 0.015045 | 7.72 |
| KITTI | Default | 2.045 | 1.335 | 0.015446 | 5.10 |
| KITTI | pitch band 10 40 | 1.957 | 1.349 | 0.015678 | 5.13 |
| KITTI | voxel 0p4 | 3.178 | 1.399 | 0.015898 | 22.94 |
| KITTI | rounds 600 tol 1e5 | 2.045 | 1.335 | 0.015446 | 5.87 |
| KITTI | Default | 32.662 | 2.193 | 0.018837 | 3.75 |

**Notes**: Paper average 1.59 % over the reported sequences.

---
