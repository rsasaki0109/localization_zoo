# Experiment Results

_Generated at 2026-05-17T22:34:20+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| CT-ICP architecture-level tuning on KITTI Odometry 07 (full) | `ready` | `prior_best` | 2.229 | 18.5 | `experiments/results/ct_icp_kitti_seq_07_full_arch_close_matrix.json` |

## CT-ICP architecture-level tuning on KITTI Odometry 07 (full)

- **Problem ID**: `ct_icp_arch_close_kitti_seq_07_full`
- **Question**: Do tighter max_correspondence_dist, lower keypoint_voxel_size, higher planarity_threshold, larger ceres budget close more than the 4.5x parameter ceiling?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_seq_07_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_07_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_seq_07_full_arch_close_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Prior best (dense + ceres 6 + map 20) | reference | 2.229 | 14.9 | 90.1 | 3.45 | 3.95 | Adopt as current default |
| dense + ceres 6 + max_corr 2.0 | tighter correspondence | 11.631 | 15.7 | 52.0 | 2.85 | 3.55 | Keep as reference variant |
| dense + ceres 6 + max_corr 4.0 | moderate tight | 2.507 | 14.4 | 83.4 | 2.85 | 3.55 | Keep as active challenger |
| dense + smaller keypoint voxel 0.5 | more keypoints | 3.011 | 10.8 | 66.1 | 2.85 | 3.55 | Keep as reference variant |
| dense + planarity 0.5 | strict planar features | 7.802 | 18.5 | 64.3 | 2.85 | 3.55 | Keep as reference variant |
| dense + max_corr 4.0 + keypoint 0.5 + ceres 8 + map 30 | all-in | 2.857 | 10.5 | 67.4 | 2.25 | 3.15 | Keep as reference variant |

### Observations

1. `prior_best` is the current default for this problem.
2. `high_planarity` is the fastest observed variant at 18.5 FPS.
3. `prior_best` is the most accurate observed variant at 2.229 m ATE.

### Variant Notes

#### `prior_best`

- Intent: Reference: tuned-v1 winner from d81dba7.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/prior_best/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/prior_best/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/prior_best/run.log`
- Readability proxy: 3.45 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.95 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `tighter_max_corr`

- Intent: Tighten outlier rejection (default 100 m^2 = 10m linear, paper 1-2 m^2).
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-max-correspondence-distance 2.0`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/tighter_max_corr/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-max-correspondence-distance 2.0`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/tighter_max_corr/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/tighter_max_corr/run.log`
- Readability proxy: 2.85 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.55 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `max_corr_4`

- Intent: Halfway between 100 m^2 and 2 m^2.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-max-correspondence-distance 4.0`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/max_corr_4/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-max-correspondence-distance 4.0`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/max_corr_4/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/max_corr_4/run.log`
- Readability proxy: 2.85 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.55 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `smaller_keypoint`

- Intent: Halve keypoint voxel to keep more keypoints per scan.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-keypoint-voxel-size 0.5`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/smaller_keypoint/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-keypoint-voxel-size 0.5`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/smaller_keypoint/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/smaller_keypoint/run.log`
- Readability proxy: 2.85 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.55 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `high_planarity`

- Intent: Require strongly planar features (paper-style strict threshold).
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-planarity-threshold 0.5`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/high_planarity/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 20 --ct-icp-planarity-threshold 0.5`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/high_planarity/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/high_planarity/run.log`
- Readability proxy: 2.85 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.55 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

#### `all_combined`

- Intent: Apply all promising knobs together.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 8 --ct-icp-max-frames-in-map 30 --ct-icp-max-correspondence-distance 4.0 --ct-icp-keypoint-voxel-size 0.5`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/all_combined/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 8 --ct-icp-max-frames-in-map 30 --ct-icp-max-correspondence-distance 4.0 --ct-icp-keypoint-voxel-size 0.5`
- Summary: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/all_combined/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_seq_07_full_arch_close_matrix/all_combined/run.log`
- Readability proxy: 2.25 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.15 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Anchor matches first GT pose; subsequent frames rely on CT-ICP's own continuous-time motion prior (no GT seed).

