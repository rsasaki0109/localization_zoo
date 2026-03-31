# VGICP-SLAM

lightweight `VGICP-SLAM` 版です。`Voxel-GICP` を front-end に使って local
submap へ直接合わせ、`Scan Context` で loop candidate を見つけて
`Voxel-GICP` loop edge を pose graph に追加します。

この実装で入れている要素:

- `Voxel-GICP` による scan-to-submap front-end
- keyframe sliding-window local submap
- `Scan Context` による軽量 loop candidate 検出
- `Voxel-GICP` による loop refinement
- Ceres ベースの lightweight pose graph optimization

full original system の大規模 backend や map correction 全部を持つ版ではなく、
この repo の `Voxel-GICP` / `Scan Context` 資産をつないだ compact graph SLAM
実装です。
