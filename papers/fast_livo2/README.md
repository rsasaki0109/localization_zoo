# FAST-LIVO2

lightweight `FAST-LIVO2` 系の再現版です。`FAST-LIO2` の direct LiDAR-inertial odometry
を front-end に使い、その frame pose に対して landmark reprojection residual を直接
掛ける compact visual-lidar-inertial odometry にしています。

この実装で入れている要素:

- `FAST-LIO2` front-end による raw LiDAR-IMU odometry
- per-frame visual reprojection refinement
- LiDAR prior を残した local pose correction
- corrected pose の次フレームへの持ち回り

full original の image tracking や high-rate visual frontend 全体までは移しておらず、
この repo の `FAST-LIO2` を visual local fusion 側へ拡張した compact `FAST-LIVO2`
版です。
