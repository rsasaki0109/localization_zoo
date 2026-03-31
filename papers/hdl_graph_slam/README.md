# HDL Graph SLAM

lightweight `hdl_graph_slam` 再現版です。`NDT` の local registration front-end に、
`Scan Context` の loop candidate 検出、`GICP` による loop edge、floor height prior を
持つ小さな pose graph backend を重ねています。

この実装で入れている要素:

- `NDT` による local submap への scan matching
- keyframe 化した scan の `Scan Context` 記述子
- `GICP` による loop closure 相対姿勢推定
- odom edge + loop edge + floor prior の lightweight graph optimization

full system の GPS / IMU / floor plane segmentation までを完全移植したものではなく、
この repo の既存資産で `hdl_graph_slam` の主要な graph SLAM 構成を小さく再現した版です。
