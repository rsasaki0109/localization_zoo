# Experiment Results

_Generated at 2026-04-07T13:50:03+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.105 | 5.8 | `experiments/results/aloam_kitti_raw_0009_full_matrix.json` |
| CT-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_window` | 0.556 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_window` | 1.357 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_matrix.json` |
| CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `balanced_window` | 6.820 | 3.1 | `experiments/results/ct_icp_istanbul_window_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `balanced_window` | 7.539 | 2.8 | `experiments/results/ct_icp_istanbul_window_c_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `balanced_window` | 4.673 | 44.9 | `experiments/results/ct_icp_kitti_raw_0009_full_matrix.json` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `balanced_window` | 1.659 | 54.8 | `experiments/results/ct_icp_kitti_raw_0009_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `balanced_window` | 1.659 | 71.4 | `experiments/results/ct_icp_kitti_raw_0009_nogt_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_window` | 6.972 | 37.6 | `experiments/results/ct_icp_kitti_raw_0061_full_matrix.json` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_window` | 1.475 | 56.9 | `experiments/results/ct_icp_kitti_raw_0061_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_window` | 6.115 | 59.8 | `experiments/results/ct_icp_mcd_kth_day_06_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_window` | 0.325 | 71.7 | `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_window` | 1.652 | 71.6 | `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json` |
| CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_window` | 75.075 | 2.7 | `experiments/results/ct_icp_profile_matrix.json` |
| CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data | `blocked` | `-` | - | - | `experiments/results/ct_lio_public_readiness_matrix.json` |
| CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window | `ready` | `seed_only_fast` | 0.412 | 0.5 | `experiments/results/ct_lio_reference_profile_matrix.json` |
| DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `experiments/results/dlio_kitti_raw_0009_full_matrix.json` |
| DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `experiments/results/dlo_kitti_raw_0009_full_matrix.json` |
| F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.452 | 28.6 | `experiments/results/floam_kitti_raw_0009_full_matrix.json` |
| GICP throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.135 | 1.7 | `experiments/results/gicp_hdl_400_reference_b_matrix.json` |
| GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.091 | 1.7 | `experiments/results/gicp_hdl_400_reference_matrix.json` |
| GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 1.166 | 5.7 | `experiments/results/gicp_istanbul_window_b_matrix.json` |
| GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.982 | 4.3 | `experiments/results/gicp_istanbul_window_c_matrix.json` |
| GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 1.170 | 23.0 | `experiments/results/gicp_kitti_raw_0009_full_matrix.json` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 1.177 | 25.8 | `experiments/results/gicp_kitti_raw_0009_matrix.json` |
| GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `dense_recent_map` | 1.510 | 32.0 | `experiments/results/gicp_kitti_raw_0009_nogt_matrix.json` |
| GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 1.055 | 22.8 | `experiments/results/gicp_kitti_raw_0061_full_matrix.json` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.781 | 25.7 | `experiments/results/gicp_kitti_raw_0061_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.630 | 24.7 | `experiments/results/gicp_mcd_kth_day_06_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.017 | 28.7 | `experiments/results/gicp_mcd_ntu_day_02_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.317 | 31.2 | `experiments/results/gicp_mcd_tuhh_night_09_matrix.json` |
| GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.994 | 6.3 | `experiments/results/gicp_profile_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.218 | 0.4 | `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 1.646 | 0.5 | `experiments/results/kiss_icp_hdl_400_reference_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `dense_local_map` | 143.921 | 3.6 | `experiments/results/kiss_icp_istanbul_window_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 131.691 | 3.7 | `experiments/results/kiss_icp_istanbul_window_c_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 5.383 | 21.9 | `experiments/results/kiss_icp_kitti_raw_0009_full_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 2.412 | 24.4 | `experiments/results/kiss_icp_kitti_raw_0009_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_recent_map` | 2.412 | 28.2 | `experiments/results/kiss_icp_kitti_raw_0009_nogt_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 4.343 | 11.2 | `experiments/results/kiss_icp_kitti_raw_0061_full_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.679 | 28.3 | `experiments/results/kiss_icp_kitti_raw_0061_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 5.568 | 11.3 | `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast_recent_map` | 0.017 | 66.7 | `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 1.104 | 24.1 | `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json` |
| KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 182.960 | 4.0 | `experiments/results/kiss_icp_profile_matrix.json` |
| LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.066 | 9.5 | `experiments/results/lego_loam_kitti_raw_0009_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_icp_only_half_threads` | 0.168 | 6.1 | `experiments/results/litamin2_hdl_400_reference_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `paper_icp_only_half_threads` | 0.121 | 6.4 | `experiments/results/litamin2_hdl_400_reference_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.222 | 20.9 | `experiments/results/litamin2_istanbul_window_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `paper_icp_only_half_threads` | 0.741 | 21.2 | `experiments/results/litamin2_istanbul_window_c_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_icp_only_half_threads` | 1.145 | 48.8 | `experiments/results/litamin2_kitti_raw_0009_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `paper_icp_only_half_threads` | 1.053 | 43.7 | `experiments/results/litamin2_kitti_raw_0009_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `paper_cov_half_threads` | 122.275 | 87.3 | `experiments/results/litamin2_kitti_raw_0009_nogt_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_icp_only_half_threads` | 0.944 | 58.1 | `experiments/results/litamin2_kitti_raw_0061_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_cov_half_threads` | 0.511 | 68.6 | `experiments/results/litamin2_kitti_raw_0061_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_cov_half_threads` | 0.401 | 64.2 | `experiments/results/litamin2_mcd_kth_day_06_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `paper_icp_only_half_threads` | 0.045 | 105.6 | `experiments/results/litamin2_mcd_ntu_day_02_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_cov_half_threads` | 0.194 | 106.5 | `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.213 | 23.5 | `experiments/results/litamin2_profile_matrix.json` |
| MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.610 | 3.3 | `experiments/results/mulls_kitti_raw_0009_full_matrix.json` |
| NDT throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.065 | 0.9 | `experiments/results/ndt_hdl_400_reference_b_matrix.json` |
| NDT throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.035 | 0.8 | `experiments/results/ndt_hdl_400_reference_matrix.json` |
| NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.007 | 2.1 | `experiments/results/ndt_istanbul_window_b_matrix.json` |
| NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.005 | 1.9 | `experiments/results/ndt_istanbul_window_c_matrix.json` |
| NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_coarse_map` | 0.255 | 30.9 | `experiments/results/ndt_kitti_raw_0009_full_matrix.json` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_coarse_map` | 0.279 | 36.1 | `experiments/results/ndt_kitti_raw_0009_matrix.json` |
| NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_coarse_map` | 121.733 | 25.0 | `experiments/results/ndt_kitti_raw_0009_nogt_matrix.json` |
| NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_coarse_map` | 0.247 | 23.8 | `experiments/results/ndt_kitti_raw_0061_full_matrix.json` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_coarse_map` | 0.319 | 41.2 | `experiments/results/ndt_kitti_raw_0061_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_coarse_map` | 0.136 | 31.2 | `experiments/results/ndt_mcd_kth_day_06_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `balanced_local_map` | 0.013 | 44.9 | `experiments/results/ndt_mcd_ntu_day_02_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_coarse_map` | 0.063 | 40.8 | `experiments/results/ndt_mcd_tuhh_night_09_matrix.json` |
| NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.070 | 2.0 | `experiments/results/ndt_profile_matrix.json` |
| Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 0.437 | 92.4 | `experiments/results/small_gicp_kitti_raw_0009_full_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.806 | 107.9 | `experiments/results/small_gicp_mcd_kth_day_06_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.031 | 113.8 | `experiments/results/small_gicp_mcd_ntu_day_02_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.250 | 107.2 | `experiments/results/small_gicp_mcd_tuhh_night_09_matrix.json` |
| Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense_recent_map` | 0.640 | 110.1 | `experiments/results/voxel_gicp_kitti_raw_0009_full_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `dense_recent_map` | 0.926 | 124.2 | `experiments/results/voxel_gicp_mcd_kth_day_06_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.121 | 117.2 | `experiments/results/voxel_gicp_mcd_ntu_day_02_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `dense_recent_map` | 0.286 | 116.4 | `experiments/results/voxel_gicp_mcd_tuhh_night_09_matrix.json` |

## A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `aloam_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which A-LOAM profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `aloam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/aloam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 6.463 | 2.6 | 70.0 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 6.105 | 5.8 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 6.187 | 2.7 | 72.3 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 5.8 FPS.
3. `fast` is the most accurate observed variant at 6.105 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline settings for KITTI (n_scans=64).
- CLI args: `--aloam-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods aloam --summary-json experiments/results/runs/aloam_kitti_raw_0009_full_matrix/kitti_default/summary.json --aloam-kitti-profile`
- Summary: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.

#### `fast`

- Intent: Reduce feature density and solver work for speed.
- CLI args: `--aloam-kitti-profile --aloam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods aloam --summary-json experiments/results/runs/aloam_kitti_raw_0009_full_matrix/fast/summary.json --aloam-kitti-profile --aloam-fast-profile`
- Summary: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.

#### `dense`

- Intent: Increase feature density and solver iterations for accuracy.
- CLI args: `--aloam-kitti-profile --aloam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods aloam --summary-json experiments/results/runs/aloam_kitti_raw_0009_full_matrix/dense/summary.json --aloam-kitti-profile --aloam-dense-profile`
- Summary: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.


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


## CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which CT-ICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 4.673 | 40.6 | 95.2 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 5.853 | 44.9 | 89.9 | 4.65 | 4.75 | Keep as active challenger |
| Dense window | drift-oriented | 5.658 | 17.4 | 60.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 44.9 FPS.
3. `balanced_window` is the most accurate observed variant at 4.673 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Fast.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Dense.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0009`
- **Question**: Which CT-ICP profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 1.659 | 34.4 | 81.4 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 2.728 | 54.8 | 80.4 | 4.65 | 4.75 | Keep as active challenger |
| Dense window | drift-oriented | 2.653 | 17.6 | 47.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 54.8 FPS.
3. `balanced_window` is the most accurate observed variant at 1.659 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which CT-ICP profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 1.659 | 44.5 | 81.2 | 4.65 | 4.75 | Adopt as current default |
| Fast window | throughput-oriented | 2.728 | 71.4 | 80.4 | 4.30 | 4.50 | Keep as active challenger |
| Dense window | drift-oriented | 2.653 | 26.8 | 50.0 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 71.4 FPS.
3. `balanced_window` is the most accurate observed variant at 1.659 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/balanced_window/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/balanced_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `fast_window`

