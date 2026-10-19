# F-LOAM

lightweight `F-LOAM` 再現版です。`A-LOAM` の feature-based odometry / mapping を土台に、
入力点群の thinning と mapping update の間引き、map-odom correction の維持を入れて
高速寄りの LOAM pipeline にしています。

この実装で入れている要素:

- stride 間引きした入力点群による feature extraction
- reduced feature budget の `ScanRegistration`
- lightweight `LaserOdometry`
- interval 制御付き `LaserMapping`
- map-odom correction を使った軽量 pose 補正

full original system の全最適化を完全移植したものではなく、この repo の `A-LOAM`
資産を使って `F-LOAM` の fast pipeline 側を小さく再現した版です。


<!-- SHARED_NEAR_RANGE_00_DIAGNOSTICS -->
## KITTI 00 current-replay error growth

The committed current replay has 12/4514 moving steps with length error above 20%, and 42/4514 with local translation-vector error above 20%. The existing diagnostic uses the same 100-frame blocks and GT step threshold of 0.1 m for every method.

The largest increase in pointwise position error among those blocks is poses 600–700: GT travels 98.739614 m, while error rises from 6.940767 m to 11.452315 m (increase 4.511548 m). These frame blocks diagnose the current trajectory; they are **not official KITTI RTE**. The tool uses first-pose normalization without fitting or rescaling.

The referenced historical per-run summary is absent at the inspected revision and was not found in this checkout's reachable history. Without its historical saved path, this does not locate the cause of the historical metric difference. [Plot, full trace, commands and limits](../../experiments/results/shared_near_range/diagnostics/00/README.md) use the committed current poses, with diagnostic evidence committed at `61c1f26e1019aa3f0b05b100404c8bb117448b41`. Native source remains `0b079dbbcb7c6f58c1d8367ade881c15f003e525`.
<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_02_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI02 difference and saved-path diagnosis

| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| F-LOAM | 02 | 0.937750 | 0.945974 | 52.068212 | 53.822119 |

The selected `dense` arguments are unchanged throughout this full
sequence. The historical audit retains exit **1**; native executions exit **0**.
Frozen pre-extraction controls have zero recorded metric differences and
byte-identical poses. This historical difference predates extraction; its
original cause remains unresolved. Input hashes were verified before owned
scan/alias removal. The near-range cut stays disabled.

The largest current position-error increase among the inherited fixed blocks
is frames **3000-3100**:
**36.432333 to 53.883664 m**.

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

All 5 selected stored F-LOAM full-sequence cells complete with native exits 0. 0 match historical 100 m RPE, ATE and frame count; 5 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

Every historical difference is retained below, including improvements. The corresponding historical audits exit **1**.

| Seq | Historical variant | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| 00 | `dense` | 0.921410 | 0.990961 | 9.833596 | 8.556112 |
| 02 | `dense` | 0.937750 | 0.945974 | 52.068212 | 53.822119 |
| 05 | `dense` | 0.537259 | 0.538788 | 6.035067 | 6.038695 |
| 07 | `dense` | 0.612719 | 0.590347 | 2.971748 | 3.173641 |
| 08 | `dense` | 1.399647 | 1.372876 | 20.440418 | 17.318202 |

For every differing cell, the frozen pre-extraction executable reproduces the replayed metrics and byte-identical saved poses. Thus these differences predate this extraction; their historical causes remain unresolved. Official translation/rotation RTE also matches for those frozen controls. This is not credited as a filter improvement or used to replace stored results.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
