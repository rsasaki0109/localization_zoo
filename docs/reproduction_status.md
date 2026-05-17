# Reproduction Status

_Generated at 2026-05-17T21:41:59+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

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
| LiTAMIN2 | `approximately_reproduced` | Paper-oriented front-end reimplementation | Approximate reproduction across KITTI Odometry 00/02/05/07/08 | Same metric, ~4x ratio | Geometric-mean ~4x gap between repo fast-profile pure-odometry RPE and paper-reported RPE on KITTI Odometry 00/02/05/07/08, driven by the throughput-tuned dogfooding profile (smaller voxel iteration budget, smaller local map, fewer iterations than the paper's coarse-to-fine 3 m schedule). The paper profile (`--litamin2-paper-profile`) still diverges in pure-odometry mode on long sequences because the dogfooding implementation lacks the multi-scale schedule the paper uses with the 3 m voxel. | Add a coarse-to-fine voxel schedule to the LiTAMIN2 paper profile (e.g. 3 m -> 1.5 m -> 0.75 m within each scan), increase max_iterations, and re-run KITTI Odometry 00/02/05/07/08 to close the remaining ~4x gap. Also add sequences 01/03/04/06/09/10 for full paper coverage. |
| GICP | `ported` | Compact shared implementation | Concept-only comparison | Not direct | There is no single paper benchmark and hardware protocol to reproduce against. | Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence. |
| NDT | `ported` | Compact baseline | Concept-only comparison | Not direct | There is no agreed modern NDT reference setup wired into the repo as the reproduction target. | Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required. |
| KISS-ICP | `indicative` | Compact baseline | Benchmark-comparable only | Partial only | The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation. | Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation. |
| CT-ICP | `approximately_reproduced` | Paper-oriented core reimplementation | Approximate reproduction across KITTI Odometry 00/02/05/07/08 | Same metric, ~5x ratio | Uniform ~5x gap between repo CT-ICP default pure-odometry RPE and paper-reported RPE on KITTI Odometry 00/02/05/07/08. Likely driven by the repo's throughput-tuned ceres_max_iterations and max_iterations budgets relative to the paper's full optimization, plus the smaller max_frames_in_map history vs the paper's local map. | Increase CT-ICP optimization budget (ceres_max_iterations from 2 -> 6, max_iterations from 6 -> 12, max_frames_in_map from 10 -> 30) and re-run KITTI Odometry 00/02/05/07/08 to close the remaining ~5x gap. Also add sequences 01/03/04/06/09/10 for full paper coverage. |
| CT-LIO | `ported` | Custom integration | Not comparable to a single paper | Reference-based only | There is no single paper-faithful target implementation and protocol behind this path. | Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol. |

## LiTAMIN2

- **Claim level**: `approximately_reproduced`
- **Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021
- **Method README**: `papers/litamin2/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: RPE translation [%]
- **Repo scope**: Paper-oriented front-end reimplementation. The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
- **Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08. After the 2026-05-18 velocity-model + Tr-frame-correction work, LiTAMIN2 fast-profile pure odometry now runs on the same KITTI Odometry 00-10 sequences and same RPE [%] metric as the paper, with RPE numbers in the 0.72-7.25 % range against paper's 0.40-1.42 % (1.7-7.6x ratio, geometric mean ~4x).
- **Numeric comparison**: Same metric, ~4x ratio. Five paper-evaluated sequences (00/02/05/07/08) compared directly under the same metric. Shorter sequences (07: 1.7x, 05: 2.4x) approach paper-level, longer ones (08: 7.6x, 02: 6.1x) drift more under the throughput-tuned dogfooding profile.
- **Main blocker**: Geometric-mean ~4x gap between repo fast-profile pure-odometry RPE and paper-reported RPE on KITTI Odometry 00/02/05/07/08, driven by the throughput-tuned dogfooding profile (smaller voxel iteration budget, smaller local map, fewer iterations than the paper's coarse-to-fine 3 m schedule). The paper profile (`--litamin2-paper-profile`) still diverges in pure-odometry mode on long sequences because the dogfooding implementation lacks the multi-scale schedule the paper uses with the 3 m voxel.
- **Next step**: Add a coarse-to-fine voxel schedule to the LiTAMIN2 paper profile (e.g. 3 m -> 1.5 m -> 0.75 m within each scan), increase max_iterations, and re-run KITTI Odometry 00/02/05/07/08 to close the remaining ~4x gap. Also add sequences 01/03/04/06/09/10 for full paper coverage.
- **Notes**: Paper reports RPE on KITTI Odometry 00-10 (0.40-1.42 %). After the 2026-05-18 velocity-model fix and the KITTI Tr-frame correction (kitti_poses_to_gt_csv.py now applies T_world_lidar = poses[i] * Tr from sequences/<seq>/calib.txt), LiTAMIN2 fast-profile pure-odometry on KITTI Odometry full sequences is directly paper-comparable: seq 00: 3.08 % (paper 0.65, 4.7x); seq 02: 5.08 % (0.83, 6.1x); seq 05: 0.97 % (0.40, 2.4x); seq 07: 0.72 % (0.42, 1.7x); seq 08: 7.25 % (0.96, 7.6x). Geometric mean ratio ~4x. The ratio is best on short sequences and grows with sequence length, suggesting the dogfooding profile's throughput-vs-accuracy trade-off (smaller iteration budget than paper) costs accuracy that accumulates over distance. The paper-profile 3 m voxel still diverges in pure-odometry mode because the dogfooding implementation lacks the coarse-to-fine voxel schedule the paper uses with that resolution. Policy A GT-seeded numbers (0.74 % on Raw 0009 short window, 1.26 % on MulRan parkinglot full) remain in the file but are explicitly non-comparable to paper - they measure local scan-registration quality on top of a per-frame GT prior, not pure odometry drift.

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

- **Claim level**: `approximately_reproduced`
- **Paper**: Dellenbach et al., CT-ICP: Real-time Elastic LiDAR Odometry with Loop Closure, ICRA 2022
- **Method README**: `papers/ct_icp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: RPE translation [%]
- **Repo scope**: Paper-oriented core reimplementation. The repo implements the continuous-time two-pose-per-scan core, interpolation, and point-to-plane optimization described by CT-ICP.
- **Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08. After the 2026-05-18 Tr-frame-correction work, CT-ICP default pure odometry now runs on the same KITTI Odometry sequences and same RPE [%] metric as the paper, with RPE numbers in the 1.60-3.21 % range against paper's 0.30-0.75 % (3.5-9.0x ratio, geometric mean ~5x).
- **Numeric comparison**: Same metric, ~5x ratio. Five paper-evaluated sequences (00/02/05/07/08) compared directly under the same metric. Gap is larger and more uniform than LiTAMIN2's because CT-ICP's continuous-time profile and parameters in the repo are paper-style but the optimization budget (max_iterations, ceres_max_iterations) is throughput-tuned.
- **Main blocker**: Uniform ~5x gap between repo CT-ICP default pure-odometry RPE and paper-reported RPE on KITTI Odometry 00/02/05/07/08. Likely driven by the repo's throughput-tuned ceres_max_iterations and max_iterations budgets relative to the paper's full optimization, plus the smaller max_frames_in_map history vs the paper's local map.
- **Next step**: Increase CT-ICP optimization budget (ceres_max_iterations from 2 -> 6, max_iterations from 6 -> 12, max_frames_in_map from 10 -> 30) and re-run KITTI Odometry 00/02/05/07/08 to close the remaining ~5x gap. Also add sequences 01/03/04/06/09/10 for full paper coverage.
- **Notes**: Paper reports RPE on KITTI Odometry 00-10 (0.30-0.75 %). After the 2026-05-18 KITTI Tr-frame correction, CT-ICP default pure-odometry on KITTI Odometry full sequences is directly paper-comparable: seq 00: 2.76 % (paper 0.52, 5.3x); seq 02: 3.21 % (0.58, 5.5x); seq 05: 1.60 % (0.31, 5.2x); seq 07: 2.70 % (0.30, 9.0x); seq 08: 2.63 % (0.75, 3.5x). Geometric mean ratio ~5x. The gap is more uniform than LiTAMIN2's because CT-ICP's continuous-time profile in the repo is paper-style but the optimization budget (ceres_max_iterations=2, max_iterations=6) is throughput-tuned vs the paper's full schedule. The --ct-icp-gt-seed toggle remains available for fair-prior dogfooding-style cross-method comparison against LiTAMIN2's default Policy A (e.g. MulRan parkinglot full: 76 m default -> 10 m with gt-seed) but is explicitly non-comparable to paper since the paper measures pure odometry.

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
