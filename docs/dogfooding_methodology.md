# Dogfooding Tool Methodology

This page documents methodology asymmetries inside the `pcd_dogfooding` evaluator that affect cross-method comparison. They are not algorithm bugs - they are deliberate per-method initialization choices, but they need to be visible to anyone reading the aggregate numbers.

## GT-seeded vs pure-odometry initialization

Inside `evaluation/src/pcd_dogfooding.cpp`, two distinct initialization policies are in use:

### Policy A: GT-seeded scan-to-map (default ON, toggleable via `--no-gt-seed`)

For every frame `i >= 1`, the registration is initialized with the **ground-truth pose** for that frame:

```cpp
const Eigen::Matrix4d T_init_guess = no_gt_seed ? T_est : gt[i];
```

ICP then refines locally around that prior. A "weak update" fallback (`isReasonableRefinement`) reverts to `T_est` when the local refinement looks unreasonable.

Methods using Policy A:

- `LiTAMIN2`
- `GICP`
- `small_gicp`
- `voxel_gicp`
- `NDT`
- `KISS-ICP`

**What this measures**: local scan-registration quality on top of a GT prior, not pure odometry drift. The reported ATE/RPE on long sequences therefore looks much cleaner than the same algorithm would produce as standalone odometry.

To get pure-odometry behavior from these six, pass `--no-gt-seed` on the CLI.

### Policy B: Pure odometry (no GT seed beyond first frame)

Every other tracked method anchors at `gt[0]` for the first frame and from then on relies on its own motion prediction. Examples:

- `CT-ICP` (no `no_gt_seed` parameter at all - always pure odometry)
- `ALOAM`, `BALM2`, `CLINS`, `CT-LIO`, `DLIO`, `DLO`, `FAST-LIO2`, `FAST-LIO-SLAM`
- `FLOAM`, `HDL-Graph-SLAM`, `ISC-LOAM`, `LeGO-LOAM`, `LINS`, `LIO-SAM`
- `LOAM-Livox`, `MULLS`, `Point-LIO`, `SuMa`, `VGICP-SLAM`, `X-ICP`

Per-variant notes in the aggregate JSON say "no GT seed" or "anchor matches first GT pose" for these.

**What this measures**: standalone odometry drift over the full sequence - which is what most paper-reported numbers also measure.

## Practical implications

1. **Cross-method comparison on long sequences is currently unfair.** A Policy A method on a 400 m trajectory effectively gets one ICP refinement per scan with a perfect prior; a Policy B method has to track from scratch and accumulates error. This is the direct explanation for:
   - `LiTAMIN2` MulRan parkinglot full: ATE 0.71 m / RPE 1.26 % (Policy A).
   - `CT-ICP` MulRan parkinglot full: ATE 75-80 m / RPE >100 % (Policy B).
   The sequence is the same; the registration algorithms are not what's being compared.

2. **Cross-method comparison on short sequences (≲ 30 m) is dominated by other factors.** The GT-prior advantage shrinks because there is less room to accumulate drift either way.

3. **Paper-reproduction claims should use Policy B uniformly.** Most paper-reported numbers (LiTAMIN2, CT-ICP, KISS-ICP, etc.) are standalone-odometry results. To compare honestly, runs intended as paper reproduction should pass `--no-gt-seed` to the Policy A methods.

4. **Within-method comparison is unaffected.** Comparing LiTAMIN2 profiles against each other, or CT-ICP profiles against each other, is internally consistent regardless of which policy is in use.

## Empirical confirmation (2026-05-17)

Pending manifests with `--no-gt-seed` were first run with the original previous-pose initial guess (`T_init_guess = T_est`):

| Manifest | Variant | ATE [m] | RPE trans [%] | FPS |
|----------|---------|--------:|--------------:|----:|
| `experiments/pending/litamin2_kitti_raw_0009_full_no_gt_seed_matrix.json` | `fast_cov_no_gt_seed` | 171.4 | 84.8 | 58.5 |
| `experiments/pending/litamin2_mulran_parkinglot_full_no_gt_seed_matrix.json` | `fast_cov_no_gt_seed` | 78.0 | 100.7 | 117.4 |

For reference, the same data with Policy A (GT-seeded) gave LiTAMIN2 RPE 0.74 % / 1.26 % respectively.

When the GT prior was removed with previous-pose-as-init the trajectories collapsed: KITTI Raw 0009 full drifted to ATE 171 m / RPE 85 % (a 230x degradation), MulRan parkinglot full to ATE 78 m / RPE 101 %. This was **not** evidence that LiTAMIN2 is a poor algorithm. It was evidence that the original `--no-gt-seed` code path was doing "scan-to-map with previous-pose-as-init plus a 2 m / 0.25 rad refinement-acceptance gate" - which is closer to a scan-registration probe than to standalone odometry. When the gate rejected a refinement, `T_est = T_init_guess` (the previous pose), so the trajectory stagnated and drift accumulated linearly.

## Velocity-model prior fix (2026-05-18)

The fix landed in `evaluation/src/pcd_dogfooding.cpp`: the `--no-gt-seed` initial guess for every Policy A method is now a body-frame constant-velocity prediction `T_pred = T_prev * (T_prev_prev^-1 * T_prev)` instead of the static previous pose. `velocityModelPrediction()` is shared by `runLiTAMIN2`, `runGICP`, `runSmallGICP`, `runVoxelGICP`, `runNDT`, and `runXICP`. The refinement-acceptance gate now compares against a moving prior, so normal KITTI inter-frame motion (~1 m/frame) no longer triggers rejection.

Re-running the same manifests (build 2026-05-18, velocity-model active):

| Manifest | Variant | ATE [m] | RPE trans [%] | FPS |
|----------|---------|--------:|--------------:|----:|
| KITTI Raw 0009 full | `fast_cov_no_gt_seed` | **7.45** | **4.85** | 88.2 |
| KITTI Raw 0009 full | `paper_cov_no_gt_seed` | 167.17 | 73.74 | 114.4 |
| MulRan parkinglot full | `fast_cov_no_gt_seed` | 74.40 | 103.19 | 115.4 |
| MulRan parkinglot full | `paper_cov_no_gt_seed` | 79.97 | 106.49 | 119.7 |

Interpretation:

- **KITTI Raw 0009 `fast`**: 171 m → 7.45 m ATE (23x), 85 % → 4.85 % RPE (17x). This is genuine pure-odometry behavior. The 4.85 % RPE is higher than the paper's 0.65-1.42 % range on KITTI Odometry 00-10 because (a) the fast dogfooding profile is not the paper-faithful 3 m profile, and (b) Raw 0009 is not in the paper's evaluated sequence list. The gate-stagnation pathology is gone.
- **KITTI Raw 0009 `paper`**: 3 m voxel resolution with a 25k-point local map is too coarse for pure odometry on this trajectory - the alignment itself fails, independent of the gate. The velocity-model fix does not help here because the failure mode is alignment quality, not gate rejection. This is consistent with how the paper-faithful profile was tuned for KITTI Odometry trajectory characteristics, not Raw 0009.
- **MulRan parkinglot full**: 78 m → 74 m ATE, essentially unchanged. The previous-pose vs velocity-model distinction is small at low driving speeds (parking lot), so the gate was not the dominant problem. The 74 m residual is genuine pure-odometry drift on a low-feature, slow-motion sequence; CT-ICP shows ~80 m on the same trajectory, matching within ~10 %.

The repo's Policy A numbers therefore still overstate what these methods do as standalone odometry. But the `--no-gt-seed` path now produces honest pure-odometry numbers on benign sequences (KITTI fast profile), and exposes alignment-quality limits separately from gate-stagnation artefacts.

## Cross-method `--no-gt-seed` sweep (2026-05-18)

The velocity-model fix was applied uniformly to all six Policy A methods. To verify it generalizes beyond LiTAMIN2, the full set was rerun on KITTI Raw 0009 full (443 frames, ~430 m) with `--no-gt-seed`:

| Method / variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|------------------|--------:|--------------:|----------------:|----:|
| GICP `fast_no_gt_seed` | **4.82** | **4.70** | 0.026 | 26.8 |
| GICP `balanced_no_gt_seed` | 5.11 | 4.83 | 0.026 | 15.7 |
| Small-GICP `default_no_gt_seed` | 5.47 | 5.15 | 0.030 | 44.0 |
| LiTAMIN2 `fast_cov_no_gt_seed` | 7.45 | 4.85 | 0.024 | 88.2 |
| LiTAMIN2 `fast_icp_only_no_gt_seed` | 7.45 | 4.85 | 0.024 | 87.6 |
| Voxel-GICP `default_no_gt_seed` | 41.43 | 9.88 | 0.088 | 14.0 |
| X-ICP `default_no_gt_seed` | 147.92 | 138.96 | 1.143 | 42.9 |
| NDT `default_no_gt_seed` | 310.46 | 41.53 | 0.424 | 17.9 |
| LiTAMIN2 `paper_cov_no_gt_seed` | 167.17 | 73.74 | 0.053 | 114.4 |
| LiTAMIN2 `paper_icp_only_no_gt_seed` | 167.17 | 73.74 | 0.053 | 119.9 |

Interpretation:

