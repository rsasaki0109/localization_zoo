# Reproduction Status

_Generated at 2026-10-13T02:38:18+00:00 by `evaluation/scripts/generate_reproduction_status.py`._

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
| SUMA | `approximately_reproduced` | Paper reimplementation with the paper's frame-to-model pipeline (SuMaPaper) | Near paper on KITTI Odometry 00/02/05/07/08 (paper pipeline) | Same metric, 0.86x on 00/02/05/07/08 (best of two variants per sequence) | No single setting wins everywhere: with the official model lookup on raw scans the seq 02 corridors drift forward (weakly constrained x, ground normals biased by the elevation bowl); the normalised lookup with the correction fixes 02 but costs on 00/07/08. Official SuMa (GPU/OpenGL) still cannot run here for a direct check. | Run the paper pipeline on the remaining KITTI sequences; check the official code on a GPU host. |
| ALOAM | `indicative` | Paper reimplementation | KITTI Odometry 00/02/05/07/08 | Same metric, secondary-source paper values | The LOAM paper itself does not report per-sequence KITTI RTE; the values are LOAM's KITTI numbers as cited by later papers. | Compare against the official A-LOAM implementation on the same full sequences. |
| LF-GICP | `approximately_reproduced` | Paper reimplementation (no author code) | KITTI Odometry 00/02/05/07/08 | Same metric, ~0.96x ratio (single configuration) | The lambda0 scale differs from the paper (planarity definition is ambiguous) and GEODE tunnels for gate calibration are not available. | Obtain GEODE tunnel data to check the gate on genuine absence and recalibrate tau2 with the paper's rule. |
| L-LO | `indicative` | Paper reimplementation (no author code) | KITTI Odometry 00/02/05/07/08 | Same metric, ~1.6x ratio (parameters chosen on seq 07) | The paper specifies no thresholds and its pitch estimate is not physically consistent as written; a seq 07 parameter sweep closes only ~3 % of the gap, so the remaining difference is likely in the method details. See papers/l_lo/README.md. | Ask the authors for code, or compare per-frame landmark matches against a reference implementation. |
| ELO | `indicative` | Paper reimplementation (no author code) | KITTI Odometry 00/07 with the correction and the paper normal patch | Same metric, 1.37x on 00/07 | The paper fusion weight w2 ("inlier ratio between the non-ground points and ground ones") is ambiguous; the reading implemented here does not help. | Try other readings of w2 and the ground test; run the remaining KITTI sequences. |
| IMLS-SLAM | `approximately_reproduced` | Paper reimplementation (no author code) with the paper odometry pipeline | Near paper on KITTI Odometry 00/07 (paper pipeline) | Same metric, 1.04x on 00/07 (paper pipeline) | Small-object removal (paper Table II, 0.58 -> 0.55 %) is not implemented; only seq 00/07 have been run. | Run the remaining KITTI sequences with paper_odometry_w10 (1.7 FPS) or w100 (1.1 FPS). |
| INTEN-LOAM | `indicative` | Paper reimplementation (no author code) with the paper pipeline | Gap remains on KITTI Odometry 00/07 (paper pipeline) | Same metric, 1.24x on 00/07 (07 1.04x, 00 held out 1.48x) | Seq 00 (held out) stays at 1.48x. About half of its summed yaw error (-3.3 of -5.9 deg) also appears in KISS-ICP on the same scans; the rest came from the fast section (few edges), which the KITTI edge threshold mostly fixed. | Look at seq 00 where the drift accumulates; run the remaining KITTI sequences. |
| MESH-LOAM | `approximately_reproduced` | Paper reimplementation (no author code) with the paper association and input | Near paper on KITTI Odometry 00/07 (paper path) | Same metric, 1.08x on 00/07 | No author code; the default association (smallest plane residual) and the missing elevation correction caused most of the earlier 1.80x. The global mesh (Eq. 3) does not fit in memory with marching-tetrahedra facets. | Run the paper path on the remaining KITTI sequences. |
| QUADRIC-LO | `indicative` | Paper reimplementation (no author code) | KITTI Odometry 00/07 against the paper's table | Same metric, 0.80x on 00/07 | Only seq 00 and 07 have been run. | Run the paper path on the remaining KITTI sequences. |
| DILO | `indicative` | Paper reimplementation (no author code) with a paper odometry path (plane key points only) | KITTI Odometry 00/07 with the paper pipeline (no edge key points) | Same metric, 1.31x on 00/07 vs full DiLO (plane key points only; 0.97x vs DiLO-plane) | Edge key points (Sec. 3.5) are not implemented; the remaining gap matches the paper ablation between DiLO-plane and DiLO. | Implement the edge key points and residual (eq. 17); run the remaining KITTI sequences. |

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
- **Current claim**: Near paper on KITTI Odometry 00/02/05/07/08 (paper pipeline). --suma-paper (rendered surfel model, projective frame-to-model ICP, surfel stability and fusion, official default.xml values, no loop closure). Official KITTI RTE: official defaults on raw scans 0.697 / 3.512 / 0.379 / 0.331 / 1.060 % on 00/02/05/07/08; normalised model lookup + 0.22 deg correction 0.884 / 0.784 / - / 0.521 / 1.179 %; paper 0.7 / 1.1 / 0.5 / 0.4 / 1.0 %. Best variant per sequence 0.86x; one setting for all 1.16x (official) or 1.08x (normalised, 4 sequences). The dense profile scored 1.60/1.78/1.28/1.10/2.03 %.
- **Numeric comparison**: Same metric, 0.86x on 00/02/05/07/08 (best of two variants per sequence). seq 00 0.697 vs 0.7 (1.00x), seq 02 0.784 vs 1.1 (0.71x), seq 05 0.379 vs 0.5 (0.76x), seq 07 0.331 vs 0.4 (0.83x), seq 08 1.060 vs 1.0 (1.06x). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: No single setting wins everywhere: with the official model lookup on raw scans the seq 02 corridors drift forward (weakly constrained x, ground normals biased by the elevation bowl); the normalised lookup with the correction fixes 02 but costs on 00/07/08. Official SuMa (GPU/OpenGL) still cannot run here for a direct check.
- **Next step**: Run the paper pipeline on the remaining KITTI sequences; check the official code on a GPU host.
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

