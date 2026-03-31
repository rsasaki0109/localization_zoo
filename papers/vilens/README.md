# VILENS

lightweight `VILENS` 系の再現版です。`Point-LIO` 風の local LiDAR-IMU odometry に、
既知 landmark の camera reprojection residual を後段で足して、visual-lidar-inertial
fusion を小さくまとめています。

この実装で入れている要素:

- IMU propagation + point-to-plane local map update
- known-landmark reprojection による pose refinement
- LiDAR pose prior を残した visual correction
- sliding-window keyframe map

full original の multi-camera / global smoothing / 大規模 backend まで持つ完全再現では
なく、この repo の `Point-LIO` を visual fusion 側へ拡張した compact `VILENS` 版です。
