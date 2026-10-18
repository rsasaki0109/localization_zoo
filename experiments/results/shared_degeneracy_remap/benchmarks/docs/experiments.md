# Experiment Results

_Generated at 2026-10-17T02:43:25+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/shared_degeneracy_remap/benchmarks/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| SuMa shared translation remap on KITTI 00 full | `ready` | `shared_remap_003` | 7.946 | 2.3 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_00_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 01 full | `ready` | `shared_remap_003` | 1398.495 | 2.4 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_01_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 02 full | `ready` | `shared_remap_003` | 565.520 | 2.3 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_02_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 03 full | `ready` | `shared_remap_003` | 11.878 | 2.9 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_03_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 04 full | `ready` | `shared_remap_003` | 218.136 | 6.5 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_04_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 05 full | `ready` | `shared_remap_003` | 52.906 | 2.2 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_05_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 06 full | `ready` | `shared_remap_003` | 263.407 | 3.3 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_06_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 07 full | `ready` | `shared_remap_003` | 1.197 | 2.1 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_07_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 08 full | `ready` | `shared_remap_003` | 289.339 | 2.0 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_08_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 09 full | `ready` | `shared_remap_003` | 390.892 | 1.8 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_09_full_shared_remap_matrix.json` |
| SuMa shared translation remap on KITTI 10 full | `ready` | `shared_remap_003` | 219.326 | 2.5 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_10_full_shared_remap_matrix.json` |

## SuMa shared translation remap on KITTI 00 full

- **Problem ID**: `suma_kitti_seq_00_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_00_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_00_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_00_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 7.946 | 2.3 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.3 FPS.
3. `shared_remap_003` is the most accurate observed variant at 7.946 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_00_full experiments/reference_data/kitti_seq_00_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_00_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_00_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_00_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 01 full

- **Problem ID**: `suma_kitti_seq_01_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_01_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_01_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_01_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 1398.495 | 2.4 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.4 FPS.
3. `shared_remap_003` is the most accurate observed variant at 1398.495 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_01_full experiments/reference_data/kitti_seq_01_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_01_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_01_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_01_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=13; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 02 full

- **Problem ID**: `suma_kitti_seq_02_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_02_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_02_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_02_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 565.520 | 2.3 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.3 FPS.
3. `shared_remap_003` is the most accurate observed variant at 565.520 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_02_full experiments/reference_data/kitti_seq_02_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_02_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_02_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_02_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=15; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 03 full

- **Problem ID**: `suma_kitti_seq_03_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_03_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_03_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_03_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 11.878 | 2.9 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.9 FPS.
3. `shared_remap_003` is the most accurate observed variant at 11.878 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_03_full experiments/reference_data/kitti_seq_03_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_03_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_03_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_03_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=3; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 04 full

- **Problem ID**: `suma_kitti_seq_04_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_04_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_04_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_04_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 218.136 | 6.5 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 6.5 FPS.
3. `shared_remap_003` is the most accurate observed variant at 218.136 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_04_full experiments/reference_data/kitti_seq_04_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_04_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_04_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_04_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 05 full

- **Problem ID**: `suma_kitti_seq_05_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_05_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_05_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_05_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 52.906 | 2.2 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.2 FPS.
3. `shared_remap_003` is the most accurate observed variant at 52.906 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_05_full experiments/reference_data/kitti_seq_05_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_05_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_05_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_05_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=1; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 06 full

- **Problem ID**: `suma_kitti_seq_06_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_06_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_06_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_06_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 263.407 | 3.3 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 3.3 FPS.
3. `shared_remap_003` is the most accurate observed variant at 263.407 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_06_full experiments/reference_data/kitti_seq_06_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_06_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_06_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_06_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 07 full

- **Problem ID**: `suma_kitti_seq_07_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_07_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_07_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_07_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 1.197 | 2.1 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.1 FPS.
3. `shared_remap_003` is the most accurate observed variant at 1.197 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_07_full experiments/reference_data/kitti_seq_07_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_07_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_07_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_07_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 08 full

- **Problem ID**: `suma_kitti_seq_08_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_08_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_08_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_08_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 289.339 | 2.0 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.0 FPS.
3. `shared_remap_003` is the most accurate observed variant at 289.339 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_08_full experiments/reference_data/kitti_seq_08_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_08_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_08_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_08_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=15; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 09 full

- **Problem ID**: `suma_kitti_seq_09_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_09_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_09_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_09_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 390.892 | 1.8 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 1.8 FPS.
3. `shared_remap_003` is the most accurate observed variant at 390.892 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_09_full experiments/reference_data/kitti_seq_09_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_09_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_09_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_09_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=14; opt-in local translation remap ratio=0.030000 (not in the paper)


## SuMa shared translation remap on KITTI 10 full

- **Problem ID**: `suma_kitti_seq_10_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_10_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_10_full_gt.csv`
- **Stable binary**: `experiments/results/shared_degeneracy_remap/native_wrapper.py`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_10_full_shared_remap_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + shared local translation remap 0.03 | ablation | 219.326 | 2.5 | 100.0 | 3.70 | 4.10 | Adopt as current default |

### Observations

1. `shared_remap_003` is the current default for this problem.
2. `shared_remap_003` is the fastest observed variant at 2.5 FPS.
3. `shared_remap_003` is the most accurate observed variant at 219.326 m ATE.

### Variant Notes

#### `shared_remap_003`

- Intent: Non-paper opt-in, default off. Fixed ratio transferred from the existing DiLO01 choice, without SuMa tuning;01 is a development case. Other parameters stay official/raw. Experimental evidence only, excluded from published leaderboard and paper-ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Command: `experiments/results/shared_degeneracy_remap/native_wrapper.py /workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_10_full experiments/reference_data/kitti_seq_10_full_gt.csv --methods suma --summary-json experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_10_full_shared_remap_matrix/shared_remap_003/summary.json --no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03`
- Summary: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_10_full_shared_remap_matrix/shared_remap_003/summary.json`
- Log: `experiments/results/shared_degeneracy_remap/benchmarks/runs/suma_kitti_seq_10_full_shared_remap_matrix/shared_remap_003/run.log`
- Readability proxy: 3.70 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.10 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=10; opt-in local translation remap ratio=0.030000 (not in the paper)

