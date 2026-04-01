# FAST-LIO-SLAM

lightweight `FAST-LIO-SLAM` 再現版です。`FAST-LIO2` の direct LiDAR-Inertial
front-end に、`Scan Context` の loop candidate 検出と `GICP` による loop edge、
小さな pose graph 最適化を後段として重ねています。

この実装で入れている要素:

- `FAST-LIO2` による raw point direct odometry
- keyframe 化した local scan の `Scan Context` 記述子
- `GICP` による loop closure 相対姿勢推定
- odom edge + loop edge の lightweight pose graph

full system の map correction や backend の全工夫を完全移植したものではなく、
この repo の既存資産を使って `FAST-LIO-SLAM` の front-end / loop backend 構成を
小さく再現した版です。
