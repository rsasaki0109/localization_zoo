# Experiment Results

_Generated at 2026-04-06T06:33:08+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| CT-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_window` | 0.556 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_window` | 1.357 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_matrix.json` |
| CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `balanced_window` | 6.820 | 3.1 | `experiments/results/ct_icp_istanbul_window_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `balanced_window` | 7.539 | 2.8 | `experiments/results/ct_icp_istanbul_window_c_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_window` | 6.115 | 59.8 | `experiments/results/ct_icp_mcd_kth_day_06_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_window` | 0.325 | 71.7 | `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_window` | 1.652 | 71.6 | `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json` |
| CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_window` | 75.075 | 2.7 | `experiments/results/ct_icp_profile_matrix.json` |
| CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data | `blocked` | `-` | - | - | `experiments/results/ct_lio_public_readiness_matrix.json` |
| CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window | `ready` | `seed_only_fast` | 0.412 | 0.5 | `experiments/results/ct_lio_reference_profile_matrix.json` |
| GICP throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.135 | 1.7 | `experiments/results/gicp_hdl_400_reference_b_matrix.json` |
| GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.091 | 1.7 | `experiments/results/gicp_hdl_400_reference_matrix.json` |
| GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 1.166 | 5.7 | `experiments/results/gicp_istanbul_window_b_matrix.json` |
| GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.982 | 4.3 | `experiments/results/gicp_istanbul_window_c_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.630 | 24.7 | `experiments/results/gicp_mcd_kth_day_06_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.017 | 28.7 | `experiments/results/gicp_mcd_ntu_day_02_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.317 | 31.2 | `experiments/results/gicp_mcd_tuhh_night_09_matrix.json` |
| GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.994 | 6.3 | `experiments/results/gicp_profile_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.218 | 0.4 | `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 1.646 | 0.5 | `experiments/results/kiss_icp_hdl_400_reference_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `dense_local_map` | 143.921 | 3.6 | `experiments/results/kiss_icp_istanbul_window_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 131.691 | 3.7 | `experiments/results/kiss_icp_istanbul_window_c_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 5.568 | 11.3 | `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast_recent_map` | 0.017 | 66.7 | `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 1.104 | 24.1 | `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json` |
| KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 182.960 | 4.0 | `experiments/results/kiss_icp_profile_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_icp_only_half_threads` | 0.168 | 6.1 | `experiments/results/litamin2_hdl_400_reference_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `paper_icp_only_half_threads` | 0.121 | 6.4 | `experiments/results/litamin2_hdl_400_reference_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.222 | 20.9 | `experiments/results/litamin2_istanbul_window_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `paper_icp_only_half_threads` | 0.741 | 21.2 | `experiments/results/litamin2_istanbul_window_c_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_cov_half_threads` | 0.401 | 64.2 | `experiments/results/litamin2_mcd_kth_day_06_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `paper_icp_only_half_threads` | 0.045 | 105.6 | `experiments/results/litamin2_mcd_ntu_day_02_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_cov_half_threads` | 0.194 | 106.5 | `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.213 | 23.5 | `experiments/results/litamin2_profile_matrix.json` |
| NDT throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.065 | 0.9 | `experiments/results/ndt_hdl_400_reference_b_matrix.json` |
| NDT throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.035 | 0.8 | `experiments/results/ndt_hdl_400_reference_matrix.json` |
| NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.007 | 2.1 | `experiments/results/ndt_istanbul_window_b_matrix.json` |
| NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.005 | 1.9 | `experiments/results/ndt_istanbul_window_c_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_coarse_map` | 0.136 | 31.2 | `experiments/results/ndt_mcd_kth_day_06_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `balanced_local_map` | 0.013 | 44.9 | `experiments/results/ndt_mcd_ntu_day_02_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_coarse_map` | 0.063 | 40.8 | `experiments/results/ndt_mcd_tuhh_night_09_matrix.json` |
| NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.070 | 2.0 | `experiments/results/ndt_profile_matrix.json` |

