# Experiment Results

_Generated at 2026-05-19T21:02:52+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| CT-ICP cluster A/D + GT seed on MCD NTU day_02 (108 frames) | `ready` | `dense_seeded_reference` | 0.425 | 28.4 | `experiments/results/ct_icp_mcd_ntu_day_02_seeded_matrix.json` |

## CT-ICP cluster A/D + GT seed on MCD NTU day_02 (108 frames)

- **Problem ID**: `ct_icp_mcd_ntu_day_02_seeded`
- **Question**: MCD NTU day_02 without seed: dense 0.325 m wins ATE, cluster A halves RPE. MulRan parkinglot 120 + seed shows cluster A -84%. Does adding GT seed to MCD NTU reveal a similar cluster A dominance?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_ntu_day_02_seeded_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| dense_window + GT seed | reference | 0.451 | 28.4 | 97.1 | 4.30 | 4.50 | Adopt as current default |
| cluster A + GT seed | transfer-seeded | 0.432 | 21.1 | 86.5 | 1.00 | 1.85 | Keep as reference variant |
| cluster D (ms_chol) + GT seed | alternative-seeded | 0.425 | 25.3 | 94.6 | 3.60 | 4.00 | Keep as active challenger |

### Observations

1. `dense_seeded_reference` is the current default for this problem.
2. `dense_seeded_reference` is the fastest observed variant at 28.4 FPS.
3. `cluster_d_seeded` is the most accurate observed variant at 0.425 m ATE.

### Variant Notes

#### `dense_seeded_reference`

- Intent: Seeded baseline.
- CLI args: `--ct-icp-dense-profile --ct-icp-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/dense_seeded_reference/summary.json --ct-icp-dense-profile --ct-icp-gt-seed`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/dense_seeded_reference/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/dense_seeded_reference/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. Symmetric of the GT-seeded Policy A path for dogfooding-style fair-prior comparison.

#### `cluster_a_seeded`

- Intent: Cluster A with seed (mirror of MulRan parkinglot 120 -84% pattern).
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06 --ct-icp-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/cluster_a_seeded/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06 --ct-icp-gt-seed`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/cluster_a_seeded/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/cluster_a_seeded/run.log`
- Readability proxy: 1.00 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 1.85 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. Symmetric of the GT-seeded Policy A path for dogfooding-style fair-prior comparison.

#### `cluster_d_seeded`

- Intent: ms_chol with seed.
- CLI args: `--ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky --ct-icp-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/cluster_d_seeded/summary.json --ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky --ct-icp-gt-seed`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/cluster_d_seeded/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_seeded_matrix/cluster_d_seeded/run.log`
- Readability proxy: 3.60 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.00 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. Symmetric of the GT-seeded Policy A path for dogfooding-style fair-prior comparison.

