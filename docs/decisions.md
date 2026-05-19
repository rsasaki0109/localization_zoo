# Decisions

_Generated at 2026-05-19T22:48:43+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## KISS-ICP cluster discovery on KITTI Odom seq 07 full (1102 frames)

- Current default: `fast_profile`.
- Reference variants: `balanced_reference`, `dense_profile`, `t1_transfer_v05_i12`, `t1_transfer_v03_i12`.
- Aggregate result: `experiments/results/kiss_icp_kitti_seq_07_full_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_reference | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_profile | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_profile | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| t1_transfer_v05_i12 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| t1_transfer_v03_i12 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
