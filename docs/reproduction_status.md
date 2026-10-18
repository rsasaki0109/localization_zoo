# Reproduction Status

_Generated at 2026-10-18T02:23:29+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

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
| LiTAMIN2 | `approximately_reproduced` | Paper-oriented front-end reimplementation with the paper odometry pipeline | Near paper across KITTI Odometry 00-10 (paper pipeline, no GT seed) | Same metric, ~1.05x with one fixed setting on 00-10 | None for the KITTI numbers. The shape term only helps when the map holds single-scan distributions (the first one per voxel); against aggregated map voxels it hurts (2.0x, diverges on 04 and 08), which the paper does not discuss. Loop closure is not implemented. | Loop closure (Table III rows with loop closure) is out of scope; the paper odometry could become the method default once other datasets are checked. |
| GICP | `ported` | Compact shared implementation | Concept-only comparison | Not direct | There is no single paper benchmark and hardware protocol to reproduce against. | Choose a concrete upstream GICP benchmark target, or keep GICP explicitly scoped as conceptual baseline evidence. |
| NDT | `ported` | Compact baseline | Concept-only comparison | Not direct | There is no agreed modern NDT reference setup wired into the repo as the reproduction target. | Anchor against one explicit modern NDT codebase and dataset pair if direct reproduction is required. |
| KISS-ICP | `approximately_reproduced` | Compact baseline | Matches the official code on KITTI 00/07 with upstream mechanics | KITTI 00/04 per sequence plus the 00-10 average | The remaining 1.13x on the 00-10 average is shared by the official 1.3.0 code, so it lies between the paper and the released code (for example an older release or a different deskewing setup), not in the port. | Optionally rerun an older official KISS-ICP release on 00-10 to locate the paper-vs-release difference; the port itself needs no further change for KITTI. |
| CT-ICP | `approximately_reproduced` | Paper-oriented core reimplementation with an upstream-mechanics pipeline | Matches the official code on KITTI 00-10 with upstream mechanics | Same metric, KITTI 00-10 per sequence | The remaining 1.10x on the 00-10 average (largest on 04/05/09/10) is shared by the official code on the same scans, so it lies between the paper and the released code (for example the paper's KITTI PLY timestamps or an older release), not in the port. | Optionally obtain the official KITTI PLY files (password-protected download) to check whether their per-point timestamps explain the paper-vs-release difference; the port itself needs no further change for KITTI. |
| CT-LIO | `ported` | Custom integration | Not comparable to a single paper | Reference-based only | There is no single paper-faithful target implementation and protocol behind this path. | Either keep CT-LIO explicitly custom, or add a separate faithful-track implementation with its own benchmark protocol. |
| MULLS | `approximately_reproduced` | Lightweight multi-metric variant plus a MULLS-LO reimplementation | MULLS-LO matches the official code on KITTI 00-10 | Same metric, KITTI 00-10 per sequence | The remaining 1.06x on the 00-10 average is shared by the official code on the same scans, so it lies between the paper and the released code, not in the reimplementation. | None for KITTI odometry; loop closure (MULLS-SLAM) and the other datasets of the paper are not covered. |
| SUMA | `approximately_reproduced` | Paper reimplementation with the paper's frame-to-model pipeline (SuMaPaper) | Near paper on KITTI Odometry 00-10 (paper pipeline) | Same metric, 0.97x on 00-10 (best of two variants per sequence) | No single setting wins everywhere: the official model lookup on raw scans drifts forward on degenerate stretches (02 corridors 3.2x, 01 highway 8.1x); the normalised lookup with the correction fixes 02 and halves 01 (still 2.09x) but costs elsewhere. Official SuMa (GPU/OpenGL) cannot run here for a direct check. | Look at the seq 01 highway (forward degeneracy); check the official code on a GPU host. |
| ALOAM | `indicative` | Paper reimplementation | Full KITTI Odometry coverage with one fixed setting | Same metric, 1.07x on 00-10 with one fixed setting | No sequence exceeds twice its paper value with the fixed setting. The reference is LOAM values cited by later papers, not an A-LOAM per-sequence author table; the repo implements A-LOAM rather than original LOAM. | Evaluate the fixed configuration on independent sensor conditions while preserving the full-KITTI baseline and the documented paper/implementation differences. |
| LF-GICP | `approximately_reproduced` | Paper reimplementation (no author code) | Full KITTI Odometry coverage with one fixed setting | Same metric, 1.12x on 00-10 with one fixed setting | Seq 10: 5.616 % vs 0.837 % (6.71x), 26 moving steps with length error >20 % and 34 with local vector error >20 %. The field regularization was calibrated on KITTI 00 after earlier 02/08 observations. Genuine absence scenes in GEODE remain unavailable, so full KITTI coverage does not validate that paper claim. | Trace association and optimization through the measured failure intervals; keep this negative fixed-setting baseline and any non-paper fix opt-in. |
| L-LO | `indicative` | Paper reimplementation (no author code) | Full KITTI Odometry coverage with one fixed setting | Same metric, 1.57x on 00/02-10 (01 is NA in the paper) with one fixed setting | Seq 09: 4.122 % vs 1.890 % (2.18x), 4 moving steps with length error >20 % and 9 with local vector error >20 %. Seq 10: 4.105 % vs 1.900 % (2.16x), 7 moving steps with length error >20 % and 17 with local vector error >20 %. The paper leaves numeric parameters unspecified and the tested GT-derived roll-free baselines stay above the 00/07 paper values. Those tested trajectories are not a proof of a lower bound over all possible estimates. Existing vertical-pose experiments remain negative. | Trace association and optimization through the measured failure intervals; keep this negative fixed-setting baseline and any non-paper fix opt-in. |
| ELO | `indicative` | Paper reimplementation (no author code) | KITTI Odometry 00-10 with the correction and the paper normal patch | Same metric, 1.22x on 00-10 (one setting, chosen on 07) | The paper fusion weight w2 ("inlier ratio between the non-ground points and ground ones") is ambiguous; the reading implemented here does not help. | Try other readings of w2 and the ground test; run the remaining KITTI sequences. |
| IMLS-SLAM | `approximately_reproduced` | Paper reimplementation (no author code) with the paper odometry pipeline | Near paper on KITTI Odometry 00-10 (paper pipeline) | Same metric, 1.05x on 00-10 (one setting, paper pipeline) | Small-object removal (paper Table II, 0.58 -> 0.55 %) is not implemented; the coarse first registration is not in the paper. | Run the remaining KITTI sequences with paper_odometry_w10 (1.7 FPS) or w100 (1.1 FPS). |
| INTEN-LOAM | `indicative` | Paper reimplementation (no author code) with the paper pipeline | Gap remains on KITTI Odometry 00-10 with one fixed setting | Same metric, 2.19x on 00-10 with one fixed setting | 01: 16.614 % vs 0.64 % (25.96x), with motion-length deficits at destination frames 217-253 and 420-518; 136 moving steps differ in length by >20 %. 02: 3.221 % vs 0.54 % (5.97x), with 133 such steps, principally 1878-1943 and 4460-4524. 10: 1.156 % vs 0.45 % (2.57x), but no moving step exceeds 20 % length or vector error; the 600-700 block grows from 6.7 to 9.2 m position error. Saved-pose diagnostics locate drift, without identifying its solver-level cause. | Trace association and optimization through the measured 01/02 failure intervals and the smaller accumulated errors on 10. Keep this negative fixed-setting baseline; any non-paper correction must be opt-in and validated on all sequences. |
| MESH-LOAM | `approximately_reproduced` | Paper reimplementation (no author code) with the paper association and input | Gap remains on KITTI Odometry 00-10 with one fixed setting | Same metric, 1.28x on 00-10 with one fixed setting | No author code. Full coverage shows a distributed gap (1.01-2.00x after rounding). Seq 04 is 0.819834 % vs 0.41 %, strictly below twice the paper value despite rounding to 2.00x. The earlier association and elevation fixes improved 00/07 but do not establish full reproduction. | Explain the remaining full-sequence gap, particularly 04 and 10; profile speed before attempting more coverage runs. Preserve the fixed-setting baseline and keep any non-paper fixes opt-in. |
| QUADRIC-LO | `indicative` | Paper reimplementation (no author code) | Full KITTI Odometry coverage with one fixed setting | Same metric, 0.85x on 00-10 with one fixed setting | No sequence exceeds twice its paper value with the fixed setting. No author code. The local-map implementation is compared with the scan-to-submap table; separate earlier 02 fallback-validation evidence exists and 02 is not claimed as previously unseen. | Evaluate the fixed configuration on independent sensor conditions while preserving the full-KITTI baseline and the documented paper/implementation differences. |
| DILO | `approximately_reproduced` | Paper reimplementation (no author code) with a paper odometry path (plane key points only) | Near paper on KITTI Odometry 00-10 (plane-only paper pipeline) | Same metric, 0.96x on 00-10 (one setting; 01 1.42x with a degenerate-direction remap chosen there) | Below the paper's full DiLO without its edge term. Open highway (01) needs the remap, which is not in the paper and was chosen on 01. | Run the remaining KITTI sequences. |

## LiTAMIN2

- **Claim level**: `approximately_reproduced`
- **Paper**: Yokozuka et al., LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation Applied with KL-Divergence, ICRA 2021
- **Method README**: `papers/litamin2/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper-oriented front-end reimplementation with the paper odometry pipeline. The repo reproduces the LiTAMIN2 registration front-end and also keeps a paper-like 3 m profile inside the shared benchmark harness.
- **Current claim**: Near paper across KITTI Odometry 00-10 (paper pipeline, no GT seed). --litamin2-paper-odometry follows Sec. III of the paper: raw scans voted into 3 m normal distributions, a voxel map of distributions, nearest-distribution pairing, and the ICP+Cov cost (shape term linear in the cost, Newton steps). Run unchanged on all 11 sequences it scores official KITTI RTE 1.01/2.06/1.15/0.76/0.90/0.62/0.58/0.48/0.98/0.78/0.99 % on 00-10 versus the paper's ICP+Cov row 0.78/2.10/0.95/0.96/1.05/0.55/0.55/0.48/1.01/0.69/0.80 % (Table III, no loop closure): geometric mean 1.05x. The ICP-only variant against the same map gives 1.01x versus the paper's ICP row. Table 6 picks the best variant per sequence and reaches 1.00x.
- **Numeric comparison**: Same metric, ~1.05x with one fixed setting on 00-10. Official KITTI RTE of paper_icp_cov (one setting, all sequences) vs the paper's ICP+Cov row: 00 1.009 vs 0.78, 01 2.062 vs 2.10, 02 1.148 vs 0.95, 03 0.756 vs 0.96, 04 0.902 vs 1.05, 05 0.624 vs 0.55, 06 0.578 vs 0.55, 07 0.477 vs 0.48, 08 0.984 vs 1.01, 09 0.777 vs 0.69, 10 0.986 vs 0.80; geometric mean 1.05x. paper_icp_first (ICP term only) vs the paper's ICP row: 1.01x. The earlier harness path (0.5 m pre-downsampling, at most 2500 source points, a point map rebuilt into voxels, same-voxel pairing) stayed at 1.22x on 00/02/05/07/08. Table 6 (docs/assets/paper/paper_ratio_table.csv) selects the best 100 m RPE variant per sequence; see papers/litamin2/README.md.
- **Main blocker**: None for the KITTI numbers. The shape term only helps when the map holds single-scan distributions (the first one per voxel); against aggregated map voxels it hurts (2.0x, diverges on 04 and 08), which the paper does not discuss. Loop closure is not implemented.
- **Next step**: Loop closure (Table III rows with loop closure) is out of scope; the paper odometry could become the method default once other datasets are checked.
- **Notes**: Paper values were re-verified against the arXiv PDF on 2026-10-03 (Table III, ICP+Cov without loop closure; the ICP-only row is 0.75/1.88/0.99/0.84/0.90/0.74/0.45/0.55/1.25/0.74/1.36 %). Results: experiments/results/litamin2_kitti_seq_NN_full_paper_matrix.json. The older harness-path sweeps (voxel resolution, correspondence radius and gate, covariance floor, covariance gradient, coarse-to-fine) remain in the other litamin2 aggregates; they used GT-free constant-velocity seeds but not the paper's map or input. Policy A GT-seeded numbers (0.74 % on Raw 0009 short window, 1.26 % on MulRan parkinglot full) remain non-comparable to the paper.

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

- **Claim level**: `approximately_reproduced`
- **Paper**: Vizzo et al., KISS-ICP: In Defense of Point-to-Point ICP, RAL 2023
- **Method README**: `papers/kiss_icp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Compact baseline. The repo keeps a small KISS-ICP-style local-map pipeline that preserves the main idea while simplifying the full upstream engineering stack.
- **Current claim**: Matches the official code on KITTI 00/07 with upstream mechanics. With --kiss-upstream-profile --kiss-model-deviation-threshold --kiss-geman-mcclure-kernel --kiss-map-min-point-distance 0.2236068 and the KITTI elevation correction the port scores 0.535 % (seq 00) and 0.363 % (seq 07) official KITTI RTE against 0.528 % and 0.375 % for the official KISS-ICP 1.3.0 code. The repository default keeps a simpler speed-based adaptive threshold and is not a faithful rerun.
- **Numeric comparison**: KITTI 00/04 per sequence plus the 00-10 average. The paper reports per-sequence KITTI RTE only for seq 00 (0.51 %) and 04 (0.36 %), plus the 00-10 average (0.50 %). Upstream KISS-ICP corrects the KITTI elevation angle by +0.205 deg in its KITTI loader. With the same correction and the upstream mechanics (model-deviation threshold, Geman-McClure kernel, map insertion gate) the port scores 0.535 % on seq 00 (1.05x) and 0.373 % on seq 04 (1.04x), and 0.564 % pooled over 00-10, identical to the official KISS-ICP 1.3.0 code on the same pipeline (0.564 %, 1.13x the paper's average). Before matching the adaptive threshold the best seq 00 value was 0.708 % (1.39x), and 0.954 % on raw scans (1.87x). Seq 00/04 values are the best variant among the sweeps on that sequence (selected on the evaluated sequence, so optimistic); see docs/kitti_elevation_correction.md, papers/kiss_icp/README.md, and docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The remaining 1.13x on the 00-10 average is shared by the official 1.3.0 code, so it lies between the paper and the released code (for example an older release or a different deskewing setup), not in the port.
- **Next step**: Optionally rerun an older official KISS-ICP release on 00-10 to locate the paper-vs-release difference; the port itself needs no further change for KITTI.
- **Notes**: KISS-ICP reports the KITTI average relative translational error (Table II: 0.50 % train / 0.61 % test) and per-sequence values only in the Table VI threshold ablation. The paper does not state hardware or a KITTI frame rate.

## CT-ICP

- **Claim level**: `approximately_reproduced`
- **Paper**: Dellenbach et al., CT-ICP: Real-time Elastic LiDAR Odometry with Loop Closure, ICRA 2022
- **Method README**: `papers/ct_icp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper-oriented core reimplementation with an upstream-mechanics pipeline. The repo implements the continuous-time two-pose-per-scan core, interpolation, and point-to-plane optimization described by CT-ICP. --ct-icp-upstream-mechanics runs a dedicated pipeline that follows upstream jedeschaud/ct_icp with its driving profile (papers/ct_icp/README.md).
- **Current claim**: Matches the official code on KITTI 00-10 with upstream mechanics. With --ct-icp-upstream-mechanics and the KITTI elevation correction the port scores 0.587 % official KITTI RTE pooled over sequences 00-10 against 0.585 % for the official jedeschaud/ct_icp code on the same scans (run_official_ct_icp_baseline.py), and stays within 0.05 points of it on every sequence (seq 07: 0.315 % vs 0.308 %, paper 0.31 %). The default runCTICP pipeline is not a faithful rerun: it used the point index as time, a 20-frame window and zero-motion initialisation, scored 1.03-1.18 % on seq 07, and is numerically unstable (about +/-7-11 % under 1e-6 deg input perturbations), which explains the earlier host dependence.
- **Numeric comparison**: Same metric, KITTI 00-10 per sequence. Official KITTI RTE on KITTI-corrected scans, port with --ct-icp-upstream-mechanics vs paper Table I: 00 0.525/0.49, 01 0.716/0.76, 02 0.549/0.52, 03 0.586/0.72, 04 0.717/0.39, 05 0.315/0.25, 06 0.284/0.27, 07 0.315/0.31, 08 0.902/0.81, 09 0.629/0.49, 10 0.742/0.48; pooled 00-10 0.587 % vs 0.531 % (the paper's per-sequence values weighted by segment count), 1.10x. The official code gives 0.585 % pooled on the same scans, so the remaining difference lies between the paper and the released code. Before the upstream mechanics the best of ~313 sweep variants was 4.03x (geometric mean over 00/02/05/07/08). See papers/ct_icp/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The remaining 1.10x on the 00-10 average (largest on 04/05/09/10) is shared by the official code on the same scans, so it lies between the paper and the released code (for example the paper's KITTI PLY timestamps or an older release), not in the port.
- **Next step**: Optionally obtain the official KITTI PLY files (password-protected download) to check whether their per-point timestamps explain the paper-vs-release difference; the port itself needs no further change for KITTI.
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

- **Claim level**: `approximately_reproduced`
- **Paper**: Pan et al., MULLS: Versatile LiDAR SLAM via Multi-metric Linear Least Square, ICRA 2021
- **Method README**: `papers/mulls/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Lightweight multi-metric variant plus a MULLS-LO reimplementation. The default MULLS path is a lightweight A-LOAM-feature multi-metric variant. --mulls-lo runs MULLS-LO written from the paper and the authors' KITTI profiles: dual-threshold grid ground filter, PCA pillar/beam/facade classification, multi-metric linear least-squares ICP, and a local map (papers/mulls/README.md).
- **Current claim**: MULLS-LO matches the official code on KITTI 00-10. With --mulls-lo (highway profile on seq 01) and the profile's 0.195 deg vertical-angle correction the reimplementation scores 0.572 % official KITTI RTE pooled over sequences 00-10 against 0.576 % for the official YuePanEdward/MULLS code on the same scans (run_official_mulls_baseline.py), within about 0.02 points on every sequence (seq 07: 0.319 % vs 0.311-0.332 %, paper 0.29 %). The default lightweight variant is not a faithful rerun (2.6 % on seq 07).
- **Numeric comparison**: Same metric, KITTI 00-10 per sequence. Official KITTI RTE with the 0.195 deg correction, --mulls-lo vs paper Table II MULLS-LO(mc): 00 0.536/0.51, 01 0.674/0.62 (highway profile), 02 0.585/0.55, 03 0.609/0.61, 04 0.492/0.35, 05 0.315/0.28, 06 0.297/0.24, 07 0.319/0.29, 08 0.812/0.80, 09 0.548/0.49, 10 0.691/0.61; pooled 00-10 0.572 % vs 0.539 % (the paper's per-sequence values weighted by segment count), 1.06x. The official code gives 0.576 % pooled on the same scans, so the remaining difference lies between the paper and the released code. The lightweight variant was 7.62x (00/02/05/07/08). See papers/mulls/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The remaining 1.06x on the 00-10 average is shared by the official code on the same scans, so it lies between the paper and the released code, not in the reimplementation.
- **Next step**: None for KITTI odometry; loop closure (MULLS-SLAM) and the other datasets of the paper are not covered.
- **Notes**: Paper Table II reports 0.08 s/frame for MULLS-LO(mc). Transfer runs on 00/02/05/08 ran four sequences concurrently on one host, so their FPS is depressed.

## SUMA

- **Claim level**: `approximately_reproduced`
- **Paper**: Behley and Stachniss, Efficient Surfel-Based SLAM using 3D Laser Range Data in Urban Environments, RSS 2018
- **Method README**: `papers/suma/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation with the paper's frame-to-model pipeline (SuMaPaper). Dense surfel odometry from range-image maps inside the shared benchmark harness.
- **Current claim**: Near paper on KITTI Odometry 00-10 (paper pipeline). --suma-paper (rendered surfel model, projective frame-to-model ICP, surfel stability and fusion, official default.xml values, no loop closure). Official KITTI RTE 00-10, official defaults on raw scans: 0.697 / 13.793 / 3.512 / 0.651 / 0.417 / 0.379 / 0.286 / 0.331 / 1.060 / 0.510 / 0.698 %; normalised model lookup + 0.22 deg correction: 0.884 / 3.550 / 0.784 / 1.186 / 0.802 / - / 0.530 / 0.521 / 1.179 / 0.962 / 0.897 %; paper 0.7 / 1.7 / 1.1 / 0.7 / 0.4 / 0.5 / 0.4 / 0.4 / 1.0 / 0.5 / 0.7 %. Best variant per sequence 0.97x (01 2.09x); one setting for all 1.25x (official) or 1.41x (normalised).
- **Numeric comparison**: Same metric, 0.97x on 00-10 (best of two variants per sequence). Best variant per sequence: 00 1.00x, 01 2.09x, 02 0.71x, 03 0.93x, 04 1.04x, 05 0.76x, 06 0.71x, 07 0.83x, 08 1.06x, 09 1.02x, 10 1.00x. See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: No single setting wins everywhere: the official model lookup on raw scans drifts forward on degenerate stretches (02 corridors 3.2x, 01 highway 8.1x); the normalised lookup with the correction fixes 02 and halves 01 (still 2.09x) but costs elsewhere. Official SuMa (GPU/OpenGL) cannot run here for a direct check.
- **Next step**: Look at the seq 01 highway (forward degeneracy); check the official code on a GPU host.
- **Notes**: Paper values are rounded to one decimal, so ratios carry up to ~25 % rounding uncertainty on short-error sequences.

## ALOAM

- **Claim level**: `indicative`
- **Paper**: Zhang and Singh, LOAM: Lidar Odometry and Mapping in Real-time, RSS 2014 (A-LOAM is the HKUST re-implementation)
- **Method README**: `papers/aloam/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation. A-LOAM port in the shared harness, no per-frame GT seed. Fixed kitti_default on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. kitti_default was the accuracy candidate in the existing sequence 07 profile comparison (lowest stored 100 m RPE and official RTE there). Its arguments are frozen; the added sequences do not select a profile.
- **Current claim**: Full KITTI Odometry coverage with one fixed setting. Full-sequence official KITTI RTE on 00-10, variant kitti_default, geometric mean 1.07x. kitti_default was the accuracy candidate in the existing sequence 07 profile comparison (lowest stored 100 m RPE and official RTE there). Its arguments are frozen; the added sequences do not select a profile. No sequence exceeds twice its paper value with the fixed setting. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
- **Numeric comparison**: Same metric, 1.07x on 00-10 with one fixed setting. kitti_default, official KITTI RTE [%], repo vs paper: 00 0.855 vs 0.780 (1.10x); 01 2.189 vs 1.430 (1.53x); 02 1.029 vs 0.920 (1.12x); 03 0.964 vs 0.860 (1.12x); 04 0.880 vs 0.710 (1.24x); 05 0.535 vs 0.570 (0.94x); 06 0.602 vs 0.650 (0.93x); 07 0.681 vs 0.630 (1.08x); 08 0.975 vs 1.120 (0.87x); 09 0.673 vs 0.770 (0.87x); 10 0.873 vs 0.790 (1.11x); geometric mean 1.07x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
- **Main blocker**: No sequence exceeds twice its paper value with the fixed setting. The reference is LOAM values cited by later papers, not an A-LOAM per-sequence author table; the repo implements A-LOAM rather than original LOAM.
- **Next step**: Evaluate the fixed configuration on independent sensor conditions while preserving the full-KITTI baseline and the documented paper/implementation differences.
- **Notes**: Secondary-source values: LOAM (RSS 2014) does not tabulate per-sequence KITTI RTE. The repo implementation follows A-LOAM, not the original LOAM code.

## LF-GICP

- **Claim level**: `approximately_reproduced`
- **Paper**: Im, LF-GICP: Parameter-Free Degeneracy Handling for LiDAR Odometry via a Voxel-Normal Localizability Field, arXiv 2026
- **Method README**: `papers/lf_gicp/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation (no author code). From-paper reimplementation in the shared harness, no per-frame GT seed. Fixed paper_default on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. field beta 0.25 was calibrated on sequence 00 first 500 frames against the paper dilution range, after earlier 02/08 runs exposed excessive gating. Those sequences influenced the decision to recalibrate; no added sequence influences the frozen setting.
- **Current claim**: Full KITTI Odometry coverage with one fixed setting. Full-sequence official KITTI RTE on 00-10, variant paper_default, geometric mean 1.12x. field beta 0.25 was calibrated on sequence 00 first 500 frames against the paper dilution range, after earlier 02/08 runs exposed excessive gating. Those sequences influenced the decision to recalibrate; no added sequence influences the frozen setting. Seq 10: 5.616 % vs 0.837 % (6.71x), 26 moving steps with length error >20 % and 34 with local vector error >20 %. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
- **Numeric comparison**: Same metric, 1.12x on 00-10 with one fixed setting. paper_default, official KITTI RTE [%], repo vs paper: 00 0.664 vs 0.692 (0.96x); 01 1.816 vs 1.822 (1.00x); 02 1.075 vs 1.084 (0.99x); 03 0.785 vs 1.207 (0.65x); 04 0.629 vs 0.814 (0.77x); 05 0.512 vs 0.577 (0.89x); 06 0.543 vs 0.546 (0.99x); 07 0.411 vs 0.496 (0.83x); 08 1.005 vs 0.878 (1.14x); 09 0.725 vs 0.566 (1.28x); 10 5.616 vs 0.837 (6.71x); geometric mean 1.12x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
- **Main blocker**: Seq 10: 5.616 % vs 0.837 % (6.71x), 26 moving steps with length error >20 % and 34 with local vector error >20 %. The field regularization was calibrated on KITTI 00 after earlier 02/08 observations. Genuine absence scenes in GEODE remain unavailable, so full KITTI coverage does not validate that paper claim.
- **Next step**: Trace association and optimization through the measured failure intervals; keep this negative fixed-setting baseline and any non-paper fix opt-in.
- **Notes**: Paper average 0.865 % over 00-10; same raw-scan protocol as this repository's KITTI data.

## L-LO

- **Claim level**: `indicative`
- **Paper**: Li, Fu, Sun, L-LO: Enhancing Pose Estimation Precision via a Landmark-Based LiDAR Odometry, arXiv 2023
- **Method README**: `papers/l_lo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00, 02-10; 01 reported as NA)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation (no author code). From-paper reimplementation in the shared harness, no per-frame GT seed. Fixed seq07_tuned on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. cluster_cell=0.25, ground_tolerance=0.15 and max_range=80 were selected by the existing 100 m RPE sweeps on sequence 07 and previously transferred unchanged to 00/02/05/08. Sequence 01 is NA in the paper and excluded.
- **Current claim**: Full KITTI Odometry coverage with one fixed setting. Full-sequence official KITTI RTE on 00/02-10 (01 is NA in the paper), variant seq07_tuned, geometric mean 1.57x. cluster_cell=0.25, ground_tolerance=0.15 and max_range=80 were selected by the existing 100 m RPE sweeps on sequence 07 and previously transferred unchanged to 00/02/05/08. Sequence 01 is NA in the paper and excluded. Seq 09: 4.122 % vs 1.890 % (2.18x), 4 moving steps with length error >20 % and 9 with local vector error >20 %. Seq 10: 4.105 % vs 1.900 % (2.16x), 7 moving steps with length error >20 % and 17 with local vector error >20 %. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
- **Numeric comparison**: Same metric, 1.57x on 00/02-10 (01 is NA in the paper) with one fixed setting. seq07_tuned, official KITTI RTE [%], repo vs paper: 00 1.479 vs 0.910 (1.63x); 02 4.478 vs 2.350 (1.91x); 03 3.066 vs 2.920 (1.05x); 04 2.135 vs 2.280 (0.94x); 05 1.473 vs 0.820 (1.80x); 06 1.315 vs 0.660 (1.99x); 07 1.004 vs 0.690 (1.46x); 08 1.661 vs 1.370 (1.21x); 09 4.122 vs 1.890 (2.18x); 10 4.105 vs 1.900 (2.16x); geometric mean 1.57x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
- **Main blocker**: Seq 09: 4.122 % vs 1.890 % (2.18x), 4 moving steps with length error >20 % and 9 with local vector error >20 %. Seq 10: 4.105 % vs 1.900 % (2.16x), 7 moving steps with length error >20 % and 17 with local vector error >20 %. The paper leaves numeric parameters unspecified and the tested GT-derived roll-free baselines stay above the 00/07 paper values. Those tested trajectories are not a proof of a lower bound over all possible estimates. Existing vertical-pose experiments remain negative.
- **Next step**: Trace association and optimization through the measured failure intervals; keep this negative fixed-setting baseline and any non-paper fix opt-in.
- **Notes**: Paper average 1.59 % over the reported sequences.

## ELO

- **Claim level**: `indicative`
- **Paper**: Zheng and Zhu, Efficient LiDAR Odometry for Autonomous Driving, IEEE RA-L 2021
- **Method README**: `papers/elo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official KITTI t_rel (100-800 m))
- **Repo scope**: Paper reimplementation (no author code). From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: KITTI Odometry 00-10 with the correction and the paper normal patch. The dense profile with the 0.195 deg correction the paper applies and the eq. 12 normal patch read as the full patch size scores official KITTI RTE 0.656 / 0.480 % on 00 / 07. Three forward-bias fixes of the projective model (same-surface fusion in the closest-point SRI update, no forward term from the noisy BEV ground normals, curvature threshold 0.03), chosen on 07 windows with 00 held out, bring it to 0.586 / 0.391 % vs the paper's 0.54 / 0.31 % (1.09x / 1.26x, geometric mean 1.17x). See papers/elo/README.md.
- **Numeric comparison**: Same metric, 1.22x on 00-10 (one setting, chosen on 07). dense_elevation_paper_normals_unbiased on 00-10: 0.84-1.43x, geometric mean 1.22x; 00 0.586 vs 0.54, 07 0.391 vs 0.31. Repo values: experiments/results/elo_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
- **Main blocker**: The paper fusion weight w2 ("inlier ratio between the non-ground points and ground ones") is ambiguous; the reading implemented here does not help.
- **Next step**: Try other readings of w2 and the ground test; run the remaining KITTI sequences.
- **Notes**: Paper average 0.50 % (online test 0.68 %). The frame-to-frame variant scores 1.14/0.83/0.91/1.25/0.83/0.75/0.76/0.60/1.34/1.18/2.57 %. No loop closure. Values read from the paper PDF on 2026-10-12.

## IMLS-SLAM

- **Claim level**: `approximately_reproduced`
- **Paper**: Deschaud, IMLS-SLAM: scan-to-model matching based on 3D data, ICRA 2018
- **Method README**: `papers/imls_slam/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI odometry drift (%))
- **Repo scope**: Paper reimplementation (no author code) with the paper odometry pipeline. From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: Near paper on KITTI Odometry 00-10 (paper pipeline). --imls-slam-paper-odometry follows the paper: raw scans with the 0.22 deg beam correction, the last 100 raw scans as the model, h = 0.06 m / r = 0.20 m with a per-query normal, nine planarity-weighted lists of 100 samples, 20 iterations. Official KITTI RTE 0.523 / 0.339 % on 00 / 07 vs the paper's 0.50 / 0.33 % (1.05x / 1.03x, geometric mean 1.04x). The harness profile scores 1.378 / 0.846 %; without the beam correction the paper pipeline scores 1.327 / 0.706 %. See papers/imls_slam/README.md.
- **Numeric comparison**: Same metric, 1.05x on 00-10 (one setting, paper pipeline). paper_odometry_w100_coarse_start on 00-10: 0.91-1.29x, geometric mean 1.05x; 00 0.523 vs 0.50, 07 0.339 vs 0.33; 04 11.9 % without the coarse first registration (it starts in motion). Repo values: experiments/results/imls_slam_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
- **Main blocker**: Small-object removal (paper Table II, 0.58 -> 0.55 %) is not implemented; the coarse first registration is not in the paper.
- **Next step**: Run the remaining KITTI sequences with paper_odometry_w10 (1.7 FPS) or w100 (1.1 FPS).
- **Notes**: Paper overall 0.55 % on the training set (0.69 % on the test set). Scans were de-skewed with an external odometry before matching; scan-to-model, no loop closure. Values read from the paper PDF on 2026-10-12.

## INTEN-LOAM

- **Claim level**: `indicative`
- **Paper**: Li et al., InTEn-LOAM: Intensity and Temporal Enhanced LiDAR Odometry and Mapping, Remote Sensing 2022 (arXiv 2209.05708)
- **Method README**: `papers/inten_loam/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI RTE (%))
- **Repo scope**: Paper reimplementation (no author code) with the paper pipeline. From-paper reimplementation in the shared harness, no GT seed. One fixed variant on KITTI Odometry 00-10; existing 00/07 results are reused unchanged, and 01-06 / 08-10 are new evaluation-only full runs. Table 6 is restricted to this variant; historical ablations remain stored.
- **Current claim**: Gap remains on KITTI Odometry 00-10 with one fixed setting. --inten-loam-paper-odometry uses FEF, S2S re-association, S2M on a temporal-voxel-filtered keyframe map, DOR map-insertion gating and a B-spline intensity residual. The fixed paper_odometry_plane_check variant explicitly uses s2m_plane_max_dist=0.05 and a 0.17 deg correction on all 00-10 sequences: geometric mean 2.19x. These KITTI choices were selected on 07 windows, with diagnosis of 00 frames 4000-4399 suggesting the plane check; 00 is not fully held out. The added sequences are evaluation only. Defaults and stored ablations are unchanged.
- **Numeric comparison**: Same metric, 2.19x on 00-10 with one fixed setting. paper_odometry_plane_check, official KITTI RTE [%], repo vs paper: 00 0.697 vs 0.52 (1.34x); 01 16.614 vs 0.64 (25.96x); 02 3.221 vs 0.54 (5.97x); 03 0.891 vs 0.63 (1.41x); 04 0.676 vs 0.37 (1.83x); 05 0.497 vs 0.36 (1.38x); 06 0.359 vs 0.24 (1.49x); 07 0.364 vs 0.34 (1.07x); 08 0.938 vs 0.71 (1.32x); 09 0.661 vs 0.48 (1.38x); 10 1.156 vs 0.45 (2.57x); geometric mean 2.19x. Table 6 pins this variant for every sequence. Sources: experiments/results/inten_loam_kitti_seq_NN_full_paper_comp_matrix.json and experiments/results/mesh_inten_kitti_coverage/summary.json.
- **Main blocker**: 01: 16.614 % vs 0.64 % (25.96x), with motion-length deficits at destination frames 217-253 and 420-518; 136 moving steps differ in length by >20 %. 02: 3.221 % vs 0.54 % (5.97x), with 133 such steps, principally 1878-1943 and 4460-4524. 10: 1.156 % vs 0.45 % (2.57x), but no moving step exceeds 20 % length or vector error; the 600-700 block grows from 6.7 to 9.2 m position error. Saved-pose diagnostics locate drift, without identifying its solver-level cause.
- **Next step**: Trace association and optimization through the measured 01/02 failure intervals and the smaller accumulated errors on 10. Keep this negative fixed-setting baseline; any non-paper correction must be opt-in and validated on all sequences.
- **Notes**: Paper average 0.54 % (frame-weighted). Includes scan-to-local-map refinement, no loop closure. Values taken from arXiv v1, not checked against the published version. Values read from the paper PDF on 2026-10-12.

## MESH-LOAM

- **Claim level**: `approximately_reproduced`
- **Paper**: Zhu et al., Mesh-LOAM: Real-time Mesh-Based LiDAR Odometry and Mapping, IEEE T-IV 2024
- **Method README**: `papers/mesh_loam/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI relative translational error (%))
- **Repo scope**: Paper reimplementation (no author code) with the paper association and input. From-paper reimplementation in the shared harness, no GT seed. One fixed variant on KITTI Odometry 00-10; existing 00/07 results are reused unchanged, and 01-06 / 08-10 are new evaluation-only full runs. Table 6 is restricted to this variant; historical ablations remain stored.
- **Current claim**: Gap remains on KITTI Odometry 00-10 with one fixed setting. --mesh-loam-paper-odometry uses the paper input and nearest-facet association with an explicit 0.22 deg KITTI elevation correction, not specified by the paper. paper_odometry_elevation is unchanged across 00-10: geometric mean 1.28x, rather than the historical 1.08x on 00/07 alone. Both 00/07 influenced development; the added sequences did not influence the setting. No sequence exceeds twice its paper value. The correction remains opt-in and defaults are unchanged.
- **Numeric comparison**: Same metric, 1.28x on 00-10 with one fixed setting. paper_odometry_elevation, official KITTI RTE [%], repo vs paper: 00 0.565 vs 0.53 (1.07x); 01 0.857 vs 0.64 (1.34x); 02 0.607 vs 0.52 (1.17x); 03 0.928 vs 0.63 (1.47x); 04 0.820 vs 0.41 (2.00x); 05 0.345 vs 0.34 (1.01x); 06 0.418 vs 0.35 (1.19x); 07 0.342 vs 0.31 (1.10x); 08 0.909 vs 0.82 (1.11x); 09 0.631 vs 0.47 (1.34x); 10 0.848 vs 0.56 (1.51x); geometric mean 1.28x. Table 6 pins this variant for every sequence. Sources: experiments/results/mesh_loam_kitti_seq_NN_full_paper_comp_matrix.json and experiments/results/mesh_inten_kitti_coverage/summary.json.
- **Main blocker**: No author code. Full coverage shows a distributed gap (1.01-2.00x after rounding). Seq 04 is 0.819834 % vs 0.41 %, strictly below twice the paper value despite rounding to 2.00x. The earlier association and elevation fixes improved 00/07 but do not establish full reproduction.
- **Next step**: Explain the remaining full-sequence gap, particularly 04 and 10; profile speed before attempting more coverage runs. Preserve the fixed-setting baseline and keep any non-paper fixes opt-in.
- **Notes**: Paper mean 0.51 %. The point-to-plane ablation row averages 0.71 %. Scan-to-mesh-map odometry, no loop closure. Values read from the paper PDF on 2026-10-12.

