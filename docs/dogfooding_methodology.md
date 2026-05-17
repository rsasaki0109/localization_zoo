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
