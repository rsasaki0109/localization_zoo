# Experiment Results

_Generated at 2026-05-17T21:46:37+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| LiTAMIN2 gap-closing on KITTI Odometry 07 (full) | `ready` | `voxel15_iter10` | 1.249 | 91.8 | `experiments/results/litamin2_kitti_seq_07_full_gap_close_matrix.json` |
| CT-ICP gap-closing on KITTI Odometry 07 (full) | `ready` | `dense_plus_budget` | 2.229 | 38.9 | `experiments/results/ct_icp_kitti_seq_07_full_gap_close_matrix.json` |

## LiTAMIN2 gap-closing on KITTI Odometry 07 (full)

- **Problem ID**: `litamin2_gap_close_kitti_seq_07_full`
- **Question**: Can increasing max_iterations / decreasing voxel resolution close the 1.7x gap to paper RPE 0.42% on KITTI 07?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_seq_07_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_07_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_seq_07_full_gap_close_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Baseline fast (8616c27, RPE 0.72%) | baseline | 1.856 | 89.2 | 82.2 | 4.65 | 4.75 | Keep as reference variant |
| max_iterations 12 | iteration-budget | 4.159 | 87.5 | 62.7 | 4.05 | 4.35 | Keep as reference variant |
| voxel 1.0 + iter 12 | finer-voxel + more-iter | 1.964 | 79.8 | 75.3 | 3.45 | 3.95 | Keep as reference variant |
| voxel 1.5 + iter 10 | moderate-finer + moderate-more | 1.249 | 91.8 | 100.0 | 3.45 | 3.95 | Adopt as current default |

### Observations

1. `voxel15_iter10` is the current default for this problem.
2. `voxel15_iter10` is the fastest observed variant at 91.8 FPS.
3. `voxel15_iter10` is the most accurate observed variant at 1.249 m ATE.

### Variant Notes

#### `baseline_fast`

- Intent: Reference point: existing fast profile pure-odometry.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/baseline_fast/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/baseline_fast/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/baseline_fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).

#### `iter12`

- Intent: Doubled inner-loop iterations.
- CLI args: `--no-gt-seed --litamin2-max-iterations 12`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/iter12/summary.json --no-gt-seed --litamin2-max-iterations 12`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/iter12/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/iter12/run.log`
- Readability proxy: 4.05 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.35 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).

#### `voxel1_iter12`

- Intent: Finer voxel grid and more iterations.
- CLI args: `--no-gt-seed --litamin2-voxel-resolution 1.0 --litamin2-max-iterations 12`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/voxel1_iter12/summary.json --no-gt-seed --litamin2-voxel-resolution 1.0 --litamin2-max-iterations 12`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/voxel1_iter12/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/voxel1_iter12/run.log`
- Readability proxy: 3.45 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.95 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).

#### `voxel15_iter10`

- Intent: Halfway between baseline and aggressive tuning.
- CLI args: `--no-gt-seed --litamin2-voxel-resolution 1.5 --litamin2-max-iterations 10`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/voxel15_iter10/summary.json --no-gt-seed --litamin2-voxel-resolution 1.5 --litamin2-max-iterations 10`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/voxel15_iter10/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_gap_close_matrix/voxel15_iter10/run.log`
- Readability proxy: 3.45 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.95 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).


## CT-ICP gap-closing on KITTI Odometry 07 (full)

- **Problem ID**: `ct_icp_gap_close_kitti_seq_07_full`
- **Question**: How much does increasing optimization budget (max_iter, ceres_iter, max_frames_in_map) close the 9x gap to paper RPE 0.30% on KITTI 07?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_seq_07_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_07_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_seq_07_full_gap_close_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Baseline default (8616c27, RPE 2.70%) | baseline | 6.659 | 38.9 | 66.7 | 5.00 | 5.00 | Keep as reference variant |
| Existing --ct-icp-dense-profile | dense built-in | 3.794 | 20.8 | 56.1 | 4.65 | 4.75 | Keep as reference variant |
| max_iter 12, ceres_iter 6, larger map | budget-maximized | 5.428 | 23.3 | 50.5 | 3.20 | 3.80 | Keep as reference variant |
| dense + ceres_iter 6 + max_frames 20 | dense-and-budget | 2.229 | 20.9 | 76.9 | 3.45 | 3.95 | Adopt as current default |

### Observations

1. `dense_plus_budget` is the current default for this problem.
2. `baseline_default` is the fastest observed variant at 38.9 FPS.
3. `dense_plus_budget` is the most accurate observed variant at 2.229 m ATE.

### Variant Notes

#### `baseline_default`

- Intent: Reference: existing default profile (voxel 1.5, max_iter 8, ceres_iter 1).
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/baseline_default/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/baseline_default/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/baseline_default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `dense_profile`

- Intent: Built-in dense profile (max_iter 10, ceres_iter 2, max_frames_in_map 10).
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/dense_profile/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/dense_profile/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/dense_profile/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `max_iter_ceres_budget`

- Intent: Push the optimization budget close to what the paper uses.
- CLI args: `--ct-icp-max-iterations 12 --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/max_iter_ceres_budget/summary.json --ct-icp-max-iterations 12 --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/max_iter_ceres_budget/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/max_iter_ceres_budget/run.log`
- Readability proxy: 3.20 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.80 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `dense_plus_budget`

- Intent: Combine built-in dense profile with extra ceres budget and longer map memory.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/dense_plus_budget/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/dense_plus_budget/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_gap_close_matrix/dense_plus_budget/run.log`
- Readability proxy: 3.45 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.95 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

