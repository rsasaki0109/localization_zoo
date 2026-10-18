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

<!-- SHARED_NEAR_RANGE_00_PRIMARY -->
## Shared-loader replay: KITTI 00 frozen comparison complete

The full 00 replay of the stored `kitti_default` arguments completes all
4541 frames, with the optional near-range cut still off. The historical
100 m RPE changes from **0.838536%** to
**0.842687%**; ATE changes from **12.833331 m**
to **12.378148 m**. RPE worsens. ATE improves, so these outcomes are retained separately.
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

The committed current replay has 12/4514 moving steps with length error above 20%, and 41/4514 with local translation-vector error above 20%. The existing diagnostic uses the same 100-frame blocks and GT step threshold of 0.1 m for every method.

The largest increase in pointwise position error among those blocks is poses 3800–3900: GT travels 105.799422 m, while error rises from 12.616529 m to 19.496309 m (increase 6.879780 m). These frame blocks diagnose the current trajectory; they are **not official KITTI RTE**. The tool uses first-pose normalization without fitting or rescaling.

The referenced historical per-run summary is absent at the inspected revision and was not found in this checkout's reachable history. Without its historical saved path, this does not locate the cause of the historical metric difference. [Plot, full trace, commands and limits](../../experiments/results/shared_near_range/diagnostics/00/README.md) use the committed current poses, with diagnostic evidence committed at `61c1f26e1019aa3f0b05b100404c8bb117448b41`. Native source remains `0b079dbbcb7c6f58c1d8367ade881c15f003e525`.
<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->
