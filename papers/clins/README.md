# CLINS

lightweight `CLINS` 系の pipeline 版です。`CT-LIO` の continuous-time registration を
front-end に使い、フレーム列として local map を持ち回る最小構成の continuous-time
LiDAR-inertial odometry にしています。

この実装で入れている要素:

- `CT-LIO` registration を使った continuous-time scan-to-map
- small fixed-lag / bias-aware 設定をデフォルト化した local pipeline
- keyframe 条件付きの local map 更新
- sliding-window voxel map

full spline backend や global smoothing まで持つ完全再現ではなく、この repo の
`CT-LIO` を sequence-level に使いやすくした compact `CLINS` 版です。
