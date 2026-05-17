# Decisions

_Generated at 2026-05-17T21:19:38+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## LiTAMIN2 paper-comparable on KITTI Odometry 07 (full, 1101 frames)

- Current default: `paper_cov_gt_seed`.
- Reference variants: `paper_cov_no_gt_seed`, `paper_icp_only_no_gt_seed`, `fast_cov_no_gt_seed`.
- Aggregate result: `experiments/results/litamin2_kitti_seq_07_full_paper_comp_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| paper_cov_no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_cov_no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_cov_gt_seed | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP paper-comparable on KITTI Odometry 07 (full, 1101 frames)

- Current default: `gt_seeded`.
- Reference variants: `default_pure_odometry`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_full_paper_comp_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default_pure_odometry | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| gt_seeded | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
