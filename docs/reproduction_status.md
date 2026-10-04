# Reproduction Status

_Generated at 2026-10-04T19:52:00+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

This page records what the repository can currently claim about reproducing original-paper results.
The tracked subset below is intentionally conservative: if the implementation, metric, dataset, or protocol diverges, the repo should say so explicitly.

## Claim Level Legend

Every tracked family carries a `claim_level` that classifies how strongly the repo's evidence supports a reproduction claim. Levels are listed from strongest to weakest.

| Level | Meaning |
|-------|---------|
| `reproduced` | Same dataset variant, same sequence, same evaluation protocol, same sensor condition as the source paper, ideally full sequence, with metric values consistent with the paper or official leaderboard. |
| `approximately_reproduced` | Dataset family matches but window / preprocessing / metric implementation / parameters diverge in a way that prevents one-to-one comparison. Useful for directional comparison. |
| `indicative` | Short window, Raw-vs-Odometry mismatch, custom window, or no direct paper claim to match. Useful inside the repo benchmark, not a paper-result reproduction. |
| `smoke` | Demonstrates that the contract works end-to-end. Not for accuracy comparison. |
| `ported` | Inputs and outputs are wired up but no paper-comparable benchmark is targeted (e.g. concept-only baselines, custom integrations). |

## Tracked Families

| Method | Claim Level | Repo Scope | Current Claim | Numeric Comparison | Main Blocker | Next Step |
|--------|-------------|------------|---------------|--------------------|--------------|-----------|
| LiTAMIN2 | `approximately_reproduced` | Paper-oriented front-end reimplementation | Approximate reproduction across KITTI Odometry 00/02/05/07/08 | Same metric, ~1.2x ratio (best-of-sweep) | Remaining ~1.2x best-of-sweep ratio vs paper (official KITTI RTE); no single variant is best on every sequence. Correspondence pruning, covariance floor, covariance-shape-gradient, line-search, and coarse-to-fine voxel controls are now sweepable. Radius-1 plus 1.5 m gate improves seq02/05 full ATE/RPE, but seq07 regresses and seq08 trades lower ATE for worse RPE. A weaker covariance floor (1e-4) improves speed and some sequences but leaves seq02/05/07/08 geometric-mean RPE flat (0.806 -> 0.807). The raw opt-in covariance gradient is not stable at full sequence scale: geometric-mean RPE worsens to 1.451 %. A damped covariance-gradient variant (0.1x + line search) avoids that collapse and reaches 0.806 % geometric-mean RPE, but it trades seq02 ATE from 50.622 m to 81.961 m. A 3.0 -> 2.0 -> 1.0 coarse-to-fine voxel schedule helps short seq02 smoke but full-sequence geometric-mean RPE worsens to 1.185 % because seq02/08 regress. Remaining gaps likely involve adaptive stage acceptance, map refresh, and side-by-side comparison against the upstream implementation. Also sequences 01/03/04/06/09/10 not yet exported. | Export and run KITTI Odometry 01/03/04/06/09/10 for full paper coverage. Investigate adaptive stage acceptance / map-refresh policies for the coarse-to-fine path, or compare side-by-side with the upstream LiTAMIN2 reference implementation to identify which internal step introduces the ceiling. |
| GICP | `ported` | Compact shared implementation | Concept-only comparison | Not direct | There is no single paper benchmark and hardware protocol to reproduce against. | Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence. |
| NDT | `ported` | Compact baseline | Concept-only comparison | Not direct | There is no agreed modern NDT reference setup wired into the repo as the reproduction target. | Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required. |
| KISS-ICP | `indicative` | Compact baseline | Benchmark-comparable only | Partial only (one sequence) | The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation. | Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation. |
| CT-ICP | `approximately_reproduced` | Paper-oriented core reimplementation | Approximate reproduction across KITTI Odometry 00/02/05/07/08 | Same metric, ~4.0x ratio (parameter-tuning floor) | Parameter-only ceiling at ~4.0x best-of-sweep ratio (official KITTI RTE) on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations. | Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.0x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage. |
| CT-LIO | `ported` | Custom integration | Not comparable to a single paper | Reference-based only | There is no single paper-faithful target implementation and protocol behind this path. | Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol. |
| MULLS | `indicative` | Derived variant | KITTI Odometry 00/02/05/07/08 | Same metric, ~7.6x ratio | The derived variant implements only the multi-metric registration core; the gap to MULLS-LO is ~7.6x on the official KITTI RTE. | Compare step-by-step with the upstream MULLS implementation to locate the missing components. |
| SUMA | `indicative` | Paper reimplementation | KITTI Odometry 00/02/05/07/08 | Same metric, ~2.2x ratio | The CPU reimplementation reaches ~2.2x the paper's Frame-to-Model RTE on the official metric. | Compare surfel stability and map fusion against the upstream SuMa implementation. |
| ALOAM | `indicative` | Paper reimplementation | KITTI Odometry 00/02/05/07/08 | Same metric, secondary-source paper values | The LOAM paper itself does not report per-sequence KITTI RTE; the values are LOAM's KITTI numbers as cited by later papers. | Compare against the official A-LOAM implementation on the same full sequences. |
| LF-GICP | `approximately_reproduced` | Paper reimplementation (no author code) | KITTI Odometry 00/02/05/07/08 | Same metric, ~0.96x ratio (single configuration) | The lambda0 scale differs from the paper (planarity definition is ambiguous) and GEODE tunnels for gate calibration are not available. | Obtain GEODE tunnel data to check the gate on genuine absence and recalibrate tau2 with the paper's rule. |

