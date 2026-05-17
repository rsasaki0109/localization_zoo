# Decisions

_Generated at 2026-05-17T22:34:20+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## CT-ICP architecture-level tuning on KITTI Odometry 07 (full)

- Current default: `prior_best`.
- Active challengers: `max_corr_4`.
- Reference variants: `tighter_max_corr`, `smaller_keypoint`, `high_planarity`, `all_combined`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_full_arch_close_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| prior_best | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| tighter_max_corr | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| max_corr_4 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| smaller_keypoint | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| high_planarity | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| all_combined | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
