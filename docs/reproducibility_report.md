# Reproducibility Report — 42 LiDAR odometry papers with no public code

**What happens when you reimplement more than forty LiDAR odometry / LIO papers that
ship no reference implementation, and run them all under one protocol?**

This report summarizes the honest outcome of that experiment: every method
below was implemented from the paper text alone (no author code exists),
integrated behind this repository's shared C++ API, and run as pure odometry on
full KITTI Odometry sequences with a uniform protocol. The outcome: **about
half produce competitive odometry — but only a handful show their *claimed
mechanism* actually mattering; a third degrade or diverge; and the rest are
correct mechanisms that go silent on this benchmark.** The pattern of *which*
bucket each method lands in is itself the most useful finding.

Related pages: [reproduction status for established methods](reproduction_status.md)
· [experiment log](experiments.md) · [README leaderboard](../README.md).

## Protocol

- **Data**: KITTI Odometry full sequences 00 (4541 frames) and 07 (1101
  frames); selected methods also on KITTI Raw, MulRan, MCD, NCLT, HDL-400.
- **Odometry only**: first-pose anchor, `--no-gt-seed` (no ground-truth pose is
  ever fed to the estimator), uniform `--*-dense-profile` preprocessing.
- **No IMU**: KITTI Odometry ships no IMU, so LIO methods run with a
  constant-velocity fallback replacing inertial prediction. This is a real
  protocol deviation and is accounted for in the verdicts below.
- **Metrics**: RPE as drift %/100 m (seed-independent local accuracy, the
  primary ranking metric) and ATE in meters (unbounded whole-run drift).
- **References**: KISS-ICP and CT-ICP compact baselines run under the *same*
  profile (KISS-ICP: 0.872 % seq 00 / 0.618 % seq 07).

Raw JSON for every run lives under
[`docs/benchmarks/kitti_full_new_methods/`](benchmarks/kitti_full_new_methods/),
and each module's README records its per-method deviations.

## Verdict summary

| Verdict | Methods |
|---|---|
| **Claim reproduces** | I-LOAM, KC-LO, M-GCLO, Quadric-LO, Adaptive-ICP, TrICP-LO (partial) |
| **Competitive, mechanism auxiliary** | LODESTAR, Terrain-RBF-LIO, DALI-SLAM, DAMM-LOAM, CUBE-LIO, Intensity-Flow, ICPSC-LO, MCGICP-LO, SVN-ICP, Small-but-Mighty, NHC-LIO |
| **Near-redundant on KITTI** (mechanism correct, no effect) | MCC-LO, GMM-LO, Student-T-LO, GNC-LO, PCR-DAT |
| **Trade-off exposed** | LiDAR-IBA (ATE↓ / RPE↑), M-GCLO (RPE↓ / ATE↑), Spectral-LO (speed vs accuracy) |
| **Stable but below open baseline** | OPL-LVIO, AD-VLO, TC-MVLO, TC-LVGF, TC-VLO, V-LOAM2015, CT-VoxelMap, Vibration-LIO, BIEVR-LIO, RF-LIO |
| **Degrades or diverges** | IMLS-SLAM, R-VoxelMap, D²-LIO, DegenSense, DiLO, PL-LOAM, VLOM, InTEn-LOAM, UA-LIO |

The full per-method numbers are in the README's from-paper table;
this report focuses on *why* each method lands where it does.

## Finding 1 — Several no-code papers beat the strongest open baselines

The headline positive result: **the top ten from-paper reimplementations match
or beat KISS-ICP on seq 00**, and most also beat it on seq 07.

- **I-LOAM** (UR 2020) is the cleanest reproduction in the repository. Its
  central claim — injecting LiDAR reflectance into LOAM's correspondence and
  residual weighting cuts drift — survives a controlled ablation: running the
  *identical* pipeline with intensity on vs. off yields **−18.2 % drift on
  seq 00 and −19.7 % on seq 07**, even though KITTI intensity is uncalibrated.