- Intent: Fast.
- CLI args: `--ct-icp-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/fast_window/summary.json --ct-icp-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/fast_window/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Dense.
- CLI args: `--ct-icp-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/dense_window/summary.json --ct-icp-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/dense_window/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which CT-ICP profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 20.357 | 21.3 | 45.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 6.972 | 37.6 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 8.894 | 13.3 | 56.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 37.6 FPS.
3. `fast_window` is the most accurate observed variant at 6.972 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Fast.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Dense.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0061`
- **Question**: Which CT-ICP profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 5.029 | 33.2 | 43.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 1.475 | 56.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 1.961 | 18.7 | 54.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 56.9 FPS.
3. `fast_window` is the most accurate observed variant at 1.475 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/dense_window/run.log`
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


## DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `dlio_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which DLIO profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `dlio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/dlio_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced GICP+IMU fusion | 5.543 | 2.8 | 64.7 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 5.026 | 7.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 5.864 | 1.2 | 50.9 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 7.3 FPS.
3. `fast` is the most accurate observed variant at 5.026 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline DLIO-style GICP keyframe odometry with default IMU/LiDAR fusion weights.
- CLI args: `--dlio-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlio --summary-json experiments/results/runs/dlio_kitti_raw_0009_full_matrix/kitti_default/summary.json --dlio-kitti-profile`
- Summary: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration prior between scans when imu.csv and per-frame timestamps align; otherwise LiDAR motion prior only (no GT seed).

