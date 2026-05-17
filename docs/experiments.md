# Experiment Results

_Generated at 2026-05-17T15:55:45+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| LiTAMIN2 pure-odometry (no GT seed) on MulRan ParkingLot (full sequence) | `ready` | `paper_icp_only_no_gt_seed` | 74.403 | 206.2 | `experiments/results/litamin2_mulran_parkinglot_full_no_gt_seed_matrix.json` |

## LiTAMIN2 pure-odometry (no GT seed) on MulRan ParkingLot (full sequence)

- **Problem ID**: `litamin2_profile_tradeoff_mulran_parkinglot_full_no_gt_seed`
- **Question**: Does LiTAMIN2 hold up on MulRan parkinglot full as standalone odometry (Policy B), or does it diverge similarly to CT-ICP once the GT prior is removed?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mulran_parkinglot_full`
- **Reference CSV**: `experiments/reference_data/mulran_parkinglot_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_mulran_parkinglot_full_no_gt_seed_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance (no GT seed) | throughput-oriented | 74.403 | 115.4 | 78.0 | 4.65 | 4.75 | Keep as reference variant |
| Fast local-map + ICP-only (no GT seed) | single-term simplification | 74.403 | 94.1 | 72.8 | 4.30 | 4.50 | Keep as reference variant |
| Paper-like 3m + covariance (no GT seed) | paper-oriented | 79.965 | 119.7 | 75.5 | 4.30 | 4.50 | Keep as reference variant |
| Paper-like 3m + ICP-only (no GT seed) | paper-oriented simplification | 79.965 | 206.2 | 96.5 | 3.95 | 4.25 | Adopt as current default |

### Observations

1. `paper_icp_only_no_gt_seed` is the current default for this problem.
2. `paper_icp_only_no_gt_seed` is the fastest observed variant at 206.2 FPS.
3. `fast_cov_no_gt_seed` is the most accurate observed variant at 74.403 m ATE.

### Variant Notes

#### `fast_cov_no_gt_seed`

- Intent: Repository default local-map profile measured as pure odometry to match Policy B methods.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mulran_parkinglot_full experiments/reference_data/mulran_parkinglot_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/fast_cov_no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/fast_cov_no_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/fast_cov_no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).

#### `fast_icp_only_no_gt_seed`

- Intent: Disable covariance-shape term and run as pure odometry.
- CLI args: `--litamin2-icp-only --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mulran_parkinglot_full experiments/reference_data/mulran_parkinglot_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/fast_icp_only_no_gt_seed/summary.json --litamin2-icp-only --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/fast_icp_only_no_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/fast_icp_only_no_gt_seed/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed). Covariance-shape term disabled.

#### `paper_cov_no_gt_seed`

- Intent: Paper-like 3m local map measured as pure odometry.
- CLI args: `--litamin2-paper-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mulran_parkinglot_full experiments/reference_data/mulran_parkinglot_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/paper_cov_no_gt_seed/summary.json --litamin2-paper-profile --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/paper_cov_no_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/paper_cov_no_gt_seed/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed).

#### `paper_icp_only_no_gt_seed`

- Intent: Paper profile without covariance cost, pure odometry.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mulran_parkinglot_full experiments/reference_data/mulran_parkinglot_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/paper_icp_only_no_gt_seed/summary.json --litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/paper_icp_only_no_gt_seed/summary.json`
- Log: `experiments/results/runs/litamin2_mulran_parkinglot_full_no_gt_seed_matrix/paper_icp_only_no_gt_seed/run.log`
- Readability proxy: 3.95 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.25 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses velocity-model prediction as scan-to-map initial guess (no GT seed). Covariance-shape term disabled.