- **KC-LO** (ECCV 2004): correspondence-free kernel correlation with σ
  annealing leads seq 07 (0.514 %) and beats KISS-ICP on both sequences — at
  ~1.4 FPS, an honest 8× throughput cost.
- **M-GCLO** (ISPRS Ann. 2024) leads seq 00 drift (0.835 %) via multiple
  ground-plane constraints, with the predicted trade-off: local drift drops
  while absolute trajectory error grows (19 m ATE) — ground constraints anchor
  the local frame, not the global one.
- **Quadric-LO** (arXiv 2023): point-to-quadric residuals beat KISS-ICP on both
  sequences, but quadric fitting is expensive (0.62 FPS) and on plane-dominated
  KITTI most surfels fall back to planes anyway.

## Finding 2 — On geometry-rich, IMU-free KITTI, robust estimation goes silent

The most consistent cross-method result: **soft-assignment and robust-kernel
mechanisms are near-redundant on well-conditioned KITTI**. After voxel/range
gating with a constant-velocity prediction, few heavy-tail outliers remain, so:

- **MCC-LO** (correntropy/Welsch weights): adaptive bandwidth collapses to its
  floor; mean weight ≈ 0.90 → effectively Gaussian. Still the best of the
  robust family (0.892 % seq 00) — but not *because* of correntropy.
- **GMM-LO** (EM soft assignment + annealing): anneals to mean weight ≈ 0.98 —
  effectively hard ICP.
- **Student-T-LO** (heavy-tail IRLS): mechanism unit-tested and correct;
  mean weight ≈ 0.80 with nothing to downweight.
- **GNC-LO** (graduated non-convexity): hard rejection discards ~14 % of
  matches that were mostly fine — slightly *worse* than the soft variants.
- **PCR-DAT** (density-adaptive factor switching): KITTI's uniform density
  rarely triggers the switch.

The same pattern holds for constraint-style mechanisms: **NHC-LIO**'s
nonholonomic factor and **Terrain-RBF-LIO**'s terrain prior are implemented and
verified, but urban KITTI geometry already constrains those directions —
ablations show near-zero effect. These mechanisms are built for sparse, noisy,
or degenerate regimes that KITTI does not exercise; the benchmark cannot
falsify them, only report their silence.

The LiDAR-visual adapter wave (**OPL-LVIO**, **AD-VLO**, **TC-MVLO**,
**TC-LVGF**, **TC-VLO**, **V-LOAM2015**) is more positive than the older
pseudo-image PL-LOAM/VLOM attempts: all six run stably around **0.90-1.07 %**
drift on KITTI seq 00/07. The caveat is equally clear: range-image visual
proxies and line residuals remain auxiliary to the scan-to-map point-to-plane
core, so they still trail the strongest open LiDAR-only baseline on this
benchmark.

RF-LIO adds the same lesson for dynamic removal: adaptive multi-resolution
range-image foreground removal works mechanically and removes about **246-273
points/frame**, but on mostly static KITTI it degrades drift to **1.35 % / 1.27
%** versus the delayed-removal ID-LIO baseline. The mechanism is designed for
high-dynamic scenes, not for this static urban benchmark.

## Finding 3 — A third of the methods degrade or diverge, reproducibly

Honest negatives, kept in the leaderboard rather than dropped:

- **InTEn-LOAM** (~53–67 % drift): enabling its mapping stage *increases*
  seq 00 drift from 52.7 % to 68.4 % in this port.
- **PL-LOAM / VLOM** (LiDAR-visual, ~91–154 % drift): on KITTI Odometry there
  is no RGB, and a depth-gradient pseudo-image cannot feed a visual front-end.
  Crucially, rerunning both on KITTI Raw *with real RGB* still yields ~99 %
  drift — the simplified visual tracker, not the missing camera, is the
  bottleneck. Reproducing these papers requires the full ORB-SLAM2-class
  stack they build on.
- **DiLO** (18–19 %): direct projective alignment is the fastest method in the
  repo (25–32 FPS) but frame-to-keyframe composition accumulates drift that
  scan-to-map methods do not.
