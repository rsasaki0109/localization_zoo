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

Related pages: [paper-ready reproducibility plan](paper_ready_reproducibility.md)
· [frozen evidence bundle](benchmarks/paper_ready_bundle.json)
· [reproduction status for established methods](reproduction_status.md)
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
[`docs/benchmarks/kitti_full_new_methods/`](benchmarks/kitti_full_new_methods/).
The current manuscript-facing seed bundle is
[`docs/benchmarks/paper_ready_bundle.json`](benchmarks/paper_ready_bundle.json),
and each module's README records its per-method deviations.

## Verdict summary

| Verdict | Methods |
|---|---|
| **Claim reproduces** | I-LOAM, KC-LO, M-GCLO, Quadric-LO, Adaptive-ICP, TrICP-LO (partial) |
| **Competitive, mechanism auxiliary** | DegenSense, D²-LIO, LODESTAR, Terrain-RBF-LIO, DALI-SLAM, DAMM-LOAM, CUBE-LIO, Intensity-Flow, ICPSC-LO, MCGICP-LO, SVN-ICP, Small-but-Mighty, NHC-LIO |
| **Near-redundant on KITTI** (mechanism correct, no effect) | MCC-LO, GMM-LO, Student-T-LO, GNC-LO, PCR-DAT |
| **Trade-off exposed** | LiDAR-IBA (ATE↓ / RPE↑), M-GCLO (RPE↓ / ATE↑), Spectral-LO (speed vs accuracy) |
| **Stable but below open baseline** | OPL-LVIO, AD-VLO, TC-MVLO, TC-LVGF, TC-VLO, V-LOAM2015, CT-VoxelMap, Vibration-LIO, BIEVR-LIO, RF-LIO, UA-LIO, DiLO |
| **Degrades / high drift** | IMLS-SLAM, R-VoxelMap, PL-LOAM, VLOM, InTEn-LOAM |

The full per-method numbers are in the README's from-paper table;
this report focuses on *why* each method lands where it does.

## Finding 1 — Several no-code papers beat the strongest open baselines

The headline positive result: **the top ten from-paper reimplementations match
or beat KISS-ICP on seq 00**, and all ten also beat it on seq 07.

- **DegenSense / D²-LIO** are now competitive no-IMU KITTI fallbacks
  (0.811-0.814 % on seq 00, 0.541-0.558 % on seq 07). The key fix was
  methodological: degeneracy sensing is still reported, but compensation or
  directional regularization is applied only when a real IMU packet is present.
  The previous constant-velocity fallback was not an inertial reference and
  over-constrained weak directions.
- **I-LOAM** (UR 2020) is the cleanest reproduction in the repository. Its
  central claim — injecting LiDAR reflectance into LOAM's correspondence and
  residual weighting cuts drift — survives a controlled ablation: running the
  *identical* pipeline with intensity on vs. off yields **−18.2 % drift on
  seq 00 and −19.7 % on seq 07**, even though KITTI intensity is uncalibrated.
  The paired raw artifacts are committed in
  [`i_loam_intensity_ablation.json`](benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json).
- **KC-LO** (ECCV 2004): correspondence-free kernel correlation leads seq 07
  (0.510 %) and beats KISS-ICP on both sequences. The sigma-schedule ablation
  shows that fixed σ=0.4 m keeps RPE within 1 % of the annealed profile while
  improving throughput from ~1.4 FPS to 2.6-3.1 FPS; annealing is a convergence
  safety knob here, not the main positive signal.
- **M-GCLO** (ISPRS Ann. 2024) leads the paper-mechanism seq 00 row
  (0.835 %) via multiple ground-plane constraints. The committed ground-factor
  ablation shows the mechanism is mostly an anchoring/stability term under this
  KITTI protocol: disabling ground keeps translational RPE similar or lower
  (0.833 % seq 00, 0.600 % seq 07), but ATE more than doubles on both
  sequences and rotational drift worsens.
  A dataset-free rolling-ground stress now exercises the intended non-flat
  regime: M-GCLO tracks the 60-frame trajectory at **0.116 m** ATE and
  **0.500 %** drift with ground constraints, while disabling ground worsens to
  **0.150 m** and **0.675 %**. KISS-ICP is intentionally a sanity/failure
  reference on this repeated synthetic geometry (22.159 m ATE). Artifact:
  [`m_gclo_nonflat_ground_stress_summary.json`](benchmarks/nonflat_ground_stress/m_gclo_nonflat_ground_stress_summary.json).
