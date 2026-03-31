# LVI-SAM

軽量な `LVI-SAM` 風プロトタイプです。既存の `LIO-SAM` 系構成に対して、

- `A-LOAM` front-end による LiDAR odometry / mapping
- `Scan Context + GICP` による loop closure
- `IMU preintegration` の回転拘束
- 既知 landmark の camera reprojection factor

を同じ keyframe pose graph に載せて、compact な visual-lidar-inertial SLAM にしています。

## Components

- Front-end: `aloam::ScanRegistration`, `aloam::LaserOdometry`, `aloam::LaserMapping`
- Loop detection: `scan_context::ScanContextManager`
- Loop edge refinement: `gicp::GICPRegistration`
- IMU factor: `imu_preintegration::ImuPreintegrator`
- Visual factor: landmark reprojection residual
- Back-end: 小規模 `Ceres` pose graph

## API

```cpp
localization_zoo::lvi_sam::LviSam pipeline;
pipeline.setLandmarks(landmarks);
auto result = pipeline.process(cloud, visual_observations, imu_samples);
```

`process()` は raw pose と optimized pose、keyframe / loop / visual factor 使用有無を返します。

## Notes

- full `LVI-SAM` の visual frontend や iSAM2 をそのまま再現したものではありません
- visual 側は既知 landmark の reprojection factor に絞っています
- この repo の `LIO-SAM` と `R2LIVE` の中間に当たる compact 実装です
