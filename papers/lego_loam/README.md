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

<!-- SHARED_NEAR_RANGE_05_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI05 difference

The selected historical variant `kitti_default` completes **2761**
frames with native exit **0**, using its original arguments unchanged.

| Metric | Historical | Replay |
|---|---:|---:|
| 100 m RPE [%] | 0.559431 | 0.515567 |
| ATE [m] | 6.449096 | 5.218196 |

RPE improves; ATE improves. The historical audit retains exit **1**. The frozen
pre-extraction control has **0** differences in RPE, ATE, frames and official
translation/rotation RTE, with byte-identical saved poses. The difference
therefore predates extraction; its original cause remains unresolved.

The current first-pose-normalized position error grows most in inherited block
**2600-2700**, from **8.956928 to 14.455088 m**.
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
**0.527758%** to **0.526845%**;
ATE changes from **2.589775 m** to **2.557870 m**.
RPE improves; ATE improves. Both metrics are retained. The historical audit
still exits **1**. Frozen pre-extraction controls reproduce the metrics with
zero recorded differences and byte-identical saved poses; comparison exits **0**.
The difference predates extraction; its original cause remains unresolved.

With inherited **100**-step blocks, the largest current position-error increase
is poses **800-900**: **1.143478 to 5.069158 m**.
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
**1.375340%** to **1.374146%**;
ATE changes from **17.896468 m** to **19.120563 m**.
RPE improves; ATE worsens. Both metrics are retained. The historical audit
still exits **1**. Frozen pre-extraction controls reproduce the metrics with
zero recorded differences and byte-identical saved poses; comparison exits **0**.
The difference predates extraction; its original cause remains unresolved.

The largest inherited **100**-step increase in current position error is
poses **0-100**: **0.000000 to 17.480118 m**.
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