- **Five variants converge (RPE 4-5 %)**: GICP fast/balanced, Small-GICP, LiTAMIN2 fast cov/icp-only. The velocity-model fix is working as intended for the point-to-point/plane registration family on a benign driving sequence. Paper-reported RPE for LiTAMIN2 on KITTI Odometry 00-10 is 0.40-1.42 %; the dogfooding numbers sit ~3-5x higher because the dogfooding profiles are tuned for throughput, not paper faithfulness, and Raw 0009 is not in the paper's evaluated set.
- **Voxel-GICP partial drift (41 m / 9.9 %)**: converges but with larger drift than its non-voxel counterparts. The fixed voxel grid may be losing spatial resolution at the edges of the local map.
- **NDT, X-ICP, LiTAMIN2 paper profile diverge**: these are not gate-stagnation cases (the velocity-model fix removed that pathology). They are alignment-quality cases: NDT's density-based objective and X-ICP's regularized cost both rely on a finer prior than velocity-model provides; LiTAMIN2's 3 m paper voxel is too coarse for Raw 0009. These divergences are honest pure-odometry behavior with the current dogfooding parameters - they tell us the *algorithms*, not the tool, struggle without a tighter prior.
- **GICP edges out LiTAMIN2 in this regime (4.82 m vs 7.45 m ATE)**: this inverts the Policy A ranking, where LiTAMIN2 looks better because of its gate-fallback safety net. In honest pure-odometry mode, GICP and Small-GICP win on accuracy while LiTAMIN2 wins on throughput.

## Gap-closing tuning sweep (2026-05-18)

Applied parameter tuning derived from KITTI 07 to the full paper-evaluated set (00/02/05/07/08).

**LiTAMIN2 winning tune: `--litamin2-voxel-resolution 1.0 --litamin2-max-iterations 12 --no-gt-seed`**

| Seq | Paper RPE | Baseline | Baseline ratio | Tuned | Tuned ratio |
|----:|----------:|---------:|---------------:|------:|------------:|
|  00 | 0.65 % | 3.08 % | 4.7x | **0.92 %** | **1.4x** |
|  02 | 0.83 % | 5.08 % | 6.1x | **0.98 %** | **1.2x** |
|  05 | 0.40 % | 0.97 % | 2.4x | 0.63 % | 1.6x |
|  07 | 0.42 % | 0.72 % | 1.7x | 0.54 % | 1.3x |
|  08 | 0.96 % | 7.25 % | 7.6x | **1.30 %** | **1.3x** |
| **Geom. mean** | | | **3.91x** | | **1.35x** |

LiTAMIN2 now sits inside the paper's reported RPE range (0.40-1.42 %) on every measured sequence. The remaining ~35 % overhead vs paper is consistent across long and short sequences, suggesting it is a stable per-frame accuracy ceiling tied to the specific dogfooding implementation rather than a drift integration effect. Claim level for `litamin2` advances to **`approximately_reproduced`** with paper-range numbers.

