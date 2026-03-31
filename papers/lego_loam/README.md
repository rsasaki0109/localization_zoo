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
