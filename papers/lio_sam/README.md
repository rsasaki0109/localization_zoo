# LIO-SAM

軽量な `LIO-SAM` 風プロトタイプです。既存の `A-LOAM` front-end に対して、

- `Scan Context` で keyframe loop candidate を検出
- `GICP` で loop relative pose を推定
- `IMU preintegration` の回転拘束を pose graph に追加

という最小構成で `LiDAR-Inertial Smoothing and Mapping` の骨格を再現しています。

## Components

- Front-end: `aloam::ScanRegistration`, `aloam::LaserOdometry`, `aloam::LaserMapping`
- Loop detection: `scan_context::ScanContextManager`
- Loop edge refinement: `gicp::GICPRegistration`
- IMU factor: `imu_preintegration::ImuPreintegrator`
- Back-end: 小規模 `Ceres` pose graph

## API

```cpp
localization_zoo::lio_sam::LioSam pipeline;
auto result = pipeline.process(cloud, imu_samples);
```

`process()` は現在 pose、keyframe 追加有無、loop 検出有無を返します。

## Notes

- full `LIO-SAM` の factor graph / iSAM2 をそのまま再現したものではありません
- back-end は keyframe pose graph のみで、bias state は含みません
- まず breadth を増やして dogfooding できる最小版として置いています
