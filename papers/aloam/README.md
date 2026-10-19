# A-LOAM: Advanced implementation of LOAM

## 論文情報

- **原著**: Ji Zhang, Sanjiv Singh, "LOAM: Lidar Odometry and Mapping in Real-time", RSS 2014
- **A-LOAM実装者**: Tong Qin, Shaozu Cao (HKUST)
- **元リポジトリ**: https://github.com/HKUST-Aerial-Robotics/A-LOAM
- **ライセンス**: BSD-3-Clause

## 手法概要

LiDAR点群から幾何特徴（エッジ・平面）を抽出し、2段階のマッチングで高精度オドメトリを実現。

### 3ステージパイプライン

1. **ScanRegistration**: 曲率ベースでエッジ点・平面点を抽出
2. **LaserOdometry**: scan-to-scan マッチング（高頻度・低精度）
3. **LaserMapping**: scan-to-map マッチング（低頻度・高精度）

### コスト関数

**エッジ残差** (点と直線の距離):
```
r_edge = ((Rp+t - a) × (Rp+t - b)) / ||a - b||
```

**平面残差** (点と平面の距離):
```
r_plane = n · (Rp+t) + d
```

## 実装ノート

- ROS非依存の純粋C++実装
- 元のA-LOAMからアルゴリズム部分のみを抽出・再構成
- Ceres Solver による AutoDiff 最適化

<!-- FIXED-KITTI-RESULTS:START -->

## Full KITTI comparison with one configuration

Fixed variant: `kitti_default`.

kitti_default was the accuracy candidate in the existing sequence 07 profile comparison (lowest stored 100 m RPE and official RTE there). Its arguments are frozen; the added sequences do not select a profile.

This pipeline never uses per-frame GT seeding; GT is used only for the initial rigid evaluation anchor. Keep the original variant arguments verbatim, including an explicit --no-gt-seed where present.

Reference values are LOAM values cited by later papers, not a per-sequence A-LOAM author table.

No new estimator behavior or default change is introduced by this coverage run.