## LiTAMIN2

- **Claim level**: `approximately_reproduced`
- **Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021
- **Method README**: `papers/litamin2/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper-oriented front-end reimplementation. The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
- **Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08. Official KITTI RTE of the best swept variant per sequence: 0.92/1.32/0.73/0.61/0.98 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.78/0.95/0.55/0.48/1.01 % (LiTAMIN2 ICP+Cov without loop closure, Table III): 0.97-1.39x, geometric mean 1.22x.
- **Numeric comparison**: Same metric, ~1.2x ratio (best-of-sweep). Official KITTI RTE, repo vs paper: seq 00 0.920 vs 0.78 (1.18x), seq 02 1.324 vs 0.95 (1.39x), seq 05 0.734 vs 0.55 (1.33x), seq 07 0.613 vs 0.48 (1.28x), seq 08 0.977 vs 1.01 (0.97x). Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: Remaining ~1.2x best-of-sweep ratio vs paper (official KITTI RTE); no single variant is best on every sequence. Correspondence pruning, covariance floor, covariance-shape-gradient, line-search, and coarse-to-fine voxel controls are now sweepable. Radius-1 plus 1.5 m gate improves seq02/05 full ATE/RPE, but seq07 regresses and seq08 trades lower ATE for worse RPE. A weaker covariance floor (1e-4) improves speed and some sequences but leaves seq02/05/07/08 geometric-mean RPE flat (0.806 -> 0.807). The raw opt-in covariance gradient is not stable at full sequence scale: geometric-mean RPE worsens to 1.451 %. A damped covariance-gradient variant (0.1x + line search) avoids that collapse and reaches 0.806 % geometric-mean RPE, but it trades seq02 ATE from 50.622 m to 81.961 m. A 3.0 -> 2.0 -> 1.0 coarse-to-fine voxel schedule helps short seq02 smoke but full-sequence geometric-mean RPE worsens to 1.185 % because seq02/08 regress. Remaining gaps likely involve adaptive stage acceptance, map refresh, and side-by-side comparison against the upstream implementation. Also sequences 01/03/04/06/09/10 not yet exported.
- **Next step**: Export and run KITTI Odometry 01/03/04/06/09/10 for full paper coverage. Investigate adaptive stage acceptance / map-refresh policies for the coarse-to-fine path, or compare side-by-side with the upstream LiTAMIN2 reference implementation to identify which internal step introduces the ceiling.
- **Notes**: Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6). Moving the voxel resolution from 2.0 m to 1.0 m delivered most of the gap closure. A new correspondence sweep knob is promising but selective: on KITTI seq02 200-frame no-GT smoke, radius 1 plus a 1.5 m gate improves ATE 22.522 -> 1.042 m and drift 2.330 -> 0.730 m/100m. On full sequences, the same gate improves seq02 ATE/RPE 50.622 m / 0.975 % -> 44.005 m / 0.936 % and seq05 7.350 m / 0.626 % -> 6.069 m / 0.611 %, but worsens seq07 1.964 m / 0.535 % -> 2.315 m / 0.575 % and seq08 RPE 1.295 % -> 1.311 % despite lower ATE. Across seq02/05/07/08, geometric-mean RPE is 0.806 % baseline vs 0.810 % with the gate. A weaker covariance floor (`--litamin2-min-cov-eigenvalue 1e-4`) is faster and improves seq05, but cross-sequence geometric-mean RPE is also flat at 0.807 %. The opt-in covariance-gradient path (`--litamin2-covariance-gradient`) wires the covariance-shape cost into the rotation update, but full-sequence results regress: seq02/05/07/08 produce RPE 5.679/0.684/0.845/1.351 %, or 1.451 % geometric mean. Adding `--litamin2-covariance-gradient-weight 0.1 --litamin2-line-search` avoids the raw collapse and gives RPE 0.980/0.609/0.552/1.282 %, or 0.806 % geometric mean, but seq02 ATE worsens to 81.961 m. The `--litamin2-coarse-to-fine-voxels 3.0,2.0,1.0` path strongly improves seq02 108-frame smoke, but full sequences produce RPE 2.348/0.613/0.530/2.582 %, or 1.185 % geometric mean, so it is not a paper default. The paper-oriented default remains unchanged. Policy A GT-seeded numbers (0.74 % on Raw 0009 short window, 1.26 % on MulRan parkinglot full) remain in the file but are explicitly non-comparable to paper - they measure local scan-registration quality on top of a per-frame GT prior, not pure odometry drift.

## GICP

- **Claim level**: `ported`
- **Paper**: Segal et al., Generalized-ICP, RSS 2009
- **Method README**: `papers/gicp/README.md`
- **Reported dataset**: Custom indoor/outdoor datasets
- **Reported metric**: qualitative alignment quality
- **Repo scope**: Compact shared implementation. The repo keeps a self-contained GICP-style registration loop with local covariance modeling under the stable evaluator.
- **Current claim**: Concept-only comparison. The implementation is useful as benchmark evidence for the GICP idea, but not as a paper-result reproduction of a specific historical codebase.
- **Numeric comparison**: Not direct. The original paper predates the repo's modern public benchmark suite and does not provide a standardized numeric target that can be matched one-to-one here.
- **Main blocker**: There is no single paper benchmark and hardware protocol to reproduce against.
- **Next step**: Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence.
- **Notes**: Original GICP paper predates KITTI. No standardized numeric benchmark reported. Comparison is with the concept, not specific numbers. As supporting evidence under the dogfooding-fixed --no-gt-seed pure-odometry path (build 2026-05-18, velocity-model prior), GICP fast on KITTI Raw 0009 full reports ATE 4.82 m / RPE 4.70 % (best in the Policy A family on this sequence, edging LiTAMIN2's 7.45 m). This is honest pure-odometry behavior but not a paper-reproduction claim; it is an internal calibration of where GICP sits versus other Policy A methods when the GT-seed asymmetry is removed.

## NDT

- **Claim level**: `ported`
- **Paper**: Biber & Strasser, The Normal Distributions Transform: A New Approach to Laser Scan Matching, IROS 2003; Magnusson, The Three-Dimensional Normal-Distributions Transform, PhD Thesis 2009
- **Method README**: `papers/ndt/README.md`
- **Reported dataset**: Custom 2D/3D datasets
- **Reported metric**: qualitative
- **Repo scope**: Compact baseline. The repo implements an NDT-style registration baseline against voxel Gaussian models with a compact optimizer.
- **Current claim**: Concept-only comparison. This path is benchmark-useful, but it is not a direct reproduction of the original NDT papers or of a single modern NDT codebase.
- **Numeric comparison**: Not direct. The original papers predate KITTI-style public odometry evaluation, and modern NDT implementations use materially different code paths.
- **Main blocker**: There is no agreed modern NDT reference setup wired into the repo as the reproduction target.
- **Next step**: Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required.
- **Notes**: Original NDT papers predate KITTI. Modern NDT implementations (e.g., Autoware) use NDT on KITTI but those numbers are from different codebases.

## KISS-ICP

- **Claim level**: `indicative`
- **Paper**: Vizzo et al., KISS-ICP: In Defense of Point-to-Point ICP, RAL 2023
- **Method README**: `papers/kiss_icp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Compact baseline. The repo keeps a small KISS-ICP-style local-map pipeline that preserves the main idea while simplifying the full upstream engineering stack.
- **Current claim**: Benchmark-comparable only. The current implementation is close enough for same-contract comparisons, but it should not be presented as a faithful rerun of the upstream KISS-ICP project.
- **Numeric comparison**: Partial only (one sequence). The paper reports per-sequence KITTI RTE only for seq 00 (0.51 %) and 04 (0.36 %), plus the 00-10 average (0.50 %). With the current default correspondence search the best re-run seq 00 variant reaches 0.954 % official KITTI RTE (1.87x); --kiss-legacy-27-neighborhood (the upstream search) reproduces the pre-2026-08 aggregates. Values are the official KITTI RTE of the best variant among sweeps re-run with the current code on 2026-10-04 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation.
- **Next step**: Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation.
- **Notes**: KISS-ICP reports the KITTI average relative translational error (Table II: 0.50 % train / 0.61 % test) and per-sequence values only in the Table VI threshold ablation. The paper does not state hardware or a KITTI frame rate.