## CT-ICP throughput and drift trade-off on the second public HDL-400 reference window

- **Problem ID**: `ct_icp_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 0.873 | 1.2 | 57.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 1.211 | 2.4 | 73.0 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 0.556 | 0.5 | 60.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 2.4 FPS.
3. `dense_window` is the most accurate observed variant at 0.556 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the public HDL-400 reference window

- **Problem ID**: `ct_icp_profile_tradeoff_hdl_400_reference`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 1.698 | 1.3 | 66.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 1.513 | 2.4 | 94.8 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 1.357 | 0.5 | 60.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 2.4 FPS.
3. `dense_window` is the most accurate observed variant at 1.357 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `ct_icp_profile_tradeoff_istanbul_window_b`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 6.820 | 1.6 | 75.3 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 35.035 | 3.1 | 59.7 | 4.65 | 4.75 | Keep as reference variant |
| Dense window | drift-oriented | 22.322 | 0.6 | 25.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 3.1 FPS.
3. `balanced_window` is the most accurate observed variant at 6.820 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_b_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_b_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_b_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `ct_icp_profile_tradeoff_istanbul_window_c`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 7.539 | 1.3 | 73.4 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 42.692 | 2.8 | 58.8 | 4.65 | 4.75 | Keep as reference variant |
| Dense window | drift-oriented | 15.285 | 0.5 | 33.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 2.8 FPS.
3. `balanced_window` is the most accurate observed variant at 7.539 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_c_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_c_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_c_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `ct_icp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which CT-ICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 6.295 | 34.6 | 77.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 6.525 | 59.8 | 96.9 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 6.115 | 15.3 | 62.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 59.8 FPS.
3. `dense_window` is the most accurate observed variant at 6.115 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `ct_icp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which CT-ICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 0.926 | 39.2 | 44.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 2.498 | 71.7 | 56.5 | 4.65 | 4.75 | Keep as reference variant |
| Dense window | drift-oriented | 0.325 | 22.0 | 65.3 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 71.7 FPS.
3. `dense_window` is the most accurate observed variant at 0.325 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `ct_icp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which CT-ICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 2.387 | 39.2 | 62.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 3.553 | 71.6 | 73.3 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 1.652 | 19.4 | 63.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 71.6 FPS.
3. `dense_window` is the most accurate observed variant at 1.652 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `ct_icp_profile_tradeoff`
- **Question**: Which CT-ICP odometry profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 75.075 | 1.4 | 74.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 79.761 | 2.7 | 97.1 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 76.350 | 0.5 | 58.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 2.7 FPS.
3. `balanced_window` is the most accurate observed variant at 75.075 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_profile_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_profile_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_profile_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_profile_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_profile_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_profile_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_profile_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_profile_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_profile_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data

- **Problem ID**: `ct_lio_public_gt_readiness`
- **Question**: What still blocks a GT-backed CT-LIO public benchmark contract after the reference-based HDL-400 comparison is available?
- **Status**: `blocked`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_60`
- **Reference CSV**: `MISSING: repository-usable open GT CSV for HDL-400 is not prepared yet`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_lio_public_readiness_matrix.json`
- **Blocker**: A reference-based CT-LIO comparison now exists separately, but this repository still does not ship an independently curated GT CSV aligned to the extracted HDL-400 LiDAR+IMU sequence.
- **Next step**: No action planned. CT-LIO GT-backed is excluded from the main study scope. If an independent GT source becomes available for HDL-400, this can be revisited.

### Planned Variants

| Variant | Style | Intent | Args |
|---------|-------|--------|------|
| IMU preintegration default | balanced | Baseline CT-LIO profile with IMU bias estimation and a short fixed-lag prior. | `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 4` |
| Seed-only fast | throughput-oriented | Keep the IMU seed but avoid the heavier lag smoother path. | `--ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1` |
| History smoother dense | accuracy-oriented | Use a slightly longer lag window and one extra relinearization pass once a GT-backed public benchmark contract is ready. | `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2` |

## CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window

- **Problem ID**: `ct_lio_reference_tradeoff`
- **Question**: Which CT-LIO profile should stay as the current default when compared against a shared public reference trajectory export?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_lio_reference_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| IMU preintegration default | balanced | 2.224 | 0.5 | 59.2 | 4.05 | 4.35 | Keep as reference variant |
| Seed-only fast | throughput-oriented | 0.412 | 0.4 | 90.5 | 3.80 | 4.20 | Adopt as current default |
| History smoother dense | accuracy-oriented | 1.722 | 0.5 | 61.3 | 3.45 | 3.95 | Keep as reference variant |

### Observations

1. `seed_only_fast` is the current default for this problem.
2. `imu_preintegration_default` is the fastest observed variant at 0.5 FPS.
3. `seed_only_fast` is the most accurate observed variant at 0.412 m ATE.

### Variant Notes

#### `imu_preintegration_default`

- Intent: Baseline CT-LIO profile with IMU bias estimation and a short fixed-lag prior.
- CLI args: `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 4`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_reference_profile_matrix/imu_preintegration_default/summary.json --ct-lio-estimate-bias --ct-lio-fixed-lag-window 4`
- Summary: `experiments/results/runs/ct_lio_reference_profile_matrix/imu_preintegration_default/summary.json`
- Log: `experiments/results/runs/ct_lio_reference_profile_matrix/imu_preintegration_default/run.log`
- Readability proxy: 4.05 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.35 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

#### `seed_only_fast`

- Intent: Keep the IMU seed but avoid the heavier lag smoother path.
- CLI args: `--ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_reference_profile_matrix/seed_only_fast/summary.json --ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1`
- Summary: `experiments/results/runs/ct_lio_reference_profile_matrix/seed_only_fast/summary.json`
- Log: `experiments/results/runs/ct_lio_reference_profile_matrix/seed_only_fast/run.log`
- Readability proxy: 3.80 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.20 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

#### `history_smoother_dense`

- Intent: Use a slightly longer lag window and one extra relinearization pass for stronger motion regularization without the unbounded smoother cost.
- CLI args: `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_reference_profile_matrix/history_smoother_dense/summary.json --ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2`
- Summary: `experiments/results/runs/ct_lio_reference_profile_matrix/history_smoother_dense/summary.json`
- Log: `experiments/results/runs/ct_lio_reference_profile_matrix/history_smoother_dense/run.log`
- Readability proxy: 3.45 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.95 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.


## GICP throughput and accuracy trade-off on the second public HDL-400 reference window

- **Problem ID**: `gicp_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which GICP dogfooding profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.207 | 0.8 | 56.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.284 | 1.7 | 73.8 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.135 | 0.5 | 63.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 1.7 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.135 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_b_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_b_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the public HDL-400 reference window

- **Problem ID**: `gicp_profile_tradeoff_hdl_400_reference`
- **Question**: Which GICP dogfooding profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.161 | 0.8 | 53.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.193 | 1.7 | 73.4 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.091 | 0.5 | 64.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 1.7 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.091 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `gicp_profile_tradeoff_istanbul_window_b`
- **Question**: Which GICP dogfooding profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.233 | 3.1 | 74.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.166 | 5.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.228 | 1.6 | 61.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 5.7 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.166 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_b_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_b_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_b_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_b_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `gicp_profile_tradeoff_istanbul_window_c`
- **Question**: Which GICP dogfooding profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.254 | 2.3 | 66.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.982 | 4.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.295 | 1.3 | 53.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 4.3 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.982 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_c_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_c_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_c_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_c_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_c_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_c_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_c_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `gicp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which GICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.026 | 12.9 | 56.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.630 | 24.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.175 | 7.2 | 41.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 24.7 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.630 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `gicp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which GICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.058 | 20.3 | 50.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.256 | 28.7 | 53.3 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.017 | 13.0 | 72.7 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 28.7 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.017 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `gicp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which GICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.490 | 17.2 | 59.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.317 | 31.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.549 | 9.2 | 43.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 31.2 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.317 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `gicp_profile_tradeoff`
- **Question**: Which GICP dogfooding profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.994 | 3.4 | 77.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.074 | 6.3 | 96.3 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.009 | 1.9 | 64.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 6.3 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.994 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_profile_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_profile_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_profile_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_profile_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_profile_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_profile_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_profile_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_profile_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_profile_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window

- **Problem ID**: `kiss_icp_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.498 | 0.2 | 43.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.218 | 0.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 0.447 | 0.1 | 39.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 0.4 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.218 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the public HDL-400 reference window

