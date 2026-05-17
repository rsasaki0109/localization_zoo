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

## Recommended next steps

These do not need to happen all at once.

- **Short term**: add `--no-gt-seed` to a curated subset of LiTAMIN2 and CT-ICP-equivalent profiles when running paper-reproduction-bound manifests, so the resulting aggregate JSON carries the "pure odometry" variant of those methods.
- **Medium term**: introduce a parallel set of pending manifests (`experiments/pending/*_no_gt_seed_matrix.json`) that mirror the current paper-reproduction targets with `--no-gt-seed` passed in `args`. This keeps the GT-seeded results for benchmark dogfooding comparisons and surfaces a paper-comparable counterpart.
- **Longer term**: decide whether `CT-ICP` and the other Policy B methods should grow a symmetric `--gt-seed` toggle (for dogfooding-style fair-prior measurements), so both policies are explicit and visible per-aggregate.

Either direction is fine; the important property is that future reproduction claims should never quote numbers from Policy A methods on long sequences without naming the GT-seed asymmetry.

## Cross-references

- Per-variant notes (`note` field in `experiments/results/*_matrix.json`) already mark each variant's policy.
- [`docs/reproduction_status.md`](reproduction_status.md) summarizes the claim-level implications.
- [`evaluation/data/paper_reported_numbers.json`](../evaluation/data/paper_reported_numbers.json) notes fields call this out for `litamin2` and `ct_icp` specifically.