## L-LO

- **Claim level**: `indicative`
- **Paper**: Li, Fu, Sun, L-LO: Enhancing Pose Estimation Precision via a Landmark-Based LiDAR Odometry, arXiv 2023
- **Method README**: `papers/l_lo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00, 02-10; 01 reported as NA)
- **Reported metric**: KITTI RTE translation [%] (official 100-800 m segments)
- **Repo scope**: Paper reimplementation (no author code). Frame-to-frame landmark odometry with convex-hull similarity and overlap maximisation; the paper gives no numeric parameters.
- **Current claim**: KITTI Odometry 00/02/05/07/08. Official KITTI RTE: 1.48/4.48/1.47/1.00/1.66 % on KITTI Odometry 00/02/05/07/08 versus the paper's 0.91/2.35/0.82/0.69/1.37 % (Table I, t_rel): 1.21-1.91x, geometric mean 1.58x. Seq 07 is the best 100 m RPE of the seq 07 sweeps (l_lo_kitti_seq_07_*_sweep_matrix.json, 30 settings); 00/02/05/08 use that choice unchanged, so those rows are held out. The untuned default gives 1.62x, so parameter choice moves the ratio by only ~3 %; see papers/l_lo/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Numeric comparison**: Same metric, ~1.6x ratio (parameters chosen on seq 07). Official KITTI RTE, repo vs paper: seq 00 1.479 vs 0.91 (1.63x), seq 02 4.478 vs 2.35 (1.91x), seq 05 1.473 vs 0.82 (1.80x), seq 07 1.004 vs 0.69 (1.46x), seq 08 1.661 vs 1.37 (1.21x). At least three landmark matches on every frame except 6 of 4660 on seq 02. Seq 07 is the best 100 m RPE of the seq 07 sweeps (l_lo_kitti_seq_07_*_sweep_matrix.json, 30 settings); 00/02/05/08 use that choice unchanged, so those rows are held out. The untuned default gives 1.62x, so parameter choice moves the ratio by only ~3 %; see papers/l_lo/README.md and docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: The paper specifies no thresholds and its pitch estimate is not physically consistent as written; a seq 07 parameter sweep closes only ~3 % of the gap, so the remaining difference is likely in the method details. See papers/l_lo/README.md.
- **Next step**: Ask the authors for code, or compare per-frame landmark matches against a reference implementation.
- **Notes**: Paper average 1.59 % over the reported sequences.

## ELO

- **Claim level**: `indicative`
- **Paper**: Zheng and Zhu, Efficient LiDAR Odometry for Autonomous Driving, IEEE RA-L 2021
- **Method README**: `papers/elo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (official KITTI t_rel (100-800 m))
- **Repo scope**: Paper reimplementation (no author code). From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: KITTI Odometry 00/07 with the correction and the paper normal patch. The dense profile with the 0.195 deg correction the paper applies and the eq. 12 normal patch read as the full patch size scores official KITTI RTE 0.656 / 0.480 % on 00 / 07 vs the paper's 0.54 / 0.31 % (1.21x / 1.55x, geometric mean 1.37x; the harness profile was 2.60x). The other paper mechanics as read here (adaptive fusion w = 0.7 w2, two-sided ground test, 0.1 m single-vertex BEV, raw input, plain Gauss-Newton) each make seq 07 worse; all together (--elo-paper-profile) score 1.638 / 1.111 %. See papers/elo/README.md.
- **Numeric comparison**: Same metric, 1.37x on 00/07. dense_elevation_paper_normals: seq 00 0.656 vs 0.54 (1.21x), seq 07 0.480 vs 0.31 (1.55x). Repo values: experiments/results/elo_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
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
- **Current claim**: Near paper on KITTI Odometry 00/07 (paper pipeline). --imls-slam-paper-odometry follows the paper: raw scans with the 0.22 deg beam correction, the last 100 raw scans as the model, h = 0.06 m / r = 0.20 m with a per-query normal, nine planarity-weighted lists of 100 samples, 20 iterations. Official KITTI RTE 0.523 / 0.339 % on 00 / 07 vs the paper's 0.50 / 0.33 % (1.05x / 1.03x, geometric mean 1.04x). The harness profile scores 1.378 / 0.846 %; without the beam correction the paper pipeline scores 1.327 / 0.706 %. See papers/imls_slam/README.md.
- **Numeric comparison**: Same metric, 1.04x on 00/07 (paper pipeline). paper_odometry_w100: seq 00 0.523 vs 0.50 (1.05x), seq 07 0.339 vs 0.33 (1.03x); paper_odometry_w10: 0.529 / 0.378 %. Repo values: experiments/results/imls_slam_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
- **Main blocker**: Small-object removal (paper Table II, 0.58 -> 0.55 %) is not implemented; only seq 00/07 have been run.
- **Next step**: Run the remaining KITTI sequences with paper_odometry_w10 (1.7 FPS) or w100 (1.1 FPS).
- **Notes**: Paper overall 0.55 % on the training set (0.69 % on the test set). Scans were de-skewed with an external odometry before matching; scan-to-model, no loop closure. Values read from the paper PDF on 2026-10-12.