**CT-ICP winning tune: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20`**

| Seq | Paper RPE | Baseline | Baseline ratio | Tuned-v1 (dense+budget) | Tuned-v1 ratio | Tuned-v2 (voxel 1.0) | Tuned-v2 ratio |
|----:|----------:|---------:|---------------:|------------------------:|---------------:|---------------------:|---------------:|
|  00 | 0.52 % | 2.76 % | 5.3x | 2.20 % | 4.2x | 2.14 % | 4.1x |
|  02 | 0.58 % | 3.21 % | 5.5x | **3.35 %** | **5.8x** | 4.95 % | 8.5x |
|  05 | 0.31 % | 1.60 % | 5.2x | 1.28 % | 4.1x | 1.38 % | 4.4x |
|  07 | 0.30 % | 2.70 % | 9.0x | 2.14 % | 7.1x | 1.90 % | 6.3x |
|  08 | 0.75 % | 2.63 % | 3.5x | 2.10 % | 2.8x | 2.02 % | 2.7x |
| **Geom. mean** | | | **5.44x** | | **4.58x** | | **4.84x** |

CT-ICP gap is sticky at ~4.5x even under aggressive parameter tuning. Tuned-v2 (voxel 1.0 + iter 12 + ceres 6 + map 30) wins on KITTI 07 in isolation but generalizes worse than tuned-v1 - it diverges to 8.5x on the highway-like seq 02 where finer voxels lose features. Tuned-v1 (the existing `--ct-icp-dense-profile` plus extra ceres budget and longer map memory) is the more robust default. The remaining ~4.5x gap probably needs architectural attention to the continuous-time optimization rather than more parameters.

### CT-ICP architecture-level tuning attempt (2026-05-18)

Two algorithm-level avenues were attempted to close past the 4.5x parameter ceiling:

**1. Velocity-model initial guess for `runCTICP`.** Replaced the static `init.begin = init.end = prev.end_pose` with continuity at begin plus a body-frame velocity extrapolation at end (`init.end = prev.end * prev.begin.inverse() * prev.end`). Identical to the velocity-model fix that worked for Policy A. Result: catastrophic divergence at frame ~40 of KITTI 07 (first sharp turn). Unlike LiTAMIN2's Policy A path, CT-ICP has no refinement-acceptance gate, so a single bad extrapolation propagates and `findCorrespondences` returns empty for the rest of the trajectory. Reverted.

**2. New CLI flags + architecture sweep on KITTI 07.** Exposed `--ct-icp-max-correspondence-distance` (algorithm default 100 m² = 10 m linear; paper-style is 1-2 m²) and `--ct-icp-knn` for tuning. Swept on KITTI 07:

| Variant | RPE [%] | Ratio |
|--------|--------:|------:|
| prior_best (dense + ceres 6 + map 20) | 2.14 | 7.1x |
| max_corr 2.0 (tight) | 2.67 | 8.9x (worse) |
| max_corr 4.0 (moderate) | 2.03 | 6.8x |
| keypoint 0.5 (more keypoints) | 2.25 | 7.5x |
| planarity 0.5 (strict planar) | 2.81 | 9.4x (worse) |
| all_combined (max_corr 4, keypoint 0.5, ceres 8, map 30) | 1.86 | **6.2x** |

On KITTI 07 `all_combined` wins at 6.2x. Generalizing to 00/02/05/08:

| Seq | Paper | Baseline | Tune-v1 | Tune-v2 | Arch | Best |
|----:|------:|---------:|--------:|--------:|-----:|-----:|
|  00 | 0.52 | 5.3x | 4.2x | 4.1x | 5.8x | tune-v1/v2 |
|  02 | 0.58 | 5.5x | 5.8x | 8.5x | 8.1x | baseline |
|  05 | 0.31 | 5.2x | 4.1x | 4.4x | 3.6x | arch |
|  07 | 0.30 | 9.0x | 7.1x | 6.3x | 6.2x | arch |
|  08 | 0.75 | 3.5x | 2.8x | 2.7x | 2.6x | arch |
| **Geom. mean** | | **5.44x** | **4.58x** | **4.84x** | **4.88x** | |

Arch tune (all_combined) wins on 05/07/08 but loses on 00/02 (long sequences with sparser features where finer voxels and tighter correspondences hurt). **Tune-v1 (`--ct-icp-dense-profile + --ct-icp-ceres-max-iterations 6 + --ct-icp-max-frames-in-map 20`) remains the most robust single-config default at 4.58x.**

**Conclusion: ~4.5x is the parameter-tuning ceiling for this repo's CT-ICP.** Closing further requires algorithm-level changes - specifically: a refinement-acceptance gate analogous to LiTAMIN2's so that velocity-model bootstrap is safe; or rewriting `findCorrespondences` to handle the multi-scale neighborhood the paper uses; or matching the paper's Ceres setup more faithfully (DENSE_NORMAL_CHOLESKY vs DENSE_QR, different LM trust region defaults). These are out of scope for parameter tuning.

### CT-ICP architecture-2: multi-scale + DENSE_NORMAL_CHOLESKY (2026-05-18)

Algorithm-level follow-up implementing all three avenues from the previous section:

1. **Multi-scale `findCorrespondences`** (`papers/ct_icp/src/ct_icp_registration.cpp`): fall back from 3×3×3 (27 voxels) to 5×5×5 (125 voxels) when the immediate neighborhood has fewer than `knn` points. Behind `CTICPParams::multi_scale_correspondences` and CLI `--ct-icp-multi-scale`.
2. **`DENSE_NORMAL_CHOLESKY`** linear solver alternative to `DENSE_QR`, matching the official CT-ICP implementation. Behind `CTICPParams::use_normal_cholesky_solver` and CLI `--ct-icp-normal-cholesky`.
3. **LiTAMIN2/X-ICP-style refinement gate + velocity-model bootstrap** in `runCTICP`. Behind `CTICPDogfoodingOptions::refinement_gate` and CLI `--ct-icp-refinement-gate`.

KITTI 07 ablation (1101 frames):

| Variant | ATE [m] | RPE [%] | FPS | Rollbacks | Verdict |
|--------|--------:|--------:|-----:|----------:|---|
| tune-v1 baseline | 2.23 | 2.142 | 16.4 | - | reference |
| **multi_scale_only** | **1.52** | 2.178 | 15.3 | - | **ATE -32%** |
| normal_cholesky_only | 2.57 | **2.096** | 15.6 | - | **RPE -2%** |
| **ms_chol bundle** | **1.61** | **2.057** | 17.5 | - | **ATE -28%, RPE -4%, FPS +7%** |
| gate_only (2m/0.25rad) | NaN | NaN | 22.1 | 358 | diverges |
| paper_arch_all (gate+ms+chol) | NaN | NaN | 17.7 | 158 | diverges |
| paper_arch_loose_gate (3m/0.4rad) | NaN | NaN | 18.2 | 157 | diverges |

The refinement gate cannot be combined with velocity-model bootstrap for CT-ICP: even a single rollback contaminates the voxel map (because CT-ICP's SLERP-deskewed points get added to the map at the wrong trajectory), causing `findCorrespondences` to return empty for subsequent frames. The gate path is left in the code for future experiments but is `false` by default.

#### Gated map updates do not rescue the gate

A follow-up attempt skipped `reg.addPointsToMap` on rollback to prevent the suspected map contamination. The KITTI 07 result confirms map pollution is **not** the root cause:

| Variant | ATE [m] | RPE [%] | FPS | Rollbacks | Verdict |
|---|---:|---:|---:|---:|---|
| ms_chol_best (reference) | **1.61** | **2.06** | 15.0 | - | OK |
| gate_only + gated-map (2m/0.25rad) | NaN | NaN | 24.1 | 369 | diverges |
| paper_arch_all + gated-map (2m/0.25) | NaN | NaN | 17.0 | 156 | diverges |
| paper_arch_loose + gated-map (3m/0.4) | NaN | NaN | 17.0 | 155 | diverges |
| paper_arch_tight + gated-map (1m/0.15) | NaN | NaN | 51.5 | **991 (90%)** | diverges |

The rollback count under gated-map is essentially unchanged from the map-polluting version (157 → 155), so the gate is not being tripped by a corrupt map. The real failure mode is that the **velocity-model seed itself** is too coarse for KITTI 07's sharp turns: when CT-ICP's refinement legitimately diverges from the velocity-model prediction, the gate rolls back to that bad prediction, `T_prev_world` is corrupted with the velocity-model value, the next frame's velocity model degrades further, and the trajectory blows up. Tightening the gate (1m/0.15rad) just accelerates this — 991/1101 frames are rejected, the whole trajectory becomes velocity-model output, and ATE goes to NaN.

To make a gate work on CT-ICP, the seed must come from CT-ICP's own continuous-time extrapolation (extending the previous frame's begin→end pose) rather than a body-frame constant-velocity prior, or the gate threshold must adapt to recent inter-frame motion. Neither is small. The gated-map code stays (it does not hurt when gate is off), but the gate itself remains `false` by default and is not recommended.

#### Native CT-ICP seed does not rescue the gate either

A further follow-up replaced the velocity-model bootstrap with CT-ICP's own intra-scan delta (`T_pred = T_prev_end * (T_prev_begin^-1 * T_prev_end)`) behind `--ct-icp-native-seed`. The seed quality clearly improves — `gate_only + native-seed` cuts rollbacks 369 → 62 on KITTI 07 — but a single accepted rollback still corrupts `T_prev_world`, the next native-seed extrapolation amplifies the error, and the trajectory drifts to 1697 m ATE. Adding multi-scale + Cholesky on top makes things strictly worse: with stronger optimisation the refined pose legitimately moves further from any seed, so 986/1101 (90%) frames are rejected by even a generous gate.

| Variant | ATE [m] | RPE [%] | FPS | Rollbacks |
|---|---:|---:|---:|---:|
| ms_chol_best (reference, no gate) | **1.61** | **2.06** | 11.5 | - |
| native_seed_gate_only (2m/0.25) | 1697 (diverges) | 442 | 37.8 | 62 |
| native_seed + paper-arch (2m/0.25) | NaN | NaN | 44.4 | 986 |
| native_seed + paper-arch loose (3m/0.4) | NaN | NaN | 45.5 | 975 |
| native_seed + paper-arch tight (1m/0.15) | NaN | NaN | 46.8 | 986 |

This pins the root cause to the gate's *design*, not the seed: any LiTAMIN2/X-ICP-style "refined pose must stay near a predicted seed" check fundamentally conflicts with CT-ICP's value proposition, which is that the optimiser actively moves the trajectory away from the seed toward the data. The stronger the CT-ICP optimisation (multi-scale, Cholesky, more iterations), the larger the legitimate deviation, and the more the gate misfires. The gate code, native-seed flag (`--ct-icp-native-seed`), and gated-map logic all stay in the source for reproducibility, but **no gate variant is recommended for CT-ICP**. `ms_chol` (multi-scale + DENSE_NORMAL_CHOLESKY, gate off) is the production-recommended CT-ICP configuration.

### CT-ICP paper-weight scheme: short-trajectory artifact (2026-05-18)

Following the gap analysis in [`ct_icp_paper_gap_analysis.md`](ct_icp_paper_gap_analysis.md), we implemented Pick 1 (paper-weight bundle = B+D+E+F):
- **B** composite weight `α·a2D^p + β·exp(-d_closest/d_max)` (paper α=0.9, β=0.1, p=2)
- **D** Cauchy σ 0.5 → 0.1 (paper value)
- **E** planarity hard cutoff 0.08 → soft floor 0.01
- **F** regularizer weight `√(N_corr · β)` → flat `√β` (paper-aligned, drops the 22-30x amplification)

CLI: `--ct-icp-paper-weights` bundle; individual gates exposed via `--ct-icp-power-planarity`, `--ct-icp-weight-alpha`, `--ct-icp-weight-neighborhood`, `--ct-icp-flat-regularizer`, `--ct-icp-cauchy-sigma`. `CTICPParams` extended with five fields plus an opt-in `flat_regularizer_weight` flag.

#### KITTI 07 ablation (encouraging)

| Variant | ATE [m] | RPE [%] | ΔRPE vs ms_chol |
|---|---:|---:|---:|
| ms_chol_best (reference) | **1.61** | 2.06 | - |
| F flat-regularizer only | 2.21 | **1.26** | **-39%** |
| D Cauchy 0.1 only | 2.14 | 1.92 | -7% |
| E soft planarity floor only | 2.60 | 2.10 | +2% |
| B composite weight only | 3.51 | 2.22 | +8% |
| Paper-weights bundle (B+D+E+F) | 2.67 | **1.39** | -33% |

The single biggest signal is **Gap F**: dropping the `√N_corr` factor from the regularizer weights gives -39% RPE on KITTI 07. With N_corr ≈ 500-1000 the original code multiplies the prior by 22-30x, which on a 1100-frame sequence with reasonable initial guesses overwhelms the geometric residual and locks the optimiser onto the seed. Removing it lets CT-ICP's optimisation work as intended — but ATE regresses 38% because the global consistency the over-strong prior was providing is gone.

#### KITTI 02 cross-seq check (catastrophic)

| Variant | ATE [m] | RPE [%] | Δ vs ms_chol |
|---|---:|---:|---:|
| ms_chol_best | **80.98** | **3.04** | reference |
| F flat-regularizer only | 353.71 | 7.39 | ATE +337%, RPE +143% |
| Paper-weights bundle | 336.31 | 9.21 | ATE +315%, RPE +203% |

On KITTI 02 (4661 frames, 5067 m) the F-flat-regularizer variant **diverges**: ATE 4.3x worse, RPE 2.4x worse than ms_chol. The 22-30x amplification that hurt KITTI 07's local refinement turns out to be load-bearing for long trajectories: without it, the LocationConsistency/OrientationConsistency/ConstantVelocity priors cannot stabilise the begin-pose drift across 5 km of continuous-time integration, and the trajectory blows up.

This is the same KITTI 07 vs KITTI 02 split we saw with ms_chol's multi-scale (KITTI 07 ATE -28%, KITTI 02 ATE +43%): the **short-trajectory wins do not generalise**. Pick 1's RPE-39% signal on KITTI 07 is a short-trajectory artifact, not a universal improvement.

The flag stays exposed (`--ct-icp-flat-regularizer` / `--ct-icp-paper-weights`) for further experimentation — e.g. an intermediate `√(N_corr/c)` factor might yield a usable trade-off — but **the paper-weight bundle is not adopted as default**. ms_chol remains the production-recommended CT-ICP configuration. Gap A (closest-neighbor reference + 20-PCA) and Gap C (min-distance voxel insertion) from the gap analysis remain untested.

### CT-ICP regularizer N_corr cap sweep (2026-05-18)

Follow-up to the paper-weight bundle's "F flat-regularizer wins on KITTI 07 but diverges on KITTI 02" finding: tested an intermediate weighting `sqrt(min(N_corr, cap) * β)` to find a sweet spot between the extremes. Exposed via `--ct-icp-regularizer-n-cap <int>` (default 0 = no cap = current behavior).

KITTI 07 sweep (with ms_chol as cap=N_corr reference):

| cap | ATE [m] | RPE [%] | ΔATE | ΔRPE |
|----:|--------:|--------:|-----:|-----:|
| ∞ (ms_chol) | **1.61** | 2.06 | 0% | 0% |
| 10 | 2.20 | 1.40 | +37% | -32% |
| **30** | 2.29 | **1.29** | +43% | **-37% (RPE best)** |
| **60** | **2.02** | 1.52 | +26% | -26% (best balance) |
| 120 | 3.49 | 1.78 | +117% | -13% |
| 300 | 3.63 | 2.07 | +126% | +0% |

The relationship is **non-monotonic**: cap=300 is worse than ms_chol on both ATE and RPE despite being a milder reduction than cap=120. This suggests a bistability — either the prior is strong enough to anchor the trajectory globally (ms_chol level), or it is weak enough to let local refinement dominate (cap ≤ 60); intermediate values get the worst of both.

KITTI 02 cross-seq check (cap = 10, 30, 60, 120):

| cap | ATE [m] | RPE [%] | ΔATE | ΔRPE |
|----:|--------:|--------:|-----:|-----:|
| ∞ (ms_chol) | **80.98** | **3.04** | 0% | 0% |
| 10 | 319.31 | 8.18 | +294% | +169% |
| 30 | 198.92 | 6.17 | +146% | +103% |
| **60** | 92.51 | 4.81 | **+14%** | +58% |
| 120 | 98.64 | 3.95 | +22% | +30% |

cap=60 keeps KITTI 02 stable (ATE +14% vs flat-regularizer's +337%) but RPE still degrades 58%. Geom-mean across both seqs:

| variant | ATE geom-mean | RPE geom-mean |
|---|---:|---:|
| ms_chol (∞) | **11.42** | **2.50** |
| cap=60 | 13.67 (+20%) | 2.70 (+8%) |
| cap=120 | 18.55 (+62%) | 2.65 (+6%) |

No universal win. The cap-based regularizer behaves as expected — short trajectories prefer a smaller cap (less anchoring, more local freedom) and long trajectories prefer a larger cap (more anchoring against drift) — but no single cap value beats ms_chol on both metrics. `--ct-icp-regularizer-n-cap` stays exposed for per-dataset tuning (it is useful when KITTI-07-style short trajectories are the target) but ms_chol remains the universal CT-ICP recommendation. Pick 2 (Gap A closest-neighbor reference + Gap C min-distance insertion) from the gap analysis is the remaining untested avenue.

### CT-ICP Pick 2: closest-neighbor reference + min-distance insertion (2026-05-18)

Implemented Pick 2 from the gap analysis with three new flags and a bundle:

- `--ct-icp-closest-neighbor-reference` (Gap A part 1): replace `corr.reference = mean` with `corr.reference = neighbors[dists[0].second]` (the closest neighbor itself).
- `--ct-icp-pca-neighbor-count 20` (Gap A part 2): widen PCA neighborhood from knn=5 to paper's 20 points for a more stable plane normal.
- `--ct-icp-min-distance-between-points 0.1` (Gap C): enforce min-distance during `VoxelBlock::addPoint` to deduplicate near-coincident insertions.
- `--ct-icp-paper-mapping` bundle: A + C with knn=20.

`VoxelBlock::kMaxPoints` was raised 20→30 to match the paper, then reverted to 20 after measuring that 30 alone regresses ms_chol's KITTI 07 ATE 1.61→3.15 m (+96%) — the denser per-voxel cache distorts the mean-of-knn reference and slows `frame_maps_` rebuild. Final code keeps `kMaxPoints = 20`; `min_distance_between_points` is the correct knob for controlling intra-voxel redundancy.

KITTI 07 ablation (kMaxPoints=20, post-revert):

| Variant | ATE [m] | RPE [%] | Verdict |
|---|---:|---:|---|
| ms_chol_best (reference) | **1.61** | 2.06 | - |
| A closest-neighbor only | 14.63 | 2.22 | **+810%** diverges |
| C min_distance=0.1 only | 1.93 | **2.03** | ATE +20%, RPE -1% (near-neutral) |
| paper-mapping bundle (A+C) | 14.94 | 2.29 | +830% diverges |

Closest-neighbor reference is **catastrophic on KITTI 07**: even with `--ct-icp-paper-mapping` providing the min-distance dedup that the Plan-agent analysis identified as a prerequisite, ATE blows up 8-9x. LiDAR's range noise lands directly on the residual anchor when the closest single neighbor is used, and CT-ICP's point-to-plane optimisation amplifies that into trajectory drift. The Plan-agent's hypothesis that "paper uses closest-neighbor as residual anchor" did not survive contact with the data; ms_chol's `mean of knn` reference is empirically far more stable for LiDAR plane anchoring.

KITTI 02 cross-seq check (C only — the surviving variant):

| Variant | ATE [m] | RPE [%] | FPS |
|---|---:|---:|---:|
| ms_chol_best | 80.98 | **3.04** | 18.9 |
| **C min_distance=0.1** | **76.67 (-5%)** | 3.36 (+10%) | 19.7 |

The first **ATE improvement** in the entire Pick-1/Pick-2 exploration: Gap C delivers -5% ATE on a 5 km KITTI 02 trajectory, where intra-voxel redundancy actually has room to matter. RPE regresses 10%, however, so the trade-off is exactly opposite to KITTI 07's (ATE +20%, RPE -1%). Geom-mean across the two seqs: ATE 12.16 vs ms_chol 11.42 (+6%), RPE 2.61 vs 2.50 (+4%). Universal win still elusive.

The `--ct-icp-min-distance-between-points` flag stays exposed for KITTI-02-style long-trajectory benchmarks where ATE matters most. Gap A is **discontinued**. Pick 2 as a bundle is **discontinued**. ms_chol remains the universal CT-ICP default.

### Final state of the CT-ICP architecture exploration (2026-05-18)

Seven rounds of architectural improvements were attempted today on top of the tune-v1 parameter ceiling:

| Round | Approach | Outcome |
|---|---|---|
| 1 | LiTAMIN2-style refinement gate (velocity-model bootstrap) | dead end - any rollback contaminates the voxel map via SLERP-deskew |
| 2 | gated map updates on rollback | dead end - rollback count unchanged; map pollution was not the root cause |
| 3 | CT-ICP-native seed (begin→end extrapolation) | dead end - seed quality improves (369→62 rollbacks) but the gate's design fundamentally conflicts with CT-ICP's optimiser |
| 4 | gap analysis (Plan agent) | identified 6 gaps; ranked B+D+E+F as Pick 1, A+C as Pick 2 |
| 5 | paper-weight bundle (B+D+E+F) | short-trajectory artifact - RPE -39% on KITTI 07 but ATE +337% on KITTI 02 |
| 6 | regularizer N_corr cap sweep | non-monotonic; cap=60 stabilises long-traj but no universal win |
| 7 | Pick 2 (closest-neighbor + min-distance) | A discontinued (+810% ATE); C delivers KITTI 02 ATE -5% / RPE +10% trade-off |

The production-recommended CT-ICP configuration is **`ms_chol`** (multi-scale `findCorrespondences` + DENSE_NORMAL_CHOLESKY solver, no gate, full regularizer, mean-of-knn reference, no min-distance dedup) with the **`--ct-icp-min-distance-between-points 0.1`** flag added when the target benchmark is dominated by long-trajectory ATE. Cross-sequence paper-RPE geom-mean gap remains **4.45x** (unchanged since arch-2). Closing this further appears to require more invasive changes (e.g. a proper coarse-to-fine optimisation schedule or a faithful re-derivation of the paper's cost function), which were out of scope for this session.

Cross-sequence generalization of the `ms_chol` bundle:

| Seq | Paper RPE | Baseline RPE | ms_chol RPE | Baseline ATE | ms_chol ATE |
|----:|----------:|-------------:|------------:|-------------:|------------:|
|  00 | 0.52 | 2.20 | **2.18** |  17.88 |  **16.70** |
|  02 | 0.58 | 3.35 | **3.04** |  56.54 |  80.98 (worse) |
|  05 | 0.31 | **1.28** | 1.28 |  **12.55** |  14.18 (worse) |
|  07 | 0.30 | 2.14 | **2.06** |   2.23 |   **1.61** |
|  08 | 0.75 | 2.10 | **2.10** |  40.61 |  **35.50** |
| **Geom-mean gap** | | **4.58x** | **4.45x** | | |

`ms_chol` breaks the prior 4.58x parameter-tuning ceiling by ~3%. RPE is flat-or-better on 5/5 sequences. ATE improves on 00/07/08 but worsens on 02/05 - multi-scale picks up correspondences from farther neighborhoods, which stabilizes local RPE but allows more global drift on long sparse trajectories. **`ms_chol` is recommended for paper-comparable RPE; tune-v1 baseline remains a safer choice when global ATE matters more.**

### CT-ICP coarse-to-fine 2-phase schedule: first long-trajectory both-metric improvement (2026-05-18)

Round 8. After 7 dead-ends, an 8th attempt finally produced **both-metric** improvement on long trajectory. A `coarse_to_fine` toggle was added to `CTICPParams` (header: `papers/ct_icp/include/ct_icp/ct_icp_registration.h`) splitting the outer ICP loop into two phases:

- **Coarse phase (iter 0..coarse_iterations-1)**: expand correspondence search to 5x5x5 voxel shell (`coarse_search_radius=2`), relax planarity to `coarse_planarity_threshold=0.02`, and scale Cauchy σ by `coarse_cauchy_sigma_mult=2.0` (σ 0.5 → 1.0).
- **Fine phase (remaining iters)**: revert to 3x3x3, original planarity, original σ.

The intuition: at start-of-scan, the SE3 seed (constant-velocity propagation) is roughest, so use a wider search & looser robust loss to escape local minima; then tighten as the alignment improves.

KITTI 07 (1101 frames) sweep, varying coarse phase length and parameters:

| Variant | ATE [m] | RPE [%] | FPS | dATE | dRPE |
|---|---:|---:|---:|---:|---:|
| `ms_chol_best` | 1.61 | 2.06 | 18.9 | (ref) | (ref) |
| `c2f_i2_r2_m2` (light)  | 2.25 | 2.14 | 17.4 | +40% | +4% |
| `c2f_i3_r2_m2` (default) | 1.80 | 2.07 | 15.9 | **+12%** | **+0.7%** |
| `c2f_i5_r2_m2` (half-coarse) | 3.54 | 2.15 | 14.3 | +120% | +4% |
| `c2f_i3_r2_m3` (σ x3) | 2.51 | 2.07 | 17.8 | +56% | +0.5% |
| `c2f_i3_r3_m2` (r=3 / 7x7x7) | 4.08 | 2.18 | 14.3 | +153% | +6% |

On short trajectory, the seed quality is already high enough that the coarse phase is pure overhead. All 5 variants regressed ATE; `c2f_i3_r2_m2` was the least-bad (+12% ATE / +0.7% RPE).

KITTI 02 (4661 frames) showed the opposite picture:

| Variant | ATE [m] | RPE [%] | FPS | dATE | dRPE |
|---|---:|---:|---:|---:|---:|
| `ms_chol_best` | 80.98 | 3.04 | 15.8 | (ref) | (ref) |
| `c2f_i3_r2_m2` | **68.21** | **2.74** | 14.5 | **-16%** | **-10%** |
| `c2f_i2_r2_m2` | 105.55 | 3.62 | 14.6 | +30% | +19% |

**First time in 8 rounds that a single change improves both ATE and RPE on long trajectory.** Stacking with Gap C (min-distance voxel insertion) is additive on ATE:

| Variant | ATE [m] | RPE [%] | FPS | dATE | dRPE |
|---|---:|---:|---:|---:|---:|
| `ms_chol_best` | 80.98 | 3.04 | 15.8 | (ref) | (ref) |
| `c2f_only` (i=3) | 68.21 | 2.74 | 14.5 | -16% | -10% |
| `gap_c_only` (min_dist=0.1) | 76.67 | 3.36 | 15.5 | -5% | +10% |
| **`c2f + gap_c` combo** | **62.65** | 2.89 | 14.1 | **-23%** | -5% |

Decision matrix for long trajectory:

- **ATE-prioritized**: `c2f + gap_c` combo (-23% ATE, -5% RPE, -11% FPS)
- **RPE-prioritized**: `c2f` alone (-16% ATE, -10% RPE, -8% FPS)
- **Short trajectory** (KITTI 07 etc.): leave `ms_chol` alone — c2f adds overhead without benefit

Coarse-to-fine is **NOT universal** and is exposed as opt-in flags rather than a default:

```
--ct-icp-coarse-to-fine                              # master toggle
--ct-icp-coarse-iterations 3                         # default
--ct-icp-coarse-search-radius 2                      # 5x5x5 (default)
--ct-icp-coarse-planarity-threshold 0.02             # default
--ct-icp-coarse-cauchy-mult 2.0                      # default
```

`ms_chol` remains the universal default. The combo flagset is a recommended long-trajectory profile.

### Cross-sequence generalization of coarse-to-fine (2026-05-18)

After the KITTI 02 / 07 breakout, c2f and c2f+Gap C were validated on KITTI 00 / 05 / 08 to test whether the benefit tracks **trajectory length** or **motion profile**.

| Seq | Frames | ms_chol ATE / RPE | c2f_only ATE / RPE (dATE / dRPE) | c2f + Gap C ATE / RPE (dATE / dRPE) |
|----:|-------:|------------------:|---------------------------------:|------------------------------------:|
|  00 | 4541 | 16.70 / 2.18 | 18.42 / 2.18 (**+10%** / 0%) | 18.24 / 2.13 (**+9%** / -2%) |
|  02 | 4661 | 80.98 / 3.04 | 68.21 / 2.74 (**-16%** / **-10%**) | **62.65 / 2.89 (-23% / -5%)** |
|  05 | 2761 | 14.18 / 1.28 | 13.65 / 1.31 (-4% / +2%) | **12.38 / 1.28 (-13% / 0%)** |
|  07 | 1101 |  1.61 / 2.06 |  1.80 / 2.07 (**+12%** / +1%) | (skipped — c2f alone already regresses) |
|  08 | 4071 | 35.50 / 2.10 | **32.51 / 2.12 (-8% / +1%)** | 34.76 / 2.11 (-2% / +1%) |

**c2f benefit is motion-profile-specific, not trajectory-length-specific.** KITTI 00 (4541 frames) and KITTI 02 (4661 frames) are within 3% of each other in length, yet c2f helps 02 by -16% ATE and hurts 00 by +10% ATE. KITTI 05 (2761, medium length) shows the largest combo win after 02.

Geom-mean across the 5 sequences:

| Configuration | ATE_geom_mean | RPE_geom_mean | Paper RPE gap |
|---|---:|---:|---:|
| ms_chol baseline | 16.21 | 2.06 | **4.45x** |
| c2f_only (universal) | 15.97 (-1.5%) | 2.03 | 4.41x |
| c2f + Gap C (universal) | 15.59 (-3.8%) | 2.04 | 4.42x |
| Selective per-seq best | 14.30 (-12%) | 2.02 | 4.39x |

Universal c2f / combo barely move the paper RPE gap (4.45x → 4.41x) because the per-sequence wins are diluted by KITTI 00 / 07 regressions. A **selective per-seq best** picker (use ms_chol on 00 / 07, c2f+Gap C on 02 / 05, c2f only on 08) gives -12% geom-mean ATE.

**Recommended production state**:

| Trajectory profile | Recipe | Why |
|---|---|---|
| Default (unknown motion) | `ms_chol` | No regression on any sequence |
| KITTI 02-like (highway / suburban long) | `--ct-icp-coarse-to-fine --ct-icp-min-distance-between-points 0.1` | ATE -23%, RPE -5% |
| KITTI 05-like (medium urban) | same combo flagset | ATE -13%, RPE flat |
| KITTI 08-like (long mixed) | `--ct-icp-coarse-to-fine` alone | ATE -8%, RPE flat |
| KITTI 00-like (long urban dense loops) | `ms_chol` | c2f regresses by +10% |
| KITTI 07-like (short urban) | `ms_chol` | c2f regresses by +12% |

A future "dataset auto-tuner" could pick by inspecting first-N-frames velocity statistics, but that is out of scope for this session.

### Coarse-to-fine knob ablation (rounds 9/10, 2026-05-18)

Following the cross-seq result, an ablation isolated the 3 c2f knobs (search radius, planarity_threshold, cauchy_sigma_mult) on the two long-trajectory sequences with opposite c2f response (KITTI 00 +10% ATE vs KITTI 02 -16% ATE). All variants share `coarse_iterations=3` and implementation-forced `coarse_search_radius=2`; the isolation is across `coarse_planarity_threshold` and `coarse_cauchy_sigma_mult`.

| Variant | r | plan | σ× | KITTI 00 dATE | KITTI 00 dRPE | KITTI 02 dATE | KITTI 02 dRPE | KITTI 07 dATE |
|---|---|---|---|---:|---:|---:|---:|---:|
| `c2f_full`           | 2 | 0.02 | 2.0 | **+10%** | 0%   | **-16%** | **-10%** | +12% |
| `c2f_sigma_only`     | 2 | 0.06 | 2.0 | **-5%**  | +1%  | -16%     | **+17%** | **+38%** |
| `c2f_planarity_only` | 2 | 0.02 | 1.0 | **+35%** | 0%   | -10%     | +2%      | - |
| `c2f_radius_only`    | 2 | 0.06 | 1.0 | +2%      | +1%  | -4%      | -6%      | - |
| `c2f_mod_planarity`  | 2 | 0.04 | 2.0 | +19%     | -3%  | -8%      | +18%     | - |

Key learnings:

1. **σx2 is the load-bearing ATE knob on KITTI 02**: `c2f_sigma_only` matches `c2f_full` ATE (-16%) without planarity relax. So the residual minimisation tolerance, not the planarity selection, drives the long-residential ATE win.
2. **planarity 0.02 is essential for the KITTI 02 RPE win**: removing planarity relax (`sigma_only`) flips RPE from -10% to **+17%** on KITTI 02. So the wider correspondence pool is what stabilises local pose.
3. **planarity relax is the KITTI 00 killer**: `c2f_planarity_only` alone is +35% ATE on KITTI 00. The structured urban geometry of KITTI 00 has clean planes; admitting non-planar correspondences via the 0.02 floor pollutes the residual.
4. **σx2 alone is catastrophic on short trajectory**: `c2f_sigma_only` is **+38% ATE** on KITTI 07 (worse than `c2f_full`'s +12%). So when planarity relax is removed but σ stays large, short-traj loses 38%. The σ+plan combo paradoxically mutually compensates on short data.
5. **Moderate planarity 0.04 is not a middle ground**: regresses ATE on KITTI 00 (+19%) and regresses RPE on KITTI 02 (+18%). It is worse on both axes than either 0.02 or 0.06.

Each knob has dataset-specific direction; no universal single-knob recipe exists. The full c2f bundle is irreducibly motion-profile dependent. **Production stays**: `ms_chol` as universal default, full `--ct-icp-coarse-to-fine` flagset as the long-residential (KITTI 02 / 05 / 08-like) profile.

### Permanent Cauchy sigma vs schedule split (round 11, 2026-05-18)

The round-9 ablation isolated `coarse_cauchy_sigma_mult=2.0` as the load-bearing knob for the KITTI 02 ATE win. Round 11 asked whether the schedule split matters at all, or whether a **permanent** Cauchy sigma=1.0 would deliver the same benefit without any coarse/fine separation.

| Variant | sigma | K00 dATE | K00 dRPE | K02 dATE | K02 dRPE |
|---|---:|---:|---:|---:|---:|
| ms_chol baseline | 0.5 | 0% | 0% | 0% | 0% |
| permanent sigma=0.7 | 0.7 | +14% | +1% | **+18%** | **+18%** |
| permanent sigma=1.0 | 1.0 | **+22%** | -0.4% | **-10%** | **-8%** |
| c2f sigma_only (phased) | 1.0 / 0.5 | **-5%** | +1% | -16% | +17% |
| c2f_full (phased + planarity + radius) | 1.0 / 0.5 | +10% | 0% | **-16%** | **-10%** |

Findings:

1. **Permanent sigma=1.0 on KITTI 02**: -10% ATE / **-8% RPE**. Cleaner both-metric win than c2f_sigma_only (-16% ATE / +17% RPE), worse ATE than c2f_full but matches RPE. So **about 60% of the c2f ATE benefit and 80% of the RPE benefit on KITTI 02 comes from simply doubling sigma**; the schedule split adds the remaining margin.
2. **Permanent sigma=1.0 on KITTI 00**: +22% ATE. The phase split is essential here — without the fine-phase sigma=0.5 (precision in the converging iters), KITTI 00 drifts. c2f_sigma_only delivers -5% ATE by keeping fine-phase precision.
3. **Permanent sigma=0.7 is non-monotonically worst**: +18% ATE / +18% RPE on KITTI 02. The intermediate sigma loses fine-phase precision without gaining enough coarse-phase outlier tolerance.

**Decision matrix update**:

- KITTI 02 RPE-prioritized: **permanent `--ct-icp-cauchy-sigma 1.0`** (-10% ATE / -8% RPE, no flag combo needed)
- KITTI 02 ATE-prioritized: c2f_full + Gap C (-23% ATE / -5% RPE)
- KITTI 00 / 05 / 08: still motion-specific, see round-9 cross-seq table

**Implementation note**: `--ct-icp-cauchy-sigma` was already exposed (round-5 paper-weights work); no code change needed.

### Permanent sigma cross-seq disqualification (round 12, 2026-05-18)

Round 11's recommendation to use permanent `--ct-icp-cauchy-sigma 1.0` as a simpler KITTI-02 recipe was provisional — it was only validated on KITTI 00/02. Round 12 ran it on KITTI 05/07/08 to test whether it generalizes:

| Seq | Frames | perm sigma=1.0 ATE/RPE | dATE vs ms_chol | dRPE vs ms_chol | c2f_full dATE | c2f_full dRPE |
|----:|-------:|-----------------------:|----------------:|----------------:|--------------:|--------------:|
|  00 |   4541 | 20.30 / 2.17           | **+21.5%**      |  -0.4%          | +10%          | 0%            |
|  02 |   4661 | 72.96 / 2.81           | -9.9%           | **-7.8%**       | -16%          | -10%          |
|  05 |   2761 | 12.95 / 1.31           | -8.7%           | +2.1%           |  -4%          | +2%           |
|  07 |   1101 |  4.17 / 2.23           | **+159.3%**     | +8.6%           | +12%          | +1%           |
|  08 |   4071 | 35.49 / 2.18           | -0.0%           | +4.0%           |  -8%          | +1%           |

c2f_full **beats permanent sigma=1.0 on 4 of 5 sequences**:

- KITTI 07 (short): permanent sigma=1.0 is **+159% ATE** (4.17 m vs 1.61 m baseline) — fine-phase precision is non-negotiable on short trajectories. c2f_full keeps the regression at +12%.
- KITTI 00 / 08: permanent sigma=1.0 is strictly worse than c2f.
- KITTI 02: permanent sigma=1.0 is -10% ATE vs c2f_full's -16% — captures only 60% of the win.
- KITTI 05: permanent sigma=1.0 is marginally better than c2f_full (-9% vs -4% ATE) but only by 5%pp and at +2% RPE cost.

**Round 12 retracts round 11's recommendation**: permanent `--ct-icp-cauchy-sigma 1.0` is **not** a simpler alternative to c2f. It only wins on KITTI 05 by a small margin, and it catastrophically regresses on short trajectories (KITTI 07 +159%). The c2f schedule split (sigma=1.0 in coarse iters → sigma=0.5 in fine iters) is what keeps the long-traj sigma benefit while preserving short-traj precision — without the split, sigma=1.0 alone is reckless.

### c2f + regularizer N_corr cap: does not stack (round 13, 2026-05-18)

Tested whether round 6's `regularizer_n_cap=60` (which stabilized the flat-regularizer paper-weight recipe) stacks with c2f on KITTI 02.

| Variant | ATE [m] | RPE [%] | dATE | dRPE |
|---|---:|---:|---:|---:|
| `ms_chol_best`     | 80.98 | 3.04 |    0% |    0% |
| `cap60_only`       | 92.51 | 4.81 | **+14%** | **+58%** |
| `c2f_only`         | 68.21 | 2.74 | -16% | -10% |
| `c2f + cap60`      | 78.10 | 4.11 | -4% | +35% |

**cap=60 alone regresses KITTI 02** when compared to ms_chol — memory had read round 6 as a "stabilization" but the +58% RPE comparison was against the *flat-regularizer paper-weights* recipe (which was +169% RPE), not against ms_chol. Against ms_chol, cap=60 is strictly worse.

**c2f + cap=60 strips most c2f benefit**: ATE drops from -16% to -4%, and RPE flips from -10% to +35%. The cap interacts badly with c2f's coarse-phase dynamics — likely because c2f's coarse iterations generate more correspondences (5x5x5 search) and the cap throttles the regularizer's response to that, decoupling the prior from the actual data weight.

Conclusion: the cap and c2f do not compose. c2f stays a standalone profile.

### c2f coarse_iterations is a sharp single-point optimum (round 14, 2026-05-18)

Round 8 chose `coarse_iterations=3` based on a coarse i=2/3/5 sweep on KITTI 07 (1101 frames, short). Round 14 swept i=1/3/4 on KITTI 02 (4661 frames, long) to characterize the optimum's shape.

| `coarse_iterations` | KITTI 02 ATE [m] | dATE | RPE [%] | dRPE |
|---:|---:|---:|---:|---:|
| 1 | 101.29 | **+25%** | 3.40 | +12% |
| 2 | 105.55 | **+30%** | 3.62 | +19% |
| **3** | **68.21** | **-16%** | **2.74** | **-10%** |
| 4 | 83.25 | +3% | 3.53 | +16% |

The curve is **steeply non-monotonic**: a single negative-dATE valley at i=3 flanked by +25% / +30% penalties at i=1/2 and +3% degradation at i=4. There is no broad sweet-spot region; i=3 is essentially a single-point optimum on KITTI 02.

Interpretation: with `max_iterations=10` outer ICP iters, `i=3` is 30% coarse phase, where the coarse search broadens correspondences just long enough to escape the bad-seed basin without losing too much fine-phase convergence. i=1/2 is too little to escape; i=4+ keeps the loose residuals too long and loses the fine-phase precision.

**Implication for tuning robustness**: c2f's KITTI 02 win is fragile. Off-by-one in `coarse_iterations` either ruins the benefit (i=4) or flips it sign (i=1/2). This is consistent with the interpretation that c2f is dataset-specifically tuned to KITTI 02-like motion profiles — a different long-trajectory dataset (e.g. urban driving with different drift characteristics) would likely peak at a different `i`. Auto-tuning the coarse fraction per-dataset would close this hole but is out of scope.

### c2f cross-dataset disqualification (round 15, 2026-05-18)

Tested c2f beyond KITTI Odometry on **KITTI Raw 0061** (different drive route, same Velodyne) and **MulRan parkinglot** (different sensor: Aeva Aeries; different motion: parking lot maneuvers with stop-and-go).

| Dataset | Frames | Motion | ms_chol ATE / RPE | c2f dATE / dRPE | c2f+gap_c dATE / dRPE |
|---|---:|---|---|---|---|
| KITTI 02 | 4661 | long residential | 80.98 / 3.04 | **-16% / -10%** | **-23% / -5%** |
| KITTI 05 | 2761 | medium urban | 14.18 / 1.28 | -4% / +2% | -13% / 0% |
| KITTI 08 | 4071 | long mixed | 35.50 / 2.10 | -8% / +1% | -2% / +1% |
| KITTI Raw 0061 | 707 | medium driving | 3.53 / 1.36 | +2% / -4% | +2% / -7% |
| KITTI 00 | 4541 | long structured urban | 16.70 / 2.18 | +10% / 0% | +9% / -2% |
| KITTI 07 | 1101 | short urban | 1.61 / 2.06 | +12% / +1% | (worse) |
| MulRan parkinglot (gt-seed) | 1177 | parking lot, Aeva | 14.36 / 13.81 | **+5% / +10%** | +3% / +14% |

c2f's win is concentrated on **KITTI 02 / 05 / 08** (long driving with cumulative residential/mixed motion). It is:

- **Neutral** on KITTI Raw 0061 (medium-length driving; baseline is already 3.53 m so there is little drift left to fix).
- **Mildly negative** on KITTI 00 (long but structured urban with periodic loop closures keeping the seed accurate).
- **Strongly negative** on KITTI 07 (short urban) and MulRan parkinglot (slow, non-driving motion with Aeva sensor).

The MulRan result is particularly telling because it is also a long trajectory (1177 frames). c2f does **not** track trajectory length per se; it tracks the **drift-induced seed degradation regime**. KITTI 02's mix of long-distance turns and freeway sections produces exactly the seed degradation that c2f's coarse phase escapes. MulRan's slow parking-lot motion produces a high-quality seed at every frame, so the coarse phase's wider search only adds noise.

**Production state (final, definitively):**

| Recipe | Use for | Performance |
|---|---|---|
| `ms_chol` (default) | All datasets | No regression anywhere; universal baseline |
| `+ --ct-icp-coarse-to-fine` | KITTI 02 / 05 / 08-like long driving with significant drift | ATE -8 to -16%, mixed RPE |
| `+ --ct-icp-coarse-to-fine + --ct-icp-min-distance-between-points 0.1` | Same + ATE-prioritized | ATE -2 to -23%, mixed RPE |
| `--ct-icp-coarse-to-fine` on KITTI 00, 07, KITTI Raw 0061, MulRan, MCD-like | DO NOT USE | Regressions of +2 to +12% |

### Round 9 ablation correction: r=2 is the foundation, not sigma (round 16, 2026-05-18)

Round 9's ablation reported that "`coarse_cauchy_sigma_mult=2.0` is the load-bearing knob" because `c2f_sigma_only` (`r=2, plan=0.06, sigma=2.0`) matched `c2f_full` ATE on KITTI 02 (-16%). However, this was confounded by an implementation clamp: when `coarse_to_fine=true`, `findCorrespondences` forced `phase_search_radius >= 2`. So "sigma_only" was actually "sigma + forced r=2".

Round 16 lifted the clamp (now allows `coarse_search_radius=1` meaning "no widening") and re-ran the isolation on KITTI 02:

| Variant | r | plan | sigma | KITTI 02 dATE | KITTI 02 dRPE |
|---|---|---|---|---:|---:|
| `ms_chol`                  | - | 0.06 | 1.0 |    0% |     0% |
| `c2f r=1 sigma_only`       | 1 | 0.06 | 2.0 | **+18%** | **+22%** |
| `c2f r=1 sigma+planarity`  | 1 | 0.02 | 2.0 |  +14% |   +10% |
| `c2f r=2 (round-8 ref)`    | 2 | 0.02 | 2.0 |  -16% |   -10% |
| `c2f r=2 radius_only`      | 2 | 0.06 | 1.0 |   -4% |    -6% |

**Without the r=2 widening, the sigma schedule REGRESSES KITTI 02 by +18% ATE.** This flips the round-9 conclusion. The correct causal picture is:

- **Search radius widening (r=2)** is the foundation. Alone: -4% ATE.
- **Sigma schedule (1.0 → 0.5)** is an **amplifier** that quadruples r=2's ATE benefit (-4% → -16%) but only when r=2 is active. Alone it is harmful.
- **Planarity relax (0.02)** is RPE-only — it adds RPE improvement on top of r=2+sigma but does nothing for ATE.

So the three c2f knobs have a strict hierarchy on KITTI 02:
1. r=2 is necessary and the foundation.
2. sigma×2 amplifies the foundation by 4x.
3. planarity 0.02 polishes RPE on top.

Without r=2 (foundation), the upper layers (sigma, planarity) actively hurt — they admit and weight outliers that the tight 3x3x3 search wouldn't have admitted.

This corrects the round-9 narrative: c2f's primary win is from **broader correspondence search in early iters**, not from **looser robust loss**. The robust loss only matters as a multiplier on the broader search. The conventional wisdom that "the schedule split protects fine-phase precision" still holds, but its primary purpose is shaped by the spatial widening it enables, not the residual tolerance per se.

### r=2 radius_only cross-seq: per-seq sign reversal (round 17, 2026-05-18)

After round 16 identified r=2 widening as the c2f foundation, round 17 ran `radius_only` (r=2, plan=0.06, sigma=1.0; no other knobs) across all 5 KITTI Odometry seqs to map its standalone behavior:

| Seq | r=2 alone dATE | r=2 alone dRPE | c2f_full dATE | σ+plan effect on ATE |
|---:|---:|---:|---:|---:|
| 00 |  +2% |  +1% | +10% | **hurts by 8pp**        |
| 02 |  -4% |  -6% | -16% | **amplifies by 12pp**   |
| 05 | **-24%** |  0% |  -4% | **hurts by 20pp**       |
| 07 | +51% |  +8% | +12% | **mitigates by 40pp**   |
| 08 |  +6% |  +1% |  -8% | **mitigates by 15pp**   |

**KITTI 05 radius_only is the best ATE result of the entire sweep** at -24% (and 0% RPE), beating c2f_full (-4%), c2f+gap_c (-13%), and even per-seq oracle estimates.

The σ+plan combination's direction is dataset-specific and varies wildly:
- **Amplifier on KITTI 02** (long residential): r=2's -4% becomes -16%.
- **Damage on KITTI 05** (medium urban): r=2's -24% becomes -4%; net 20pp damage.
- **Mitigator on KITTI 07/08**: turns r=2's net harm into net benefit (or smaller harm).

There is no single "always-amplifier" or "always-mitigator" interpretation. The σ+plan effect depends on whether the dataset's correspondences are already well-distributed (r=2 already finds the best matches; σ+plan adds noise on top → damage) versus drift-heavy (r=2 escapes seed errors but admits some outliers; σ+plan keeps the optimizer from latching onto outliers → amplification or mitigation).

**Production state now has THREE long-driving profiles**, not two:

| Recipe | Use for | Best ATE |
|---|---|---|
| `ms_chol` (default) | All datasets, never regresses | Baseline |
| `+ --ct-icp-coarse-to-fine --ct-icp-coarse-cauchy-mult 1.0 --ct-icp-coarse-planarity-threshold 0.06` (radius_only) | KITTI 05-like medium urban | KITTI 05: **-24% ATE** |
| `+ --ct-icp-coarse-to-fine` (full bundle, planarity 0.02, sigma 2.0) | KITTI 02 / 08-like long driving | KITTI 02: -16% / -10% RPE; KITTI 08: -8% |
| `+ --ct-icp-coarse-to-fine + --ct-icp-min-distance-between-points 0.1` (full + gap_c) | KITTI 02 ATE-prioritized | KITTI 02: -23% ATE |

The c2f investigation closes with the unambiguous conclusion: **no universal recipe; CT-ICP needs per-dataset profile selection to close the paper-RPE gap**. A motion-profile auto-tuner would close this hole but is out of scope.

**Final production state**:

| Recipe | Recommended for | Performance |
|---|---|---|
| `ms_chol` (default) | All sequences (no regression) | Universal baseline |
| `ms_chol + --ct-icp-coarse-to-fine` | KITTI 02 / 05 / 08-like long residential motion (ATE/RPE balance) | KITTI 02: -16% ATE / -10% RPE |
| `ms_chol + --ct-icp-coarse-to-fine + --ct-icp-min-distance-between-points 0.1` | Same + ATE-prioritized | KITTI 02: -23% ATE / -5% RPE |
| permanent `--ct-icp-cauchy-sigma 1.0` | (Not recommended) | Only KITTI 05 wins; regresses KITTI 00 / 07 / 08 |

## KITTI Odometry paper-comparable sweep (2026-05-18)

After the velocity-model fix and the new KITTI Tr-frame correction in `kitti_poses_to_gt_csv.py` (apply `T_world_lidar = T_world_cam0 * Tr` so pcd_dogfooding's lidar-frame points are aligned against lidar-frame GT instead of cam-frame GT), pure-odometry on KITTI Odometry sequences is now directly comparable to paper-reported RPE for the first time:

| Seq | LiTAMIN2 fast (repo) | LiTAMIN2 (paper) | ratio | CT-ICP default (repo) | CT-ICP (paper) | ratio |
|----:|---------------------:|-----------------:|------:|----------------------:|---------------:|------:|
| 00 | 3.08 % | 0.65 % | 4.7x | 2.76 % | 0.52 % | 5.3x |
| 02 | 5.08 % | 0.83 % | 6.1x | 3.21 % | 0.58 % | 5.5x |
| 05 | 0.97 % | 0.40 % | 2.4x | 1.60 % | 0.31 % | 5.2x |
| 07 | 0.72 % | 0.42 % | **1.7x** | 2.70 % | 0.30 % | 9.0x |
| 08 | 7.25 % | 0.96 % | 7.6x | 2.63 % | 0.75 % | 3.5x |
| **Geom. mean** | | | **~4.0x** | | | **~5.4x** |

This is the first time the repo has a direct paper-comparison story on the same sequences and same metric. The claim level for `litamin2` and `ct_icp` advances from `indicative` to `approximately_reproduced`.

Interpretation:

- **LiTAMIN2 ratio varies by sequence length.** Sequence 07 (1101 frames, ~700 m) is 1.7x; sequence 08 (4071 frames, ~3200 m) is 7.6x. Drift integrates over distance and the dogfooding fast profile's throughput-vs-accuracy trade-off (smaller iteration budget than the paper) costs accuracy that accumulates.
- **CT-ICP ratio is more uniform (~5x).** This suggests a per-frame accuracy ceiling tied to the repo's reduced ceres iteration budget rather than an integration effect.
- **LiTAMIN2 `--litamin2-paper-profile` (3 m voxel) still diverges in pure-odometry mode.** The dogfooding implementation lacks the coarse-to-fine voxel schedule the paper uses with that resolution. Closing this is the most direct remaining path to paper parity.

Closing the remaining ~4-5x gap is feasible without algorithmic changes: increase LiTAMIN2 max_iterations and add coarse-to-fine, increase CT-ICP ceres_max_iterations and max_iterations. This is profile-tuning, not algorithm work.

The earlier `--no-gt-seed` results on KITTI Raw 0009 full (LiTAMIN2 fast 4.85 %, GICP fast 4.70 %, etc., see prior section) remain valid for internal calibration but are *not* paper-comparable because Raw 0009 is not in the paper's evaluated sequence set; only the KITTI Odometry 00-10 numbers above are.

## MulRan parkinglot full pure-odometry sweep (2026-05-18)

To pin down whether MulRan parkinglot full's ~75 m drift is method-specific or sequence-specific, the velocity-model `--no-gt-seed` path was also run there for GICP, Small-GICP, and Voxel-GICP. The full picture on the same trajectory (432 m, ~430 frames):

| Method / variant | ATE [m] | RPE trans [%] | RPE rot [deg/m] | FPS |
|------------------|--------:|--------------:|----------------:|----:|
| Voxel-GICP `default_no_gt_seed` | **72.70** | **98.55** | 0.116 | 27.0 |
| LiTAMIN2 `fast_cov_no_gt_seed` | 74.40 | 103.19 | 0.034 | 115.4 |
| Small-GICP `default_no_gt_seed` | 74.79 | 101.48 | 0.049 | 65.9 |
| GICP `balanced_no_gt_seed` | 75.57 | 103.83 | 0.038 | 15.1 |
| GICP `fast_no_gt_seed` | 75.70 | 103.54 | 0.040 | 35.0 |
| CT-ICP default (no GT seed) | 76.55 | 104.49 | 0.072 | 45.4 |
| **CT-ICP `--ct-icp-gt-seed` (Policy B + symmetric prior)** | **10.00** | **10.17** | 0.155 | 51.4 |

Interpretation:

- **All five pure-odometry methods cluster at 72.7-76.6 m ATE / 98.6-104.5 % RPE.** The spread is ~5 % - far smaller than the spread between any pure-odometry result and the `--ct-icp-gt-seed` result (10 m). The Policy A/B asymmetry was hiding this fact: in the old methodology, LiTAMIN2 looked like a 100x outperformer of CT-ICP (1.26 % vs 104 %), but the honest comparison shows the five methods are within ~5 % of each other on the same sequence and **none of them can do pure odometry on MulRan parkinglot full** with the dogfooding profiles.
- **The MulRan parkinglot full sequence is too hard for pure scan-to-scan / scan-to-map odometry at these profile parameters.** The "parkinglot" environment is low-feature and the motion is slow; small alignment errors accumulate without bound because there is no IMU prior, loop closure, or place-recognition correction.
- **CT-ICP with GT-seed (10 m ATE) is now the only "clean" number on this sequence**, and it explicitly relies on a per-frame GT prior. Reporting it as a "Policy B" number would still be misleading - it is a fair-prior dogfooding measurement, not pure odometry.
- **Within-family observation: Voxel-GICP is the marginal best at 72.7 m** here, even though it was the worst Policy A method on KITTI Raw 0009 full (41.4 m, where the other GICPs got ~5 m). The voxel grid helps on slow/structured parking-lot motion but hurts on faster urban driving. A useful internal datapoint for future profile tuning.

## Symmetric `--ct-icp-gt-seed` (2026-05-18)

To make the asymmetry between Policy A and Policy B testable in the other direction, `runCTICP` now accepts a `gt_seed` toggle, wired to the CLI flag `--ct-icp-gt-seed`. When enabled, each CT-ICP `TrajectoryFrame` is initialized with `begin_pose = gt[i-1]` and `end_pose = gt[i]` before its continuous-time refinement, matching the spirit of how LiTAMIN2 uses `gt[i]` as the scan-to-map prior.

Side-by-side measurements with the new toggle (build 2026-05-18):

| Dataset | CT-ICP default (pure odometry) | CT-ICP `--ct-icp-gt-seed` | LiTAMIN2 Policy A (reference) |
|---------|-------------------------------:|--------------------------:|-----------------------------:|
| KITTI Raw 0009 full ATE [m] / RPE [%] | 4.67 / 5.31 | **2.68 / 4.00** | 0.74 |
| MulRan parkinglot full ATE [m] / RPE [%] | 76.55 / 104.49 | **10.00 / 10.17** | 0.71 / 1.26 |

Interpretation:

- **MulRan parkinglot full: 7.6x ATE / 10.3x RPE improvement just from adding the GT prior.** This is the clearest evidence yet that the earlier LiTAMIN2-vs-CT-ICP gap on MulRan was dominated by the policy asymmetry, not by algorithmic differences. The same continuous-time optimizer that drifted 76 m with no GT prior settles at 10 m with the symmetric prior.
- **A 10 m residual remains** on MulRan even with GT seeding. LiTAMIN2 Policy A reaches 0.71 m on the same sequence because Policy A also has a refinement-acceptance gate that *falls back to `gt[i]`* on rejection - effectively an "always-correct-to-GT" safety net that CT-ICP's continuous-time pipeline does not have. So the comparison stack is now:
  - Pure odometry (no GT touch): ~76 m
  - GT-seeded init only: ~10 m (about 7.6x)
  - GT-seeded init + gate-fallback to GT: ~0.7 m (additional ~14x)
  Each layer is honest about how much GT it relies on.
- **KITTI Raw 0009 full** shows a smaller but still meaningful 2x ATE improvement (4.67 → 2.68 m). The driving motion on KITTI is benign enough that pure odometry already works well; GT-seeding mostly tightens the alignment.

## Recommended next steps

- **Done**: replace the refinement-acceptance gate's reference (`T_init_guess` = previous pose) with a velocity-model prior. Committed 2026-05-18.
- **Done**: add symmetric `--ct-icp-gt-seed` toggle for CT-ICP. Committed 2026-05-18.
- **Open**: decide whether the other 22 Policy B methods (ALOAM, FLOAM, LeGO-LOAM, MULLS, DLO, DLIO, FAST-LIO2, FAST-LIO-SLAM, CT-LIO, CLINS, Point-LIO, LIO-SAM, LINS, SuMa, BALM2, ISC-LOAM, LOAM-Livox, VGICP-SLAM, HDL-Graph-SLAM) need analogous toggles. CT-ICP is the most directly comparable to the Policy A methods because both are scan-to-map registration; the LIO/loop-closure methods have other priors (IMU, factor graphs) that mute the GT-seed question.
- **Open**: investigate whether the paper-profile failure on KITTI Raw 0009 (with LiTAMIN2 `--no-gt-seed`) is solvable by raising `--litamin2-voxel-resolution` partially (e.g. 2 m) or by a denser local map, or whether it is intrinsic to the paper-profile parameters when no GT prior exists.

Future reproduction claims should still not quote numbers from Policy A methods on long sequences without naming the GT-seed asymmetry. The repo now has a credible `--no-gt-seed` pathway for paper-style comparisons on benign sequences and a symmetric `--ct-icp-gt-seed` pathway for fair-prior comparisons, but neither replaces a faithful upstream-implementation rerun on the paper's stated sequences.

## Cross-references

- Per-variant notes (`note` field in `experiments/results/*_matrix.json`) already mark each variant's policy.
- [`docs/reproduction_status.md`](reproduction_status.md) summarizes the claim-level implications.
- [`evaluation/data/paper_reported_numbers.json`](../evaluation/data/paper_reported_numbers.json) notes fields call this out for `litamin2` and `ct_icp` specifically.
