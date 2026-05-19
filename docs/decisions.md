# Decisions

_Generated at 2026-05-19T21:02:52+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## CT-ICP cluster A/D + GT seed on MCD NTU day_02 (108 frames)

- Current default: `dense_seeded_reference`.
- Active challengers: `cluster_d_seeded`.
- Reference variants: `cluster_a_seeded`.
- Aggregate result: `experiments/results/ct_icp_mcd_ntu_day_02_seeded_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| dense_seeded_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| cluster_a_seeded | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| cluster_d_seeded | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
