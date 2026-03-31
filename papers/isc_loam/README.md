# ISC-LOAM

lightweight `ISC-LOAM` 系の再現版です。front-end は既存 `F-LOAM` を使い、raw
`PointXYZI` の intensity から作る scan-context 風 descriptor で loop candidate を
見つけ、`GICP` loop edge を pose graph に追加します。

この実装で入れている要素:

- `F-LOAM` front-end による odometry / mapping
- intensity-aware scan context descriptor
- raw intensity descriptor による lightweight loop candidate 検出
- `GICP` による loop refinement
- Ceres ベースの lightweight pose graph optimization

full original system の全 backend を完全移植したものではなく、この repo の `F-LOAM`
と `GICP` 資産をつないだ compact loop-closure 版です。
