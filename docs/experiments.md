# Experiment Results

_Generated at 2026-05-19T21:05:28+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| CT-ICP cluster A/D + GT seed on MCD TUHH night_09 (108 frames) | `ready` | `dense_seeded_reference` | 0.832 | 27.4 | `experiments/results/ct_icp_mcd_tuhh_night_09_seeded_matrix.json` |

## CT-ICP cluster A/D + GT seed on MCD TUHH night_09 (108 frames)

- **Problem ID**: `ct_icp_mcd_tuhh_night_09_seeded`
- **Question**: KTH (6.12 m baseline) + seed = -61% drift, NTU (0.325 m) + seed = +39%. Where does TUHH (1.65 m) sit on the self-anchor vs drift-limited axis?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, rpe_trans_pct, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_tuhh_night_09_seeded_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| dense_window + GT seed | reference | 1.182 | 27.4 | 85.2 | 4.30 | 4.50 | Adopt as current default |
| cluster A + GT seed | transfer-seeded | 0.832 | 17.7 | 82.3 | 1.00 | 1.85 | Keep as active challenger |
| cluster D (ms_chol) + GT seed | alternative-seeded | 1.658 | 24.0 | 68.9 | 3.60 | 4.00 | Keep as reference variant |

### Observations

1. `dense_seeded_reference` is the current default for this problem.
2. `dense_seeded_reference` is the fastest observed variant at 27.4 FPS.
3. `cluster_a_seeded` is the most accurate observed variant at 0.832 m ATE.

### Variant Notes

#### `dense_seeded_reference`

- Intent: Seeded baseline.
- CLI args: `--ct-icp-dense-profile --ct-icp-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/dense_seeded_reference/summary.json --ct-icp-dense-profile --ct-icp-gt-seed`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/dense_seeded_reference/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/dense_seeded_reference/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. Symmetric of the GT-seeded Policy A path for dogfooding-style fair-prior comparison.

#### `cluster_a_seeded`

- Intent: Cluster A with seed.
- CLI args: `--ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06 --ct-icp-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/cluster_a_seeded/summary.json --ct-icp-dense-profile --ct-icp-ceres-max-iterations 6 --ct-icp-max-frames-in-map 50 --ct-icp-coarse-to-fine --ct-icp-coarse-iterations 2 --ct-icp-coarse-search-radius 2 --ct-icp-coarse-cauchy-mult 2.0 --ct-icp-coarse-planarity-threshold 0.06 --ct-icp-gt-seed`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/cluster_a_seeded/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/cluster_a_seeded/run.log`
- Readability proxy: 1.00 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 1.85 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. Symmetric of the GT-seeded Policy A path for dogfooding-style fair-prior comparison.

#### `cluster_d_seeded`

- Intent: ms_chol with seed.
- CLI args: `--ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky --ct-icp-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/cluster_d_seeded/summary.json --ct-icp-dense-profile --ct-icp-multi-scale --ct-icp-normal-cholesky --ct-icp-gt-seed`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/cluster_d_seeded/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_seeded_matrix/cluster_d_seeded/run.log`
- Readability proxy: 3.60 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.00 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. Symmetric of the GT-seeded Policy A path for dogfooding-style fair-prior comparison.

