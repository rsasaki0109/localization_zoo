# Experiment Results

_Generated at 2026-05-19T22:48:43+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| KISS-ICP cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `fast_profile` | 1.391 | 35.6 | `experiments/results/kiss_icp_kitti_seq_07_full_sweep_matrix.json` |

## KISS-ICP cluster discovery on KITTI Odom seq 07 full (1102 frames)

- **Problem ID**: `kiss_icp_kitti_seq_07_full_sweep`
- **Question**: Does LiTAMIN2 T1 (voxel=0.5 + iter=12) recipe pattern transfer to KISS-ICP on KITTI Odom?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_seq_07_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_07_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_seq_07_full_sweep_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| balanced (default) | reference | 1.799 | 20.8 | 67.9 | 5.00 | 5.00 | Keep as reference variant |
| fast profile | throughput | 2.525 | 35.6 | 77.5 | 4.65 | 4.75 | Adopt as current default |
| dense profile | accuracy | 1.800 | 14.2 | 58.6 | 4.65 | 4.75 | Keep as reference variant |
| T1 transfer (voxel=0.5 + iter=12) | transfer | 1.391 | 13.0 | 68.2 | 3.80 | 4.20 | Keep as reference variant |
| T1 transfer (voxel=0.3 + iter=12) | transfer | 1.450 | 12.8 | 65.9 | 3.80 | 4.20 | Keep as reference variant |

### Observations

1. `fast_profile` is the current default for this problem.
2. `fast_profile` is the fastest observed variant at 35.6 FPS.
3. `t1_transfer_v05_i12` is the most accurate observed variant at 1.391 m ATE.

### Variant Notes

#### `balanced_reference`

- Intent: Baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/balanced_reference/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/balanced_reference/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/balanced_reference/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_profile`

- Intent: Fast profile.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/fast_profile/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/fast_profile/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/fast_profile/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_profile`

- Intent: Dense profile.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/dense_profile/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/dense_profile/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/dense_profile/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `t1_transfer_v05_i12`

- Intent: LiTAMIN2 T1 recipe transfer.
- CLI args: `--kiss-voxel-size 0.5 --kiss-max-iterations 12`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/t1_transfer_v05_i12/summary.json --kiss-voxel-size 0.5 --kiss-max-iterations 12`
- Summary: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/t1_transfer_v05_i12/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/t1_transfer_v05_i12/run.log`
- Readability proxy: 3.80 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.20 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

#### `t1_transfer_v03_i12`

- Intent: Finer voxel variant.
- CLI args: `--kiss-voxel-size 0.3 --kiss-max-iterations 12`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/t1_transfer_v03_i12/summary.json --kiss-voxel-size 0.3 --kiss-max-iterations 12`
- Summary: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/t1_transfer_v03_i12/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_seq_07_full_sweep_matrix/t1_transfer_v03_i12/run.log`
- Readability proxy: 3.80 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.20 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

