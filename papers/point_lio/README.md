# Point-LIO

lightweight `Point-LIO` 系の再現版です。`LINS` の iterated filter update に、
`FAST-LIO2` 風の raw-point / planarity correspondence を合わせて、feature-less な
direct LiDAR-inertial odometry を小さくまとめています。

この実装で入れている要素:

- raw point voxel downsample
- IMU propagation 付き iterated filter update
- kNN 近傍 PCA による point-to-plane correspondence
- sliding-window keyframe map
- capped correspondence 数での lightweight update

full original system の高帯域化やすべてのロバスト化を完全移植したものではなく、
この repo の `RELEAD/LINS` と `FAST-LIO2` の中間を埋める compact LIO 実装です。