Exact fixed-setting CLI (substitute the sequence for NN):

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods aloam --aloam-kitti-profile --summary-json aloam_seq_NN.json
```

Full sequences, official KITTI translational RTE (100-800 m), no GT seed.
Existing committed results are reused on 00, 02, 05, 07, 08.
The other rows are added full-sequence runs. Every row uses the same variant.


| Seq | Paper RTE [%] | Fixed RTE [%] | Repo / paper | FPS |
|---|---:|---:|---:|---:|
| 00 (reused) | 0.780 | 0.855 | 1.10x | 1.265 |
| 01 | 1.430 | 2.189 | 1.53x | 1.100 |
| 02 (reused) | 0.920 | 1.029 | 1.12x | 1.269 |
| 03 | 0.860 | 0.964 | 1.12x | 1.145 |
| 04 | 0.710 | 0.880 | 1.24x | 0.814 |
| 05 (reused) | 0.570 | 0.535 | 0.94x | 1.182 |
| 06 | 0.650 | 0.602 | 0.93x | 0.954 |
| 07 (reused) | 0.630 | 0.681 | 1.08x | 0.639 |
| 08 (reused) | 1.120 | 0.975 | 0.87x | 1.180 |
| 09 | 0.770 | 0.673 | 0.87x | 0.765 |
| 10 | 0.790 | 0.873 | 1.11x | 0.831 |

Geometric mean over all sequences with paper values: **1.07x**
with the fixed setting. Table 6 is restricted to this same variant by
`repo_variants` in `evaluation/data/paper_reported_numbers.json`.
Historical ablations remain available in their original
result aggregates.

FPS describes the recorded execution: the new runs use one OpenMP
thread per method and may overlap on the same sequence. Reused FPS
comes from historical runs on their recorded hosts.

Source aggregates and all unrounded rows are recorded in
`experiments/results/remaining_kitti_coverage/summary.json`.
New per-run summaries, host records, logs, estimated poses, evaluated GT
and step diagnostics are committed under the corresponding
`experiments/results/runs/` directory. Exact execution commands and the
isolated-output launcher used for overlapping methods are recorded
in `experiments/results/remaining_kitti_coverage/`.

### Sequences above twice the paper value

No sequence exceeds twice its paper value with this fixed setting.

Saved-pose diagnostics are generated with
`evaluation/scripts/diagnose_kitti_steps.py --estimate <trajectory_kitti.txt>
--gt <evaluated_gt_kitti.txt> --summary <summary.json> --method "A-LOAM"
--output <step_diagnostics.json>`; each JSON records the exact command.
The script checks its full-sequence RTE against the original run summary.

<!-- FIXED-KITTI-RESULTS:END -->


<!-- SHARED_NEAR_RANGE_00_DIAGNOSTICS -->
## KITTI 00 current-replay error growth

The committed current replay has 13/4514 moving steps with length error above 20%, and 42/4514 with local translation-vector error above 20%. The existing diagnostic uses the same 100-frame blocks and GT step threshold of 0.1 m for every method.

The largest increase in pointwise position error among those blocks is poses 600–700: GT travels 98.739614 m, while error rises from 7.208486 m to 11.810234 m (increase 4.601748 m). These frame blocks diagnose the current trajectory; they are **not official KITTI RTE**. The tool uses first-pose normalization without fitting or rescaling.

The referenced historical per-run summary is absent at the inspected revision and was not found in this checkout's reachable history. Without its historical saved path, this does not locate the cause of the historical metric difference. [Plot, full trace, commands and limits](../../experiments/results/shared_near_range/diagnostics/00/README.md) use the committed current poses, with diagnostic evidence committed at `61c1f26e1019aa3f0b05b100404c8bb117448b41`. Native source remains `0b079dbbcb7c6f58c1d8367ade881c15f003e525`.
<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_02_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI02 difference and saved-path diagnosis

| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| A-LOAM | 02 | 0.923231 | 0.926292 | 50.114358 | 50.789841 |

The selected `kitti_default` arguments are unchanged throughout this full
sequence. The historical audit retains exit **1**; native executions exit **0**.
Frozen pre-extraction controls have zero recorded metric differences and
byte-identical poses. This historical difference predates extraction; its
original cause remains unresolved. Input hashes were verified before owned
scan/alias removal. The near-range cut stays disabled.

The largest current position-error increase among the inherited fixed blocks
is frames **0-100**:
**0.000000 to 19.558007 m**.

The referenced historical per-run path is unavailable in the inspected tree/history.
Current error growth cannot locate a gap from that unavailable trajectory.

The [protocol, all steps/CSVs and plot](../../experiments/results/shared_near_range/diagnostics/02/README.md)
come from committed evidence `8f5841e3946995cedfa9c0e2fc484dbd2c02373e`. First-pose normalization uses no fitted
alignment or rescaling. These observations locate errors without assigning a
cause or replacing published accuracy. The
[cleaned checkpoint](../../experiments/results/shared_near_range/checkpoints/03/summary.json)
retains all historical differences and audit exits.
<!-- /SHARED_NEAR_RANGE_02_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_05_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI05 difference

The selected historical variant `kitti_default` completes **2761**
frames with native exit **0**, using its original arguments unchanged.

| Metric | Historical | Replay |
|---|---:|---:|
| 100 m RPE [%] | 0.528651 | 0.512096 |
| ATE [m] | 4.839948 | 5.192731 |

RPE improves; ATE worsens. The historical audit retains exit **1**. The frozen
pre-extraction control has **0** differences in RPE, ATE, frames and official
translation/rotation RTE, with byte-identical saved poses. The difference
therefore predates extraction; its original cause remains unresolved.

The current first-pose-normalized position error grows most in inherited block
**2600-2700**, from **9.331710 to 14.176748 m**.
This locates current error growth against GT, not its cause or a change in
historical accuracy. It does not replace official KITTI RTE.

The referenced historical summary/path is absent in the inspected tree and was
not found by its exact-path reachable-history probe. No paired historical path
is available here; the current trace cannot explain the historical difference.

[Raw replay, receipts and verified cleanup](../../experiments/results/shared_near_range/sequences/05/),
[all diagnostic blocks, traces, commands and limits](../../experiments/results/shared_near_range/diagnostics/05/README.md),
and the [cleaned checkpoint](../../experiments/results/shared_near_range/checkpoints/05/summary.json)
retain all values and actual exits. Native source remains `0b079dbb`; current-main
acceleration/passive tracing is outside this frozen replay's scope. No estimator,
registration setting, default or published result changes. Raw diagnosis
`6285c60b` precedes this prose; active06 scans and executables are untouched.
<!-- /SHARED_NEAR_RANGE_05_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_07_DIAGNOSTICS -->
## KITTI07 historical replay and saved-path diagnosis

The full historical `kitti_default` cell completes **1,101**
frames with native exit **0**. Stored/replayed 100 m RPE changes from
**0.607458%** to **0.605313%**;
ATE changes from **2.544355 m** to **2.505243 m**.
RPE improves; ATE improves. Both metrics are retained. The historical audit
still exits **1**. Frozen pre-extraction controls reproduce the metrics with
zero recorded differences and byte-identical saved poses; comparison exits **0**.
The difference predates extraction; its original cause remains unresolved.

With inherited **100**-step blocks, the largest current position-error increase
is poses **800-900**: **1.728689 to 4.569446 m**.
This uses first-pose normalization, zero-based indices, no fitted alignment or
rescaling. Every block/frame is retained. The exact referenced historical path
is unavailable in the inspected tree/history, so this diagnoses current error
growth without locating a historical difference or establishing its cause.

[All numerical pairs, commands, complete traces and limits](../../experiments/results/shared_near_range/diagnostics/07/README.md)
use original native evidence `ed37e2fd` (lossless import `9c9976c6`), analysis
source/protocol `59e69241` and raw results `6db8522a`, committed before this prose.
Native source remains `0b079dbb`. Historical per-cell arguments are unchanged;
this is not a new fixed-setting accuracy study. Existing results, paper tables
and defaults are preserved.
<!-- /SHARED_NEAR_RANGE_07_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_08_DIAGNOSTICS -->
## KITTI08 historical replay and saved-path diagnosis

The full historical `kitti_default` cell completes **4,071** frames
with native exit **0**. Stored/replayed 100 m RPE changes from
**1.390107%** to **1.394630%**;
ATE changes from **18.110228 m** to **18.661434 m**.
RPE worsens; ATE worsens. Both metrics are retained. The historical audit
still exits **1**. Frozen pre-extraction controls reproduce the metrics with
zero recorded differences and byte-identical saved poses; comparison exits **0**.
The difference predates extraction; its original cause remains unresolved.

The largest inherited **100**-step increase in current position error is
poses **0-100**: **0.000000 to 17.406467 m**.
GT travels **50.647955 m** in that block, shorter than the official KITTI
**100-800 m** segments. These first-pose-normalized position errors use zero-based
indices, no fitted alignment and no rescaling; they are not official RTE or a
historical gap. All blocks and every frame are retained.

The exact referenced historical path is unavailable in the inspected tree/history.
This locates current error growth without manufacturing a historical gap or cause.

[All numerical pairs, plot, traces, commands and limits](../../experiments/results/shared_near_range/diagnostics/08/README.md)
use original native evidence `15157bab` (lossless import `d0a76218`), analysis
source/protocol `d969608e` and raw results `9197f428`, committed before this prose.
Native source remains `0b079dbb`; historical per-cell arguments and default-off
preprocessing are unchanged. This is not a new fixed-setting accuracy study
or validation of current-main acceleration/tracing. Existing published results
and paper tables are preserved.
<!-- /SHARED_NEAR_RANGE_08_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 11 selected stored A-LOAM full-sequence cells complete with native exits 0. 6 match historical 100 m RPE, ATE and frame count; 5 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

Every historical difference is retained below, including improvements. The corresponding historical audits exit **1**.

| Seq | Historical variant | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| 00 | `kitti_default` | 0.891327 | 0.914174 | 12.043760 | 9.520570 |
| 02 | `kitti_default` | 0.923231 | 0.926292 | 50.114358 | 50.789841 |
| 05 | `kitti_default` | 0.528651 | 0.512096 | 4.839948 | 5.192731 |
| 07 | `kitti_default` | 0.607458 | 0.605313 | 2.544355 | 2.505243 |
| 08 | `kitti_default` | 1.390107 | 1.394630 | 18.110228 | 18.661434 |

For every differing cell, the frozen pre-extraction executable reproduces the replayed metrics and byte-identical saved poses. Thus these differences predate this extraction; their historical causes remain unresolved. Official translation/rotation RTE also matches for those frozen controls. This is not credited as a filter improvement or used to replace stored results.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