## CT-ICP

- **Claim level**: `approximately_reproduced`
- **Paper**: Dellenbach et al., CT-ICP: Real-time Elastic LiDAR Odometry with Loop Closure, ICRA 2022
- **Method README**: `papers/ct_icp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper-oriented core reimplementation. The repo implements the continuous-time two-pose-per-scan core, interpolation, and point-to-plane optimization described by CT-ICP.
- **Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08. Official KITTI RTE: 1.66/3.47/1.58/1.07/1.75 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.49/0.52/0.25/0.31/0.81 % (Table I, KITTI-corrected): 2.16-6.68x, geometric mean 4.03x. Parameter tuning has not closed the gap. Values are the official KITTI RTE of the best variant among all ~313 CT-ICP KITTI sweep variants re-run with the current code on 2026-10-05 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Numeric comparison**: Same metric, ~4.0x ratio (parameter-tuning floor). Official KITTI RTE, repo vs paper: seq 00 1.664 vs 0.49 (3.40x), seq 02 3.472 vs 0.52 (6.68x), seq 05 1.577 vs 0.25 (6.31x), seq 07 1.067 vs 0.31 (3.44x), seq 08 1.749 vs 0.81 (2.16x). Values are the official KITTI RTE of the best variant among all ~313 CT-ICP KITTI sweep variants re-run with the current code on 2026-10-05 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: Parameter-only ceiling at ~4.0x best-of-sweep ratio (official KITTI RTE) on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations.
- **Next step**: Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.0x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage.
- **Notes**: Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6). The --ct-icp-gt-seed toggle remains available for fair-prior dogfooding-style cross-method comparison; GT-seeded rows are excluded from Table 6.

## CT-LIO

- **Claim level**: `ported`
- **Paper**: Zheng et al., Continuous-Time Fixed-Lag Smoothing for LiDAR-Inertial-Odometry (CT-LIO), IROS 2023 / derived from CT-ICP + IMU integration
- **Method README**: `papers/ct_lio/README.md`
- **Reported dataset**: KITTI raw, NTU VIRAL, custom
- **Reported metric**: ATE [m]
- **Repo scope**: Custom integration. This repo combines CT-ICP-style continuous-time interpolation with IMU preintegration as a compact local prototype built from shared components.
- **Current claim**: Not comparable to a single paper. The code path is intentionally a custom integration, so its numbers should not be presented as reproduction of one upstream CT-LIO or CLINS paper result.
- **Numeric comparison**: Reference-based only. The current results are useful inside the repo benchmark, but they are not direct paper-result comparisons.
- **Main blocker**: There is no single paper-faithful target implementation and protocol behind this path.
- **Next step**: Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol.
- **Notes**: CT-LIO in this repo is a custom integration. Not directly comparable to any single published paper. Reference-based comparison only.

## MULLS

- **Claim level**: `indicative`
- **Paper**: Pan et al., MULLS: Versatile LiDAR SLAM via Multi-metric Linear Least Square, ICRA 2021
- **Method README**: `papers/mulls/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Derived variant. Multi-metric scan-to-map alignment with edge, plane, and point residuals inside the shared benchmark harness.
- **Current claim**: KITTI Odometry 00/02/05/07/08. Official KITTI RTE of the dense variant: 3.79/4.11/2.68/2.63/4.27 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.51/0.55/0.28/0.29/0.80 % (MULLS-LO(mc)): 5.34-9.56x, geometric mean 7.62x. The repo variant is a derived multi-metric scan-to-map registration, not the full MULLS pipeline. Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Numeric comparison**: Same metric, ~7.6x ratio. Official KITTI RTE, repo vs paper: seq 00 3.793 vs 0.51 (7.44x), seq 02 4.107 vs 0.55 (7.47x), seq 05 2.678 vs 0.28 (9.56x), seq 07 2.627 vs 0.29 (9.06x), seq 08 4.271 vs 0.80 (5.34x). Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The derived variant implements only the multi-metric registration core; the gap to MULLS-LO is ~7.6x on the official KITTI RTE.
- **Next step**: Compare step-by-step with the upstream MULLS implementation to locate the missing components.
- **Notes**: Paper Table II reports 0.08 s/frame for MULLS-LO(mc). Transfer runs on 00/02/05/08 ran four sequences concurrently on one host, so their FPS is depressed.