## INTEN-LOAM

- **Claim level**: `indicative`
- **Paper**: Li et al., InTEn-LOAM: Intensity and Temporal Enhanced LiDAR Odometry and Mapping, Remote Sensing 2022 (arXiv 2209.05708)
- **Method README**: `papers/inten_loam/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI RTE (%))
- **Repo scope**: Paper reimplementation (no author code) with the paper pipeline. From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: Gap remains on KITTI Odometry 00/07 (paper pipeline). --inten-loam-paper-odometry follows the paper (FEF on the range image, S2S with re-association every iteration, S2M on a temporal-voxel-filtered keyframe map, DOR gating map insertion, B-spline intensity residual, 0.2 deg correction). Official KITTI RTE, no GT seed: seq 07 0.352 vs 0.34 (1.04x), seq 00 0.769 vs 0.52 (1.48x); geometric mean 1.24x. The plane map voxel (0.2 m) and the KITTI edge threshold (0.15, 4 per sector) were chosen on seq 07; seq 00 is held out. The earlier pipeline scored 42-48 % (it associated untransformed points once and removed ~40 % of the input in DOR).
- **Numeric comparison**: Same metric, 1.24x on 00/07 (07 1.04x, 00 held out 1.48x). paper_odometry: seq 00 0.769 vs 0.52 (1.48x); seq 07 0.352 vs 0.34 (1.04x). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: Seq 00 (held out) stays at 1.48x. About half of its summed yaw error (-3.3 of -5.9 deg) also appears in KISS-ICP on the same scans; the rest came from the fast section (few edges), which the KITTI edge threshold mostly fixed.
- **Next step**: Look at seq 00 where the drift accumulates; run the remaining KITTI sequences.
- **Notes**: Paper average 0.54 % (frame-weighted). Includes scan-to-local-map refinement, no loop closure. Values taken from arXiv v1, not checked against the published version. Values read from the paper PDF on 2026-10-12.

## MESH-LOAM

- **Claim level**: `approximately_reproduced`
- **Paper**: Zhu et al., Mesh-LOAM: Real-time Mesh-Based LiDAR Odometry and Mapping, IEEE T-IV 2024
- **Method README**: `papers/mesh_loam/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI relative translational error (%))
- **Repo scope**: Paper reimplementation (no author code) with the paper association and input. From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: Near paper on KITTI Odometry 00/07 (paper path). Official KITTI RTE on full sequences, no GT seed, paper path (--mesh-loam-paper-odometry: nearest-facet association, 0.1 m input, 0.22 deg correction): seq 00 0.567 vs 0.53 (1.07x); seq 07 0.340 vs 0.31 (1.10x); geometric mean 1.08x, from 1.80x with the dense profile. Repo values are the best variant of experiments/results/mesh_loam_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
- **Numeric comparison**: Same metric, 1.08x on 00/07. seq 00 0.567 vs 0.53 (1.07x); seq 07 0.340 vs 0.31 (1.10x). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: No author code; the default association (smallest plane residual) and the missing elevation correction caused most of the earlier 1.80x. The global mesh (Eq. 3) does not fit in memory with marching-tetrahedra facets.
- **Next step**: Run the paper path on the remaining KITTI sequences.
- **Notes**: Paper mean 0.51 %. The point-to-plane ablation row averages 0.71 %. Scan-to-mesh-map odometry, no loop closure. Values read from the paper PDF on 2026-10-12.

