# Reproduction Status

_Generated at 2026-10-03T03:29:22+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

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
| CT-ICP | `approximately_reproduced` | Paper-oriented core reimplementation | Approximate reproduction across KITTI Odometry 00/02/05/07/08 | Same metric, ~4.2x ratio (parameter-tuning floor) | Parameter-only ceiling at ~4.2x best-of-sweep ratio (official KITTI RTE) on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations. | Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.2x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage. |
| CT-LIO | `ported` | Custom integration | Not comparable to a single paper | Reference-based only | There is no single paper-faithful target implementation and protocol behind this path. | Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol. |

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
- **Numeric comparison**: Partial only (one sequence). The paper reports per-sequence KITTI RTE only for seq 00 (0.51 %) and 04 (0.36 %), plus the 00-10 average (0.50 %). The compact pipeline's best swept variant on full seq 00 reaches 1.211 % official KITTI RTE (2.37x); its stored 100 m RPE (0.857 %) no longer reproduces with the current code (1.069 %). Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6).
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
- **Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08. Official KITTI RTE of the best swept variant per sequence: 1.91/3.70/1.58/1.10/1.77 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.49/0.52/0.25/0.31/0.81 % (Table I, KITTI-corrected): 2.19-7.12x, geometric mean 4.23x. Parameter tuning has not closed the gap. Re-runs with the current code differ from the stored 100 m RPE by up to 0.11 points, so these aggregates predate later CT-ICP code changes.
- **Numeric comparison**: Same metric, ~4.2x ratio (parameter-tuning floor). Official KITTI RTE, repo vs paper: seq 00 1.908 vs 0.49 (3.89x), seq 02 3.701 vs 0.52 (7.12x), seq 05 1.577 vs 0.25 (6.31x), seq 07 1.102 vs 0.31 (3.55x), seq 08 1.772 vs 0.81 (2.19x). Paper values were re-verified against the arXiv PDF on 2026-10-03. Repo values are the official KITTI RTE (100-800 m) from re-running, with the current code, the variant with the best 100 m RPE in the full non-GT-seeded sweep for that sequence (experiments/results/kitti_rte_rescore.json); selection on the evaluated sequence makes them optimistic. See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: Parameter-only ceiling at ~4.2x best-of-sweep ratio (official KITTI RTE) on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations.
- **Next step**: Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.2x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage.
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
