# Reproduction Status

_Generated at 2026-05-17T20:33:36+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

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
| LiTAMIN2 | `indicative` | Paper-oriented front-end reimplementation | Partial reproduction path | Indirect only | No repository-stored full KITTI Odometry (00-10) sweep with the paper's reported metric is exported today. The Raw 0009 full pure-odometry probe (4.85 % RPE) is a calibration, not the paper's actual evaluation set. | Acquire KITTI Odometry sequences 00-10 and run the paper-profile variants with --no-gt-seed (velocity-model) to produce direct paper-comparable RPE numbers. Investigate why the 3 m paper profile diverges on Raw 0009 without GT seeding - likely needs a richer local map or finer voxel for pure-odometry mode. |
| GICP | `ported` | Compact shared implementation | Concept-only comparison | Not direct | There is no single paper benchmark and hardware protocol to reproduce against. | Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence. |
| NDT | `ported` | Compact baseline | Concept-only comparison | Not direct | There is no agreed modern NDT reference setup wired into the repo as the reproduction target. | Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required. |
| KISS-ICP | `indicative` | Compact baseline | Benchmark-comparable only | Partial only | The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation. | Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation. |
| CT-ICP | `indicative` | Paper-oriented core reimplementation | Core close, evaluation still indirect | Indirect only | The repo does not yet export the paper's full KITTI Odometry (00-10) evaluation, only Raw 0009 windowed and full runs. Cross-method comparison is now methodology-honest with the new --ct-icp-gt-seed flag, but paper-faithful sequence/metric coverage is still missing. | Acquire KITTI Odometry sequences 00-10 and run CT-ICP both pure-odometry (default) and --ct-icp-gt-seed for direct paper-comparable RPE side-by-side, plus publish a methodology-aware comparison against LiTAMIN2 on the same sequences. |
| CT-LIO | `ported` | Custom integration | Not comparable to a single paper | Reference-based only | There is no single paper-faithful target implementation and protocol behind this path. | Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol. |

## LiTAMIN2

