# Experiment Results

_Generated at 2026-05-19T21:01:27+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| CT-ICP cluster A/D bake-off on KITTI Raw 0009 200-frame short window | `ready` | `balanced_reference` | 1.659 | 40.9 | `experiments/results/ct_icp_kitti_raw_0009_200_recipes_matrix.json` |

## CT-ICP cluster A/D bake-off on KITTI Raw 0009 200-frame short window

- **Problem ID**: `ct_icp_kitti_raw_0009_200_recipes`
- **Question**: 0009_full had balanced_window winning 4.67 m, cluster A/D both regressed +14-17%. Does the 200-frame variant (balanced 1.66 m) show the same balanced-only winner pattern, or does the shorter window change the cluster?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0009_200_recipes_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| balanced_window (current best 1.66 m) | reference | 1.659 | 40.9 | 100.0 | 5.00 | 5.00 | Adopt as current default |
| cluster A (map=50 + c2f σ×2) | long-traj-recipe | 2.493 | 17.9 | 55.2 | 1.00 | 2.10 | Keep as reference variant |
| cluster D (ms_chol) | short-traj-recipe | 2.296 | 24.6 | 66.2 | 3.95 | 4.25 | Keep as reference variant |

### Observations

1. `balanced_reference` is the current default for this problem.
2. `balanced_reference` is the fastest observed variant at 40.9 FPS.
3. `balanced_reference` is the most accurate observed variant at 1.659 m ATE.

### Variant Notes

#### `balanced_reference`

- Intent: Confirm baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/balanced_reference/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/balanced_reference/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/balanced_reference/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `cluster_a`

- Intent: Cluster A on 200-frame 0009.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/cluster_a/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/cluster_a/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/cluster_a/run.log`
- Readability proxy: 1.00 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 2.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `cluster_d_ms_chol`

- Intent: ms_chol on 200-frame 0009.
- CLI args: `--ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/cluster_d_ms_chol/summary.json --ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/cluster_d_ms_chol/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_200_recipes_matrix/cluster_d_ms_chol/run.log`
- Readability proxy: 3.95 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.25 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

