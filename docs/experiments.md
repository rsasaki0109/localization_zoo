# Experiment Results

_Generated at 2026-05-19T20:32:55+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| CT-ICP recipe sensitivity test on MCD TUHH night_09 (108 frames) | `ready` | `dense_reference` | 1.534 | 22.8 | `experiments/results/ct_icp_mcd_tuhh_night_09_recipes_matrix.json` |

## CT-ICP recipe sensitivity test on MCD TUHH night_09 (108 frames)

- **Problem ID**: `ct_icp_mcd_tuhh_night_09_recipes`
- **Question**: Completing 3-site MCD picture. KTH 6.12 m saturated (all recipes tied), NTU 0.325 m revealed cluster A RPE -50% / ATE +14% flip. What does TUHH 1.65 m show?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_tuhh_night_09_recipes_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| dense_window (current best 1.65 m) | reference | 1.652 | 22.8 | 96.4 | 4.65 | 4.75 | Adopt as current default |
| cluster A (map=50 + c2f σ×2) | long-traj-recipe | 1.534 | 16.6 | 86.4 | 1.00 | 2.10 | Keep as reference variant |
| cluster D (ms_chol) | short-traj-recipe | 1.749 | 21.7 | 91.5 | 3.95 | 4.25 | Keep as active challenger |

### Observations

1. `dense_reference` is the current default for this problem.
2. `dense_reference` is the fastest observed variant at 22.8 FPS.
3. `cluster_a` is the most accurate observed variant at 1.534 m ATE.

### Variant Notes

#### `dense_reference`

- Intent: Confirm 1.65 m baseline.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/dense_reference/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/dense_reference/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/dense_reference/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `cluster_a`

- Intent: Test cluster A on TUHH night.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/cluster_a/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/cluster_a/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/cluster_a/run.log`
- Readability proxy: 1.00 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 2.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `cluster_d_ms_chol`

- Intent: Test ms_chol on TUHH night.
- CLI args: `--ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/cluster_d_ms_chol/summary.json --ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/cluster_d_ms_chol/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_recipes_matrix/cluster_d_ms_chol/run.log`
- Readability proxy: 3.95 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.25 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

