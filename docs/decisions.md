# Decisions

_Generated at 2026-05-17T15:55:45+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## LiTAMIN2 pure-odometry (no GT seed) on MulRan ParkingLot (full sequence)

- Current default: `paper_icp_only_no_gt_seed`.
- Reference variants: `fast_cov_no_gt_seed`, `fast_icp_only_no_gt_seed`, `paper_cov_no_gt_seed`.
- Aggregate result: `experiments/results/litamin2_mulran_parkinglot_full_no_gt_seed_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_cov_no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_no_gt_seed | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
