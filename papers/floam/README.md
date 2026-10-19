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

<!-- SHARED_NEAR_RANGE_00_PRIMARY -->
## Shared-loader replay: KITTI 00 frozen comparison complete

The full 00 replay of the stored `dense` arguments completes all
4541 frames, with the optional near-range cut still off. The historical
100 m RPE changes from **0.921410%** to
**0.990961%**; ATE changes from **9.833596 m**
to **8.556112 m**. RPE worsens. ATE improves, so these outcomes are retained separately.
The native process exits 0, but the historical audit exits **1**.

The frozen pre-extraction replay also finishes with native exit 0. Its
RPE, ATE, frame count and official translation/rotation RTE have **zero**
recorded difference from the extracted implementation, and saved poses are
byte-identical. This historical difference therefore predates the extraction;
its original cause remains unresolved. No setting is reselected, and the
stored aggregate and published paper comparison are preserved.
[Closed control commands, hashes and comparison](../../experiments/results/shared_near_range/checks/00_closed_controls/summary.json)
were committed at `7d55897901c17b0c64567115a6f59db06cf585b0`.

[Completed candidate commands, actual exits, logs and summaries](../../experiments/results/shared_near_range/sequences/00/audit/)
were committed at `485fa00601644731827dae601eb3b43c7543efeb`.
The full [frozen comparison](../../experiments/results/shared_near_range/sequences/00/frozen_comparison.json)
and [verified input cleanup](../../experiments/results/shared_near_range/sequences/00/cleanup.json)
were committed at `10cf31616365b41878ec9311f0b705e798716c51`.
The [committed checkpoint](../../experiments/results/shared_near_range/checkpoints/00/summary.json)
now covers cleaned 04 and 00; the whole-zoo audit remains incomplete.
Defaults remain off.
<!-- /SHARED_NEAR_RANGE_00_PRIMARY -->

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

<!-- SHARED_NEAR_RANGE_05_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI05 difference

The selected historical variant `dense` completes **2761**
frames with native exit **0**, using its original arguments unchanged.

| Metric | Historical | Replay |
|---|---:|---:|
| 100 m RPE [%] | 0.537259 | 0.538788 |
| ATE [m] | 6.035067 | 6.038695 |

RPE worsens; ATE worsens. The historical audit retains exit **1**. The frozen
pre-extraction control has **0** differences in RPE, ATE, frames and official
translation/rotation RTE, with byte-identical saved poses. The difference
therefore predates extraction; its original cause remains unresolved.

The current first-pose-normalized position error grows most in inherited block
**2600-2700**, from **12.187404 to 16.211867 m**.
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
