# Original-Paper Comparison

> Generated: 2026-10-03T04:51:30+00:00

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
**Numeric comparison**: Partial only (one sequence) — The paper reports per-sequence KITTI RTE only for seq 00 (0.51 %) and 04 (0.36 %), plus the 00-10 average (0.50 %). The compact pipeline's best swept variant on full seq 00 reaches 1.045 % official KITTI RTE (2.05x), re-run with --kiss-legacy-27-neighborhood so it reproduces the stored aggregate bit for bit; the post-2026-08-02 default search gives 1.211 %. Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6).
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
| KITTI | fast profile | 20.927 | 0.857 | 0.007979 | 30.58 |
| KITTI | fast | 86.812 | 1.130 | 0.009880 | 32.67 |
| KITTI | fast | 9.420 | 0.622 | 0.005475 | 23.48 |
| KITTI | fast profile | 2.525 | 0.671 | 0.007763 | 35.57 |
| KITTI | fast | 19.592 | 1.380 | 0.006661 | 30.81 |
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
**Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08 — Official KITTI RTE of the best swept variant per sequence: 1.91/3.70/1.58/1.10/1.77 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.49/0.52/0.25/0.31/0.81 % (Table I, KITTI-corrected): 2.19-7.12x, geometric mean 4.23x. Parameter tuning has not closed the gap. Re-runs with the current code differ from the stored 100 m RPE by up to 0.11 points, so these aggregates predate later CT-ICP code changes.
**Numeric comparison**: Same metric, ~4.2x ratio (parameter-tuning floor) — Official KITTI RTE, repo vs paper: seq 00 1.908 vs 0.49 (3.89x), seq 02 3.701 vs 0.52 (7.12x), seq 05 1.577 vs 0.25 (6.31x), seq 07 1.102 vs 0.31 (3.55x), seq 08 1.772 vs 0.81 (2.19x). Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6).
**Main blocker**: Parameter-only ceiling at ~4.2x best-of-sweep ratio (official KITTI RTE) on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations.
**Next step**: Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.2x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage.

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
| KITTI | cluster D (ms_chol) | 2.579 | 1.606 | 0.015756 | 7.20 |
| KITTI | Fast window | 3.871 | 2.230 | 0.032283 | 12.69 |
| KITTI | balanced_window (current best 4.67 m) | 4.105 | 1.750 | 0.020566 | 7.41 |
| KITTI | Fast window | 2.692 | 2.159 | 0.023410 | 16.89 |
| KITTI | Balanced window | 1.659 | - | - | 44.51 |
| KITTI | fast_window (current best 1.48 m) | 1.475 | 1.361 | 0.035598 | 59.36 |
| KITTI | Fast window | 6.972 | - | - | 37.60 |
| KITTI | fast_window (current best 6.97 m) | 6.972 | 1.722 | 0.039063 | 57.76 |
| KITTI | Fast window | 1.475 | - | - | 56.88 |
| KITTI | fine σ=0.375 (slightly tighter) | 13.907 | 2.195 | 0.028680 | 14.21 |
| KITTI | coarse search radius=2 (5x5x5, cluster A default) | 12.694 | 2.103 | 0.026199 | 16.19 |
| KITTI | cluster A + GT seed | 4.906 | 5.760 | 0.050591 | 15.23 |
| KITTI | cluster D full no seed (current winner 1.61 m) | 1.607 | 2.057 | 0.019671 | 21.50 |
| KITTI | cluster A + GT seed | 6.024 | 9.133 | 0.113997 | 16.76 |
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
| KITTI | corr=5 | 13.322 | 2.176 | 0.027836 | 15.45 |
| KITTI | baseline (dense + iter=6 + map=20) | 56.537 | 3.348 | 0.032189 | 19.93 |
| KITTI | bare + corr=8 | 14.968 | 2.218 | 0.028736 | 22.33 |
| KITTI | c2f_sigma_only cauchy=2.0 (existing winner) | 15.927 | 2.197 | 0.028434 | 19.39 |
| KITTI | coarse_iter=2 | 14.780 | 2.110 | 0.028141 | 18.53 |
| KITTI | corr_dist=8 m² (2.8 m linear) | 14.363 | 2.113 | 0.027721 | 17.46 |
| KITTI | map=30 | 16.541 | 2.073 | 0.027126 | 17.99 |
| KITTI | Dense window | 16.778 | 2.577 | 0.030548 | 26.91 |
| KITTI | full recipe (existing 13.32 m) | 13.322 | 2.176 | 0.027836 | 18.67 |
| KITTI | map=50 + c2f σ×2 (no ms_chol) | 12.694 | 2.103 | 0.026199 | 18.71 |
| KITTI | bare map=30 | 15.803 | 2.078 | 0.026035 | 18.69 |
| KITTI | Fast window | 2.824 | 3.773 | 0.047622 | 74.88 |
| KITTI | map=50 + c2f σ×2 (existing 12.69) | 12.694 | 2.103 | 0.026199 | 18.62 |
| KITTI | baseline + corr_dist=8 m² | 50.635 | 3.189 | 0.033420 | 16.37 |
| KITTI | corr=8 (existing seq 02 winner) | 50.635 | 3.189 | 0.033420 | 17.24 |
| KITTI | bare + corr=8 (existing winner) | 50.635 | 3.189 | 0.033420 | 21.64 |
| KITTI | dense + iter=6 + map=20 (existing winner) | 56.537 | 3.348 | 0.032189 | 16.80 |
| KITTI | baseline map=15 | 66.558 | 2.642 | 0.032795 | 19.09 |
| KITTI | corr=8 + map=20 (existing winner) | 50.635 | 3.189 | 0.033420 | 18.02 |
| KITTI | bare + map=50 + corr=4 | 8.783 | 1.177 | 0.023619 | 19.68 |
| KITTI | arch_tuned corr=4 (existing winner) | 9.097 | 1.127 | 0.023189 | 15.74 |
| KITTI | arch_tuned map=30 (existing winner) | 9.097 | 1.127 | 0.023189 | 18.29 |
| KITTI | bare map=50 | 9.337 | 1.190 | 0.024517 | 18.01 |
| KITTI | ms_chol (existing 1.61 m) | 1.607 | 2.057 | 0.019671 | 14.85 |
| KITTI | ms_chol corr_dist=default (existing winner) | 1.607 | 2.057 | 0.019671 | 16.98 |
| KITTI | Fast window | 10.705 | 2.452 | 0.039323 | 74.86 |
| KITTI | ms_chol map=20 (existing winner) | 1.607 | 2.057 | 0.019671 | 19.70 |
| KITTI | Fast window | 0.978 | 1.359 | 0.082217 | 77.32 |
| KITTI | c2f_only = ms_chol + c2f (existing winner) | 32.511 | 2.120 | 0.025611 | 17.87 |
| KITTI | c2f_only corr_dist=default (existing winner) | 32.511 | 2.120 | 0.025611 | 17.35 |
| KITTI | cauchy=3.0 | 34.397 | 2.099 | 0.026268 | 18.44 |
| KITTI | coarse_iter=6 | 35.598 | 2.126 | 0.025911 | 13.63 |
| KITTI | c2f_only map=20 (existing winner) | 32.511 | 2.120 | 0.025611 | 13.44 |
| KITTI | simplified (map=50 + c2f σ×2) — existing 27.85 m | 27.850 | 1.949 | 0.024600 | 15.74 |
| KITTI | bare + corr=8 (existing seq 02 winner) | 50.635 | 3.189 | 0.033420 | 21.80 |
| KITTI | map=50 + c2f σ×2 + corr=4 (combine both) | 7.762 | 1.198 | 0.023988 | 18.92 |
| KITTI | ms_chol (existing seq 07 winner) | 1.607 | 2.057 | 0.019671 | 20.27 |
| KITTI | map=50 + c2f σ×2 (no ms_chol) | 27.850 | 1.949 | 0.024600 | 18.33 |

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