- **Quadric-LO** (arXiv 2023): point-to-quadric residuals beat KISS-ICP on both
  sequences. The committed plane-fallback ablation shows fallback is rare on
  KITTI (about 0.5-0.6 % of used correspondences); disabling it keeps RPE within
  ~1.5 % while improving throughput by 1.6-1.8x. The orchard-like synthetic
  curved-object stress makes the same mechanism boundary explicit: fallback-on
  uses about **1242 quadric** vs **12 plane** correspondences/frame, and
  disabling fallback keeps drift effectively unchanged (**0.612 % → 0.609 %**).
  Artifact:
  [`quadric_curved_stress_summary.json`](benchmarks/quadric_curved_stress/quadric_curved_stress_summary.json).
  This is still supporting synthetic evidence; public curved-object or
  non-urban validation is needed before a T0 claim.

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
drift on KITTI seq 00/07. PL-LOAM and VLOM both improve after rendering LiDAR
intensity instead of raw depth gradients into their pseudo-images, but they
still land at **90.10 % / 87.39 %** and **89.15 % / 81.60 %** drift,
respectively. VLOM's seq 07 catastrophe is fixed, while seq 00 ATE gets worse;
the caveat is equally clear: range-image visual proxies and line residuals
remain auxiliary to the scan-to-map point-to-plane core, so they still trail the
strongest open LiDAR-only baseline on this benchmark.

RF-LIO adds the same lesson for dynamic removal: adaptive multi-resolution
range-image foreground removal works mechanically and removes about **246-273
points/frame**, but on mostly static KITTI it degrades drift to **1.35 % / 1.27
%** versus the delayed-removal ID-LIO baseline. The mechanism is designed for
high-dynamic scenes, not for this static urban benchmark.

DiLO is now stable rather than a degradation case after adding bounded 1-pixel
projective lookup in the spherical range image. It runs at **1.20 % / 1.53 %**
RPE on KITTI seq 00/07 while staying fast (~65 FPS), but still trails the best
scan-to-map rows because it remains a direct frame-to-keyframe front-end rather
than a persistent local-map estimator.

The committed synthetic dynamic-object stress makes that caveat reproducible
without external data. `evaluation/scripts/run_dynamic_object_stress.py`
generates clean and crossing-object fixtures, then writes
[`rf_id_lio_dynamic_object_stress_summary.json`](benchmarks/dynamic_object_stress/rf_id_lio_dynamic_object_stress_summary.json).
On the 30-frame stress, ID-LIO degrades from **0.676 m** clean ATE to
**130.549 m** with moving foreground boxes; default RF-LIO degrades from
**2.487 m** to **49.932 m**, and a conservative removal cap improves RF-LIO to
**41.632 m**. This is a mechanism stress and failure-boundary artifact, not a
replacement for a public high-dynamic dataset.

## Finding 3 — A third of the methods degrade or diverge, reproducibly

Honest negatives, kept in the leaderboard rather than dropped:

- **InTEn-LOAM** (~53–67 % drift): enabling its mapping stage *increases*
  seq 00 drift from 52.7 % to 68.4 % in this port.
- **PL-LOAM / VLOM** (LiDAR-visual, ~82–90 % drift): on KITTI Odometry there is
  no RGB. LiDAR-intensity pseudo-images are much better than the earlier
  depth-gradient front-ends: PL-LOAM drops from ~117-143 % to
  **87.39-90.10 %**, and VLOM drops from **91.46 / 153.87 %** to
  **89.15 / 81.60 %**. They remain degradation cases; VLOM's seq 00 ATE even
  worsens from 249 m to 315 m while RPE improves. Crucially, rerunning the
  visual front-end on KITTI Raw *with real RGB* still yields ~99 % drift — the
  simplified tracker, not only the missing camera, is the bottleneck.
  Reproducing these papers requires the full ORB-SLAM2-class stack they build
  on.
- **Spectral-LO** (~10 %): FFT phase-correlation odometry runs at ~25–27 FPS
  with zero divergence — honest fast-but-coarse; the tighter 40 m BEV window
  improves RPE, while absolute ATE remains higher than scan-to-map methods.
- **UA-LIO** no longer diverges under strict exact-frame association
  (1.13% / 0.97% RPE on seq00/07), but this KITTI run has no IMU packets, so it
  is only evidence for the D2D + ground scan-to-map fallback, not the full LIO
  state estimator.
- **R-VoxelMap** no longer diverges on seq 07 after low-correspondence recovery
  falls back to scan-to-scan ICP, but it remains a degradation case
  (**58.33 % / 35.81 %** RPE). **IMLS-SLAM** on a voxelized map degenerates to
  plain point-to-plane (1.0 %) — the implicit surface needs the native point
  density the paper assumes.

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
5. **Ground/terrain constraints trade local drift, attitude, and ATE.**
   M-GCLO's ground-factor ablation shows that KITTI translational RPE alone can
   miss the mechanism: ground off is similar or lower in translational RPE, but
   worse in ATE and rotational drift. LiDAR-IBA's FEJ shows a related split.
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
