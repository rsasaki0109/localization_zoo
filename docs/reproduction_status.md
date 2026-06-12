# Reproduction Status

_Generated at 2026-06-12T23:42:14+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

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
| LiTAMIN2 | `approximately_reproduced` | Paper-oriented front-end reimplementation | Approximate reproduction across KITTI Odometry 00/02/05/07/08 (paper range) | Same metric, ~1.35x ratio, paper range | Stable ~1.35x ratio remaining vs paper after gap-closing tune (voxel 1.0 + max_iter 12). Correspondence pruning, covariance floor, and covariance-shape-gradient controls are now sweepable. Radius-1 plus 1.5 m gate improves seq02/05 full ATE/RPE, but seq07 regresses and seq08 trades lower ATE for worse RPE. A weaker covariance floor (1e-4) improves speed and some sequences but leaves seq02/05/07/08 geometric-mean RPE flat (0.806 -> 0.807). The raw opt-in covariance gradient is not stable at full sequence scale: geometric-mean RPE worsens to 1.451 %. A damped covariance-gradient variant (0.1x + line search) avoids that collapse and reaches 0.806 % geometric-mean RPE, but it trades seq02 ATE from 50.622 m to 81.961 m. Remaining gaps likely involve adaptive correspondence/covariance behavior, refresh_interval, stronger step control, and the missing coarse-to-fine schedule. Also sequences 01/03/04/06/09/10 not yet exported. | Export and run KITTI Odometry 01/03/04/06/09/10 for full paper coverage. Investigate adaptive correspondence gating, damped covariance-shape derivatives, or the missing coarse-to-fine schedule rather than promoting one static radius/gate/floor/gradient setting; alternative: side-by-side with the upstream LiTAMIN2 reference implementation to identify which internal step introduces the ceiling. |
| GICP | `ported` | Compact shared implementation | Concept-only comparison | Not direct | There is no single paper benchmark and hardware protocol to reproduce against. | Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence. |
| NDT | `ported` | Compact baseline | Concept-only comparison | Not direct | There is no agreed modern NDT reference setup wired into the repo as the reproduction target. | Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required. |
| KISS-ICP | `indicative` | Compact baseline | Benchmark-comparable only | Partial only | The public aggregates are windowed runs of the compact pipeline, not full-sequence reruns of the reference implementation. | Run full KITTI sequences and publish an explicit deviation sheet against the upstream KISS-ICP implementation. |
| CT-ICP | `approximately_reproduced` | Paper-oriented core reimplementation | Approximate reproduction across KITTI Odometry 00/02/05/07/08 | Same metric, ~4.5x ratio (parameter-tuning floor) | Parameter-only ceiling at ~4.5x ratio confirmed on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations. | Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.5x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage. |
| CT-LIO | `ported` | Custom integration | Not comparable to a single paper | Reference-based only | There is no single paper-faithful target implementation and protocol behind this path. | Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol. |

## LiTAMIN2

