# Decisions

_Generated at 2026-05-17T21:46:37+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## LiTAMIN2 gap-closing on KITTI Odometry 07 (full)

- Current default: `voxel15_iter10`.
- Reference variants: `baseline_fast`, `iter12`, `voxel1_iter12`.
- Aggregate result: `experiments/results/litamin2_kitti_seq_07_full_gap_close_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| baseline_fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| iter12 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| voxel1_iter12 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| voxel15_iter10 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP gap-closing on KITTI Odometry 07 (full)

- Current default: `dense_plus_budget`.
- Reference variants: `baseline_default`, `dense_profile`, `max_iter_ceres_budget`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_full_gap_close_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| baseline_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_profile | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| max_iter_ceres_budget | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_plus_budget | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
