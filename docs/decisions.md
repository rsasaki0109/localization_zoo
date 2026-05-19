# Decisions

_Generated at 2026-05-19T21:01:27+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## CT-ICP cluster A/D bake-off on KITTI Raw 0009 200-frame short window

- Current default: `balanced_reference`.
- Reference variants: `cluster_a`, `cluster_d_ms_chol`.
- Aggregate result: `experiments/results/ct_icp_kitti_raw_0009_200_recipes_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| cluster_a | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| cluster_d_ms_chol | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