- **Claim level**: `indicative`
- **Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021
- **Method README**: `papers/litamin2/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: RPE translation [%]
- **Repo scope**: Paper-oriented front-end reimplementation. The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
- **Current claim**: Partial reproduction path. Paper-like cost terms and profiles exist, but the stored evidence is still short-window ATE under the shared evaluator rather than the paper's full-sequence KITTI RPE study.
- **Numeric comparison**: Indirect only. Paper numbers and repo numbers differ in metric, evaluation window, and hardware, so the comparison is directional rather than direct.
- **Main blocker**: No repository-stored full KITTI Odometry (00-10) sweep with the paper's reported metric is exported today. The Raw 0009 full pure-odometry probe (4.85 % RPE) is a calibration, not the paper's actual evaluation set.
- **Next step**: Acquire KITTI Odometry sequences 00-10 and run the paper-profile variants with --no-gt-seed (velocity-model) to produce direct paper-comparable RPE numbers. Investigate why the 3 m paper profile diverges on Raw 0009 without GT seeding - likely needs a richer local map or finer voxel for pure-odometry mode.
- **Notes**: Paper reports RPE on KITTI Odometry 00-10 (0.40-1.42 %). Repo-side LiTAMIN2 RPE on KITTI Raw 0009 short window (0.74 %), HDL-400 reference (1.31 %), and MulRan parkinglot full (1.26 %) all sit inside that paper range but use Policy A GT-seeded scan-to-map initialization (T_init_guess = gt[i] for every frame), so they measure local scan-registration quality on top of a GT prior, not pure odometry drift. As of 2026-05-18 the --no-gt-seed code path uses a velocity-model prior (T_pred = T_prev * T_prev_prev^-1 * T_prev) instead of the previous stagnating previous-pose init; under this fixed pure-odometry path, LiTAMIN2 fast profile on KITTI Raw 0009 full reports ATE 7.45 m / RPE 4.85 %, which is honest pure-odometry behavior but still 3-5x worse than the paper's KITTI Odometry numbers (the dogfooding profile is throughput-tuned and Raw 0009 is not in the paper's evaluated set). The 3 m paper profile diverges (ATE 167 m / RPE 73 %) when run without GT seeding because that voxel resolution is too coarse for Raw 0009 without a tight prior. Direct paper-reproduction still requires full KITTI Odometry sequences (00-10) plus a paper-faithful profile - the velocity-model probe is a stepping stone, not the target.

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
- **Reported metric**: ATE [m] (mean over full sequence)
- **Repo scope**: Compact baseline. The repo keeps a small KISS-ICP-style local-map pipeline that preserves the main idea while simplifying the full upstream engineering stack.
- **Current claim**: Benchmark-comparable only. The current implementation is close enough for same-contract comparisons, but it should not be presented as a faithful rerun of the upstream KISS-ICP project.
- **Numeric comparison**: Partial only. The paper and repo are both KITTI-oriented and ATE-like, but the repo still uses compact internals, shorter windows, and different hardware.
- **Main blocker**: The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation.
- **Next step**: Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation.
- **Notes**: KISS-ICP reports near real-time performance (~100 FPS) on KITTI. ATE values are for full sequences; our 108-frame windows will differ.

## CT-ICP

- **Claim level**: `indicative`
- **Paper**: Dellenbach et al., CT-ICP: Real-time Elastic LiDAR Odometry with Loop Closure, ICRA 2022
- **Method README**: `papers/ct_icp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: RPE translation [%]
- **Repo scope**: Paper-oriented core reimplementation. The repo implements the continuous-time two-pose-per-scan core, interpolation, and point-to-plane optimization described by CT-ICP.
- **Current claim**: Core close, evaluation still indirect. The underlying formulation is near the paper core, but the exported evidence is still short-window ATE under the shared evaluator instead of the paper's full KITTI RPE study.
- **Numeric comparison**: Indirect only. Metric, sequence protocol, and hardware still differ from the published study, so current numbers should not be treated as direct CT-ICP reproduction results.
- **Main blocker**: The repo does not yet export the paper's full KITTI Odometry (00-10) evaluation, only Raw 0009 windowed and full runs. Cross-method comparison is now methodology-honest with the new --ct-icp-gt-seed flag, but paper-faithful sequence/metric coverage is still missing.
- **Next step**: Acquire KITTI Odometry sequences 00-10 and run CT-ICP both pure-odometry (default) and --ct-icp-gt-seed for direct paper-comparable RPE side-by-side, plus publish a methodology-aware comparison against LiTAMIN2 on the same sequences.
- **Notes**: Paper reports RPE on KITTI Odometry 00-10 (0.30-0.75 %). Repo-side CT-ICP RPE on KITTI Raw 0009 short window (2.20 %) sits above that range. As of 2026-05-18 runCTICP accepts an optional --ct-icp-gt-seed flag that initializes each scan's TrajectoryFrame with GT begin/end poses, symmetric to how LiTAMIN2 uses gt[i] in its Policy A path. Cross-method sweep on long sequences with the new flag (build 2026-05-18): CT-ICP default (pure odometry) ATE 4.67 m / RPE 5.31 % vs CT-ICP --ct-icp-gt-seed ATE 2.68 m / RPE 4.00 % on KITTI Raw 0009 full; CT-ICP default 76.55 m / 104.49 % vs CT-ICP --ct-icp-gt-seed 10.00 m / 10.17 % on MulRan parkinglot full. The MulRan numbers definitively show the earlier LiTAMIN2-vs-CT-ICP gap was dominated by Policy A vs Policy B asymmetry, not algorithmic merit. A ~14x residual remains between gt-seeded CT-ICP and Policy A LiTAMIN2 on MulRan because Policy A also has a refinement-acceptance gate that falls back to gt[i] on rejection - a safety net CT-ICP does not have. Direct paper-reproduction still requires full KITTI Odometry runs; the repo's cross-method comparisons are now methodology-honest but the dogfooding profile is throughput-tuned, not paper-faithful.

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
