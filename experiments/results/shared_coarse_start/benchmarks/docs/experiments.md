# Experiment Results

_Generated at 2026-10-17T10:09:42+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/shared_coarse_start/benchmarks/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| SuMa shared initial coarse registration on KITTI 00 full | `ready` | `shared_coarse_start_1x5x10` | 7.919 | 1.4 | `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_00_full_shared_coarse_start_matrix.json` |
| SuMa shared initial coarse registration on KITTI 01 full | `ready` | `shared_coarse_start_1x5x10` | 150.501 | 2.0 | `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_01_full_shared_coarse_start_matrix.json` |
| SuMa shared initial coarse registration on KITTI 02 full | `ready` | `shared_coarse_start_1x5x10` | 57.617 | 2.4 | `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_02_full_shared_coarse_start_matrix.json` |
| SuMa shared initial coarse registration on KITTI 03 full | `ready` | `shared_coarse_start_1x5x10` | 1.477 | 2.8 | `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_03_full_shared_coarse_start_matrix.json` |
| SuMa shared initial coarse registration on KITTI 04 full | `ready` | `shared_coarse_start_1x5x10` | 0.929 | 1.9 | `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_04_full_shared_coarse_start_matrix.json` |
| SuMa shared initial coarse registration on KITTI 05 full | `ready` | `shared_coarse_start_1x5x10` | 3.242 | 2.8 | `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_05_full_shared_coarse_start_matrix.json` |
| SuMa shared initial coarse registration on KITTI 06 full | `ready` | `shared_coarse_start_1x5x10` | 2.970 | 2.2 | `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_06_full_shared_coarse_start_matrix.json` |

## SuMa shared initial coarse registration on KITTI 00 full

- **Problem ID**: `suma_kitti_seq_00_full_shared_coarse_start`
- **Question**: Does a fixed initial coarse-to-fine stage derived from the existing IMLS-SLAM choice transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_00_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_00_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_00_full_shared_coarse_start_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + initial coarse registration (1 frame, 5x distance, 10 iterations) | ablation | 7.919 | 1.4 | 100.0 | 2.50 | 3.30 | Adopt as current default |

### Observations

1. `shared_coarse_start_1x5x10` is the current default for this problem.
2. `shared_coarse_start_1x5x10` is the fastest observed variant at 1.4 FPS.
3. `shared_coarse_start_1x5x10` is the most accurate observed variant at 7.919 m ATE.

### Variant Notes

#### `shared_coarse_start_1x5x10`

- Intent: Non-paper opt-in, default off. One initial registration and ten coarse iterations derive from existing IMLS04 calibration; 5x distance multiplier derives from its r=1.0/0.2. Different per-method residuals and stopping rules remain local. No SuMa sweep; historical full KITTI results already known. Experimental evidence only; no published leaderboard/Table6/paper ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Command: `experiments/results/shared_coarse_start/native_wrapper.py /workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_00_full experiments/reference_data/kitti_seq_00_full_gt.csv --methods suma --summary-json experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_00_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json --no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Summary: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_00_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json`
- Log: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_00_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/run.log`
- Readability proxy: 2.50 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.30 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)


## SuMa shared initial coarse registration on KITTI 01 full

- **Problem ID**: `suma_kitti_seq_01_full_shared_coarse_start`
- **Question**: Does a fixed initial coarse-to-fine stage derived from the existing IMLS-SLAM choice transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_01_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_01_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_01_full_shared_coarse_start_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + initial coarse registration (1 frame, 5x distance, 10 iterations) | ablation | 150.501 | 2.0 | 100.0 | 2.50 | 3.30 | Adopt as current default |

### Observations

1. `shared_coarse_start_1x5x10` is the current default for this problem.
2. `shared_coarse_start_1x5x10` is the fastest observed variant at 2.0 FPS.
3. `shared_coarse_start_1x5x10` is the most accurate observed variant at 150.501 m ATE.

### Variant Notes

#### `shared_coarse_start_1x5x10`

- Intent: Non-paper opt-in, default off. One initial registration and ten coarse iterations derive from existing IMLS04 calibration; 5x distance multiplier derives from its r=1.0/0.2. Different per-method residuals and stopping rules remain local. No SuMa sweep; historical full KITTI results already known. Experimental evidence only; no published leaderboard/Table6/paper ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Command: `experiments/results/shared_coarse_start/native_wrapper.py /workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_01_full experiments/reference_data/kitti_seq_01_full_gt.csv --methods suma --summary-json experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_01_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json --no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Summary: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_01_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json`
- Log: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_01_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/run.log`
- Readability proxy: 2.50 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.30 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=92; opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)


