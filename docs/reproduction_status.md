# Reproduction Status

_Generated at 2026-04-15T13:56:41+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

This page records what the repository can currently claim about reproducing original-paper results.
The tracked subset below is intentionally conservative: if the implementation, metric, dataset, or protocol diverges, the repo should say so explicitly.

## Tracked Families

| Method | Repo Scope | Current Claim | Numeric Comparison | Main Blocker | Next Step |
|--------|------------|---------------|--------------------|--------------|-----------|
| LiTAMIN2 | Paper-oriented front-end reimplementation | Partial reproduction path | Indirect only | No repository-stored full KITTI odometry sweep with the paper's reported metric is exported today. | Add full KITTI Odometry runs for the paper-profile variants and export RPE translation [%] alongside ATE. |
| GICP | Compact shared implementation | Concept-only comparison | Not direct | There is no single paper benchmark and hardware protocol to reproduce against. | Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence. |
| NDT | Compact baseline | Concept-only comparison | Not direct | There is no agreed modern NDT reference setup wired into the repo as the reproduction target. | Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required. |
| KISS-ICP | Compact baseline | Benchmark-comparable only | Partial only | The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation. | Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation. |
| CT-ICP | Paper-oriented core reimplementation | Core close, evaluation still indirect | Indirect only | The repo does not yet export the paper's full KITTI evaluation protocol and metric set. | Add full KITTI Odometry CT-ICP reruns with the paper-reported metric and publish side-by-side results. |
| CT-LIO | Custom integration | Not comparable to a single paper | Reference-based only | There is no single paper-faithful target implementation and protocol behind this path. | Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol. |

## LiTAMIN2

- **Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021
- **Method README**: `papers/litamin2/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: RPE translation [%]
- **Repo scope**: Paper-oriented front-end reimplementation. The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
- **Current claim**: Partial reproduction path. Paper-like cost terms and profiles exist, but the stored evidence is still short-window ATE under the shared evaluator rather than the paper's full-sequence KITTI RPE study.
- **Numeric comparison**: Indirect only. Paper numbers and repo numbers differ in metric, evaluation window, and hardware, so the comparison is directional rather than direct.
- **Main blocker**: No repository-stored full KITTI odometry sweep with the paper's reported metric is exported today.
- **Next step**: Add full KITTI Odometry runs for the paper-profile variants and export RPE translation [%] alongside ATE.
- **Notes**: Paper reports RPE (relative pose error), not ATE. Direct comparison with repo ATE requires caution.

## GICP

- **Paper**: Segal et al., Generalized-ICP, RSS 2009
- **Method README**: `papers/gicp/README.md`
- **Reported dataset**: Custom indoor/outdoor datasets
- **Reported metric**: qualitative alignment quality
- **Repo scope**: Compact shared implementation. The repo keeps a self-contained GICP-style registration loop with local covariance modeling under the stable evaluator.
- **Current claim**: Concept-only comparison. The implementation is useful as benchmark evidence for the GICP idea, but not as a paper-result reproduction of a specific historical codebase.
- **Numeric comparison**: Not direct. The original paper predates the repo's modern public benchmark suite and does not provide a standardized numeric target that can be matched one-to-one here.
- **Main blocker**: There is no single paper benchmark and hardware protocol to reproduce against.
- **Next step**: Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence.
- **Notes**: Original GICP paper predates KITTI. No standardized numeric benchmark reported. Comparison is with the concept, not specific numbers.

## NDT

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

- **Paper**: Dellenbach et al., CT-ICP: Real-time Elastic LiDAR Odometry with Loop Closure, ICRA 2022
- **Method README**: `papers/ct_icp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: RPE translation [%]
- **Repo scope**: Paper-oriented core reimplementation. The repo implements the continuous-time two-pose-per-scan core, interpolation, and point-to-plane optimization described by CT-ICP.
- **Current claim**: Core close, evaluation still indirect. The underlying formulation is near the paper core, but the exported evidence is still short-window ATE under the shared evaluator instead of the paper's full KITTI RPE study.
- **Numeric comparison**: Indirect only. Metric, sequence protocol, and hardware still differ from the published study, so current numbers should not be treated as direct CT-ICP reproduction results.
- **Main blocker**: The repo does not yet export the paper's full KITTI evaluation protocol and metric set.
- **Next step**: Add full KITTI Odometry CT-ICP reruns with the paper-reported metric and publish side-by-side results.
- **Notes**: CT-ICP reports RPE, not ATE. Continuous-time formulation. Our repo measures ATE on short windows.

## CT-LIO

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
