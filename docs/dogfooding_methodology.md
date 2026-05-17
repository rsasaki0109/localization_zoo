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

Pending manifests with `--no-gt-seed` were run on the two long-trajectory datasets to verify the asymmetry empirically:

| Manifest | Variant | ATE [m] | RPE trans [%] | FPS |
|----------|---------|--------:|--------------:|----:|
| `experiments/pending/litamin2_kitti_raw_0009_full_no_gt_seed_matrix.json` | `fast_cov_no_gt_seed` | 171.4 | 84.8 | 58.5 |
| `experiments/pending/litamin2_mulran_parkinglot_full_no_gt_seed_matrix.json` | `fast_cov_no_gt_seed` | 78.0 | 100.7 | 117.4 |

For reference, the same data with Policy A (GT-seeded) gave LiTAMIN2 RPE 0.74 % / 1.26 % respectively.

So when the GT prior is removed:

- LiTAMIN2 on KITTI Raw 0009 full drifts to ATE 171 m / RPE 85 % (a 230x degradation).
- LiTAMIN2 on MulRan parkinglot full drifts to ATE 78 m / RPE 101 % (an 80x degradation, matching CT-ICP's pure-odometry numbers on the same data within a few percent).

This is **not** evidence that LiTAMIN2 is a poor algorithm. It is evidence that the `--no-gt-seed` code path in this tool is doing "scan-to-map with previous-pose-as-init plus a 2 m / 0.25 rad refinement-acceptance gate" - which is closer to a scan-registration probe than to standalone odometry. When the gate rejects a refinement, `T_est = T_init_guess` (the previous pose), so the trajectory stagnates and drift accumulates linearly.

The repo's Policy A numbers therefore overstate what these methods do as standalone odometry, and the repo's Policy B numbers (CT-ICP and similar) understate it because they hit the same refinement-acceptance gate.

## Recommended next steps

These do not need to happen all at once.

- **Short term**: keep Policy A as the default for dogfooding-style benchmark comparisons (consistent and reproducible), but stop quoting Policy A long-sequence RPE as paper-comparable. The pending no-GT-seed manifests document what happens when the GT prior is removed; do not promote them to active until the methodology below is fixed.
- **Medium term**: replace the refinement-acceptance gate's reference (`T_init_guess` = previous pose) with a velocity-model prior (`T_init_guess` = prev + (prev - prev_prev)) so the gate measures deviation from a constant-velocity prediction, not from a static pose. This is what most real odometry pipelines do and is what would make `--no-gt-seed` produce paper-comparable numbers.
- **Longer term**: decide whether `CT-ICP` and other Policy B methods should grow a symmetric `--gt-seed` toggle (for dogfooding-style fair-prior measurements), so both policies are explicit and visible per-aggregate.

Either direction is fine; the important property is that future reproduction claims should never quote numbers from Policy A methods on long sequences without naming the GT-seed asymmetry, and should not treat the current Policy B / `--no-gt-seed` numbers as standalone-odometry equivalents either.

## Cross-references

- Per-variant notes (`note` field in `experiments/results/*_matrix.json`) already mark each variant's policy.
- [`docs/reproduction_status.md`](reproduction_status.md) summarizes the claim-level implications.
- [`evaluation/data/paper_reported_numbers.json`](../evaluation/data/paper_reported_numbers.json) notes fields call this out for `litamin2` and `ct_icp` specifically.
