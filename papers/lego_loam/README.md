# LeGO-LOAM

`LeGO-LOAM` の軽量再現実装です。full range-image segmentation をそのまま写す代わりに、

- scan line を yaw 順に並べた ground-aware front-end
- lower ring 間の slope と高さを使う簡易 ground 判定
- ground を優先した surface feature と非 ground の edge feature 抽出
- 下流は既存 `A-LOAM` の odometry / mapping を再利用

という形で、地上車両向け `LeGO-LOAM` の流れを小さく再現しています。

## API

```cpp
localization_zoo::lego_loam::LeGOLOAM pipeline;
auto result = pipeline.process(cloud);
```

## Notes

- semantic segmentation や loop closure は含みません
- ground plane が強い地上車両シーンを想定した front-end です
- breadth を増やしつつ既存 LOAM 系との比較ができる最小版として置いています


<!-- SHARED_NEAR_RANGE_00_DIAGNOSTICS -->
## KITTI 00 current-replay error growth

The committed current replay has 12/4514 moving steps with length error above 20%, and 41/4514 with local translation-vector error above 20%. The existing diagnostic uses the same 100-frame blocks and GT step threshold of 0.1 m for every method.

The largest increase in pointwise position error among those blocks is poses 3800–3900: GT travels 105.799422 m, while error rises from 12.616529 m to 19.496309 m (increase 6.879780 m). These frame blocks diagnose the current trajectory; they are **not official KITTI RTE**. The tool uses first-pose normalization without fitting or rescaling.

The referenced historical per-run summary is absent at the inspected revision and was not found in this checkout's reachable history. Without its historical saved path, this does not locate the cause of the historical metric difference. [Plot, full trace, commands and limits](../../experiments/results/shared_near_range/diagnostics/00/README.md) use the committed current poses, with diagnostic evidence committed at `61c1f26e1019aa3f0b05b100404c8bb117448b41`. Native source remains `0b079dbbcb7c6f58c1d8367ade881c15f003e525`.
<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_02_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI02 difference and saved-path diagnosis

| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| LeGO-LOAM | 02 | 0.883497 | 0.883304 | 41.755869 | 42.208784 |

The selected `kitti_default` arguments are unchanged throughout this full
sequence. The historical audit retains exit **1**; native executions exit **0**.
Frozen pre-extraction controls have zero recorded metric differences and
byte-identical poses. This historical difference predates extraction; its
original cause remains unresolved. Input hashes were verified before owned
scan/alias removal. The near-range cut stays disabled.

The largest current position-error increase among the inherited fixed blocks
is frames **4400-4500**:
**40.384993 to 60.227505 m**.

The referenced historical per-run path is unavailable in the inspected tree/history.
Current error growth cannot locate a gap from that unavailable trajectory.

The [protocol, all steps/CSVs and plot](../../experiments/results/shared_near_range/diagnostics/02/README.md)
come from committed evidence `8f5841e3946995cedfa9c0e2fc484dbd2c02373e`. First-pose normalization uses no fitted
alignment or rescaling. These observations locate errors without assigning a
cause or replacing published accuracy. The
[cleaned checkpoint](../../experiments/results/shared_near_range/checkpoints/03/summary.json)
retains all historical differences and audit exits.
<!-- /SHARED_NEAR_RANGE_02_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 5 selected stored LeGO-LOAM full-sequence cells complete with native exits 0. 0 match historical 100 m RPE, ATE and frame count; 5 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

Every historical difference is retained below, including improvements. The corresponding historical audits exit **1**.

| Seq | Historical variant | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| 00 | `kitti_default` | 0.838536 | 0.842687 | 12.833331 | 12.378148 |
| 02 | `kitti_default` | 0.883497 | 0.883304 | 41.755869 | 42.208784 |
| 05 | `kitti_default` | 0.559431 | 0.515567 | 6.449096 | 5.218196 |
| 07 | `kitti_default` | 0.527758 | 0.526845 | 2.589775 | 2.557870 |
| 08 | `kitti_default` | 1.375340 | 1.374146 | 17.896468 | 19.120563 |

For every differing cell, the frozen pre-extraction executable reproduces the replayed metrics and byte-identical saved poses. Thus these differences predate this extraction; their historical causes remain unresolved. Official translation/rotation RTE also matches for those frozen controls. This is not credited as a filter improvement or used to replace stored results.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