## SUMA

- **Claim level**: `indicative`
- **Paper**: Behley and Stachniss, Efficient Surfel-Based SLAM using 3D Laser Range Data in Urban Environments, RSS 2018
- **Method README**: `papers/suma/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation. Dense surfel odometry from range-image maps inside the shared benchmark harness.
- **Current claim**: KITTI Odometry 00/02/05/07/08. Official KITTI RTE of the dense profile: 1.60/1.78/1.28/1.10/2.03 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.7/1.1/0.5/0.4/1.0 % (Frame-to-Model, no loop closure): 1.62-2.74x, geometric mean 2.21x. Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Numeric comparison**: Same metric, ~2.2x ratio. Official KITTI RTE, repo vs paper: seq 00 1.602 vs 0.7 (2.29x), seq 02 1.785 vs 1.1 (1.62x), seq 05 1.279 vs 0.5 (2.56x), seq 07 1.095 vs 0.4 (2.74x), seq 08 2.035 vs 1.0 (2.03x). The paper rounds to one decimal. Seq 07 is the best of the KITTI 07 sweep; 00/02/05/08 re-use that choice unchanged (held-out transfer manifests *_kitti_seq_NN_full_transfer_matrix.json). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The CPU reimplementation reaches ~2.2x the paper's Frame-to-Model RTE on the official metric.
- **Next step**: Compare surfel stability and map fusion against the upstream SuMa implementation.
- **Notes**: Paper values are rounded to one decimal, so ratios carry up to ~25 % rounding uncertainty on short-error sequences.

## ALOAM

- **Claim level**: `indicative`
- **Paper**: Zhang and Singh, LOAM: Lidar Odometry and Mapping in Real-time, RSS 2014 (A-LOAM is the HKUST re-implementation)
- **Method README**: `papers/aloam/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation. Curvature features with a three-stage odometry-to-map LOAM pipeline (A-LOAM structure).
- **Current claim**: KITTI Odometry 00/02/05/07/08. Official KITTI RTE: 0.86/1.01/0.54/0.68/0.98 % on KITTI Odometry 00/02/05/07/08 versus LOAM's 0.78/0.92/0.57/0.63/1.12 % (secondary citation): 0.87-1.10x, geometric mean 1.01x. Values are the official KITTI RTE of the best variant among sweeps re-run with the current code on 2026-10-04 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Numeric comparison**: Same metric, secondary-source paper values. Official KITTI RTE, repo vs LOAM: seq 00 0.855 vs 0.78 (1.10x), seq 02 1.012 vs 0.92 (1.10x), seq 05 0.535 vs 0.57 (0.94x), seq 07 0.681 vs 0.63 (1.08x), seq 08 0.975 vs 1.12 (0.87x). Values are the official KITTI RTE of the best variant among sweeps re-run with the current code on 2026-10-04 (selected on the evaluated sequence, so optimistic); see docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The LOAM paper itself does not report per-sequence KITTI RTE; the values are LOAM's KITTI numbers as cited by later papers.
- **Next step**: Compare against the official A-LOAM implementation on the same full sequences.
- **Notes**: Secondary-source values: LOAM (RSS 2014) does not tabulate per-sequence KITTI RTE. The repo implementation follows A-LOAM, not the original LOAM code.