- **Claim level**: `approximately_reproduced`
- **Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021
- **Method README**: `papers/litamin2/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: RPE translation [%]
- **Repo scope**: Paper-oriented front-end reimplementation. The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
- **Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08 (paper range). After the 2026-05-18 velocity-model + Tr-frame-correction work and gap-closing tuning (--litamin2-voxel-resolution 1.0 + --litamin2-max-iterations 12 + --no-gt-seed), LiTAMIN2 pure odometry now runs in the 0.54-1.30 % RPE range across KITTI Odometry 00/02/05/07/08 - inside the paper's reported 0.40-1.42 % range on every measured sequence. Per-sequence ratio is 1.2-1.6x paper (geometric mean 1.35x).
- **Numeric comparison**: Same metric, ~1.35x ratio, paper range. Five paper-evaluated sequences (00/02/05/07/08): repo 0.54/0.92/0.63/0.92/0.98/1.30 % vs paper 0.42/0.65/0.40/0.42/0.83/0.96 %. Repo numbers sit inside the paper's reported per-sequence range overall but each sequence has the repo result a uniform ~1.35x above the paper value - a stable accuracy ceiling tied to the specific dogfooding implementation, not a drift integration effect.
- **Main blocker**: Stable ~1.35x ratio remaining vs paper after gap-closing tune (voxel 1.0 + max_iter 12). Correspondence pruning, covariance floor, and covariance-shape-gradient controls are now sweepable. Radius-1 plus 1.5 m gate improves seq02/05 full ATE/RPE, but seq07 regresses and seq08 trades lower ATE for worse RPE. A weaker covariance floor (1e-4) improves speed and some sequences but leaves seq02/05/07/08 geometric-mean RPE flat (0.806 -> 0.807). The raw opt-in covariance gradient is not stable at full sequence scale: geometric-mean RPE worsens to 1.451 %. A damped covariance-gradient variant (0.1x + line search) avoids that collapse and reaches 0.806 % geometric-mean RPE, but it trades seq02 ATE from 50.622 m to 81.961 m. Remaining gaps likely involve adaptive correspondence/covariance behavior, refresh_interval, stronger step control, and the missing coarse-to-fine schedule. Also sequences 01/03/04/06/09/10 not yet exported.
- **Next step**: Export and run KITTI Odometry 01/03/04/06/09/10 for full paper coverage. Investigate adaptive correspondence gating, damped covariance-shape derivatives, or the missing coarse-to-fine schedule rather than promoting one static radius/gate/floor/gradient setting; alternative: side-by-side with the upstream LiTAMIN2 reference implementation to identify which internal step introduces the ceiling.
- **Notes**: Paper reports RPE on KITTI Odometry 00-10 (0.40-1.42 %). After the 2026-05-18 velocity-model fix, KITTI Tr-frame correction (kitti_poses_to_gt_csv.py applies T_world_lidar = poses[i] * Tr from sequences/<seq>/calib.txt), and the gap-closing tune (--litamin2-voxel-resolution 1.0 + --litamin2-max-iterations 12 + --no-gt-seed), repo LiTAMIN2 pure odometry now hits paper-range RPE on every measured sequence: seq 00: 0.92 % (paper 0.65, 1.4x); seq 02: 0.98 % (0.83, 1.2x); seq 05: 0.63 % (0.40, 1.6x); seq 07: 0.54 % (0.42, 1.3x); seq 08: 1.30 % (0.96, 1.3x). Geometric-mean ratio 1.35x. The fast profile without the tune ran at ~4x geometric mean - the voxel resolution lever from 2.0 m to 1.0 m delivered most of the gap closure. A new correspondence sweep knob is promising but selective: on KITTI seq02 200-frame no-GT smoke, radius 1 plus a 1.5 m gate improves ATE 22.522 -> 1.042 m and drift 2.330 -> 0.730 m/100m. On full sequences, the same gate improves seq02 ATE/RPE 50.622 m / 0.975 % -> 44.005 m / 0.936 % and seq05 7.350 m / 0.626 % -> 6.069 m / 0.611 %, but worsens seq07 1.964 m / 0.535 % -> 2.315 m / 0.575 % and seq08 RPE 1.295 % -> 1.311 % despite lower ATE. Across seq02/05/07/08, geometric-mean RPE is 0.806 % baseline vs 0.810 % with the gate. A weaker covariance floor (`--litamin2-min-cov-eigenvalue 1e-4`) is faster and improves seq05, but cross-sequence geometric-mean RPE is also flat at 0.807 %. The opt-in covariance-gradient path (`--litamin2-covariance-gradient`) wires the covariance-shape cost into the rotation update, but full-sequence results regress: seq02/05/07/08 produce RPE 5.679/0.684/0.845/1.351 %, or 1.451 % geometric mean. Adding `--litamin2-covariance-gradient-weight 0.1 --litamin2-line-search` avoids the raw collapse and gives RPE 0.980/0.609/0.552/1.282 %, or 0.806 % geometric mean, but seq02 ATE worsens to 81.961 m. The paper-oriented default remains unchanged. The 3 m paper profile still diverges in pure-odometry mode because the dogfooding implementation lacks the coarse-to-fine voxel schedule the paper uses with that resolution. Policy A GT-seeded numbers (0.74 % on Raw 0009 short window, 1.26 % on MulRan parkinglot full) remain in the file but are explicitly non-comparable to paper - they measure local scan-registration quality on top of a per-frame GT prior, not pure odometry drift.

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
- **Current claim**: Approximate reproduction across KITTI Odometry 00/02/05/07/08. After the 2026-05-18 Tr-frame-correction work and gap-closing tuning (--ct-icp-dense-profile + --ct-icp-ceres-max-iterations 6 + --ct-icp-max-frames-in-map 20), CT-ICP pure odometry runs in the 1.28-3.35 % RPE range across KITTI Odometry 00/02/05/07/08 against paper's 0.30-0.75 %. Geometric-mean ratio 4.58x (was 5.44x with stock defaults). Gap is sticky under parameter tuning - the parameter-only ceiling is ~4.5x.
- **Numeric comparison**: Same metric, ~4.5x ratio (parameter-tuning floor). Five paper-evaluated sequences (00/02/05/07/08): repo 2.20/3.35/1.28/2.14/2.10 % vs paper 0.52/0.58/0.31/0.30/0.75 %. Aggressive voxel tuning (0.6-1.0 m) wins on individual sequences but generalizes worse (seq 02 diverges to 5.0 % with voxel 1.0). Closing more than ~4.5x likely needs algorithmic changes to the continuous-time optimization, not more parameters.
- **Main blocker**: Parameter-only ceiling at ~4.5x ratio confirmed on KITTI 00/02/05/07/08. Closing further requires architectural attention to the continuous-time optimization stack (ceres options, planarity threshold weighting, motion compensation precision) rather than more iterations.
- **Next step**: Compare repo CT-ICP step-by-step against the upstream CT-ICP reference implementation to identify which optimization-stack component introduces the remaining ~4.5x ceiling. Also export sequences 01/03/04/06/09/10 for full paper coverage.
- **Notes**: Paper reports RPE on KITTI Odometry 00-10 (0.30-0.75 %). After the 2026-05-18 Tr-frame correction and gap-closing tune (--ct-icp-dense-profile + --ct-icp-ceres-max-iterations 6 + --ct-icp-max-frames-in-map 20), CT-ICP pure odometry hits: seq 00: 2.20 % (paper 0.52, 4.2x); seq 02: 3.35 % (0.58, 5.8x); seq 05: 1.28 % (0.31, 4.1x); seq 07: 2.14 % (0.30, 7.1x); seq 08: 2.10 % (0.75, 2.8x). Geometric mean 4.58x (down from 5.44x baseline). Aggressive voxel 1.0 tune wins on seq 07 (6.3x) and 08 (2.7x) but diverges on seq 02 (8.5x) - the dense-profile-plus-budget tune is more robust. The parameter ceiling is ~4.5x; closing more needs architectural work on the continuous-time optimization, not more iterations. The --ct-icp-gt-seed toggle remains available for fair-prior dogfooding-style cross-method comparison against LiTAMIN2's default Policy A (e.g. MulRan parkinglot full: 76 m default -> 10 m with gt-seed) but is explicitly non-comparable to paper since the paper measures pure odometry.

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