- **Spectral-LO** (12–14 %): FFT phase-correlation odometry runs at ~14 FPS
  with zero divergence — honest fast-but-coarse; BEV cell quantization
  (~0.47 m) is the structural floor.
- **DegenSense** (9.9 %): degeneracy *compensation* actively hurts when the
  scene is not degenerate and the IMU half of the design is absent.
- **D²-LIO** (5.8 % seq 00 vs 0.8 % seq 07): adaptive gating constants that the
  paper leaves unspecified do not transfer across sequences.
- **R-VoxelMap** diverges on seq 07; **UA-LIO** diverges on both; **IMLS-SLAM**
  on a voxelized map degenerates to plain point-to-plane (1.0 %) — the implicit
  surface needs the native point density the paper assumes.

## Finding 4 — Recurring implementation lessons

Patterns that repeated across independent implementations; each cost at least
one divergence before being learned:

1. **Degeneracy thresholds must be relative.** Absolute eigenvalue/condition
   thresholds from papers do not transfer to other sensors or preprocessing —
   every port that kept them diverged. Translation-block-relative criteria
   transfer (LODESTAR, DALI-SLAM, DAMM-LOAM all converged on this).
2. **Motion compensation without an IMU can hurt.** Constant-velocity azimuth
   deskew amplified range error on KITTI (DALI-SLAM ships with deskew off by
   default in this repo).
3. **Adaptive kernels need floors.** MCC-LO's Silverman bandwidth shrinks past
   the convergence signal itself and stalls registration; clamping at 0.3 m
   fixed it — at the cost of disabling the very adaptivity being tested.
4. **Filter-style estimators need a separated point estimate.** Particle-mean
   estimates drift from the mode over thousands of frames; SVN-ICP stays
   healthy by extracting a Gauss-Newton MAP estimate and using the particle
   spread only for uncertainty (which does correctly grow in corridors).
5. **Ground/terrain constraints trade RPE for ATE.** They suppress local
   z-drift and can worsen global trajectory error (M-GCLO, LiDAR-IBA's FEJ
   shows the same split).
6. **Intensity helps as a weight, not as a metric.** I-LOAM's
   correspondence-level injection reproduces (−18–20 %); fusing intensity into
   the distance metric is at best weakly effective (MCGICP-LO, ICPSC-LO learns
   a geometry weight of ≈ 0.95).

## Threats to validity

- **One benchmark family dominates.** Most verdicts rest on KITTI seq 00/07 —
  geometry-rich, flat, urban, IMU-free. "Near-redundant on KITTI" is not
  "useless"; several silent mechanisms target exactly the regimes KITTI lacks.
- **LIO methods run degraded.** Constant-velocity fallback replaces inertial
  prediction; verdicts for *-LIO methods describe the LiDAR residual design,
  not the full system.
- **Ports are compact.** Backends (pose graphs, loop closure) and some
  components (RANSAC → least-squares fits, CNN predictors → analytic proxies)
  are simplified; per-module READMEs list every deviation.
- **Unspecified constants.** Where papers omit constants, repo defaults fill
  in; D²-LIO shows how much that can matter.
- **Single machine, single run.** FPS numbers are relative, not absolute.

## Takeaway

Papers without code *can* be reproduced — and a useful minority beat strong
open baselines once reimplemented. But the expected value is asymmetric: the
most common outcome is a correct mechanism whose benefit does not survive
contact with a well-conditioned benchmark, and the second most common is
divergence caused by untransferable constants. A shared harness that keeps
honest negatives on the leaderboard is what makes those two outcomes visible
at all — single-method repositories structurally cannot report them.

---

*Every number in this report is regenerable: module-level READMEs under
[`papers/`](../papers/) document per-method flags, and
[`evaluation/scripts/`](../evaluation/scripts/) contains the KITTI preparation
and benchmark runners. Corrections welcome — especially from the original
authors of any paper listed here.*