## SuMa shared initial coarse registration on KITTI 02 full

- **Problem ID**: `suma_kitti_seq_02_full_shared_coarse_start`
- **Question**: Does a fixed initial coarse-to-fine stage derived from the existing IMLS-SLAM choice transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_02_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_02_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_02_full_shared_coarse_start_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + initial coarse registration (1 frame, 5x distance, 10 iterations) | ablation | 57.617 | 2.4 | 100.0 | 2.50 | 3.30 | Adopt as current default |

### Observations

1. `shared_coarse_start_1x5x10` is the current default for this problem.
2. `shared_coarse_start_1x5x10` is the fastest observed variant at 2.4 FPS.
3. `shared_coarse_start_1x5x10` is the most accurate observed variant at 57.617 m ATE.

### Variant Notes

#### `shared_coarse_start_1x5x10`

- Intent: Non-paper opt-in, default off. One initial registration and ten coarse iterations derive from existing IMLS04 calibration; 5x distance multiplier derives from its r=1.0/0.2. Different per-method residuals and stopping rules remain local. No SuMa sweep; historical full KITTI results already known. Experimental evidence only; no published leaderboard/Table6/paper ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Command: `experiments/results/shared_coarse_start/native_wrapper.py /workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_02_full experiments/reference_data/kitti_seq_02_full_gt.csv --methods suma --summary-json experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_02_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json --no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Summary: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_02_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json`
- Log: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_02_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/run.log`
- Readability proxy: 2.50 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.30 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=71; opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)


## SuMa shared initial coarse registration on KITTI 03 full

- **Problem ID**: `suma_kitti_seq_03_full_shared_coarse_start`
- **Question**: Does a fixed initial coarse-to-fine stage derived from the existing IMLS-SLAM choice transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_03_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_03_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_03_full_shared_coarse_start_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + initial coarse registration (1 frame, 5x distance, 10 iterations) | ablation | 1.477 | 2.8 | 100.0 | 2.50 | 3.30 | Adopt as current default |

### Observations

1. `shared_coarse_start_1x5x10` is the current default for this problem.
2. `shared_coarse_start_1x5x10` is the fastest observed variant at 2.8 FPS.
3. `shared_coarse_start_1x5x10` is the most accurate observed variant at 1.477 m ATE.

### Variant Notes

#### `shared_coarse_start_1x5x10`

- Intent: Non-paper opt-in, default off. One initial registration and ten coarse iterations derive from existing IMLS04 calibration; 5x distance multiplier derives from its r=1.0/0.2. Different per-method residuals and stopping rules remain local. No SuMa sweep; historical full KITTI results already known. Experimental evidence only; no published leaderboard/Table6/paper ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Command: `experiments/results/shared_coarse_start/native_wrapper.py /workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_03_full experiments/reference_data/kitti_seq_03_full_gt.csv --methods suma --summary-json experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_03_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json --no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Summary: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_03_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json`
- Log: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_03_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/run.log`
- Readability proxy: 2.50 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.30 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)


## SuMa shared initial coarse registration on KITTI 04 full

- **Problem ID**: `suma_kitti_seq_04_full_shared_coarse_start`
- **Question**: Does a fixed initial coarse-to-fine stage derived from the existing IMLS-SLAM choice transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_04_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_04_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_04_full_shared_coarse_start_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + initial coarse registration (1 frame, 5x distance, 10 iterations) | ablation | 0.929 | 1.9 | 100.0 | 2.50 | 3.30 | Adopt as current default |

### Observations

1. `shared_coarse_start_1x5x10` is the current default for this problem.
2. `shared_coarse_start_1x5x10` is the fastest observed variant at 1.9 FPS.
3. `shared_coarse_start_1x5x10` is the most accurate observed variant at 0.929 m ATE.

### Variant Notes

#### `shared_coarse_start_1x5x10`

