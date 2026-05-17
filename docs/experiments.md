# Experiment Results

_Generated at 2026-05-17T21:19:38+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| LiTAMIN2 paper-comparable on KITTI Odometry 07 (full, 1101 frames) | `ready` | `paper_cov_gt_seed` | 1.156 | 93.9 | `experiments/results/litamin2_kitti_seq_07_full_paper_comp_matrix.json` |
| CT-ICP paper-comparable on KITTI Odometry 07 (full, 1101 frames) | `ready` | `gt_seeded` | 2.693 | 38.0 | `experiments/results/ct_icp_kitti_seq_07_full_paper_comp_matrix.json` |

## LiTAMIN2 paper-comparable on KITTI Odometry 07 (full, 1101 frames)

- **Problem ID**: `litamin2_paper_comparison_kitti_seq_07_full`
- **Question**: Does LiTAMIN2 reach the paper-reported RPE 0.42% on KITTI Odometry 07 under the paper-faithful 3 m profile with velocity-model pure-odometry init?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_seq_07_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_07_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_seq_07_full_paper_comp_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Paper-like 3m + covariance (pure odometry, velocity-model) | paper-faithful pure odometry | 125.057 | 93.9 | 50.5 | 4.30 | 4.50 | Keep as reference variant |
| Paper-like 3m + ICP-only (pure odometry) | paper-faithful simplification | 125.057 | 93.2 | 50.1 | 3.95 | 4.25 | Keep as reference variant |
| Fast local-map + covariance (pure odometry) | throughput-oriented pure odometry | 1.856 | 87.9 | 78.0 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + covariance (Policy A GT-seeded, reference) | paper-faithful Policy A reference | 1.156 | 87.7 | 96.7 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `paper_cov_gt_seed` is the current default for this problem.
2. `paper_cov_no_gt_seed` is the fastest observed variant at 93.9 FPS.
3. `paper_cov_gt_seed` is the most accurate observed variant at 1.156 m ATE.

### Variant Notes

#### `paper_cov_no_gt_seed`

- Intent: Direct paper-comparison: 3 m voxel + covariance cost + velocity-model init.
- CLI args: `--litamin2-paper-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_cov_no_gt_seed/summary.json --litamin2-paper-profile --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_cov_no_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_cov_no_gt_seed/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).

#### `paper_icp_only_no_gt_seed`

- Intent: 3 m voxel with covariance term disabled, pure odometry.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_icp_only_no_gt_seed/summary.json --litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_icp_only_no_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_icp_only_no_gt_seed/run.log`
- Readability proxy: 3.95 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.25 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed). Covariance-shape term disabled.

#### `fast_cov_no_gt_seed`

- Intent: Dogfooding fast profile measured as pure odometry for comparison against paper profile.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/fast_cov_no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/fast_cov_no_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/fast_cov_no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).

#### `paper_cov_gt_seed`

- Intent: Same paper profile but Policy A GT-seeded — non-comparable to paper, kept as Policy A upper bound.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_cov_gt_seed/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_cov_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_seq_07_full_paper_comp_matrix/paper_cov_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## CT-ICP paper-comparable on KITTI Odometry 07 (full, 1101 frames)

- **Problem ID**: `ct_icp_paper_comparison_kitti_seq_07_full`
- **Question**: Does CT-ICP reach the paper-reported RPE 0.30% on KITTI Odometry 07 in default pure-odometry mode? How much does --ct-icp-gt-seed change it?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_seq_07_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_07_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_seq_07_full_paper_comp_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (paper-comparable pure odometry) | paper-style baseline | 6.659 | 38.0 | 70.2 | 5.00 | 5.00 | Keep as reference variant |
| GT-seeded TrajectoryFrame init (Policy A reference) | fair-prior dogfooding | 2.693 | 37.2 | 99.0 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `gt_seeded` is the current default for this problem.
2. `default_pure_odometry` is the fastest observed variant at 38.0 FPS.
3. `gt_seeded` is the most accurate observed variant at 2.693 m ATE.

### Variant Notes

#### `default_pure_odometry`

- Intent: CT-ICP default = pure odometry. This is the directly paper-comparable variant.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_paper_comp_matrix/default_pure_odometry/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_paper_comp_matrix/default_pure_odometry/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_paper_comp_matrix/default_pure_odometry/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `gt_seeded`

- Intent: Symmetric Policy A reference — non-comparable to paper, kept to quantify the GT-seed advantage.
- CLI args: `--ct-icp-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_paper_comp_matrix/gt_seeded/summary.json --ct-icp-gt-seed`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_paper_comp_matrix/gt_seeded/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_paper_comp_matrix/gt_seeded/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. Symmetric of the GT-seeded Policy A path for dogfooding-style fair-prior comparison.

