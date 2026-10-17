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
## Shared-loader replay: KITTI 00 comparison in progress

The full 00 replay of the stored `kitti_default` arguments completes all
4541 frames, with the optional near-range cut still off. The historical
100 m RPE changes from **0.838536%** to
**0.842687%**; ATE changes from **12.833331 m**
to **12.378148 m**. RPE worsens. ATE improves, so these outcomes are retained separately.
The native process exits 0, but the historical audit exits **1**.

The frozen pre-extraction comparison is still running. The cause of this
historical difference and whether extraction affects this cell remain
**unresolved** until that comparison completes. No setting is reselected;
the stored aggregate and published paper comparison are preserved.

[Completed candidate commands, actual exits, logs and summaries](../../experiments/results/shared_near_range/sequences/00/audit/)
were committed at `485fa00601644731827dae601eb3b43c7543efeb`.
[Protocol and current scope](../../experiments/results/shared_near_range/README.md)
distinguish these completed runs from a cleaned sequence checkpoint: 00 input
cleanup and the whole-zoo audit are incomplete. Defaults remain off.
<!-- /SHARED_NEAR_RANGE_00_PRIMARY -->