## QUADRIC-LO

- **Claim level**: `indicative`
- **Paper**: Xia et al., Quadric Representations for LiDAR Odometry, Mapping and Localization, IEEE RA-L 2023
- **Method README**: `papers/quadric_lo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI average translation error (%))
- **Repo scope**: Paper reimplementation (no author code). From-paper reimplementation in the shared harness, no per-frame GT seed. Fixed dense_profile on every sequence with a paper value. Previously committed matching results are reused unchanged; added sequences do not select parameters. The existing dense profile is frozen from the 00/07 paper comparisons; both are treated as development data. No correction or parameter is selected on the added sequences.
- **Current claim**: Full KITTI Odometry coverage with one fixed setting. Full-sequence official KITTI RTE on 00-10, variant dense_profile, geometric mean 0.85x. The existing dense profile is frozen from the 00/07 paper comparisons; both are treated as development data. No correction or parameter is selected on the added sequences. No sequence exceeds twice its paper value with the fixed setting. The method README includes all per-sequence values and saved-pose drift diagnostics. Defaults and historical results are unchanged.
- **Numeric comparison**: Same metric, 0.85x on 00-10 with one fixed setting. dense_profile, official KITTI RTE [%], repo vs paper: 00 0.966 vs 0.920 (1.05x); 01 2.060 vs 2.610 (0.79x); 02 1.176 vs 1.160 (1.01x); 03 1.236 vs 1.450 (0.85x); 04 0.680 vs 0.870 (0.78x); 05 0.588 vs 0.760 (0.77x); 06 0.560 vs 0.550 (1.02x); 07 0.662 vs 1.090 (0.61x); 08 1.128 vs 1.680 (0.67x); 09 0.933 vs 0.850 (1.10x); 10 1.556 vs 1.760 (0.88x); geometric mean 0.85x. Sources and exact commands: experiments/results/remaining_kitti_coverage/. Table 6 is restricted to this fixed variant.
- **Main blocker**: No sequence exceeds twice its paper value with the fixed setting. No author code. The local-map implementation is compared with the scan-to-submap table; separate earlier 02 fallback-validation evidence exists and 02 is not claimed as previously unseen.
- **Next step**: Evaluate the fixed configuration on independent sensor conditions while preserving the full-KITTI baseline and the documented paper/implementation differences.
- **Notes**: Table II (scan-to-scan plus scan-to-submap correction, average 1.25 %) is compared because the repository implementation matches against a local map. Table I (scan-to-scan only) gives 2.64/5.05/3.31/1.34/1.66/1.97/1.30/2.56/2.57/3.32/2.21 %. No loop closure. Values read from the paper PDF on 2026-10-12.

## DILO

- **Claim level**: `approximately_reproduced`
- **Paper**: Han et al., DiLO: Direct light detection and ranging odometry based on spherical range images for autonomous driving, ETRI Journal 2021
- **Method README**: `papers/dilo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI average translation error (%), 100-800 m)
- **Repo scope**: Paper reimplementation (no author code) with a paper odometry path (plane key points only). From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: Near paper on KITTI Odometry 00-10 (plane-only paper pipeline). --dilo-paper-odometry (spherical range image, eq. 11 normals, IMLS-style key-point lists, Algorithm 1 window, plane residuals) with planarity > 0.98, at most one key point per 8 x 8 px cell per list, a slope-aware vertical fill (fill_slope 0.1) and the 0.18 deg correction: official KITTI RTE 0.549 / 0.372 % on 00 / 07 vs the paper's full DiLO 0.64 / 0.42 % (0.86x / 0.89x, geometric mean 0.87x) and plane-only ablation 0.84 / 0.58 %. Edge key points (Sec. 3.5) were implemented and do not help on KITTI. The settings were chosen on 07 windows with some 00 windows in view. See papers/dilo/README.md.
- **Numeric comparison**: Same metric, 0.96x on 00-10 (one setting; 01 1.42x with a degenerate-direction remap chosen there). paper_odometry_degenerate on 00-10: 0.75-1.42x, geometric mean 0.96x; 00 0.549 vs 0.64, 07 0.372 vs 0.42 (unchanged by the remap); 01 8.70 % without the remap. See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: Below the paper's full DiLO without its edge term. Open highway (01) needs the remap, which is not in the paper and was chosen on 01.
- **Next step**: Run the remaining KITTI sequences.
- **Notes**: Paper average 0.69 % (frame-weighted). The table is printed rotated; values were rebuilt from word positions and match the stated average. No loop closure. Values read from the paper PDF on 2026-10-12.
