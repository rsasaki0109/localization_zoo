# R2LIVE

lightweight `R2LIVE` 系の再現版です。`FAST-LIO2` の direct LiDAR-inertial front-end を
raw odometry として使い、その上に keyframe ごとの visual reprojection factor を載せた
compact visual-lidar-inertial SLAM にしています。

この実装で入れている要素:

- `FAST-LIO2` front-end による raw LiDAR-IMU odometry
- 既知 landmark の reprojection を使う keyframe graph optimization
- raw odom edge と visual factor の同時最適化
- first keyframe 固定の小さい Ceres backend

full original の colored map や画像追跡の全体移植ではなく、この repo の `FAST-LIO2`
資産を visual backend 付きに拡張した compact `R2LIVE` 版です。