- Intent: Non-paper opt-in, default off. One initial registration and ten coarse iterations derive from existing IMLS04 calibration; 5x distance multiplier derives from its r=1.0/0.2. Different per-method residuals and stopping rules remain local. No SuMa sweep; historical full KITTI results already known. Experimental evidence only; no published leaderboard/Table6/paper ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Command: `experiments/results/shared_coarse_start/native_wrapper.py /workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_04_full experiments/reference_data/kitti_seq_04_full_gt.csv --methods suma --summary-json experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_04_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json --no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Summary: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_04_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json`
- Log: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_04_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/run.log`
- Readability proxy: 2.50 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.30 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)


## SuMa shared initial coarse registration on KITTI 05 full

- **Problem ID**: `suma_kitti_seq_05_full_shared_coarse_start`
- **Question**: Does a fixed initial coarse-to-fine stage derived from the existing IMLS-SLAM choice transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_05_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_05_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_05_full_shared_coarse_start_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + initial coarse registration (1 frame, 5x distance, 10 iterations) | ablation | 3.242 | 2.8 | 100.0 | 2.50 | 3.30 | Adopt as current default |

### Observations

1. `shared_coarse_start_1x5x10` is the current default for this problem.
2. `shared_coarse_start_1x5x10` is the fastest observed variant at 2.8 FPS.
3. `shared_coarse_start_1x5x10` is the most accurate observed variant at 3.242 m ATE.

### Variant Notes

#### `shared_coarse_start_1x5x10`

- Intent: Non-paper opt-in, default off. One initial registration and ten coarse iterations derive from existing IMLS04 calibration; 5x distance multiplier derives from its r=1.0/0.2. Different per-method residuals and stopping rules remain local. No SuMa sweep; historical full KITTI results already known. Experimental evidence only; no published leaderboard/Table6/paper ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Command: `experiments/results/shared_coarse_start/native_wrapper.py /workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_05_full experiments/reference_data/kitti_seq_05_full_gt.csv --methods suma --summary-json experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_05_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json --no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Summary: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_05_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json`
- Log: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_05_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/run.log`
- Readability proxy: 2.50 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.30 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)


## SuMa shared initial coarse registration on KITTI 06 full

- **Problem ID**: `suma_kitti_seq_06_full_shared_coarse_start`
- **Question**: Does a fixed initial coarse-to-fine stage derived from the existing IMLS-SLAM choice transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_06_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_06_full_gt.csv`
- **Stable binary**: `experiments/results/shared_coarse_start/native_wrapper.py`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, rpe_trans_pct, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/shared_coarse_start/benchmarks/suma_kitti_seq_06_full_shared_coarse_start_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| SuMa raw paper path + initial coarse registration (1 frame, 5x distance, 10 iterations) | ablation | 2.970 | 2.2 | 100.0 | 2.50 | 3.30 | Adopt as current default |

### Observations

1. `shared_coarse_start_1x5x10` is the current default for this problem.
2. `shared_coarse_start_1x5x10` is the fastest observed variant at 2.2 FPS.
3. `shared_coarse_start_1x5x10` is the most accurate observed variant at 2.970 m ATE.

### Variant Notes

#### `shared_coarse_start_1x5x10`

- Intent: Non-paper opt-in, default off. One initial registration and ten coarse iterations derive from existing IMLS04 calibration; 5x distance multiplier derives from its r=1.0/0.2. Different per-method residuals and stopping rules remain local. No SuMa sweep; historical full KITTI results already known. Experimental evidence only; no published leaderboard/Table6/paper ratio promotion.
- CLI args: `--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Command: `experiments/results/shared_coarse_start/native_wrapper.py /workspace/localization-env/shared-coarse-start/data/dogfooding_results/kitti_seq_06_full experiments/reference_data/kitti_seq_06_full_gt.csv --methods suma --summary-json experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_06_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json --no-gt-seed --suma-paper --suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
- Summary: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_06_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/summary.json`
- Log: `experiments/results/shared_coarse_start/benchmarks/runs/suma_kitti_seq_06_full_shared_coarse_start_matrix/shared_coarse_start_1x5x10/run.log`
- Readability proxy: 2.50 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.30 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: SuMa paper pipeline (SuMaPaper: rendered surfel model, projective frame-to-model ICP, surfel stability and fusion per RSS 2018 / official default.xml); raw scans, no GT seed, no loop closure. frame-to-frame fallbacks=0; opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)