#### `fast`

- Intent: Coarser downsampling, fewer GICP iterations, lighter IMU fusion influence.
- CLI args: `--dlio-kitti-profile --dlio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlio --summary-json experiments/results/runs/dlio_kitti_raw_0009_full_matrix/fast/summary.json --dlio-kitti-profile --dlio-fast-profile`
- Summary: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration prior between scans when imu.csv and per-frame timestamps align; otherwise LiDAR motion prior only (no GT seed).

#### `dense`

- Intent: Finer voxels, deeper GICP, stronger IMU term when imu.csv is present.
- CLI args: `--dlio-kitti-profile --dlio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlio --summary-json experiments/results/runs/dlio_kitti_raw_0009_full_matrix/dense/summary.json --dlio-kitti-profile --dlio-dense-profile`
- Summary: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration prior between scans when imu.csv and per-frame timestamps align; otherwise LiDAR motion prior only (no GT seed).


## DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `dlo_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which DLO profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `dlo`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/dlo_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 5.543 | 2.6 | 63.1 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 5.026 | 7.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 5.864 | 1.2 | 50.7 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 7.3 FPS.
3. `fast` is the most accurate observed variant at 5.026 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline DLO GICP keyframe odometry for KITTI-scale LiDAR.
- CLI args: `--dlo-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlo --summary-json experiments/results/runs/dlo_kitti_raw_0009_full_matrix/kitti_default/summary.json --dlo-kitti-profile`
- Summary: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarser downsampling and fewer GICP iterations.
- CLI args: `--dlo-kitti-profile --dlo-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlo --summary-json experiments/results/runs/dlo_kitti_raw_0009_full_matrix/fast/summary.json --dlo-kitti-profile --dlo-fast-profile`
- Summary: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Finer voxels, more map keyframes, and deeper GICP.
- CLI args: `--dlo-kitti-profile --dlo-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlo --summary-json experiments/results/runs/dlo_kitti_raw_0009_full_matrix/dense/summary.json --dlo-kitti-profile --dlo-dense-profile`
- Summary: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).


