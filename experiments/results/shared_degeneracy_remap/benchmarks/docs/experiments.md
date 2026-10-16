# Experiment Results

_Generated at 2026-10-16T22:25:31+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/shared_degeneracy_remap/benchmarks/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| SuMa shared translation remap on KITTI 01 full | `ready` | `shared_remap_003` | 1398.495 | 2.4 | `experiments/results/shared_degeneracy_remap/benchmarks/suma_kitti_seq_01_full_shared_remap_matrix.json` |

## SuMa shared translation remap on KITTI 01 full

- **Problem ID**: `suma_kitti_seq_01_full_shared_remap`
- **Question**: Does the existing DiLO 0.03 translation remap transfer to SuMa without tuning?
- **Status**: `ready`
- **Dataset PCD directory**: `/workspace/localization-env/shared-remap/data/dogfooding_results/kitti_seq_01_full`
- **Reference CSV**: `experiments/reference_data/kitti_seq_01_full_gt.csv`
- **Stable binary**: `experiments/results/shared_degeneracy_remap/native_wrapper.py`
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

