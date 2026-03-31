# DLO

lightweight `DLO` 再現版です。dense LiDAR scan を downsample し、local map への
direct scan-to-map を `GICP` で回します。map は keyframe ベースの sliding window
として維持します。

この実装で入れている要素:

- range filter + voxel downsample
- `GICP` を使う dense scan-to-map registration
- constant velocity 初期値
- keyframe ベースの local map 更新

full DLO の optimizer / ROS integration を完全移植したものではなく、
この repo の既存 `GICP` と local map 管理の上に小さく寄せた版です。
