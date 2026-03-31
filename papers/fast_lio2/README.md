# FAST-LIO2

`FAST-LIO2` の軽量再現実装です。full `ikd-Tree + IEKF` をそのまま移植する代わりに、

- raw point cloud を直接使う `feature-less scan-to-map`
- `IMU preintegration` による pose / velocity の予測
- local voxel map への incremental update
- point-to-plane 残差 + IMU prior を `Ceres` で反復最適化

という最小構成で、`FAST-LIO2` の「direct LIO」寄りの流れを再現しています。

## API

```cpp
localization_zoo::fast_lio2::FastLio2 pipeline;
auto result = pipeline.process(raw_points, imu_samples);
```

## Notes

- map は `ct_icp::VoxelHashMap` ベースの sliding window
- bias 推定や full EKF state propagation までは入れていません
- breadth を増やしつつ dogfooding しやすい core library として置いています
