# DLIO

lightweight `DLIO` 再現版です。`DLO` の direct scan-to-map を土台にして、
`IMU preintegration` から pose 初期値と速度更新を与える軽量 LiDAR-Inertial
odometry として実装しています。

この実装で入れている要素:

- dense LiDAR scan の voxel downsample
- `GICP` を使う direct scan-to-map
- IMU preintegration による pose prediction
- keyframe ベースの sliding-window local map

full `continuous-time deskew + coarse-to-fine trajectory` の完全移植ではなく、
この repo の既存 `DLO` / `IMU preintegration` 資産をつないだ小さい再現版です。