## F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `floam_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which F-LOAM profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `floam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/floam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 6.564 | 11.9 | 62.4 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 5.452 | 28.6 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 6.094 | 2.8 | 49.6 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 28.6 FPS.
3. `fast` is the most accurate observed variant at 5.452 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline F-LOAM settings for KITTI (n_scans=64).
- CLI args: `--floam-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods floam --summary-json experiments/results/runs/floam_kitti_raw_0009_full_matrix/kitti_default/summary.json --floam-kitti-profile`
- Summary: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).

#### `fast`

- Intent: Thin input and throttle mapping for speed.
- CLI args: `--floam-kitti-profile --floam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods floam --summary-json experiments/results/runs/floam_kitti_raw_0009_full_matrix/fast/summary.json --floam-kitti-profile --floam-fast-profile`
- Summary: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).

#### `dense`

- Intent: Process denser input and update mapping every frame.
- CLI args: `--floam-kitti-profile --floam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods floam --summary-json experiments/results/runs/floam_kitti_raw_0009_full_matrix/dense/summary.json --floam-kitti-profile --floam-dense-profile`
- Summary: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).


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


## GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which GICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.378 | 12.5 | 69.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.170 | 23.0 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.261 | 7.6 | 62.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 23.0 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.170 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0009`
- **Question**: Which GICP profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.455 | 14.1 | 67.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.177 | 25.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.358 | 7.6 | 58.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 25.8 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.177 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which GICP profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 2.206 | 16.3 | 59.7 | 4.65 | 4.75 | Keep as reference variant |
| Fast recent map | throughput-oriented | 14.057 | 32.0 | 55.4 | 4.30 | 4.50 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 1.510 | 10.9 | 67.1 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 32.0 FPS.
3. `dense_recent_map` is the most accurate observed variant at 1.510 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/balanced_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--gicp-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json --gicp-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/fast_recent_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--gicp-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/dense_recent_map/summary.json --gicp-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/dense_recent_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which GICP profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.173 | 13.8 | 75.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.081 | 22.8 | 98.8 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.055 | 7.3 | 66.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 22.8 FPS.
3. `dense_recent_map` is the most accurate observed variant at 1.055 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_full_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0061`
- **Question**: Which GICP profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.846 | 14.5 | 74.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.959 | 25.7 | 90.7 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.781 | 9.2 | 68.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 25.7 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.781 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_matrix/dense_recent_map/run.log`
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


## KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which KISS-ICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 5.383 | 9.5 | 71.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 5.839 | 21.9 | 96.1 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 5.456 | 7.0 | 65.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 21.9 FPS.
3. `balanced_local_map` is the most accurate observed variant at 5.383 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0009`
- **Question**: Which KISS-ICP profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 2.412 | 12.5 | 75.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 2.642 | 24.4 | 95.7 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 2.480 | 7.3 | 63.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 24.4 FPS.
3. `balanced_local_map` is the most accurate observed variant at 2.412 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which KISS-ICP profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 2.412 | 15.1 | 76.8 | 4.65 | 4.75 | Keep as reference variant |
| Fast recent map | throughput-oriented | 2.642 | 28.2 | 95.7 | 4.30 | 4.50 | Adopt as current default |
| Dense local map | drift-oriented | 2.480 | 11.7 | 69.4 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 28.2 FPS.
3. `balanced_local_map` is the most accurate observed variant at 2.412 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/balanced_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--kiss-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json --kiss-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/fast_recent_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--kiss-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json --kiss-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/dense_local_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which KISS-ICP profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 4.343 | 9.7 | 93.4 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 4.623 | 11.2 | 97.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 4.706 | 3.0 | 59.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 11.2 FPS.
3. `balanced_local_map` is the most accurate observed variant at 4.343 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0061`
- **Question**: Which KISS-ICP profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.738 | 15.4 | 73.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.679 | 28.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 0.773 | 10.0 | 61.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 28.3 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.679 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/dense_local_map/run.log`
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


## LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `lego_loam_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which LeGO-LOAM profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lego_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lego_loam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 6.066 | 2.9 | 65.5 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 6.498 | 9.5 | 96.7 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 7.249 | 3.5 | 60.2 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 9.5 FPS.
3. `kitti_default` is the most accurate observed variant at 6.066 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Ground-aware LeGO-LOAM baseline for HDL-64E (n_scans=64, sensor height ~KITTI).
- CLI args: `--lego-loam-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lego_loam --summary-json experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/kitti_default/summary.json --lego-loam-kitti-profile`
- Summary: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.

#### `fast`

- Intent: Coarser surf sampling and lighter odometry/mapping Ceres budgets.
- CLI args: `--lego-loam-kitti-profile --lego-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lego_loam --summary-json experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/fast/summary.json --lego-loam-kitti-profile --lego-loam-fast-profile`
- Summary: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.

#### `dense`

- Intent: Finer features and deeper optimization passes.
- CLI args: `--lego-loam-kitti-profile --lego-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lego_loam --summary-json experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/dense/summary.json --lego-loam-kitti-profile --lego-loam-dense-profile`
- Summary: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.


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


## LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which LiTAMIN2 profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.145 | 37.9 | 88.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 1.145 | 48.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.438 | 43.3 | 84.2 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.438 | 28.1 | 68.6 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 48.8 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.145 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove covariance-shape term.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Paper 3m profile.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Paper 3m + ICP-only.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0009`
- **Question**: Which LiTAMIN2 profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.053 | 27.1 | 81.0 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 1.053 | 30.0 | 84.3 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 1.397 | 33.8 | 76.4 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.397 | 43.7 | 87.7 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `paper_icp_only_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 43.7 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.053 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which LiTAMIN2 profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 122.280 | 63.6 | 86.4 | 4.65 | 4.75 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 122.280 | 81.7 | 96.8 | 4.30 | 4.50 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 122.275 | 87.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Paper-like 3m + ICP-only | paper-oriented simplification | 122.275 | 82.7 | 97.3 | 3.95 | 4.25 | Keep as active challenger |

### Observations

1. `paper_cov_half_threads` is the current default for this problem.
2. `paper_cov_half_threads` is the fastest observed variant at 87.3 FPS.
3. `paper_cov_half_threads` is the most accurate observed variant at 122.275 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_cov_half_threads/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `fast_icp_only_half_threads`

- Intent: Remove covariance-shape term.
- CLI args: `--litamin2-icp-only --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed). Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Paper 3m profile.
- CLI args: `--litamin2-paper-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `paper_icp_only_half_threads`

- Intent: Paper 3m + ICP-only.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 3.95 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.25 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed). Covariance-shape term disabled.


## LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which LiTAMIN2 profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.944 | 53.5 | 96.0 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 0.944 | 58.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.325 | 36.3 | 66.8 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.325 | 43.2 | 72.8 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 58.1 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.944 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove covariance-shape term.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Paper 3m profile.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Paper 3m + ICP-only.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0061`
- **Question**: Which LiTAMIN2 profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.511 | 67.7 | 99.4 | 5.00 | 5.00 | Adopt as current default |
| Fast local-map + ICP-only | single-term simplification | 0.511 | 64.9 | 97.3 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 1.130 | 67.2 | 71.6 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.130 | 68.6 | 72.6 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_cov_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 68.6 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.511 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_icp_only_half_threads/run.log`
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


## MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `mulls_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which MULLS profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `mulls`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/mulls_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 4.933 | 1.2 | 64.5 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 4.610 | 3.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 5.004 | 1.0 | 60.9 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 3.3 FPS.
3. `fast` is the most accurate observed variant at 4.610 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: A-LOAM-style features + multi-metric scan-to-map (line/plane/point) baseline for HDL-64E.
- CLI args: `--mulls-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods mulls --summary-json experiments/results/runs/mulls_kitti_raw_0009_full_matrix/kitti_default/summary.json --mulls-kitti-profile`
- Summary: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).