## QUADRIC-LO

- **Claim level**: `indicative`
- **Paper**: Xia et al., Quadric Representations for LiDAR Odometry, Mapping and Localization, IEEE RA-L 2023
- **Method README**: `papers/quadric_lo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI average translation error (%))
- **Repo scope**: Paper reimplementation (no author code). From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: KITTI Odometry 00/07 against the paper's table. Official KITTI RTE on full sequences, no GT seed: seq 00 0.966 vs 0.92 (1.05x); seq 07 0.662 vs 1.09 (0.61x); geometric mean 0.80x. Repo values are the best variant of experiments/results/quadric_lo_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
- **Numeric comparison**: Same metric, 0.80x on 00/07. seq 00 0.966 vs 0.92 (1.05x); seq 07 0.662 vs 1.09 (0.61x). See docs/assets/paper/paper_ratio_table.csv (Table 6).
- **Main blocker**: Only seq 00 and 07 have been run.
- **Next step**: Run the paper path on the remaining KITTI sequences.
- **Notes**: Table II (scan-to-scan plus scan-to-submap correction, average 1.25 %) is compared because the repository implementation matches against a local map. Table I (scan-to-scan only) gives 2.64/5.05/3.31/1.34/1.66/1.97/1.30/2.56/2.57/3.32/2.21 %. No loop closure. Values read from the paper PDF on 2026-10-12.

## DILO

- **Claim level**: `indicative`
- **Paper**: Han et al., DiLO: Direct light detection and ranging odometry based on spherical range images for autonomous driving, ETRI Journal 2021
- **Method README**: `papers/dilo/README.md`
- **Reported dataset**: KITTI Odometry (sequences 00-10)
- **Reported metric**: KITTI RTE translation [%] (KITTI average translation error (%), 100-800 m)
- **Repo scope**: Paper reimplementation (no author code) with a paper odometry path (plane key points only). From-paper reimplementation in the shared harness; see the method README.
- **Current claim**: KITTI Odometry 00/07 with the paper pipeline (no edge key points). --dilo-paper-odometry follows Sec. 3 without edge key points: 192x2048 sin-elevation range image with vertical fill, Farid-filter normals, eleven planarity-weighted key-point lists, the eq. 18 residual and Algorithm 1 (10-frame window, pairwise Tukey steps, log-map pose averaging). With the 0.22 deg beam correction it scores official KITTI RTE 0.797 / 0.579 % on 00 / 07: 0.95x / 1.00x the paper's plane-only ablation (DiLO-plane, 0.84 / 0.58 %) and 1.24x / 1.38x its full DiLO (0.64 / 0.42 %, the Table 6 value; geometric mean 1.31x, from 4.12x). Without the correction 1.308 / 0.619 %; without the window re-estimation 4.897 / 1.061 %. Two guards not in the paper (planar prediction, rejecting registrations far from it) were needed on KITTI. See papers/dilo/README.md.
- **Numeric comparison**: Same metric, 1.31x on 00/07 vs full DiLO (plane key points only; 0.97x vs DiLO-plane). paper_odometry_elevation: seq 00 0.797 vs 0.64 (1.24x), seq 07 0.579 vs 0.42 (1.38x); vs DiLO-plane 0.84 / 0.58. Repo values: experiments/results/dilo_kitti_seq_NN_full_paper_comp_matrix.json (Table 6).
- **Main blocker**: Edge key points (Sec. 3.5) are not implemented; the remaining gap matches the paper ablation between DiLO-plane and DiLO.
- **Next step**: Implement the edge key points and residual (eq. 17); run the remaining KITTI sequences.
- **Notes**: Paper average 0.69 % (frame-weighted). The table is printed rotated; values were rebuilt from word positions and match the stated average. No loop closure. Values read from the paper PDF on 2026-10-12.
