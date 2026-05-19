# Decisions

_Generated at 2026-05-19T20:32:55+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## CT-ICP recipe sensitivity test on MCD TUHH night_09 (108 frames)

- Current default: `dense_reference`.
- Active challengers: `cluster_d_ms_chol`.
- Reference variants: `cluster_a`.
- Aggregate result: `experiments/results/ct_icp_mcd_tuhh_night_09_recipes_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| dense_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| cluster_a | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| cluster_d_ms_chol | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