#### `fast`

- Intent: Coarser voxels, lighter Ceres, shorter submap window, sparser point metric.
- CLI args: `--mulls-kitti-profile --mulls-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods mulls --summary-json experiments/results/runs/mulls_kitti_raw_0009_full_matrix/fast/summary.json --mulls-kitti-profile --mulls-fast-profile`
- Summary: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).

#### `dense`

- Intent: Finer map voxels, more knn, deeper Ceres, longer history.
- CLI args: `--mulls-kitti-profile --mulls-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods mulls --summary-json experiments/results/runs/mulls_kitti_raw_0009_full_matrix/dense/summary.json --mulls-kitti-profile --mulls-dense-profile`
- Summary: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).


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


## NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which NDT profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.255 | 21.1 | 84.1 | 5.00 | 5.00 | Keep as active challenger |
| Fast coarse map | throughput-oriented | 0.307 | 30.9 | 91.5 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.291 | 14.5 | 67.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 30.9 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.255 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Fast.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_full_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_full_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0009`
- **Question**: Which NDT profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.279 | 22.9 | 81.7 | 5.00 | 5.00 | Keep as active challenger |
| Fast coarse map | throughput-oriented | 0.374 | 36.1 | 87.3 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.358 | 13.5 | 57.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 36.1 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.279 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which NDT profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 122.662 | 22.8 | 95.2 | 4.65 | 4.75 | Keep as active challenger |
| Fast coarse map | throughput-oriented | 122.547 | 25.0 | 99.7 | 4.30 | 4.50 | Adopt as current default |
| Dense local map | accuracy-oriented | 121.733 | 12.2 | 74.4 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 25.0 FPS.
3. `dense_local_map` is the most accurate observed variant at 121.733 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/balanced_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `fast_coarse_map`

- Intent: Fast.
- CLI args: `--ndt-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/fast_coarse_map/summary.json --ndt-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--ndt-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json --ndt-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/dense_local_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which NDT profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.259 | 15.3 | 79.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.247 | 23.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.276 | 10.1 | 66.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 23.8 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.247 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Fast.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_full_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_full_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0061`
- **Question**: Which NDT profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.325 | 28.4 | 83.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.319 | 41.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.361 | 15.5 | 63.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 41.2 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.319 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_matrix/dense_local_map/run.log`
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


## Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `small_gicp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which Small-GICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.105 | 51.8 | 47.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.437 | 92.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.090 | 41.0 | 42.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 92.4 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.437 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `small_gicp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which Small-GICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.070 | 57.7 | 64.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.806 | 107.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.898 | 37.5 | 62.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 107.9 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.806 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `small_gicp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which Small-GICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.101 | 69.6 | 45.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.111 | 113.8 | 63.8 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.031 | 56.8 | 74.9 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 113.8 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.031 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `small_gicp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which Small-GICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.325 | 54.1 | 63.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.466 | 107.2 | 76.8 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.250 | 40.2 | 68.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 107.2 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.250 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `voxel_gicp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which Voxel-GICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.102 | 18.8 | 37.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.253 | 52.7 | 49.5 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.640 | 110.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 110.1 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.640 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `voxel_gicp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which Voxel-GICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.926 | 18.2 | 57.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.263 | 42.2 | 53.6 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.981 | 124.2 | 97.2 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 124.2 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.926 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `voxel_gicp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which Voxel-GICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.598 | 44.4 | 29.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.518 | 64.9 | 39.3 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.121 | 117.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 117.2 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.121 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `voxel_gicp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which Voxel-GICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.381 | 26.2 | 48.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.286 | 57.5 | 74.7 | 4.65 | 4.75 | Keep as active challenger |
| Dense recent map | accuracy-oriented | 0.478 | 116.4 | 79.9 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 116.4 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.286 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