## LF-GICP

- **Claim level**: `approximately_reproduced`
- **Paper**: Im, LF-GICP: Parameter-Free Degeneracy Handling for LiDAR Odometry via a Voxel-Normal Localizability Field, arXiv 2026
- **Method README**: `papers/lf_gicp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation (no author code). GICP scan-to-map backend with the voxel-normal localizability field, median/hysteresis gate, and soft Fisher-information weighting, following the paper's Table SI constants.
- **Current claim**: KITTI Odometry 00/02/05/07/08. Official KITTI RTE of the paper-default configuration: 0.66/1.08/0.51/0.41/1.01 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.692/1.084/0.577/0.496/0.878 % (Table I): 0.83-1.14x, geometric mean 0.96x. The gate fires on 0-19 % of frames (paper: <= 8 % on open roads). Repo values are the paper_default variant only (the vanilla-backend ablation is excluded); see docs/assets/paper/paper_ratio_table.csv (Table 6) and papers/lf_gicp/README.md for the planarity/lambda0 calibration note.
- **Numeric comparison**: Same metric, ~0.96x ratio (single configuration). Official KITTI RTE, repo vs paper: seq 00 0.664 vs 0.692 (0.96x), seq 02 1.075 vs 1.084 (0.99x), seq 05 0.512 vs 0.577 (0.89x), seq 07 0.411 vs 0.496 (0.83x), seq 08 1.005 vs 0.878 (1.14x). Disabling the gate gives 0.734/1.074/0.512/0.483/0.918 %: the gate helps on 00/07, is neutral on 02/05, and hurts on 08, where it fires on 19 % of frames. Repo values are the paper_default variant only (the vanilla-backend ablation is excluded); see docs/assets/paper/paper_ratio_table.csv (Table 6) and papers/lf_gicp/README.md for the planarity/lambda0 calibration note.
- **Main blocker**: The lambda0 scale differs from the paper (planarity definition is ambiguous) and GEODE tunnels for gate calibration are not available.
- **Next step**: Obtain GEODE tunnel data to check the gate on genuine absence and recalibrate tau2 with the paper's rule.
- **Notes**: Paper average 0.865 % over 00-10; same raw-scan protocol as this repository's KITTI data.