- **Problem ID**: `kiss_icp_profile_tradeoff_hdl_400_reference`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 4.300 | 0.2 | 42.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.646 | 0.5 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 3.464 | 0.1 | 36.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 0.5 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.646 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_istanbul_window_b`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 143.956 | 3.4 | 97.8 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 143.921 | 3.6 | 99.8 | 4.65 | 4.75 | Keep as active challenger |
| Dense local map | drift-oriented | 144.086 | 3.6 | 99.9 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_local_map` is the current default for this problem.
2. `dense_local_map` is the fastest observed variant at 3.6 FPS.
3. `fast_recent_map` is the most accurate observed variant at 143.921 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_b_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_b_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_istanbul_window_c`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 131.691 | 3.4 | 95.2 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 131.692 | 3.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 131.752 | 3.3 | 93.4 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 3.7 FPS.
3. `balanced_local_map` is the most accurate observed variant at 131.691 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_c_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_c_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which KISS-ICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 6.095 | 4.2 | 64.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 5.568 | 11.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 6.109 | 1.9 | 53.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 11.3 FPS.
3. `fast_recent_map` is the most accurate observed variant at 5.568 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which KISS-ICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.024 | 42.2 | 66.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.026 | 66.7 | 83.2 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 0.017 | 34.0 | 75.5 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 66.7 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.017 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which KISS-ICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.104 | 9.4 | 69.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.303 | 24.1 | 92.4 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 1.139 | 6.3 | 61.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 24.1 FPS.
3. `balanced_local_map` is the most accurate observed variant at 1.104 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `kiss_icp_profile_tradeoff`
- **Question**: Which KISS-ICP odometry profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 183.178 | 3.7 | 95.5 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 182.960 | 4.0 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 183.226 | 3.4 | 91.7 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 4.0 FPS.
3. `fast_recent_map` is the most accurate observed variant at 182.960 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_profile_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_profile_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_profile_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_profile_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_profile_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_profile_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_profile_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_profile_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_profile_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window

- **Problem ID**: `litamin2_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a deeper HDL-400 public reference set without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.168 | 4.9 | 89.9 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 0.168 | 5.2 | 92.8 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 0.274 | 5.8 | 78.3 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.274 | 6.1 | 80.8 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 6.1 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.168 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window

- **Problem ID**: `litamin2_profile_tradeoff_hdl_400_reference`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a different public dataset family without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.133 | 5.2 | 85.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 0.133 | 5.6 | 88.9 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + covariance | paper-oriented | 0.121 | 6.2 | 98.4 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.121 | 6.4 | 100.0 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `paper_icp_only_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 6.4 FPS.
3. `paper_cov_half_threads` is the most accurate observed variant at 0.121 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `litamin2_profile_tradeoff_istanbul_window_b`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a second public Istanbul window without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.222 | 18.6 | 94.3 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 1.222 | 20.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.332 | 15.0 | 81.6 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.332 | 17.0 | 86.5 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 20.9 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.222 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `litamin2_profile_tradeoff_istanbul_window_c`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a third public Istanbul window without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.930 | 18.0 | 82.4 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 0.930 | 21.2 | 89.9 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 0.741 | 14.8 | 85.0 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.741 | 17.2 | 90.8 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `paper_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 21.2 FPS.
3. `paper_cov_half_threads` is the most accurate observed variant at 0.741 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `litamin2_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which LiTAMIN2 profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.401 | 64.2 | 100.0 | 5.00 | 5.00 | Adopt as current default |
| Fast local-map + ICP-only | single-term simplification | 0.401 | 53.6 | 91.8 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 0.953 | 45.2 | 56.3 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.953 | 54.5 | 63.5 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_cov_half_threads` is the current default for this problem.
2. `fast_cov_half_threads` is the fastest observed variant at 64.2 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.401 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `litamin2_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which LiTAMIN2 profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.075 | 99.5 | 77.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 0.075 | 100.6 | 77.9 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + covariance | paper-oriented | 0.045 | 102.8 | 98.7 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.045 | 105.6 | 100.0 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `paper_icp_only_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 105.6 FPS.
3. `paper_cov_half_threads` is the most accurate observed variant at 0.045 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `litamin2_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which LiTAMIN2 profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.194 | 106.5 | 100.0 | 5.00 | 5.00 | Adopt as current default |
| Fast local-map + ICP-only | single-term simplification | 0.194 | 105.8 | 99.7 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 0.291 | 97.9 | 79.2 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.291 | 106.4 | 83.2 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_cov_half_threads` is the current default for this problem.
2. `fast_cov_half_threads` is the fastest observed variant at 106.5 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.194 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `litamin2_profile_tradeoff`
- **Question**: Which LiTAMIN2 profile should stay as the current default without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.213 | 21.5 | 95.6 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 1.213 | 23.5 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.264 | 15.5 | 81.0 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.264 | 18.0 | 86.3 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 23.5 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.213 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_profile_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_profile_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_profile_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_profile_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## NDT throughput and accuracy trade-off on the second public HDL-400 reference window

- **Problem ID**: `ndt_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which NDT dogfooding profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.090 | 0.5 | 63.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.065 | 0.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.087 | 0.2 | 51.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 0.9 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.065 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_b_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_b_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the public HDL-400 reference window

- **Problem ID**: `ndt_profile_tradeoff_hdl_400_reference`
- **Question**: Which NDT dogfooding profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.045 | 0.4 | 65.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.051 | 0.8 | 84.4 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.035 | 0.2 | 65.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 0.8 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.035 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `ndt_profile_tradeoff_istanbul_window_b`
- **Question**: Which NDT dogfooding profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.062 | 1.5 | 41.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.007 | 2.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.067 | 1.0 | 30.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 2.1 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.007 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_b_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_b_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_b_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_b_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `ndt_profile_tradeoff_istanbul_window_c`
- **Question**: Which NDT dogfooding profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.035 | 1.4 | 44.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.005 | 1.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.105 | 1.0 | 28.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 1.9 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.005 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_c_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_c_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_c_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_c_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_c_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_c_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_c_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `ndt_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which NDT profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.166 | 18.3 | 70.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.208 | 31.2 | 82.7 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.136 | 10.9 | 67.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 31.2 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.136 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_kth_day_06_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_kth_day_06_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `ndt_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which NDT profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.014 | 32.7 | 80.8 | 5.00 | 5.00 | Adopt as current default |
| Fast coarse map | throughput-oriented | 0.024 | 44.9 | 76.0 | 4.65 | 4.75 | Keep as active challenger |
| Dense local map | accuracy-oriented | 0.013 | 22.0 | 74.5 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `balanced_local_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 44.9 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.013 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_ntu_day_02_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_ntu_day_02_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `ndt_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which NDT profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.075 | 26.0 | 74.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.070 | 40.8 | 94.7 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.063 | 14.2 | 67.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 40.8 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.063 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `ndt_profile_tradeoff`
- **Question**: Which NDT dogfooding profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.109 | 1.4 | 67.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.070 | 2.0 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.122 | 1.0 | 54.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 2.0 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.070 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_profile_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_profile_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_profile_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_profile_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_profile_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_profile_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_profile_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_profile_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_profile_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

